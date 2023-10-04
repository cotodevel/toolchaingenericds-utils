TARGET = toolchaingenericds-utils
LIBS = -lm	-lzcustom
CC = gcc
CFLAGS = -g -Wall

.PHONY: default all clean

default: installzlib	$(TARGET)
all: default

SRCS =  ./	ToolchainGenericDSFS/ TGDSVideoConverter/	http/
OBJECTS = $(foreach dir,$(SRCS), $(patsubst %.c, %.o, $(wildcard $(dir)*.c)  ) )	$(foreach dir,$(SRCS), $(patsubst %.cpp, %.o, $(wildcard $(dir)*.cpp)  ) )

HDRS = ./	/ToolchainGenericDSFS /TGDSVideoConverter	http/
HEADERS = $(foreach dirres,$(HDRS),-I "$(dirres)" )

installzlib:
	-@echo 'setup zlib'
	-@cd	$(CURDIR)/zlib-1.2.11 &&	./configure --prefix=/usr/local/zlib
	-@sudo $(MAKE)	-C	$(CURDIR)/zlib-1.2.11
	-@sudo mv	$(CURDIR)/zlib-1.2.11/libz.a	$(CURDIR)/libzcustom.a
	

%.o: %.cpp 
	 g++  $(CPPFLAGS) $(HEADERS) -c $< -o $@	-static -static-libstdc++

%.o: %.c 
	$(CC) $(CFLAGS) $(HEADERS) -c $< -o $@	-static -static-libgcc

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	-@g++	$(OBJECTS)	-Wall	-L	$(CURDIR)	$(LIBS)	-o	$@
	-@sudo mv	$(CURDIR)/$(TARGET)	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(TARGET)
	-@echo '$(TARGET) build OK';

clean:
	-rm -f *.o $(OBJECTS) $(TARGET) libzcustom.a
	-$(MAKE)	clean	-C	zlib-1.2.11/
		
debug:
	-@echo '$(OBJECTS) ';
