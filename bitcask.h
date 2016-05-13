#ifndef __BITCASK_H__
#define __BITCASK_H__

#include <stdint.h>
#include <string>
#include <map>

//using namespace std;

class ValEntry;
/**
 * a bitcask db storage, support:get(),set(),del()
 */ 
class BitcaskDB {
	public:
		int open(const char *path);
		int close();

		int set(const char *key, size_t klen, char *val, size_t vlen);
		int get(const char *key, size_t klen, std::string *val);
		int del(const char *key, size_t klen);
	private:
		std::map<uint64_t, ValEntry> mem_dict_;
};

#endif

