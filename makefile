SRCS=$(wildcard *.c)
NAME=rice10

all:
	gcc -oFast $(SRCS) -o $(NAME)