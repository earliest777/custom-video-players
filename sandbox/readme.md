# Sandbox

Misc Tests for various parts of this project
* main.cpp: testing X11 video playing
* simple.cpp: testing audio streaming with the pulse audio simple API & FFMPEG
* stream.cpp: testing audio streaming with the pulse audio async streams API & FFMPEG
* videoDemux.cpp: testing video display using libavcodec decoding and OpenGL rendering

# Audio Test

Some simple tests with streaming and the pulse audio API

In conclusion, pthread + simple pulse API is insuffecent and choppy, its much better to use the Async Pulse Stream API