#include"slab.h"
#include"cache.h"
#include"buddy.h"
#include"slabH.h"

#include<string.h>
#include<math.h>
#include<stdio.h>

extern buddy* Buddy;

void kmem_init(void * space, int block_num)
{
	buddyInit(space, block_num);
}

kmem_cache_t * kmem_cache_create(const char * name, size_t size, void(*ctor)(void *), void(*dtor)(void *))
{
	bool exists = false;
	kmem_cache_t* cache = NULL;
	kmem_cache_t *pom = Buddy->headCache;
	while (pom != NULL) {
		if (pom->sizeOfObject == size) {
			cache = pom;
			exists = true;
			break;
		}
		pom = pom->nextCache;
	}
	if(!exists)
		cache = cache_create(name, size, ctor, dtor);

	if (cache != NULL && !exists) {
		addCacheToList(cache);
	}
	return cache;
}

int kmem_cache_shrink(kmem_cache_t * cachep)
{
	int numOfBlocks = cacheShrink(cachep);
	return numOfBlocks;
}

void * kmem_cache_alloc(kmem_cache_t * cachep)
{
	void * addr = cache_alloc(cachep);
	if (cachep->constructor != NULL && addr!=NULL)
		cachep->constructor(addr);
	return addr;
}

void kmem_cache_free(kmem_cache_t * cachep, void * objp)
{
	cache_free(cachep, objp);
}

void * kmalloc(size_t size)
{
	size = (int)ceil(log2(size));
	if (size < 5 || size > 17) {
		printf("Cache with that object size is not exists\n");
		return NULL;
	}
	char name[8];
	sprintf_s(name, sizeof(name), "size-%d", size);
	kmem_cache_t *cache = NULL;
	if (Buddy->cacheBuffers[size - 5] == NULL) {
		cache = cache_create(name, pow(2, size), NULL, NULL);
		if (cache == NULL) {
			return NULL;
		}
		Buddy->cacheBuffers[size - 5] = cache;
	}
	else
		cache = Buddy->cacheBuffers[size - 5];
	void *addr = NULL;
	if (cache != NULL) {
		addr = cache_alloc(cache);
	}
	return addr;
}

void kfree(const void * objp)
{
	WaitForSingleObject(Buddy->global, INFINITE);
	for (int i = 0; i < NUMBER_OF_BUFFERS;i++) {
		if (Buddy->cacheBuffers[i] != NULL) {
			cache_free(Buddy->cacheBuffers[i], objp);
		}
	}
	ReleaseMutex(Buddy->global);
}

void kmem_cache_destroy(kmem_cache_t * cachep)
{
	if (cachep == NULL) return;

	cacheShrink(cachep);
	WaitForSingleObject(cachep->lock, INFINITE);
	if (cachep->slabs[NOTFULLSLAB] != NULL || cachep->slabs[FULLSLAB] != NULL) {
		cachep->codeOfError = 2;
		ReleaseMutex(cachep->lock);
		return;
	}
	deleteCacheFromList(cachep);
	buddyFree(cachep, cachep->numberOfBlocksForCashe * BLOCK_SIZE);
	ReleaseMutex(cachep->lock);
}

void kmem_cache_info(kmem_cache_t * cachep)
{
	WaitForSingleObject(Buddy->printMutex, INFINITE);
	printf("Name of cache: %s\n", cachep->nameOfCashe);
	printf("Size of one object in cache: %u\n", cachep->sizeOfObject);
	printf("Size of slabs in blocks: %u\n", cachep->numberOfBlocksForSlab);
	printf("Number of slabs in cache: %d\n", cachep->numberOfSlabs);
	printf("Number of object in one slab: %u\n", cachep->numberOfObjectsPerSlab);

	int numberOfSlotsInUse = 0;
	slab *head = cachep->slabs[NOTFULLSLAB];
	while (head != NULL) {
		numberOfSlotsInUse += head->slotsInUse;
		head = head->nextSlab;
	}
	head = cachep->slabs[FULLSLAB];
	while (head!=NULL)
	{
		numberOfSlotsInUse += head->slotsInUse;
		head = head->nextSlab;
	}
	double prosentage = (double)numberOfSlotsInUse / (double)(cachep->numberOfSlabs * cachep->numberOfObjectsPerSlab);
	printf("Prosentage of occupancy: %f\n", prosentage);
	ReleaseMutex(Buddy->printMutex);
}

int kmem_cache_error(kmem_cache_t * cachep)
{
	WaitForSingleObject(cachep->lock, INFINITE);
	switch (cachep->codeOfError)
	{
	case 0: printf("There is no error in system\n");break;
	case 1: printf("Buddy doesn't have enough memory to allocate\n");break;
	case 2: printf("Cache is not empty\n");break;
	default:
		printf("The error is unknown\n");
		break;
	}
	ReleaseMutex(cachep->lock);
	return cachep->codeOfError;
}
