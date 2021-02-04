#include"slab.h"
#include"cache.h"
#include"buddy.h"
#include<string.h>
#include<math.h>
#include<stdio.h>

void kmem_init(void * space, int block_num)
{
	buddyInit(space, block_num);
}

kmem_cache_t * kmem_cache_create(const char * name, size_t size, void(*ctor)(void *), void(*dtor)(void *))
{
	kmem_cache_t* cache = cache_create(name, size, ctor, dtor);
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
	char name[8];
	sprintf_s(name, sizeof(name), "size-%d", size);
	kmem_cache_t *cache = kmem_cache_create(name, (size_t)pow(2, size), NULL, NULL);
	void* addr = kmem_cache_alloc(cache);
	return addr;
}
