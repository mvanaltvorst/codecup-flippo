CC = g++
CCFLAGS = -Wall -Wextra -O2 -g --std=c++11

.PHONY: all

all: mcts

%: %.cpp
	$(CC) $(CCFLAGS) -o $@ %<
