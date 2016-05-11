#ifndef __BITCASK_H__
#define __BITCASK_H__

#include <stdint.h>
#include <string>

//using namespace std;

/**
 * a bitcask db storage, support:get(),set(),del()
 */ 
class BitcaskDB {
	public:
		int open(const char *path);
		int close();

		int set(const char *key, char *value, size_t vlen);
		int get(const char *key, std::string *value);
		int del(const char *key);
	private:
	
};

#endif

