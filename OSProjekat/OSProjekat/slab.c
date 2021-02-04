#include"slab.h"
#include"cache.h"
#include"buddy.h"

void kmem_init(void * space, int block_num)
{
	buddyInit(space, block_num);
}

kmem_cache_t * kmem_cache_create(const char * name, size_t size, void(*ctor)(void *), void(*dtor)(void *))
{
	kmem_cache_t* cache = cache_create(name, size, ctor, dtor);
	return cache;
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
