# Sandbox

Misc Tests for various parts of this project
* main.cpp: testing X11 video playing
* simple.cpp: testing audio streaming with the pulse audio simple API & FFMPEG
* stream.cpp: testing audio streaming with the pulse audio async streams API & FFMPEG
* videoDemux.cpp: testing video display using libavcodec decoding and OpenGL rendering

# Audio Test

Some simple tests with streaming and the pulse audio API

In conclusion, pthread + simple pulse API is insuffecent and choppy, its much better to use the Async Pulse Stream API

# Video Demux Test (using sw_scale)

swscale is very slow, callgrind results speak for themselves:
```
2,316,787,200  ???:0x000000000001c500 [/usr/lib/x86_64-linux-gnu/libswscale.so.5.5.100]
1,470,300,000  ???:0x0000000000259080 [/usr/lib/x86_64-linux-gnu/dri/swrast_dri.so]
1,262,855,880  ???:0x000000000484b450 [???]
1,097,675,280  ???:0x0000000000068120 [/usr/lib/x86_64-linux-gnu/libswscale.so.5.5.100]
  680,742,657  ???:0x000000000032ef60 [/usr/lib/x86_64-linux-gnu/libavcodec.so.58.54.100]
  619,298,380  ???:0x0000000000372120 [/usr/lib/x86_64-linux-gnu/libavcodec.so.58.54.100]
  508,319,804  ???:0x000000000034b0d0 [/usr/lib/x86_64-linux-gnu/libavcodec.so.58.54.100]
  478,995,875  ???:0x000000000032bdc0 [/usr/lib/x86_64-linux-gnu/libavcodec.so.58.54.100]
```
Large CPU used by swscale for the flip and YUV conversion.

## Notes

In the avframe object the YUV data is encoded with 8 bits per pixel
