/*
Name: 龚政平
StudentId: 517030910399
*/

#include "cachelab.h"
#include "stdlib.h"
#include "getopt.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"

#define ADDRESS_SIZE = 64;

typedef struct {
  char valid;
  unsigned long tag;
  int LRU;
}Line;

typedef struct {
  Line *lines;
}Set;

typedef struct {
  int S;
  int E;
  Set *sets;
}Cache;

void printHelp();
void init(int s, int E, int b, Cache *cache);
void cacheSimulator(int s, int E, int b, char *file, int isVerbose, Cache *cache);
int getEmptyIndex(Cache *cache, int index, int tag);
int getHitIndex(Cache *cache, int index, int tag);

void store(Cache *cache, int index, int tag, int isVerbose);
void load(Cache *cache, int index, int tag, int isVerbose);
void modify(Cache *cache, int index, int tag, int isVerbose);

int hits, misses, evictions;
int s, E, b;
char file[100];
int isVerbose;

int main(int argc, char *argv[])
{
  Cache cache;

  int ch;
  while((ch = getopt(argc, argv, "hvs:E:b:t:")) != -1){
    switch(ch) {
      case 'h':
        printHelp();
        break;
      case 'v':
        isVerbose = 1;
        break;
      case 's':
        s = atoi(optarg);
        break;

void printSummary(int hits, int misses, int evictions){
  printf("hits:%d misses:%d evictions:%d", hits, misses, evictions);
}
      case 'E':
        E = atoi(optarg);
        break;
      case 'b':
        b = atoi(optarg);
        break;
      case 't':
        strcpy(file, optarg);
        break;
      default:
        printf("Unknown option: %c\n", (char)optopt);
        break;
    }
  }

  init(s, E, b, &cache);

  cacheSimulator(s, E, b, file, isVerbose, &cache);

  printSummary(hits, misses, evictions);

  return 0;
}

void init(int s, int E, int b, Cache *cache){
  cache->S = 2 << s;
  cache->E = E;
  cache->sets = (Set *)malloc(cache->S * sizeof(Set));

  int i, j;
  for (i = 0; i < cache->S; i++){
    cache->sets[i].lines = (Line *)malloc(E * sizeof(Line));
    for (j = 0; j < cache->E; j++){
      cache->sets[i].lines[j].valid = 0;
      cache->sets[i].lines[j].LRU = 0;
    }
  }
}

void cacheSimulator(int s, int E, int b, char* file, int isVerbose, Cache* Cache) {
  FILE *File;                        /* pointer to FILE object */
  File = fopen(file, "r");
  char access_type;                   /* L-load S-store M-modify */
  unsigned long address;              /* 64-bit hexa memory address */
  int size;                           /* # of bytes accessed by operation */

  int tag_move_bits = b + s;

  while (fscanf(File, " %c %lx,%d", &access_type, &address, &size) > 0) {
    if (access_type == 'I') {
      continue;
    } else {
      int tag = address >> tag_move_bits;
      int index = (address >> b) & ((1 << s) - 1);

      if (isVerbose == 1) {
        printf("%c %lx,%d ", access_type, address, size);
      }
      if (access_type == 'S') {
        store(Cache, index, tag, isVerbose);
      }
      if (access_type == 'M') {
        modify(Cache, index, tag, isVerbose);
      }
      if (access_type == 'L') {
        load(Cache, index, tag, isVerbose);
      }
      if (isVerbose == 1) {
        printf("\n");
      }
    }
  }

  fclose(File);
  return;
}

int getEmptyIndex(Cache *cache, int index, int tag) {
  int i;
  int emptyIndex = -1;
  for (i = 0; i < cache->E; ++i) {
    if (cache->sets[index].lines[i].valid == 0) {
      emptyIndex = i;
      break;
    }
  }

  return emptyIndex;
}

int getHitIndex(Cache *cache, int index, int tag) {
    int i;
    int hitIndex = -1;

    for (i = 0; i < cache->E; i++) {
        if (cache->sets[index].lines[i].valid == 1 &&
            cache->sets[index].lines[i].tag == tag) {
            hitIndex = i;
            break;
        }
    }

    return hitIndex;
}

void load(Cache *cache, int index, int tag, int isVerbose) {
  int hitIndex = getHitIndex(cache, index, tag);
  if (hitIndex == -1) {
    misses++;
    if (isVerbose) {
      printf("miss ");
    }
    int emptyIndex = getEmptyIndex(cache, index, tag);
    if (emptyIndex == -1) {
      evictions++;
      if (isVerbose) {
        printf("eviction ");
      }

      int i;
      int firstFlag = 1;
      for (i = 0; i < cache->E; i++) {
        if (cache->sets[index].lines[i].LRU == cache->E - 1 && firstFlag) {
          cache->sets[index].lines[i].valid = 1;
          cache->sets[index].lines[i].LRU = 0;
          cache->sets[index].lines[i].tag = tag;
          firstFlag = 0;
        } else {
          cache->sets[index].lines[i].LRU++;
        }
      }
    } else {
      int i;
      for (i = 0; i < cache->E; i++) {
        if (i != emptyIndex) {
          cache->sets[index].lines[i].LRU++;
        } else {
          cache->sets[index].lines[i].valid = 1;
          cache->sets[index].lines[i].tag = tag;
          cache->sets[index].lines[i].LRU = 0;
        }
      }
    }
  } else {
    hits++;
    if (isVerbose) {
      printf("hit ");
    }

    int hits = cache->sets[index].lines[hitIndex].LRU;
    int i;
    for (i = 0; i < cache->E; i++) {
      if (i != hitIndex) {
        if (cache->sets[index].lines[i].LRU < hits) {
          cache->sets[index].lines[i].LRU++;
        }
      } else {
        cache->sets[index].lines[i].LRU = 0;
      }
    }
  }
}

void store(Cache *cache, int index, int tag, int isVerbose) {
  load(cache, index, tag, isVerbose);
}

void modify(Cache *cache, int index, int tag, int isVerbose) {
  load(cache, index, tag, isVerbose);
  store(cache, index, tag, isVerbose);
}

void printHelp(){
  printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n"
  "-h: Optional help flag that prints usage info\n"
  "-v: Optional verbose flag that displays trace info\n"
  "-s <s>: Number of set index bits (S = 2s is the number of sets)\n"
  "-E <E>: Associativity (number of lines per set)\n"
  "-b <b>: Number of block bits (B = 2b is the block size)\n"
  "-t <tracefile>: Name of the valgrind trace to replay\n");
}
