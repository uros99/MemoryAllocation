#include<stdio.h>
#include<math.h>

#include"buddy.h"
#include"slab.h"

buddy* Buddy = NULL;

void buddy_init(void* space, int blockNumber) {

	if (space == NULL || blockNumber<0) {
		printf("\n Greska, dodeljena adresa je NULL");
		return;
	}

	Buddy = (buddy*)space;

	int arr = (int)log2(blockNumber);

	int numberOfBlocksForBuddy = (sizeof(buddy) + (sizeof(int*)*arr)) / BLOCK_SIZE;
	if (numberOfBlocksForBuddy == 0) {
		numberOfBlocksForBuddy++;
	}
	int numOfBlocks = blockNumber - numberOfBlocksForBuddy;
	Buddy->numOfBlocks = numOfBlocks;
	int sizeOfFreeArr = (int)log2(blockNumber - numberOfBlocksForBuddy);
	Buddy->sizeOfFreeArr = sizeOfFreeArr;

	size_t memory = (size_t)((char*)space + numberOfBlocksForBuddy * BLOCK_SIZE);
	printf("%d\n", sizeof(size_t));
	memory += BLOCK_SIZE - 1;
	unsigned int maska = BLOCK_SIZE - 1;
	memory &= ~maska;
	printf("%d", memory);
	Buddy->myMem = (void*)memory;

	for (int index = 0; index < Buddy->sizeOfFreeArr - 1;index++) {
		*((int*)((buddy*)Buddy + 1) + index) = -1;
	}

	*((int*)((buddy*)Buddy + 1) + Buddy->sizeOfFreeArr - 1) = 0;
	*(int*)(block(0)) = -1;
}

void buddy_destroy() {
	if (Buddy != NULL) {
		for (int i = 0; i < Buddy->sizeOfFreeArr; i++) {
			*((int*)((buddy*)Buddy + 1) + i) = -1;
		}
		for (int i = 0; i < Buddy->numOfBlocks;i++) {
			*(int*)(block(i)) = -1;
		}

		//obrisati cash
		//obrisati bafere

		Buddy->myMem = NULL;
		Buddy = NULL;
	}
}

void * block(int index)
{
	return (void*)((char*)Buddy->myMem + BLOCK_SIZE * index);
}
