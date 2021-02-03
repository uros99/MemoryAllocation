#pragma once

struct kmem_cache_s;

typedef struct Slab {
	struct Slab *nextSlab;
	struct kmem_cache_s *cache;
} slab;