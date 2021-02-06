#pragma once
#include"cache.h"
#include<Windows.h>

typedef struct slab {
	struct slab *nextSlab;
	struct slab *prevSlab;
	kmem_cache_t *cache;
	unsigned int numberOfObjects;
	unsigned int numberOfBlocks;
	unsigned int slotsInUse;
	int sizeOfSlabInByte;

	void *mem;
	void *endAddrOfSlab;
	void* freeSlots;
} slab;

slab* allocSlab(kmem_cache_t *cache);
void* allocSlot(slab* slabArg);
void deleteSlot(slab* slabArg, void *addrObj);
void insertInList(int index, slab* slabTmp);
void removeFromList(int index, slab* slabTmp);
void printSlab(slab *slabArg);
void clearSlab(slab* slabTmp);