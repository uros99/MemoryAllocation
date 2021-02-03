#include"cashe.h"
#include"buddy.h"
#include<stdbool.h>

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
		void * addr = buddyAlloc(size);
		kmem_cache_t *cache = (kmem_cache_t*)addr;
		cache->memory = addr;
		cache->constructor = ctor;
		cache->destructor = dtor;
		cache->nameOfCashe = name;
		cache->sizeOfObject = size;

		kmem_cache_t* tmp = Buddy->headCache;
		Buddy->headCache = cache;
		cache->nextCashe = tmp;
		return cache;
	}
}