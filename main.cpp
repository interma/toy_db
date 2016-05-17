#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "bitcask.h"

void test_util() {
	//test log
	FILE* f = fopen("main.log", "a");
	assert(f != NULL);
	PosixLogger logger(f);
	
	logger.Logv("N main: %s\n", "start");

	//test hash
	char str[] = "interma";
	printf("hash of [%s] is %llu\n", str, hash(str,strlen(str)));

	//test crc32
	char str2[] = "interma123";
	printf("crc of [%s] is %u\n", str, crc(str,strlen(str)));
	printf("crc of [%s] is %u\n", str2, crc(str2,strlen(str2)));
}

int main (int argc, char **argv)
{
	test_util();

	//test db
	BitcaskDB db("./data/");
	db.open();

	char key1[] = "key1";
	char key2[] = "key2";
	char key3[] = "key3";
	char val[] = "value1++++long";
	db.set(key1,strlen(key1),val,strlen(val));
	db.set(key2,strlen(key2),val,strlen(val));

	int ret = 0;	
	std::string buf;
	ret = db.get(key1,strlen(key1),&buf);
	if (ret == 0)
		printf("get key value[%s]\n",buf.c_str());
	else
		printf("not get key[%s]\n", key1);

	db.del(key3,strlen(key3));
	db.del(key1,strlen(key1));
	ret = db.get(key1,strlen(key1),&buf);
	assert(ret == 1);

	return 0;
}
