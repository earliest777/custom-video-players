
CC = g++
FLAGS = -g -std=c++17
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=bin/%.opp)
SHADERS = $(wildcard src/*.glsl)
LIBS = -L"/usr/lib/x86_64-linux-gnu/" -lX11 -lpulse -lGL -lglfw -lavformat -lavcodec

bin/Overhead: $(OBJECTS) $(SHADERS)
	cp $(SHADERS) bin
	$(CC) $(FLAGS) -o $@ $(OBJECTS) $(LIBS) 

bin/%.opp : src/%.cpp src/%.hpp
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.opp : src/%.cpp
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm bin/*