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
	printf("\thash of [%s] is %llu\n", str, hash(str,strlen(str)));

	//test crc32
	char str2[] = "interma123";
	printf("\tcrc of [%s] is %u\n", str, crc(str,strlen(str)));
	printf("\tcrc of [%s] is %u\n", str2, crc(str2,strlen(str2)));
	
	printf("util test ok\n");
}

void test_db() {
	//test db
	BitcaskDB db("./data/");
	db.open();

	int ret = 0;	
	char key1[] = "key1";
	char key2[] = "key2";
	char key3[] = "key3";
	char val[] = "value1++++long";
	db.set(key1,strlen(key1),val,strlen(val));
	ret = db.set(key2,strlen(key2),val,strlen(val));
	assert (ret == 0);

	std::string buf;
	ret = db.get(key2,strlen(key2),&buf);
	assert (ret == 0);
	//printf("\tkey[%s]=>val[%s]\n", key2, buf.c_str());
	//return;
	assert(strcmp(buf.c_str(),val) == 0);

	db.del(key3,strlen(key3));
	db.del(key1,strlen(key1));
	ret = db.get(key1,strlen(key1),&buf);
	assert(ret == 1);

	//write big file
	uint64_t fsize;
	const char fname[] = "./test/img.png";
	ret = GetFileSize(fname, &fsize);
	assert(ret == 0);
	printf("\tfile[%s] size[%llu]\n", fname, fsize);
	//char data_buf[200*1024];
	
	//write existed key
	char val2[] = "interma";
	ret = db.set(key2,strlen(key2),val2,strlen(val2));
	assert(ret == 0);
	ret = db.get(key2,strlen(key2),&buf);
	assert(ret == 0);
	assert(strcmp(buf.c_str(),val2) == 0);

	printf("db test ok\n");
}

int main (int argc, char **argv)
{
	test_util();
	test_db();

	return 0;
}
