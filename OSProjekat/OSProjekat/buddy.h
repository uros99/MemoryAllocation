#pragma once
#include"cache.h"
#define NUMBER_OF_BUFFERS (13)

typedef struct Buddy {
	int numOfBlocks;
	void* myMem;
	int sizeOfFreeArr;
	int sizeOfBuddy;
	kmem_cache_t *headCache;
	kmem_cache_t *cacheBuffers[NUMBER_OF_BUFFERS];
} buddy;

void buddyInit(void* space, int blockNumber);
void* buddyAlloc(size_t cashSize);
void buddyFree(void* addr, size_t size);
int blockToInd(void* addr);
void deleteFromList(int blockNum, int numberOfBlocks);
void insertToList(int blockNum, int numberOfBlocks);
void buddyDelete();
void printBuddyMem();
void* block(int index);