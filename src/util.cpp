
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util.h"


static uint64_t gettid() {
	pthread_t tid = pthread_self();
	uint64_t thread_id = 0;
	memcpy(&thread_id, &tid, std::min(sizeof(thread_id), sizeof(tid)));
	return thread_id;
}

uint64_t MurmurHash64A ( const void * key, int len, uint64_t seed );
uint64_t hash(const void * key, int len) {
	return MurmurHash64A(key,len,0);
}

uint32_t crc32 (uint32_t crc, const unsigned char *buf, size_t len);
uint32_t crc (const void *buf, size_t len, uint32_t cr) {
	return crc32(cr, reinterpret_cast<const unsigned char *>(buf), len);
}

int GetFileSize(const char *fname, uint64_t* size) {
	struct stat sbuf;
	if (stat(fname, &sbuf) != 0) {
		*size = 0;
		return -1;
	} else {
		*size = sbuf.st_size;
	}
	return 0;
}

void PosixLogger::Logv(const char* format, ...) {
	const uint64_t thread_id = gettid();

	va_list ap;
	va_start(ap, format);

	// We try twice: the first time with a fixed-size stack allocated buffer,
	// and the second time with a much larger dynamically allocated buffer.
	char buffer[500];
	for (int iter = 0; iter < 2; iter++) {
		char* base;
		int bufsize;
		if (iter == 0) {
			bufsize = sizeof(buffer);
			base = buffer;
		} else {
			bufsize = 30000;
			base = new char[bufsize];
		}
		char* p = base;
		char* limit = base + bufsize;

		struct timeval now_tv;
		gettimeofday(&now_tv, NULL);
		const time_t seconds = now_tv.tv_sec;
		struct tm t;
		localtime_r(&seconds, &t);
		p += snprintf(p, limit - p,
			"%04d/%02d/%02d-%02d:%02d:%02d.%06d %llx ",
			t.tm_year + 1900,
			t.tm_mon + 1,
			t.tm_mday,
			t.tm_hour,
			t.tm_min,
			t.tm_sec,
			static_cast<int>(now_tv.tv_usec),
			static_cast<long long unsigned int>(thread_id));

		// Print the message
		if (p < limit) {
			va_list backup_ap;
			va_copy(backup_ap, ap);
			p += vsnprintf(p, limit - p, format, backup_ap);
			va_end(backup_ap);
		}
		va_end(ap);

		// Truncate to available space if necessary
		if (p >= limit) {
			if (iter == 0) {
				continue;       // Try again with larger buffer
			} else {
				p = limit - 1;
			}
		}

		// Add newline if necessary
		if (p == base || p[-1] != '\n') {
			*p++ = '\n';
		}

		assert(p <= limit);
		fwrite(base, 1, p - base, file_);
		fflush(file_);
		if (base != buffer) {
			delete[] base;
		}
		break;
	}
}
