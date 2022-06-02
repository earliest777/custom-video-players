// Copyright 2022 James T Oswald


extern "C" {
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/imgutils.h>
#include<libswscale/swscale.h>
}

#include<stdexcept>

void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
    if (glfwInit() == GLFW_FALSE) {
        const char* errorMessage;
        glfwGetError(&errorMessage);
        throw std::runtime_error("Could not init glfw: " + std::string(errorMessage));
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    // const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window_ = glfwCreateWindow(800 , 600, "OpenGLVideoPlayer", NULL, NULL);
    if (window_ == nullptr) {
        const char* errorMessage;
        glfwGetError(&errorMessage);
        throw std::runtime_error("Could not create window: " + std::string(errorMessage));
    }
    glfwMakeContextCurrent(window_);
    // glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetKeyCallback(window_, onKeyPress);

    uint32_t window_width_, window_height_;
    int* width_ptr = reinterpret_cast<int*>(&window_width_);
    int* height_ptr = reinterpret_cast<int*>(&window_height_);
    glfwGetWindowSize(window_, width_ptr, height_ptr);

    GLenum glewInitError = glewInit();
    if (glewInitError != GLEW_OK) {
        const char* errorCString = reinterpret_cast<const char*>(glewGetErrorString(glewInitError));
        std::string errorString = std::string(errorCString);
        throw std::runtime_error("Could not create window: " + errorString);
    }

    av_log_set_level(AV_LOG_QUIET);

    // Set up the Format Context
    AVFormatContext* format_context_ = nullptr;
    int error = avformat_open_input(&format_context_, "../assets/sample-5s.mp4", nullptr, nullptr);
    if (error != 0)
        throw std::runtime_error("Failed to open avformat");

    // Pull video info
    AVStream* video_stream = nullptr;
    for (int i = 0; i < format_context_->nb_streams; i++) {
        video_stream = format_context_->streams[i];
        if (video_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            break;
        }
    }

    AVCodec* decoder_ = avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (!decoder_)
        throw std::runtime_error("Could not find the decoder");
    AVCodecContext* decoder_context_ = avcodec_alloc_context3(decoder_);
    avcodec_parameters_to_context(decoder_context_, video_stream->codecpar);
    if (avcodec_open2(decoder_context_, decoder_, nullptr))
        throw std::runtime_error("Could not open the decoder");

    // Setup conversion context
    SwsContext* conversion_context = sws_getContext(
        video_stream->codecpar->width, video_stream->codecpar->height,
        AV_PIX_FMT_YUV420P, window_width_, window_height_, AV_PIX_FMT_RGB24,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);


    glClearColor(0.3f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    AVFrame* frame_ = av_frame_alloc();
    AVFrame* frame2_ = av_frame_alloc();

    frame2_->width = window_width_;
    frame2_->height = window_height_;
    frame2_->format = AV_PIX_FMT_RGB24;
    int bufSize  = av_image_get_buffer_size(AV_PIX_FMT_RGB24, frame2_->width, frame2_->height, 1);
    uint8_t* buf = reinterpret_cast<uint8_t*>(av_malloc(bufSize));
    av_image_fill_arrays(frame2_->data, frame2_->linesize, buf,
        static_cast<AVPixelFormat>(frame2_->format), frame2_->width, frame2_->height, 1);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    /*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    */

    // Forced to bind a frame buffer for the fbo
    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);

    AVPacket* packet = av_packet_alloc();
    while (!glfwWindowShouldClose(window_)) {
        av_read_frame(format_context_, packet);
        if (packet->buf == NULL) {
            break;
            // Restart the video if its over
            // av_seek_frame(format_context_, -1, 0, 0);
            // av_read_frame(format_context_, packet);
        }
        avcodec_send_packet(decoder_context_, packet);
        if (avcodec_receive_frame(decoder_context_, frame_) == 0) {
            // Convert the YUV420P frame out of the mp4 into a raw RGB24 frame

            // fix image flipping from the ffmpeg mailing list
            // https://lists.ffmpeg.org/pipermail/ffmpeg-user/2011-May/000976.html
            // I invented the rest of this code to deal with flipping the U and V planes respecitively,
            // which are half the size of the main plane
            frame_->data[0] += frame_->linesize[0]*(frame_->height - 1);
            frame_->data[1] += frame_->linesize[1]*(frame_->height / 2 - 1);
            frame_->data[2] += frame_->linesize[2]*(frame_->height / 2 - 1);
            frame_->linesize[0] = -frame_->linesize[0];
            frame_->linesize[1] = -frame_->linesize[1];
            frame_->linesize[2] = -frame_->linesize[2];

            sws_scale(conversion_context, frame_->data, frame_->linesize, 0,
                frame_->height, frame2_->data, frame2_->linesize);

            // Load the RGB24 frame as an OpenGL texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width_, window_height_, 0,
                GL_RGB, GL_UNSIGNED_BYTE, frame2_->data[0]);

            // Convert the texture to a frame buffer?
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

            glBlitFramebuffer(0, 0, window_width_, window_height_, 0, 0, window_width_, window_height_,
                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glfwSwapBuffers(window_);
        }
    }
    glfwDestroyWindow(window_);
    av_packet_free(&packet);
    av_frame_free(&frame_);
}
