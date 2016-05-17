
GCC=g++
CPPFLAGS = -g -Wall -W -Winline -Werror  -Wno-unused-parameter -Wno-unused-function

INCLUDE_PATH=-I. \

LIB_PATH=-L. -lpthread

OBJ=main

all : $(OBJ) output;
$(OBJ) : \
	main.o \
	util.o \
	murhash2.o \
	crc32.o \
	bitcask.o \

	$(GCC) -o $@ $^  $(INCLUDE_PATH) $(LIB_PATH) 

%.o     : %.cpp
	$(GCC) $(CPPFLAGS) -c $< -o $@ $(INCLUDE_PATH)

output :
	rm -rf output; mkdir -p output ; cp $(OBJ) output

.PHONY: clean 
clean:
	rm -fr output;
	rm -f *.o $(OBJ);
