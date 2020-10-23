#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv){
	char* i = malloc(atoi(argv[1]));
	sleep(100);
	return 0;
}
