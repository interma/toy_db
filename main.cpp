#include <stdio.h>
#include <assert.h>
#include "util.h"
#include "bitcask.h"

int main (int argc, char **argv)
{
	FILE* f = fopen("main.log", "a");
	assert(f != NULL);
	PosixLogger logger(f);
	
	logger.Logv("N main: %s\n", "start");
	printf("i am run!");
	return 0;
}
