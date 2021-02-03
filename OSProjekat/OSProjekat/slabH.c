#include"slabH.h"
#include"slab.h"
#include"buddy.h"
#include"cache.h"
#include<stdio.h>

slab * allocSlab(kmem_cache_t * cache)
{
	size_t size = cache->numberOfBlocksForSlab * BLOCK_SIZE + sizeof(slab);
	void *mem = buddyAlloc(size);
	slab *slabTmp = (slab*)mem;
	slabTmp->cache = cache;
	slabTmp->mem = mem;
	slabTmp->numberOfObjects = cache->numberOfObjectsPerSlab;
	slabTmp->numberOfBlocks = cache->numberOfBlocksForSlab;
	slabTmp->slotsInUse = 0;
	slabTmp->nextSlab = NULL;
	slabTmp->prevSlab = NULL;

	slabTmp->freeSlots = (char*)mem + sizeof(slab);
	void *pom = slabTmp->freeSlots;
	int i = 1;
	while ((char*)pom < (char*)mem + BLOCK_SIZE * (slabTmp->numberOfBlocks)) {
		if (i == slabTmp->numberOfObjects-1) {
			*(int*)pom = -1;
			pom = (char*)pom + slabTmp->cache->sizeOfObject;
			break;
		}
		*(int*)pom = i;
		pom = (char*)pom + slabTmp->cache->sizeOfObject;
		i++;
	}
	insertInList(NOTFULLSLAB, slabTmp);
	cache->numberOfSlabs++;
	return slabTmp;
}

void * allocSlot(slab* slabArg)
{
	int head = *(int*)((char*)slabArg->freeSlots);
	void *addr = slabArg->freeSlots;
	slabArg->freeSlots = (char*)slabArg->mem + sizeof(slab) + head * slabArg->cache->sizeOfObject;
	if (*(int*)slabArg->freeSlots == -1) {
		removeFromList(NOTFULLSLAB, slabArg);
		insertInList(FULLSLAB, slabArg);
	}
	slabArg->slotsInUse++;
	return addr;
}

void insertInList(int index, slab * slabTmp)
{
	if (slabTmp->cache->slabs[index] == NULL) {
		slabTmp->cache->slabs[index] = slabTmp;
		slabTmp->prevSlab = NULL;
		slabTmp->nextSlab = NULL;
	}
	else {
		slab* pom = slabTmp->cache->slabs[index];
		slabTmp->cache->slabs[index] = slabTmp;
		slabTmp->nextSlab = pom;
		slabTmp->prevSlab = NULL;
		pom->prevSlab = slabTmp;
	}
	return slabTmp->cache->slabs[index];
}

void removeFromList(int index, slab * slabTmp)
{
	if (slabTmp->cache->slabs[index] == slabTmp) {
		slabTmp->cache->slabs[index] = slabTmp->nextSlab;
		if (slabTmp->nextSlab != NULL) {
			slabTmp->nextSlab->prevSlab = NULL;
		}
		slabTmp->nextSlab = NULL;
		slabTmp->prevSlab = NULL;
	}
	else {
		slab *prev = slabTmp->prevSlab;
		slab *next = slabTmp->nextSlab;
		prev->nextSlab = next;
		if (next != NULL)
			next->prevSlab = prev;
		slabTmp->nextSlab = NULL;
		slabTmp->prevSlab = NULL;
	}
	return slabTmp->cache->slabs[index];
}

void printSlab(slab *slabArg) {
	printf("Number of available blocks: %d\n", slabArg->numberOfBlocks);
	printf("Number of max objects in one slab: %d\n", slabArg->numberOfObjects);
	for (unsigned int i = 0; i < slabArg->numberOfObjects;i++) {
		void * addr = (char*)slabArg->freeSlots + (i * slabArg->cache->sizeOfObject);
		printf("%d\n", *(int*)addr);
	}
}