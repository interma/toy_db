
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <dirent.h>
#include <assert.h>
#include <string>
#include <unistd.h>

#include "bitcask.h"
#include "util.h"
		
using namespace std;

int BitcaskDB::open() {
	//logger
	string log_path(db_path_);
	log_path.append("LOG.log");	
	FILE* f = fopen(log_path.c_str(), "a");
	if (f == NULL) {
		perror("open logger file fail");
		return -1;
	}
	logger_ = new PosixLogger(f);

	//data file
	string data_path(db_path_);
	data_path.append("data");	
	//TODO recover from data
	data_fd_ = ::open(data_path.c_str(), O_CREAT | O_RDWR);
	if (data_fd_ < 0) {
		perror("open data file fail");
		return -1;
	}

	//var
	pthread_mutex_init(&lock_, NULL);	

	logger_->Logv("N open db\n");
	return 0;
}

BitcaskDB::~BitcaskDB() {
	logger_->Logv("N close db\n");
	delete logger_;
	close(data_fd_);
}

int BitcaskDB::set(const char *key, size_t klen, char *val, size_t vlen) {
	ValEntry entry;
	entry.len = klen+vlen+sizeof(size_t)*2;
	//use key hash64 as sign 
	uint64_t sign = hash(key, klen);

	pthread_mutex_lock(&lock_);
	//get offset
	entry.offset = data_offset_;
	data_offset_ += entry.len;
	
	//write data
	write_record(key,klen,val,vlen);

	//write map
	mem_dict_[sign] = entry;	

	pthread_mutex_unlock(&lock_);
	return 0;
}

int BitcaskDB::write_record(const char *key, size_t klen, char *val, size_t vlen) {
	write(data_fd_, &klen, sizeof(klen));
	write(data_fd_, &vlen, sizeof(vlen));
	write(data_fd_, key, klen);
	write(data_fd_, val, vlen);
	//syncfs(data_fd_);
	return 0;
}

int BitcaskDB::get(const char *key, size_t klen, std::string *val) {
	val->clear();
	//use key hash64 as sign 
	uint64_t sign = hash(key, klen);

	pthread_mutex_lock(&lock_);
	//get entry
	Map::iterator it = mem_dict_.find(sign);
	if (it == mem_dict_.end())
		return 1; //key not exist
	uint64_t offset = it->second.offset;
	size_t len = it->second.len;
	pthread_mutex_unlock(&lock_);

	read_record(offset,len,val);

	return 0;
}

int	BitcaskDB::read_record(uint64_t offset, size_t len, std::string *val) {
	//mmap read or buf pread?
	 
	return 0;
}

int BitcaskDB::del(const char *key, size_t klen) {
	return 0;
}



