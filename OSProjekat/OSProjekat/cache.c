#include"cache.h"
#include"buddy.h"
#include"slab.h"
#include"slabH.h"
#include<stdbool.h>
#include<math.h>
extern buddy* Buddy;

kmem_cache_t *cache_create(const char *name, size_t size, void(*ctor)(void *), void(*dtor)(void *)) {

	bool exists = false;
	kmem_cache_t *pom = Buddy->headCache;
	while (pom != NULL) {
		if (pom->sizeOfObject == size) {
			exists = true;
			break;
		}
	}

	if (exists) {
		return pom;
	}
	else {
		size_t numberOfBlocksForCashe = (size_t)ceil(sizeof(kmem_cache_t) / BLOCK_SIZE);
		size_t spaceForObjectsInSlab = BLOCK_SIZE - sizeof(slab);
		void * addr = buddyAlloc(sizeof(numberOfBlocksForCashe * BLOCK_SIZE));
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
		cache->numberOfSlabs = 0;

		cache->slabs[EMPTYSLAB] = NULL;
		cache->slabs[FULLSLAB] = NULL;
		cache->slabs[NOTFULLSLAB] = NULL;


		if (Buddy->headCache == NULL) {
			Buddy->headCache = cache;
			cache->prevCache = NULL;
			cache->nextCache = NULL;
		}
		else {
			kmem_cache_t* tmp = Buddy->headCache;
			Buddy->headCache = cache;
			cache->nextCache = tmp;
			tmp->prevCache = cache;
			cache->prevCache = NULL;
		}
		return cache;
	}
}

void* cache_alloc(kmem_cache_t * cache) {
	unsigned int index = 2;
	if (cache->slabs[NOTFULLSLAB] == NULL && cache->slabs[EMPTYSLAB] == NULL) {
		allocSlab(cache);
		index = NOTFULLSLAB;
	}
	else if (cache->slabs[EMPTYSLAB] != NULL)
		index = EMPTYSLAB;

	void *addrOfObject = allocSlot(cache->slabs[index]);
	return addrOfObject;
}

void printCache(kmem_cache_t* cache)
{
	printf("Name of cache: %s\n", cache->nameOfCashe);
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
