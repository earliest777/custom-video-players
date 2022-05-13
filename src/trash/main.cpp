

#include<string>
#include<iostream>

extern "C"{
#include<stdio.h>
#include<stdlib.h>
#include<X11/Xlib.h>
}

#include "utils.hpp"

int handler(Display* d, XErrorEvent* e)
{
    printf("Error code: %d\n", e->error_code);
    return 0;
}

int main(int argc, char** argv){
	std::string videoPath;
	if(argc <= 1){
		videoPath = "assets/gym.mp4";
	}else if(argc == 2){
		videoPath = argv[1];
	}else{
		printf("usage: Overhead.exe [MP4 FilePath]\n");
		exit(0);
	}

	//Obtain info on the video we're about to play, width, height, number of frames
	unsigned int width, height, frames, frameRate, channels, codecTimeBase;
	const std::string videoInfoCmd = "ffprobe -v error -select_streams v:0 -count_packets"
		"-show_entries stream=nb_read_packets,r_frame_rate,width,height "
		"-of csv=p=0:s=x " + videoPath;
	FILE* sizeProc = popen(videoInfoCmd.c_str(), "r");
	fscanf(sizeProc, "%dx%dx%d/1x%d\n", &width, &height, &frameRate, &frames);
	fflush(sizeProc);
	pclose(sizeProc);
	printf("Width %d\nHeight %d \nFrameRate %d \nFrames %d\n", width, height, frameRate, frames);

	//Set up X11 Window
	XSetErrorHandler(handler);
	Display* display = XOpenDisplay(NULL);
	int screen_num = DefaultScreen(display);
	Window window = XCreateSimpleWindow(display, XDefaultRootWindow(display), 100, 100, width, height, 0, 0, 0);
	Visual *visual = DefaultVisual(display,screen_num);

	//Set up Video Buffer and info
	const unsigned long long frameTime = 1000 / frameRate;
	printf("Allocating XImage with %dx%d\n", width, height);
	char* imageMem = new char[width*height*4];
	XImage* image = XCreateImage(display, visual, DefaultDepth(display,screen_num), ZPixmap, 0, imageMem, width, height, 32, 0);

	const std::string videoCmd = "ffmpeg -v error -i " + videoPath + " -f image2pipe -vcodec rawvideo -pix_fmt bgra -";
	FILE* videoStream = popen(videoCmd.c_str(), "r");
	
	//Begin main event loop
	XMapWindow(display, window);
	XSelectInput(display, window, KeyPressMask | ButtonPressMask | ExposureMask);
	XEvent event;
	bool done = false;
	unsigned int frameCounter = 0;
	unsigned long lastFrameTime = getTime();
	while(!done){
		//If we need to display a new frame based on our frame rate
		if(getTime() - lastFrameTime >= frameTime){
			if(frameCounter >= frames){
				//Close and reopen the pipe to auto-replay the video after it ends
				pclose(videoStream);
				videoStream = popen(videoCmd.c_str(), "r");
				frameCounter = 0;
			}
			fread(imageMem, 1, width*height*4, videoStream);
			XPutImage(display, window, DefaultGC(display,screen_num), image, 0, 0, 0, 0, width, height);
			lastFrameTime = getTime();
			frameCounter++;
		}

		//Handle any X events
		while(XPending(display)){
			XNextEvent(display, &event);
			std::cout<<"XEvent Handled:" + getXEventString(event)<<std::endl;
		}
	}
	pclose(videoStream);
	return 0;
}