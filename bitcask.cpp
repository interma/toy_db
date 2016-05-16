
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
	
	//read data file
	int ret = read_record(offset,len,val);
	if (ret < 0) {
		logger_->Logv("W read data fail[%lld][%lld]", offset, len);
		return -1;
	}

	return 0;
}

int	BitcaskDB::read_record(uint64_t offset, size_t len, std::string *val) {
	val->clear();
	//mmap read or buf pread?
	char read_buf[READ_BUF_SIZE];

	while (len > 0) {
		size_t need_read = len>READ_BUF_SIZE? READ_BUF_SIZE:len;
		logger_->Logv("D loop read data [%lld][%lld]", offset, len);
		ssize_t r = pread(data_fd_, read_buf, need_read, static_cast<off_t>(offset));
		if (r < 0)
			return -1;
		//jump to val
		if (offset == 0) {
			size_t klen = 0;
			memcpy(&klen, read_buf, sizeof(klen));
			//printf("keylen:%d\n", klen);		
			size_t head_key_len = sizeof(size_t)*2+klen;
			assert(head_key_len < READ_BUF_SIZE);
			val->append(read_buf+head_key_len, r-head_key_len);	
		}
		else {
			val->append(read_buf, r);	
		}
		len -= r;
		offset += r;
	}

	return 0;
}

int BitcaskDB::del(const char *key, size_t klen) {
	return 0;
}



