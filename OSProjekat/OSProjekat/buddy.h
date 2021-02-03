#pragma once
#include"cashe.h"

typedef struct Buddy {
	int numOfBlocks;
	void* myMem;
	int sizeOfFreeArr;
	int sizeOfBuddy;
	kmem_cache_t *headCache;
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