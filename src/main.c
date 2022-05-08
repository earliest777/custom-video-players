
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<X11/Xlib.h>

int handler(Display* d, XErrorEvent* e)
{
    printf("Error code: %d\n", e->error_code);
    return 0;
}

int main(int argc, char** argv){
	unsigned int width, height;
	FILE* sizeProc = popen("ffprobe -v error -select_streams v -show_entries stream=width,height -of csv=p=0:s=x assets/sample.mp4", "r");
	fscanf(sizeProc, "%dx%d\n", &width, &height);
	fflush(sizeProc);
	pclose(sizeProc);
	printf("Allocating XImage with %dx%d\n", width, height);

	XSetErrorHandler(handler);
	Display* display = XOpenDisplay(NULL);
	int screen_num = DefaultScreen(display);
	Window window = XCreateSimpleWindow(display, XDefaultRootWindow(display), 100, 100, width, height, 0, 0, 0);
	Visual *visual = DefaultVisual(display,screen_num);

	char* imageMem = malloc(width*height*4);
	XImage* image = XCreateImage(display, visual, DefaultDepth(display,screen_num), ZPixmap, 0, imageMem, width, height, 32, 0);

	XMapWindow(display, window);
	XSelectInput(display, window, KeyPressMask | ButtonPressMask | ExposureMask);
	
	FILE* vidInput = popen("ffmpeg -v error -i assets/sample.mp4 -f image2pipe -vcodec rawvideo -pix_fmt rgba -", "r");
	XEvent event;
	while(True){
		fread(imageMem, 1, width*height*4, vidInput);
		XPutImage(display, window, DefaultGC(display,screen_num), image, 0,0,0,0, width, height);
		//printf("%d\n", event.type);
		XNextEvent(display, &event);
	}
	pclose(vidInput);
	return 0;
}