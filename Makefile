

all: src/main.cpp
	g++ -g src/main.cpp $(shell pkg-config --cflags --libs x11) -o Overhead.exe