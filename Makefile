TARGET = toolchaingenericds-utils
LIBS = -lm	-lz
CC = gcc
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))	$(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	 g++  $(CPPFLAGS) -c $< -o $@	-static -static-libstdc++

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@	-static -static-libgcc

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	-@g++	$(OBJECTS)	-Wall	$(LIBS)	-o	$@
	-@echo '$(TARGET) build OK';

clean:
	-rm -f *.o $(TARGET)
