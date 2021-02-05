#include"slab.h"
#include"cache.h"
#include"buddy.h"
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
	kmem_cache_t* cache = cache_create(name, size, ctor, dtor);
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

int kmem_cache_shrink(kmem_cache_t * cachep)
{
	int numOfBlocks = cacheShrink(cachep);
	return numOfBlocks;
}

void * kmem_cache_alloc(kmem_cache_t * cachep)
{
	void * addr = cache_alloc(cachep);
	return addr;
}

void kmem_cache_free(kmem_cache_t * cachep, void * objp)
{
	cache_free(cachep, objp);
}

void * kmalloc(size_t size)
{
	if (size < 5 || size > 17)
		return NULL;
	char name[8];
	sprintf_s(name, sizeof(name), "size-%d", size);
	kmem_cache_t *cache = NULL;
	if (Buddy->cacheBuffers[size - 5] == NULL) {
		cache = cache_create(name, pow(2, size), NULL, NULL);
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
	for (int i = 0; i < NUMBER_OF_BUFFERS;i++) {
		if (Buddy->cacheBuffers[i] != NULL) {
			cache_free(Buddy->cacheBuffers[i], objp);
		}
	}
}