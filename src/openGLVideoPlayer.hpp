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
    // Glfw members for managing the window
    GLFWwindow* window_;
    uint32_t window_width_, window_height_;
    void glfwSetup();

    // Libav member for managing the video decoding
    AVFormatContext* format_context_;
    AVCodecContext* decoder_context_;
    AVCodec* decoder_;
    AVPacket* packet_;
    AVFrame* frame_;
    void libavSetup();

    // OpenGL member for managing the video playback
    GLuint shader_program_, vao_;               // Open GL handles
    GLuint textures_[3];                        // The YUV planes as 3 sepreate textures
    void openglSetup();

    // Playback rate control members
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
