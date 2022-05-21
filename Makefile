
CC = g++
FLAGS = -g -std=c++17
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=bin/%.opp)
LIBS = -lX11 -lpulse 

Overhead: $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ $(LIBS) 

bin/%.opp : src/%.cpp src/%.hpp
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.opp : src/%.cpp
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm bin/*