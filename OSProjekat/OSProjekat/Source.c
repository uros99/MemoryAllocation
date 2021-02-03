#include<stdio.h>
#include<math.h>
#include<stdlib.h>

#include"buddy.h"
#include"cashe.h"
int main() {
	void* space = malloc(4096 * 1000);
	buddyInit(space, 1000);
	void* allocMem = buddyAlloc(150000);
	void* allocMem1 = buddyAlloc(90000);
	printBuddyMem();
	printf("\n");
	buddyFree(allocMem, 150000);
	buddyFree(allocMem1, 90000);
	printBuddyMem();
	buddyDelete();
	for (;;);
}