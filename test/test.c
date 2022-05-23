#include <stdio.h>

volatile int global = 0;
int main() {
	global = 12;
	printf("hello, world\n", global);
	return 0;
}
