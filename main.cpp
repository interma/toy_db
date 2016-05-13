#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "bitcask.h"

int main (int argc, char **argv)
{
	//test log
	FILE* f = fopen("main.log", "a");
	assert(f != NULL);
	PosixLogger logger(f);
	
	logger.Logv("N main: %s\n", "start");


	//test hash
	char str[] = "interma";
	printf("hash of [%s] is %llu\n", str, hash(str,strlen(str)));

	printf("i am run!");
	return 0;
}
