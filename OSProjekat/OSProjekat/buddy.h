#pragma once

typedef struct Buddy {
	int numOfBlocks;
	void* myMem;
	int sizeOfFreeArr;
} buddy;

void buddy_init(void* space, int blockNumber);
void buddy_destroy();
void* block(int index);