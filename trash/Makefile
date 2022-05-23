

stream: stream.cpp
	g++ stream.cpp -lpulse-simple -lpulse

simple: simple.cpp
	g++ simple.cpp -lpulse-simple -lpulse -pthread

demux: videoDemux.cpp
	g++ -g videoDemux.cpp -L"/usr/lib/x86_64-linux-gnu/" -lavformat -lavcodec -lavutil