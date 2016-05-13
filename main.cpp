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

	//test db
	BitcaskDB db("./data/");
	db.open();

	char key1[] = "key1";
	char key2[] = "key2";
	char val[] = "value1++++long";
	db.set(key1,strlen(key1),val,strlen(val));
	db.set(key2,strlen(key2),val,strlen(val));
	
	std::string buf;
	db.get(key1,strlen(key1),&buf);
	printf("get key value[%s]\n",buf.c_str());

	db.del(key1,strlen(key1));
	int ret = db.get(key1,strlen(key1),&buf);
	assert(ret == 1);
	
	return 0;
}
