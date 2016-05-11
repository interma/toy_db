/**
 * copy from leveldb
 */ 
#ifndef UTIL_H_
#define UTIL_H_

#include <algorithm>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

class PosixLogger {
private:
	FILE* file_;
	PosixLogger(const PosixLogger&);
	void operator=(const PosixLogger&);
public:
	PosixLogger(FILE* f) : file_(f) { }
	~PosixLogger() { fclose(file_); }
	void Logv(const char* format, ...);
};

#endif

