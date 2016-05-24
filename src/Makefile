
GCC=g++
CPPFLAGS = -g -fPIC -Wall -W -Winline -Werror  -Wno-unused-parameter -Wno-unused-function

INCLUDE_PATH=-I. \

LIB_PATH=-L. -lpthread

TEST=test_db
A=libtoydb.a
SO=libtoydb.so

all : $(TEST) $(A) $(SO) output;
$(TEST) : \
	test_db.o \
	util.o \
	murhash2.o \
	crc32.o \
	bitcask.o \

	$(GCC) -o $@ $^  $(INCLUDE_PATH) $(LIB_PATH) 

$(A) : \
	util.o \
	murhash2.o \
	crc32.o \
	bitcask.o \

	ar crv $@ $^
	
$(SO) : \
	util.o \
	murhash2.o \
	crc32.o \
	bitcask.o \

	$(GCC) -shared -Wl,-soname,$@ -o $@ $^

%.o : %.cpp
	$(GCC) $(CPPFLAGS) -c $< -o $@ $(INCLUDE_PATH)

output :
	rm -rf ../output
	mkdir -p ../output 
	cp bitcask.h ../output
	cp $(A) ../output
	cp $(SO) ../output
	cp $(TEST) ../output

.PHONY: clean 
clean:
	rm -f *.o $(TEST) $(A) $(SO);
	rm -fr ../output;
