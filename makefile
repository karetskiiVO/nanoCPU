#the compiler
CC = g++
 
#flags:
CFLAGS  = -g -Wall
 
#sources of input
SOURCES = main.cpp  Slang.cpp

#sources of output
EXECUTABLE = test

all:
	$(CC) $(SOURCES) $(CFLAGS) -MD -o $(EXECUTABLE) -lm