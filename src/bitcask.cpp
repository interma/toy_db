
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

int BitcaskDB::open(bool trunc) {
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
	if (trunc)
		data_fd_ = ::open(data_path.c_str(), O_TRUNC | O_CREAT | O_RDWR);
	else
		data_fd_ = ::open(data_path.c_str(), O_CREAT | O_RDWR);

	if (data_fd_ < 0) {
		logger_->Logv("F open data fail\n");
		return -1;
	}

	//var
	pthread_mutex_init(&lock_, NULL);	
	
	//recover from data
	if (!trunc && this->recover() < 0) {
		logger_->Logv("F recover data fail\n");
		return -1;
	}

	logger_->Logv("N open db\n");
	return 0;
}

BitcaskDB::~BitcaskDB() {
	logger_->Logv("N close db\n");
	delete logger_;
	close(data_fd_);
}

int BitcaskDB::set(const char *key, size_t klen, char *val, size_t vlen) {
	logger_->Logv("N set db key[%s]\n", key);
	ValEntry entry;
	//record: crc32 seq keylen vallen keydata valdata 
	entry.len = klen+vlen+sizeof(size_t)*2+sizeof(uint32_t)+sizeof(uint64_t);
	//use key hash64 as sign 
	uint64_t sign = hash(key, klen);

	pthread_mutex_lock(&lock_);
	//get offset & seqnum
	entry.offset = data_offset_;
	data_offset_ += entry.len;
	uint64_t seq = seqnum_++;
	
	//write data
	write_record(seq,key,klen,val,vlen);

	//write map
	mem_dict_[sign] = entry;	

	pthread_mutex_unlock(&lock_);
	return 0;
}


/**
 * record format:
 * crc32 seq keylen vallen keydata valdata 
 */ 
int BitcaskDB::write_record(uint64_t seq, const char *key, size_t klen, char *val, size_t vlen) {
	
	//gen crc32	
	uint32_t cr = crc(&seq, sizeof(seq));
	cr = crc(&klen, sizeof(klen), cr);
	cr = crc(&vlen, sizeof(vlen), cr);
	cr = crc(key, klen, cr);
	//printf("crc1:%u\n", cr);		
	if (val != NULL)
		cr = crc(val, vlen, cr);
	//printf("crc2:%u\n", cr);		

	write(data_fd_, &cr, sizeof(cr));
	write(data_fd_, &seq, sizeof(seq));
	
	write(data_fd_, &klen, sizeof(klen));
	write(data_fd_, &vlen, sizeof(vlen));
	
	write(data_fd_, key, klen);
	if (val != NULL) //val==NULL && vlen==0 indicate del()
		write(data_fd_, val, vlen);

	//fsync(data_fd_);
	return 0;
}

int BitcaskDB::get(const char *key, size_t klen, std::string *val) {
	//use key hash64 as sign 
	uint64_t sign = hash(key, klen);

	pthread_mutex_lock(&lock_);
	//get entry
	Map::iterator it = mem_dict_.find(sign);
	if (it == mem_dict_.end()){
		pthread_mutex_unlock(&lock_);
		return 1; //key not exist
	}
	uint64_t offset = it->second.offset;
	size_t len = it->second.len;
	pthread_mutex_unlock(&lock_);
	
	//read data file
	int ret = read_record(offset,len,val);
	if (ret < 0) {
		logger_->Logv("W read data fail[%lld][%llu]", offset, len);
		return -1;
	}

	return 0;
}

int	BitcaskDB::read_record(uint64_t offset, size_t len, std::string *val) {
	val->clear();
	//mmap read or buf pread?
	char read_buf[READ_BUF_SIZE];
	
	bool first_block = true;
	uint32_t cur_cr = 0;
	uint32_t save_cr = 0;
	while (len > 0) {
		size_t need_read = len>READ_BUF_SIZE? READ_BUF_SIZE:len;
		logger_->Logv("D loop read data [%lld][%lld]", offset, len);
		ssize_t r = pread(data_fd_, read_buf, need_read, static_cast<off_t>(offset));
		if (r < 0)
			return -1;
		//data_buf: crc32 seq keylen vallen keydata valdata 
		if (first_block) {
			first_block = false;
			
			uint64_t seq = 0; 
			size_t klen = 0;
			size_t vlen = 0;
			
			memcpy(&save_cr, read_buf, sizeof(save_cr));
			memcpy(&seq, read_buf+sizeof(uint32_t), sizeof(seq));
			memcpy(&klen, read_buf+sizeof(uint32_t)+sizeof(uint64_t), sizeof(klen));
			memcpy(&vlen, read_buf+sizeof(uint32_t)+sizeof(uint64_t)+sizeof(size_t), sizeof(vlen));
			//printf("seq[%llu] klen:%u vlen:%u\n", seq, klen, vlen);		
			size_t key_offset = sizeof(uint32_t)+sizeof(uint64_t)+sizeof(size_t)*2;
			size_t val_offset = sizeof(uint32_t)+sizeof(uint64_t)+sizeof(size_t)*2+klen;
			assert(val_offset < READ_BUF_SIZE); //buf must can hold {key}

			//cal crc
			cur_cr = crc(&seq, sizeof(seq));
			cur_cr = crc(&klen, sizeof(klen), cur_cr);
			cur_cr = crc(&vlen, sizeof(vlen), cur_cr);
			cur_cr = crc(read_buf+key_offset, klen, cur_cr);
			//printf("crc1:%u\n", cur_cr);		
			
			//jump to val
			val->append(read_buf+val_offset, r-val_offset);	
		}
		else {
			val->append(read_buf, r);	
		}
		len -= r;
		offset += r;
	}

	//check crc32
	cur_cr = crc(val->data(), val->length(), cur_cr); //add val to crc
	//printf("crc2:%u\n", cur_cr);		
	if (save_cr != cur_cr) {
		logger_->Logv("W read_record crc32 not match[%u][%u]", save_cr, cur_cr);
		return -1;
	}

	return 0;
}

int BitcaskDB::del(const char *key, size_t klen) {
	//use key hash64 as sign 
	uint64_t sign = hash(key, klen);

	pthread_mutex_lock(&lock_);
	
	//set offset	
	data_offset_ += klen+sizeof(size_t)*2+sizeof(uint32_t)+sizeof(uint64_t);
	uint64_t seq = seqnum_++;
	//write data
	write_record(seq,key,klen,NULL,0); //val==0 indicate del
	//erase map
	mem_dict_.erase(sign);	

	pthread_mutex_unlock(&lock_);
	return 0;
}

int BitcaskDB::print_db(uint64_t cnt) {
	//record: crc32 seq keylen vallen keydata valdata 
	string data_path(db_path_);
	data_path.append("data");	
	int fd = ::open(data_path.c_str(), O_RDONLY);
	if (fd < 0) {
		logger_->Logv("F open data in print_db() fail\n");
		return -1;
	}
	
	printf("==begin print db==\n");

	char read_buf[READ_BUF_SIZE];
	uint64_t cur_cnt = 0;
	while (cnt == 0 || cur_cnt < cnt ){
		uint32_t cr = 0;
		uint64_t seq = 0; 
		size_t klen = 0;
		size_t vlen = 0;

		uint64_t record_head_len = sizeof(uint32_t)+sizeof(uint64_t)+sizeof(size_t)*2;
		ssize_t r = read(fd, read_buf, record_head_len);
		if (r <= 0)
			break;

		memcpy(&cr, read_buf, sizeof(cr));
		memcpy(&seq, read_buf+sizeof(uint32_t), sizeof(seq));
		memcpy(&klen, read_buf+sizeof(uint32_t)+sizeof(uint64_t), sizeof(klen));
		memcpy(&vlen, read_buf+sizeof(uint32_t)+sizeof(uint64_t)+sizeof(size_t), sizeof(vlen));

		//get key
		r = read(fd, read_buf, klen);
		if (r <= 0)
			break;
		read_buf[klen] = '\0'; 
		printf("R[%llu] key[%s] crc[%u] seq[%llu] klen[%u] vlen[%u]\n", \
			cur_cnt++,read_buf,cr,seq,klen,vlen);
		
		//jump val
		lseek(fd, vlen, SEEK_CUR);
	}

	printf("==end print db, total [%llu] records==\n", cur_cnt);
	close(fd);
	return 0;
}

/**
 * recover from last data file: rebuild mem_dict
 */ 
int BitcaskDB::recover() {
	uint64_t cur_cnt = 0;
	
	while (true){
		char read_buf[READ_BUF_SIZE];
		
		uint32_t cr = 0;
		uint64_t seq = 0; 
		size_t klen = 0;
		size_t vlen = 0;

		uint64_t record_head_len = sizeof(uint32_t)+sizeof(uint64_t)+sizeof(size_t)*2;
		ssize_t r = read(data_fd_, read_buf, record_head_len);
		if (r <= 0)
			break;

		memcpy(&cr, read_buf, sizeof(cr));
		memcpy(&seq, read_buf+sizeof(uint32_t), sizeof(seq));
		memcpy(&klen, read_buf+sizeof(uint32_t)+sizeof(uint64_t), sizeof(klen));
		memcpy(&vlen, read_buf+sizeof(uint32_t)+sizeof(uint64_t)+sizeof(size_t), sizeof(vlen));

		//get key
		r = read(data_fd_, read_buf, klen);
		if (r <= 0)
			break;
		read_buf[klen] = '\0'; 
		const char *key = read_buf;

		//jump val
		lseek(data_fd_, vlen, SEEK_CUR);
		
		ValEntry entry;
		entry.offset = data_offset_;
		entry.len = klen+vlen+sizeof(size_t)*2+sizeof(uint32_t)+sizeof(uint64_t);
		data_offset_ += entry.len;
		seqnum_++;

		uint64_t sign = hash(key, klen);
		if (vlen == 0)
			mem_dict_.erase(sign);	
		else	
			mem_dict_[sign] = entry;	
		
		cur_cnt++;
	}

	logger_->Logv("N db recover ok\n");
	return 0;
}


