SRCS=$(wildcard *.c)
NAME=rice10

all:
	gcc -Ofast $(SRCS) -o $(NAME) -pthread