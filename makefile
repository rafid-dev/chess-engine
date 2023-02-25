SRCS=$(wildcard *.c)
NAME=rice10

all:
	gcc -Ofast $(SRCS) -o $(NAME) -pthread

debug:
	gcc -Ofast -g -Wall $(SRCS) -o $(NAME) -pthread