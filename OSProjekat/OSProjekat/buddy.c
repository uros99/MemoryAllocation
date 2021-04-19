#include<stdio.h>
#include<math.h>
#include<stdbool.h>

#include"buddy.h"
#include"slab.h"

buddy* Buddy = NULL;

void buddyInit(void* space, int blockNumber) {

	if (space == NULL || blockNumber < 0) {
		printf("\n Greska, dodeljena adresa je NULL");
		return;
	}

	Buddy = (buddy*)space;

	int arr = (int)ceil(log2(blockNumber));

	int numberOfBlocksForBuddy = (sizeof(buddy) + (sizeof(int*)*arr)) / BLOCK_SIZE;
	if (numberOfBlocksForBuddy == 0) {
		numberOfBlocksForBuddy++;
	}
	Buddy->sizeOfBuddy = numberOfBlocksForBuddy;
	int numOfBlocks = blockNumber - numberOfBlocksForBuddy;
	Buddy->numOfBlocks = numOfBlocks;
	int sizeOfFreeArr = (int)ceil(log2(blockNumber - numberOfBlocksForBuddy));
	Buddy->sizeOfFreeArr = sizeOfFreeArr;
	
	size_t memory = (size_t)((char*)space + numberOfBlocksForBuddy * BLOCK_SIZE);
	memory += BLOCK_SIZE - 1;
	unsigned int maska = BLOCK_SIZE - 1;
	memory &= ~maska;
	Buddy->myMem = (void*)memory;

	for (int i = 0; i < NUMBER_OF_BUFFERS;i++) {
		Buddy->cacheBuffers[i] = NULL;
	}

	for (int index = 0; index <= Buddy->sizeOfFreeArr - 1;index++) {
		*((int*)((buddy*)Buddy + 1) + index) = -1;
	}

	*((int*)((buddy*)Buddy + 1) + Buddy->sizeOfFreeArr) = 0;
	*((int*)block(0)) = -1;

	Buddy->headCache = NULL;
}

void* buddyAlloc(size_t cashSize) {
	int blkNum = 0;
	if (!cashSize%BLOCK_SIZE) {
		blkNum = cashSize / BLOCK_SIZE;
	}
	else {
		blkNum = cashSize / BLOCK_SIZE + 1;
	}

	int tmp = (int)ceil(log2(blkNum));
	int powerTwo = 1 << tmp;
	if (powerTwo > Buddy->numOfBlocks) {
		return NULL;
	}
	int headIndex = *((int*)((buddy*)Buddy + 1) + (int)log2(powerTwo));
	if (headIndex != -1) {
		void * adrrOfHead = block(headIndex);
		int newHead = *((int*)block(headIndex));
		*((int*)((buddy*)Buddy + 1) + (int)log2(powerTwo)) = newHead;
		return adrrOfHead;
	}
	else {
		int index = (int)log2(powerTwo);
		int n = (int)ceil(log2(Buddy->numOfBlocks));
		int numOfSpliting = 0;
		for (int i = index;i <= n;i++, numOfSpliting++) {
			int firstElem = *((int*)((buddy*)Buddy + 1) + i);
			if (firstElem != -1) {
				int curr = i;
				while (numOfSpliting > 0) {
					*((int*)((buddy*)Buddy + 1) + curr) = *((int*)block(firstElem));
					int oldNext = *((int*)((buddy*)Buddy + 1) + curr - 1);
					*((int*)((buddy*)Buddy + 1) + curr - 1) = firstElem;
					int nextBlock = (int)(firstElem + pow(2,(double)(curr - 1)));
					*((int*)block(firstElem)) = nextBlock;
					*((int*)block(nextBlock)) = oldNext;
					numOfSpliting--;
					curr--;
				}
				void* retAdrr = block(*((int*)((buddy*)Buddy + 1) + index));
				*((int*)((buddy*)Buddy + 1) + index) = *((int*)block(firstElem));
				return retAdrr;
			}
		}
		return NULL;
	}
}

void buddyFree(void * addr, size_t size){
	unsigned int blockNumber = blockToInd(addr);
	unsigned int numberOfBlocks = (int)ceil((double)size / BLOCK_SIZE);
	int tmp = (int)ceil(log2(numberOfBlocks));
	numberOfBlocks = 1 << tmp;
	
	int firstElement = *((int*)((buddy*)Buddy + 1) + (int)(log2(numberOfBlocks)));
	if (*((int*)((buddy*)Buddy + 1) + (int)(log2(numberOfBlocks))) == -1) {
		*((int*)((buddy*)Buddy + 1) + (int)(log2(numberOfBlocks))) = blockNumber;
		*((int*)block(blockNumber)) = -1;
	}
	else {
		insertToList(blockNumber, numberOfBlocks);

		while (true) {
			int numberOfChunks = blockNumber / numberOfBlocks;
			bool left = false;
			if (numberOfChunks % 2 != 0)
				left = true;
			int pom = -1;
			int prev = -1;
			pom = *((int*)((buddy*)Buddy + 1) + (int)(log2(numberOfBlocks)));
			prev = -1;
			while (pom != blockNumber) {
				prev = pom;
				pom = *(int*)block(pom);
			}
			if (left == true) {
				if (blockNumber - numberOfBlocks == prev) {
					deleteFromList(blockNumber, numberOfBlocks);
					deleteFromList(prev, numberOfBlocks);
					numberOfBlocks *= 2;
					insertToList(prev, numberOfBlocks);
					blockNumber = prev;
				}
				else
					break;
			}
			else {
				int next = *(int*)block(pom);
				if (blockNumber + numberOfBlocks == next) {
					deleteFromList(blockNumber, numberOfBlocks);
					deleteFromList(next, numberOfBlocks);
					numberOfBlocks *= 2;
					insertToList(blockNumber, numberOfBlocks);
				}
				else
					break;
			}
		}
	}
}

void insertToList(int blockNum, int numberOfBlocks) {
	int head = *((int*)((buddy*)Buddy + 1) + (int)log2(numberOfBlocks));
	int prev = -1;
	
	while ((head < blockNum && head != -1)) {
		prev = head;
		head = *(int*)block(head);
	}
	if (prev == -1) {
		*((int*)((buddy*)Buddy + 1) + (int)(log2(numberOfBlocks))) = blockNum;
		*(int*)block(blockNum) = head;
	}
	else {
		*((int*)block(blockNum)) = head;
		*(int*)block(prev) = blockNum;
	}
}

void addCacheToList(kmem_cache_t * cache)
{
	if (Buddy->headCache == NULL) {
		Buddy->headCache = cache;
		cache->prevCache = NULL;
		cache->nextCache = NULL;
	}
	else {
		kmem_cache_t* tmp = Buddy->headCache;
		Buddy->headCache = cache;
		cache->nextCache = tmp;
		tmp->prevCache = cache;
		cache->prevCache = NULL;
	}
}

void deleteCacheFromList(kmem_cache_t * cache)
{
	if (Buddy->headCache == NULL) return;
	if (Buddy->headCache == cache) {
		Buddy->headCache = cache->nextCache;
		if (cache->nextCache != NULL) {
			cache->nextCache->prevCache = NULL;
		}
		cache->nextCache = NULL;
		cache->prevCache = NULL;
	}
	else {
		kmem_cache_t *prev = cache->prevCache;
		kmem_cache_t *next = cache->nextCache;
		prev->nextCache = next;
		if (next != NULL)
			next->prevCache = prev;
		cache->nextCache = NULL;
		cache->prevCache = NULL;
	}
}

void deleteFromList(int blockNum, int numberOfBlocks) {
	int head = *((int*)((buddy*)Buddy + 1) + (int)log2(numberOfBlocks));
	int prev = -1;
	while (head != blockNum) {
		prev = head;
		head = *((int*)block(head));
	}
	if (prev == -1) {
		*((int*)((buddy*)Buddy + 1) + (int)log2(numberOfBlocks)) = *((int*)block(head));
		*((int*)block(head)) = -1;
	}
	else {
		*(int*)block(prev) = *(int*)block(head);
		*((int*)block(head)) = -1;
	}
}

int blockToInd(void * addr)
{
	return (int)ceil(((char*)addr - (char*)Buddy->myMem) / BLOCK_SIZE);
}

void buddyDelete() {
	if (Buddy != NULL) {
		for (int i = 0; i < Buddy->sizeOfFreeArr; i++) {
			*((int*)((buddy*)Buddy + 1) + i) = -1;
		}
		for (int i = 0; i < Buddy->numOfBlocks;i++) {
			*(int*)(block(i)) = -1;
		}

		Buddy->headCache = NULL;
		for (int i = 0; i < NUMBER_OF_BUFFERS;i++) {
			Buddy->cacheBuffers[i] = NULL;
		}
		Buddy->myMem = NULL;
		Buddy = NULL;
	}
}

void printBuddyMem()
{
	for (int i = 0; i <= Buddy->sizeOfFreeArr; i++) {
		int head = *((int*)((buddy*)Buddy + 1) + i);
		if (head == -1)printf("%d", head);
		while (head != -1) {
			printf("%d ", head);
			head = *(int*)block(head);
		}
		printf("\n");
	}
}

void * block(int index)
{
	return (void*)((char*)Buddy->myMem + BLOCK_SIZE * index);
}
