###############################################################################
#                                                                             #
# W-SWFIT: Resource Leak                                                      #
# Authors: Paul Jordan                                                        #
# Date Created:  8 May 2016                                                   #
# Description: Makefile for the W-SWFIT Resource Leak project.                #
#                                                                             #
###############################################################################
PROGRAM=resourceleak
C_FILES:=$(shell find . -iname "*.cpp")
OBJS:=$(patsubst %.cpp, %.o, $(C_FILES))
CFLAGS=-Wall -ffast-math -O3 -std=c++11 -I./incl/ 
LDFLAGS=
SRC=src

native: CC=g++
windows: CC=/usr/local/gcc-4.8.0-qt-4.8.4-for-mingw32/win32-gcc/bin/i586-mingw32-g++

all: native

windows: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROGRAM).exe

native: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROGRAM)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(PROGRAM) *.o $(SRC)/*.o $(PROGRAM).exe
