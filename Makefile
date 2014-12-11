CC=gcc
CXX=g++

#all cpp files
CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(notdir $(CPP_FILES:.cpp=.o))

CPPFLAGS+= \
$(CFLAGS) \
-std=gnu++0x \
-Wwrite-strings \
-Wno-unused-result \
-fpermissive \
-finput-charset=UTF-8 -fexec-charset=UTF-8 \
`pkg-config fuse --cflags`

LDFLAGS= \
`pkg-config fuse --libs`


homefs: $(OBJ_FILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(OBJ) $(LDFLAGS)

#release build(default)
all: CFLAGS+=-O3
all: homefs

#debug build
debug: CFLAGS+=-g -D_DEBUG -O0
debug: homefs

#cleanp
clean:
	/bin/rm homefs *.o *.bak *~ core -f 
