#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
	char str3[] = "interm";
	char str4[] = "a123";
	printf("\t?crc of [%s] is %u\n", str2, crc(str2,strlen(str2)));
	printf("\tlast crc is %u\n", crc(str4,strlen(str4),crc(str3,strlen(str2))) );
	
	printf("util test ok\n");
}

void test_db() {
	//test db
	BitcaskDB db("./data/");
	db.open(true);

	int ret = 0;	
	char key1[] = "key1";
	char key2[] = "key2";
	char key3[] = "key3";
	char val[] = "value1++++long";
	ret = db.set(key1,strlen(key1),val,strlen(val));
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
	const char fname_w[] = "./test/img_w.png";
	ret = GetFileSize(fname, &fsize);
	assert(ret == 0);
	printf("\tfile[%s] size[%llu]\n", fname, fsize);
	char data_buf[200*1024];
	int fd = open(fname, O_RDONLY);
	read(fd, data_buf, fsize);	
	char key4[]= "img";
	ret = db.set(key4,strlen(key4),data_buf,fsize);

	
	assert(ret == 0);
	ret = db.get(key4,strlen(key4),&buf);
	assert(ret == 0);
	int fd_w = open(fname_w, O_CREAT | O_WRONLY);
	write(fd_w, buf.data(), buf.length());
	close(fd_w);
	
	//write existed key
	char val2[] = "interma";
	ret = db.set(key2,strlen(key2),val2,strlen(val2));
	assert(ret == 0);
	ret = db.get(key2,strlen(key2),&buf);
	assert(ret == 0);
	assert(strcmp(buf.c_str(),val2) == 0);

	db.print_db();
	printf("db test ok\n");
}

void test_db_with_recover() {
	BitcaskDB db("./data/");
	db.open(); //no truncate
	
	int ret = 0;	
	char key1[] = "key1";
	char key2[] = "key2";
	char val[] = "newinterma";
	std::string buf;
	ret = db.get(key1,strlen(key1),&buf);
	assert(ret == 1);
	ret = db.get(key2,strlen(key2),&buf);
	assert(ret == 0);
	
	ret = db.set(key2,strlen(key2),val,sizeof(val));
	ret = db.get(key2,strlen(key2),&buf);
	assert(ret == 0);
	assert(strcmp(buf.c_str(),val) == 0);

	db.print_db();
	printf("db with recover test ok\n");
}

int main (int argc, char **argv)
{
	test_util();
	test_db();
	test_db_with_recover();

	return 0;
}
