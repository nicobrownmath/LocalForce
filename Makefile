CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -pedantic -O3 -march=native -mtune=native -fno-stack-protector -fomit-frame-pointer #-flto
LDFLAGS =

all: LocalForce
LocalForce: LocalForce.cpp LifeAPI.h
	$(CC) $(CFLAGS) $(INSTRUMENTFLAGS) -o LocalForce LocalForce.cpp $(LDFLAGS)