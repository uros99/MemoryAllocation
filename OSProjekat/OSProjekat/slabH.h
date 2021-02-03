#pragma once
#include"cache.h"

typedef struct slab {
	struct slab *nextSlab;
	struct slab *prevSlab;
	kmem_cache_t *cache;
	unsigned int numberOfObjects;
	unsigned int numberOfBlocks;
	unsigned int slotsInUse;
	void *mem;
	void* freeSlots;
} slab;

slab* allocSlab(kmem_cache_t *cache);
void* allocSlot(slab* slabArg);
void insertInList(int index, slab* slabTmp);
void removeFromList(int index, slab* slabTmp);
void printSlab(slab *slabArg);