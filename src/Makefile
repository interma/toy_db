
GCC=g++
CPPFLAGS = -g -Wall -W -Winline -Werror  -Wno-unused-parameter -Wno-unused-function

INCLUDE_PATH=-I. \

LIB_PATH=-L. -lpthread

TEST=test_db
LIB=libtoydb.a

all : $(TEST) $(LIB) output;
$(TEST) : \
	test_db.o \
	util.o \
	murhash2.o \
	crc32.o \
	bitcask.o \

	$(GCC) -o $@ $^  $(INCLUDE_PATH) $(LIB_PATH) 

$(LIB) : \
	util.o \
	murhash2.o \
	crc32.o \
	bitcask.o \

	ar crv $@ $^

%.o : %.cpp
	$(GCC) $(CPPFLAGS) -c $< -o $@ $(INCLUDE_PATH)

output :
	rm -rf ../output
	mkdir -p ../output 
	cp bitcask.h ../output
	cp $(LIB) ../output
	cp $(TEST) ../output

.PHONY: clean 
clean:
	rm -f *.o $(TEST) $(LIB);
	rm -fr ../output;
