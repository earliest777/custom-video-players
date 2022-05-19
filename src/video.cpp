// Copyright 2022 James T Oswald

extern "C" {
#include<sys/time.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<inttypes.h>
}

#include<string>
#include<iostream>
#include<stdexcept>

#include"video.hpp"

// Helpers ==================================================================================================

// Gets the string of an X11 event
// https://stackoverflow.com/a/60763874/6342516
const std::string& getXEventString(const XEvent& e) {
    static const std::string XEventTypeStrings[37] = {
        "", "", "KeyPress", "KeyRelease", "ButtonPress"
        "ButtonRelease", "MotionNotify", "EnterNotify",
        "LeaveNotify", "FocusIn", "FocusOut", "KeymapNotify",
        "Expose", "GraphicsExpose", "NoExpose", "VisibilityNotify",
        "CreateNotify", "DestroyNotify", "UnmapNotify",
        "MapNotify", "MapRequest", "ReparentNotify",
        "ConfigureNotify", "ConfigureRequest", "GravityNotify",
        "ResizeRequest", "CirculateNotify", "CirculateRequest",
        "PropertyNotify", "SelectionClear", "SelectionRequest",
        "SelectionNotify", "ColormapNotify", "ClientMessage",
        "MappingNotify", "GenericEvent", "LASTEvent"
    };
    return XEventTypeStrings[e.type];
}

// Get the miliseconds since the unix epoch
uint64_t getTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

int X11ErrorHandler(Display* display, XErrorEvent* error) {
    constexpr int kErrorLength = 100;
    char errorString[kErrorLength];
    XGetErrorText(display, error->error_code, errorString, kErrorLength);
    throw std::runtime_error("X11 Error in X11VideoPlayer: " + std::string(errorString));
    return 0;
}

// Helpers ==================================================================================================

X11VideoPlayer::X11VideoPlayer(const std::string video_path) {
    XSetErrorHandler(X11ErrorHandler);
    display_ = XOpenDisplay(NULL);
    const int screen_num = DefaultScreen(display_);
    window_width_ = 800;  // DisplayWidth(display_, screen_num);
    window_height_ = 600;  // DisplayHeight(display_, screen_num);
    Window root_window = XDefaultRootWindow(display_);
    window_ = XCreateSimpleWindow(display_, root_window, 0, 0, window_width_, window_height_, 0, 0, 0);
    visual_ = DefaultVisual(display_, screen_num);
    gc_ = DefaultGC(display_, screen_num);

    // extract raw video properties with ffprobe
    const std::string video_info_command = "ffprobe -v error -select_streams v:0 -count_packets "
        "-show_entries stream=nb_read_packets,r_frame_rate,width,height -of csv=p=0:s=, " + video_path;
    int raw_width, raw_height, frame_rate;
    FILE* video_info_pipe = popen(video_info_command.c_str(), "r");
    fscanf(video_info_pipe, "%d,%d,%d/1,%d\n", &raw_width, &raw_height, &frame_rate, &total_frame_count_);
    fflush(video_info_pipe);
    pclose(video_info_pipe);

    // Compute how long each frame will be shown for
    frame_time_ = 1000 / frame_rate;

    // Compute scaling sizes from window and video sizes
    if (window_width_ - raw_width > window_height_ - raw_height) {
        video_width_ = window_width_;
        double scaling_factor = window_width_ / static_cast<double>(raw_width);
        video_height_ = static_cast<uint32_t>(scaling_factor * raw_height);
    } else {
        double scaling_factor = window_height_ / static_cast<double>(raw_height);
        video_width_ = static_cast<uint32_t>(scaling_factor * raw_width);
        video_height_ = window_height_;
    }

    // Prepare the BGRA frame buffer
    video_frame_buffer_size_ = video_width_ * video_height_ * 4;
    uint32_t depth = DefaultDepth(display_, screen_num);
    video_frame_buffer_ = new char[video_frame_buffer_size_];
    video_frame_ = XCreateImage(display_, visual_, depth, ZPixmap, 0,
                                video_frame_buffer_, video_width_, video_height_, 32, 0);

    // Open the video stream
    video_stream_command_ = "ffmpeg -v error -i " + video_path + " -f image2pipe -vcodec rawvideo"
        " -vf scale=" + std::to_string(video_width_) + ":" + std::to_string(video_height_) +
        " -pix_fmt bgra -";
    video_stream_ = popen(video_stream_command_.c_str(), "r");

    // Finalize X11 setup
    XMapWindow(display_, window_);
    XSelectInput(display_, window_, KeyPressMask | ButtonPressMask | ExposureMask);

    // Set update loop initial values
    frame_counter_ = 0;
    prev_frame_time_ = getTime();
    close_window_ = false;
}

X11VideoPlayer::~X11VideoPlayer() {
    pclose(video_stream_);
    XDestroyImage(video_frame_);
    XCloseDisplay(display_);
}

void X11VideoPlayer::update() {
    uint64_t cur_frame_time = getTime();
    if (cur_frame_time - prev_frame_time_ >= frame_time_) {
        // Close and reopen the pipe to auto-replay the video after it ends
        if (frame_counter_ >= total_frame_count_) {
            pclose(video_stream_);
            video_stream_ = popen(video_stream_command_.c_str(), "r");
            frame_counter_ = 0;
        }
        fread(reinterpret_cast<void*>(video_frame_buffer_), 1, video_frame_buffer_size_, video_stream_);
        XPutImage(display_, window_, gc_, video_frame_, 0, 0, 0, 0, video_width_, video_width_);
        prev_frame_time_ = getTime();
        frame_counter_++;
    }

    while (XPending(display_)) {
        XEvent event;
        XNextEvent(display_, &event);
        if (event.type == 2) {  // close window on keypress
            close_window_ = true;
        }
        std::cout << "XEvent Handled:" + getXEventString(event) << std::endl;
    }
}

const bool& X11VideoPlayer::shouldClose() const {
    return close_window_;
}