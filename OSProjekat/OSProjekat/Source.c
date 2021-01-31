#include<stdio.h>
#include"buddy.h"
#include<stdlib.h>
int main() {
	void* space = malloc(4096 * 1000);
	buddy_init(space, 1000);
	buddy_destroy();
	for (;;);
}