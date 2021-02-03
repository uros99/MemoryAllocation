#include"slab.h"
#include"cashe.h"
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
