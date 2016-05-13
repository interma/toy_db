#ifndef __BITCASK_H__
#define __BITCASK_H__

#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string>
#include <map>
#include "util.h"

//using namespace std;

/**
 * a bitcask db storage, support:get(),set(),del()
 */ 

class BitcaskDB {
	public:
		BitcaskDB(const char *db_path):\
			db_path_(db_path),data_offset_(0),data_fd_(-1),logger_(NULL) {};
		~BitcaskDB();
		
		int open();
		//int close();
		
		/**
		 *@return: -1 error; 0 success; 1 key not exist(in get)
		 */ 
		int set(const char *key, size_t klen, char *val, size_t vlen);
		int get(const char *key, size_t klen, std::string *val);
		int del(const char *key, size_t klen);

	private:
		struct ValEntry {
			uint64_t offset;
			size_t len;
		};
		
		typedef std::map<uint64_t, ValEntry> Map;
		Map mem_dict_;
		
		std::string db_path_;
		uint64_t data_offset_;
		int data_fd_;
		pthread_mutex_t lock_;
		PosixLogger *logger_;
		
		//not allow copy
		BitcaskDB(const BitcaskDB&);
		void operator=(const BitcaskDB&);

};

#endif

