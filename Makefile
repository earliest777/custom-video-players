

all: src/main.c
	gcc -g src/main.c $(shell pkg-config --cflags --libs x11) -o Overhead.exe