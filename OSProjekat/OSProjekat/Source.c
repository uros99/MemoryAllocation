#include<stdio.h>
#include<math.h>
#include<stdlib.h>

#include"slab.h"
#include"cache.h"

int main() {
	void *mem = malloc(BLOCK_SIZE * 1000);
	kmem_init(mem, 1000);
	kmem_cache_t* cache = kmem_cache_create("PCB", 500, NULL, NULL);
	void *address[100];
	for(int i = 0; i < 100; i++)
		address[i] = kmem_cache_alloc(cache);
	printCache(cache);
	for (int i = 0; i < 100; i++)
		kmem_cache_free(cache, address[i]);
	printCache(cache);
	int free = kmem_cache_shrink(cache);
	printf("\n NUMBER OF BLOCKS FREE %d\n", free);
	printCache(cache);
	void * addr = kmalloc(15);
	int* i = (int*)addr;
	*i = 5;
	kfree(i);
	void* addr1 = kmalloc(15);
	int* j = (int*)addr1;
	*j = 7;
	printf("%d\n", *i);

	for (;;);
}