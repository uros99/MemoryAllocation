#pragma once
#include<stdlib.h>
#include<stdbool.h>
#include<Windows.h>
#define EMPTYSLAB (0)
#define FULLSLAB (1)
#define NOTFULLSLAB (2)

typedef void(*ctor)(void *);
typedef void(*dtor)(void *);

typedef struct kmem_cache_s {
	struct kmem_cache_s *nextCache;
	struct kmem_cache_s *prevCache;
	struct slab *slabs[3];
	const char* nameOfCashe;
	size_t sizeOfObject;
	unsigned int numberOfObjectsPerSlab;
	size_t numberOfBlocksForCashe;
	unsigned int numberOfBlocksForSlab;
	int numberOfSlabs;

	int codeOfError;
	bool shrink;
	void* memory;

	ctor constructor;
	dtor destructor;
}kmem_cache_t;

kmem_cache_t* cache_create(const char *name, size_t size, void(*ctor)(void *), void(*dtor)(void *));
void* cache_alloc(kmem_cache_t *cache);
void cache_free(kmem_cache_t *cache, void* addrOfObject);
bool inRange(void *begin, void *end, void *obj);
int cacheShrink(kmem_cache_t* cache);
void printCache(kmem_cache_t* cache);