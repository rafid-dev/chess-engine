SRCS=$(wildcard *.c)
NAME=rice11

all:
	gcc -Ofast $(SRCS) -o $(NAME) -pthread
debug:
	gcc -Wall -Ofast $(SRCS) -o $(NAME) -pthread -g