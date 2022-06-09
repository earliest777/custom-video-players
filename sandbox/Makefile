

stream: stream.cpp
	g++ stream.cpp -lpulse-simple -lpulse

simple: simple.cpp
	g++ simple.cpp -lpulse-simple -lpulse -pthread

demux: videoDemux.cpp
	g++ -g videoDemux.cpp -L"/usr/lib/x86_64-linux-gnu/" -lavformat -lavcodec -lswscale -lavutil -lGL -lGLEW -lglfw 

shader: videoDemuxShader.cpp shaders.cpp
	g++ -g videoDemuxShader.cpp shaders.cpp -L"/usr/lib/x86_64-linux-gnu/" -lavformat -lavcodec -lavutil -lGL -lGLEW -lglfw

shaders.cpp: vert.glsl frag.glsl
	rm -f shaders.cpp
	touch shaders.cpp
	printf "//THIS FILE IS MACHINE GENERATED AND WILL BE OVERWRITTEN, IT SHOULD NOT BE EDITED\n" >> shaders.cpp
	printf "const char* vertexShader = \"" >> shaders.cpp
	cat vert.glsl | sed -z 's/\n/\\n\"\n\"/g' >> shaders.cpp
	printf "\";\nconst char* fragmentShader = \"" >> shaders.cpp
	cat frag.glsl |  sed -z 's/\n/\\n\"\n\"/g' >> shaders.cpp >> shaders.cpp
	printf "\";\n" >> shaders.cpp

clean:
	rm -f a.out
	rm -f shaders.cpp