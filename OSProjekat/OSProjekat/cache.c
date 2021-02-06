#include"cache.h"
#include"buddy.h"
#include"slab.h"
#include"slabH.h"
#include<stdbool.h>
#include<math.h>
extern buddy* Buddy;

kmem_cache_t *cache_create(const char *name, size_t size, void(*ctor)(void *), void(*dtor)(void *)) {
	WaitForSingleObject(Buddy->global, INFINITE);

	size_t numberOfBlocksForCashe = (size_t)ceil((double)sizeof(kmem_cache_t) / BLOCK_SIZE);
	size_t spaceForObjectsInSlab = BLOCK_SIZE - sizeof(slab);
	void * addr = buddyAlloc(sizeof(numberOfBlocksForCashe * BLOCK_SIZE));
	if (addr == NULL) {
		printf("There isn't enough memory for cache");
		ReleaseMutex(Buddy->global);
		return NULL;
	}
	kmem_cache_t *cache = (kmem_cache_t*)addr;
		
	size = size < sizeof(int) ? size = sizeof(int) : size;

	while (size > spaceForObjectsInSlab) {
		spaceForObjectsInSlab += BLOCK_SIZE;
	}
	cache->memory = addr;
	cache->constructor = ctor;
	cache->destructor = dtor;
	cache->nameOfCashe = name;
	cache->sizeOfObject = size;
	cache->numberOfObjectsPerSlab = spaceForObjectsInSlab / size;
	cache->numberOfBlocksForSlab = (int)ceil((double)spaceForObjectsInSlab / BLOCK_SIZE);
	cache->numberOfBlocksForCashe = numberOfBlocksForCashe;
	cache->numberOfSlabs = 0;
	cache->codeOfError = 0;
	cache->lock = CreateMutex(NULL, false, NULL);
	cache->shrink = true;
	cache->slabs[EMPTYSLAB] = NULL;
	cache->slabs[FULLSLAB] = NULL;
	cache->slabs[NOTFULLSLAB] = NULL;
	ReleaseMutex(Buddy->global);
	return cache;
}

void* cache_alloc(kmem_cache_t * cache) {
	WaitForSingleObject(Buddy->allocMutex, INFINITE);
	WaitForSingleObject(cache->lock, INFINITE);
	unsigned int index = 2;
	slab *s = NULL;
	if (cache->slabs[NOTFULLSLAB] == NULL && cache->slabs[EMPTYSLAB] == NULL) {
		s = allocSlab(cache);
		if (s == NULL) {
			ReleaseMutex(cache->lock);
			ReleaseMutex(Buddy->allocMutex);
			return NULL;
		}
		cache->shrink = false;
		index = NOTFULLSLAB;
	}
	else if (cache->slabs[EMPTYSLAB] != NULL && cache->slabs[NOTFULLSLAB] == NULL)
		index = EMPTYSLAB;

	void *addrOfObject = allocSlot(cache->slabs[index]);
	if (addrOfObject != NULL && index==0) {
		s = cache->slabs[index];
		removeFromList(EMPTYSLAB, cache->slabs[index]);
		insertInList(NOTFULLSLAB, s);
	}
	ReleaseMutex(cache->lock);
	ReleaseMutex(Buddy->allocMutex);
	return addrOfObject;
}

void cache_free(kmem_cache_t * cache, void * addrOfObject)
{
	WaitForSingleObject(Buddy->freeMutex, INFINITE);
	WaitForSingleObject(cache->lock, INFINITE);
	slab *head = cache->slabs[FULLSLAB];
	while (head != NULL) {
		if (inRange(head->mem, head->endAddrOfSlab, addrOfObject)) {
			deleteSlot(head, addrOfObject);
			removeFromList(FULLSLAB, head);
			insertInList(NOTFULLSLAB, head);
			ReleaseMutex(cache->lock);
			ReleaseMutex(Buddy->freeMutex);
			return;
		}
		head = head->nextSlab;
	}
	if (head == NULL) {
		slab *head = cache->slabs[NOTFULLSLAB];
		while (head != NULL) {
			if (inRange(head->mem, head->endAddrOfSlab, addrOfObject)) {
				deleteSlot(head, addrOfObject);
				if (head->slotsInUse == 0) {
					clearSlab(head);
					removeFromList(NOTFULLSLAB, head);
					insertInList(EMPTYSLAB, head);
				}
				ReleaseMutex(cache->lock);
				ReleaseMutex(Buddy->freeMutex);
				return;
			}
			head = head->nextSlab;
		}
	}
	if (cache->slabs[EMPTYSLAB] != NULL) {
		head = cache->slabs[EMPTYSLAB];
		while (head != NULL) {
			clearSlab(head);
			head = head->nextSlab;
		}
	}
	ReleaseMutex(cache->lock);
	ReleaseMutex(Buddy->freeMutex);
}

bool inRange(void * begin, void * end, void * obj)
{
	if ((char*)obj > (char*)begin && (char*)obj < (char*)end)
		return true;
	else
		return false;
}

int cacheShrink(kmem_cache_t * cache)
{
	WaitForSingleObject(cache->lock, INFINITE);
	static bool callFirstTime = false;
	slab* head = cache->slabs[EMPTYSLAB];
	slab* old = NULL;
	int numOfBlocks = 0;
	if (!cache->shrink) {
		cache->shrink = true;
		if (callFirstTime) {
			ReleaseMutex(cache->lock);
			return numOfBlocks;
		}
		callFirstTime = true;
	}
	while (head != NULL) {
		cache->numberOfSlabs--;
		numOfBlocks = numOfBlocks + (int)ceil((double)(((char*)head->endAddrOfSlab - (char*)head->mem) / BLOCK_SIZE));
		old = head;
		head = head->nextSlab;
		buddyFree(old->mem, old->sizeOfSlabInByte);
	}
	cache->slabs[EMPTYSLAB] = NULL;
	ReleaseMutex(cache->lock);
	return numOfBlocks;
}

void printCache(kmem_cache_t* cache)
{
	printf("Name of cache: %s\n", cache->nameOfCashe);
	printf("Number of slabs %d\n", cache->numberOfSlabs);
	printf("Empty slabs:\n");
	slab *pom = cache->slabs[EMPTYSLAB];
	while (pom != NULL) {
		printSlab(pom);
		pom = pom->nextSlab;
	}

	printf("Full slabs:\n");
	pom = cache->slabs[FULLSLAB];
	while (pom != NULL) {
		printSlab(pom);
		pom = pom->nextSlab;
	}

	printf("Not full slabs:\n");
	pom = cache->slabs[NOTFULLSLAB];
	while (pom != NULL) {
		printSlab(pom);
		pom = pom->nextSlab;
	}
}
