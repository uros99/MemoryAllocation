#include"slabH.h"
#include"slab.h"
#include"buddy.h"
#include"cache.h"
#include<stdio.h>


extern buddy* Buddy;

slab * allocSlab(kmem_cache_t * cache)
{
	size_t size = cache->numberOfBlocksForSlab * BLOCK_SIZE + sizeof(slab);
	void *mem = buddyAlloc(size);
	if (mem == NULL) {
		cache->codeOfError = 1;
		return NULL;
	}
	slab *slabTmp = (slab*)mem;
	slabTmp->cache = cache;
	slabTmp->mem = mem;
	slabTmp->numberOfObjects = cache->numberOfObjectsPerSlab;
	slabTmp->numberOfBlocks = cache->numberOfBlocksForSlab;
	slabTmp->endAddrOfSlab = (char*)mem + sizeof(slab) + BLOCK_SIZE * (slabTmp->numberOfBlocks);
	slabTmp->sizeOfSlabInByte = (int)((char*)slabTmp->endAddrOfSlab - (char*)slabTmp->mem);
	slabTmp->slotsInUse = 0;
	slabTmp->nextSlab = NULL;
	slabTmp->prevSlab = NULL;
	slabTmp->slabHandle = CreateMutex(NULL, false, NULL);
	slabTmp->freeSlots = (char*)mem + sizeof(slab);
	void *pom = slabTmp->freeSlots;
	int i = 1;
	while ((char*)pom < (char*)mem + sizeof(slab) + BLOCK_SIZE * (slabTmp->numberOfBlocks)) {
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

void clearSlab(slab* slabTmp) {
	slabTmp->freeSlots = (char*)slabTmp->mem + sizeof(slab);
	slabTmp->slotsInUse = 0;
	void *pom = slabTmp->freeSlots;
	int i = 1;
	while ((char*)pom < (char*)slabTmp->mem + sizeof(slab) + BLOCK_SIZE * (slabTmp->numberOfBlocks)) {
		if (i == slabTmp->numberOfObjects - 1) {
			*(int*)pom = -1;
			pom = (char*)pom + slabTmp->cache->sizeOfObject;
			break;
		}
		*(int*)pom = i;
		pom = (char*)pom + slabTmp->cache->sizeOfObject;
		i++;
	}
}

void * allocSlot(slab* slabArg)
{
	int head = *(int*)slabArg->freeSlots;
	void *addr = slabArg->freeSlots;
	if (head == -1) {
		slabArg->freeSlots = (char*)slabArg->mem + sizeof(slab);
	}
	else
		slabArg->freeSlots = (char*)slabArg->mem + sizeof(slab) + head * slabArg->cache->sizeOfObject;
	slabArg->slotsInUse++;
	if (slabArg->slotsInUse == slabArg->numberOfObjects) {
		removeFromList(NOTFULLSLAB, slabArg);
		insertInList(FULLSLAB, slabArg);
	}
	return addr;
}

void deleteSlot(slab * slabArg, void * addrObj)
{
	int head = ((char*)slabArg->freeSlots - ((char*)slabArg->mem + sizeof(slab))) / slabArg->cache->sizeOfObject;
	slabArg->freeSlots = addrObj;
	if(slabArg->slotsInUse == slabArg->numberOfObjects -2)
		*(int*)slabArg->freeSlots = -1;
	else
		*(int*)slabArg->freeSlots = head;
	slabArg->slotsInUse--;
}

void insertInList(int index, slab * slabTmp)
{
	WaitForSingleObject(slabTmp->slabHandle,INFINITE);
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
	ReleaseMutex(slabTmp->slabHandle);
}

void removeFromList(int index, slab * slabTmp)
{
	WaitForSingleObject(slabTmp->slabHandle, INFINITE);
	if (slabTmp->cache->slabs[index] == NULL)return;
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
	ReleaseMutex(slabTmp->slabHandle);
}

void printSlab(slab *slabArg) {
	printf("Number of available blocks: %d\n", slabArg->numberOfBlocks);
	printf("Number of max objects in one slab: %d\n", slabArg->numberOfObjects);
	void *addr = slabArg->freeSlots;
	while (*(int*)addr!=-1) {
		printf("%d\n", *(int*)addr);
		addr = (char*)slabArg->mem + sizeof(slab) + (*(int*)addr * slabArg->cache->sizeOfObject);
	}
}