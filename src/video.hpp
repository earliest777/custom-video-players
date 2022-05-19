// Copyright 2022 James T Oswald

#ifndef VIDEO_HPP_
#define VIDEO_HPP_

extern "C" {
#include<X11/Xlib.h>
}

#include<string>

class X11VideoPlayer {
 private:
    // X11 overhead members
    Display* display_;
    Window window_;
    uint32_t window_width_, window_height_;
    Visual* visual_;
    GC gc_;

    // Video playing members
    size_t video_frame_buffer_size_;
    char* video_frame_buffer_;                  // Raw binary BGRA video frame image data
    XImage* video_frame_;                       // X11 Wrapper for video_frame_buffer_
    std::string video_stream_command_;          // Command used to start the stream process
    FILE* video_stream_;                        // Pipe to read next frames from
    uint32_t video_width_, video_height_;       // the displayed dims of the video in px

    uint64_t frame_time_;                       // MS to show a frame for
    uint64_t prev_frame_time_;
    int total_frame_count_;                // Number of frames in the video
    uint32_t frame_counter_;

    bool close_window_;

 public:
    explicit X11VideoPlayer(std::string fileName);
    ~X11VideoPlayer();
    void update();
    const bool& shouldClose() const;
};

#endif  // VIDEO_HPP_

