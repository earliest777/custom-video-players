// Copyright 2022 James T Oswald

#ifndef OPENGLVIDEOPLAYER_HPP_
#define OPENGLVIDEOPLAYER_HPP_

extern "C" {
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
}

#include<string>

class OpenGLVideoPlayer {
 private:
    GLFWwindow* window_;
    uint32_t window_width_, window_height_;
    // uint32_t shader_program_, vbo_, vao_;
    uint32_t texture_;

    AVCodec* decoder_;
    AVCodecContext* decoder_context_;
    AVFormatContext* format_context_;
    AVPacket packet_;
    AVFrame frame_;
    uint32_t video_width_, video_height_;       // the displayed dims of the video in px


    uint64_t frame_time_;                       // MS to show a frame for
    uint64_t prev_frame_time_;
    uint64_t total_frame_count_;                // Number of frames in the video
    uint32_t frame_counter_;
 public:
    explicit OpenGLVideoPlayer(std::string videoPath);
    void update();
    bool shouldClose();
};

#endif  // OPENGLVIDEOPLAYER_HPP_
