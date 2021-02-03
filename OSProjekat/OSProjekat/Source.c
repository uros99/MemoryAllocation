#include<stdio.h>
#include<math.h>
#include<stdlib.h>

#include"slab.h"
#include"cache.h"
int main() {
	void *mem = malloc(BLOCK_SIZE * 1000);
	kmem_init(mem, 1000);
	kmem_cache_t* cache = kmem_cache_create("PCB", 16, NULL, NULL);
	for(int i = 0; i<100;i++)
		kmem_cache_alloc(cache);
	printCache(cache);
/*	kmem_cache_alloc(cache);
	printCache(cache);*/
	for (;;);
}