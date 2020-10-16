#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char** argv){
	sleep(2);
	printf("HI from %d\n", atoi(argv[1]));
	return 0;
}
