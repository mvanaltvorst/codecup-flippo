#TODO: learn how to do Makefiles properly

CC = g++
CCFLAGS = -Wall -Wextra -O2 -g --std=c++11

.PHONY: main

main: src/main.cpp
	$(CC) $(CCFLAGS) src/main.cpp -o $@
