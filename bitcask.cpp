
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
	return 0;
}

int BitcaskDB::get(const char *key, size_t klen, std::string *val) {
	return 0;
}

int BitcaskDB::del(const char *key, size_t klen) {
	return 0;
}



