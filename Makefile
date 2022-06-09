
CC = g++
FLAGS = -g -std=c++17
SOURCES = $(wildcard src/*.cpp) bin/shaders.cpp
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

bin/shaders.cpp: src/shaders/vert.glsl src/shaders/frag.glsl
	rm -f shaders.cpp
	touch shaders.cpp
	printf "//THIS FILE IS MACHINE GENERATED AND WILL BE OVERWRITTEN, IT SHOULD NOT BE EDITED\n\n" >> shaders.cpp
	printf "const char* vertexShader =\n\"" >> shaders.cpp
	cat src/shaders/vert.glsl | sed -z 's/\n/\\n\"\n\"/g' >> shaders.cpp
	printf "\";\n\nconst char* fragmentShader = \"" >> shaders.cpp
	cat src/shaders/frag.glsl |  sed -z 's/\n/\\n\"\n\"/g' >> shaders.cpp >> shaders.cpp
	printf "\";\n" >> shaders.cpp

clean:
	rm bin/*