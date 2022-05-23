// Copyright 2022 James T Oswald

#include"openGLVideoPlayer.hpp"

extern "C" {
#include<sys/time.h>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
}

#include<string>
#include<stdexcept>

// Helpers ==================================================================================================

// Get the miliseconds since the unix epoch
uint64_t getTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

// Load an open gl shader
uint32_t loadShader(char* filename, uint shaderType) {
    FILE* file;
    file = fopen(filename, "r");
    if (!file) {
        printf("Could not open %s\n", filename);
        perror("Error:");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* glShaderCont = reinterpret_cast<char*>(malloc(fsize + 1));
    fread(glShaderCont, 1, fsize, file);
    glShaderCont[fsize] = 0;
    fclose(file);

    uint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (char const* const*)&glShaderCont, NULL);
    glCompileShader(shader);
    free(glShaderCont);
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("%s failed to compile with error:\n%s", filename, infoLog);
        exit(1);
    }
    return shader;
}

// Load an open gl program
uint32_t loadProgram(uint vertexShader, uint fragmentShader) {
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        printf("failed to link program with error:\n%s", infoLog);
        exit(1);
    }
    return shaderProgram;
}


// Callbacks ================================================================================================

void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// Methods ==================================================================================================

OpenGLVideoPlayer::OpenGLVideoPlayer(std::string video_path) {
    // Window Setup
    if (glfwInit() == GLFW_FALSE) {
        const char* errorMessage;
        glfwGetError(&errorMessage);
        throw std::runtime_error("Could not init glfw: " + std::string(errorMessage));
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    window_ = glfwCreateWindow(mode->width, mode->height, "OpenGLVideoPlayer", monitor, NULL);
    if (window_ == nullptr) {
        const char* errorMessage;
        glfwGetError(&errorMessage);
        throw std::runtime_error("Could not create window: " + std::string(errorMessage));
    }
    glfwMakeContextCurrent(window_);
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetKeyCallback(window_, onKeyPress);

    int* width_ptr = reinterpret_cast<int*>(&window_width_);
    int* height_ptr = reinterpret_cast<int*>(&window_height_);
    glfwGetWindowSize(window_, width_ptr, height_ptr);



    // open the file file for information extraction
    if (!avformat_open_input(&format_context_, video_path.c_str(), nullptr, nullptr))
        throw std::runtime_error("Failed to open avformat");

    // look for the video stream and extrat info about it
    bool found_video_stream;
    AVStream* video_stream;
    for (int i = 0; i < format_context_->nb_streams; i++) {
        AVStream* video_stream = format_context_->streams[i];
        if (video_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            found_video_stream = true;
            break;
        }
    }
    if (!found_video_stream)
        throw std::runtime_error("File did not contain a video");

    // Initilize all our members
    video_width_ = video_stream->codec->width;
    video_height_ = video_stream->codec->height;
    frame_time_ = 1000 / video_stream->time_base.den;  // how long the frame will be on screen
    total_frame_count_ = video_stream->nb_frames;
    frame_counter_ = 0;
    prev_frame_time_ = getTime();

    // Set up the decoder
    decoder_ = avcodec_find_decoder(video_stream->codec->codec_id);
    if (!decoder_)
        throw std::runtime_error("Could not find the decoder");
    decoder_context_ = avcodec_alloc_context3(decoder_);
    if (avcodec_open2(decoder_context_, decoder_, nullptr))
        throw std::runtime_error("Could not open the decoder");

    // OpenGL setup
    GLenum glewInitError = glewInit();
    if (glewInitError != GLEW_OK) {
        const char* errorCString = reinterpret_cast<const char*>(glewGetErrorString(glewInitError));
        std::string errorString = std::string(errorCString);
        throw std::runtime_error("Could not create window: " + errorString);
    }


    /*uint32_t vertexShader = loadShader("./vert.glsl", GL_VERTEX_SHADER);
    uint32_t fragmentShader = loadShader("./frag.glsl", GL_FRAGMENT_SHADER);
    shader_program_ = loadProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        1,  1,  

    };

    uint32_t indices[] = {

    };

    glGenBuffers(1, &vbo_);
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    */

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
}

OpenGLVideoPlayer::~OpenGLVideoPlayer() {
    glfwTerminate();
    avformat_close_input(&format_context_);
}

void OpenGLVideoPlayer::update() {
    uint64_t cur_frame_time = getTime();
    if (cur_frame_time - prev_frame_time_ >= frame_time_) {
        // Close and reopen the pipe to auto-replay the video after it ends
        if (frame_counter_ >= total_frame_count_) {
            // Insert logic here to restart the av stream

            frame_counter_ = 0;
        }
        AVPacket* packet;
        int got_frame;
        av_read_frame(format_context_, packet);
        avcodec_decode_video2(decoder_context_, &frame_, &got_frame, packet);
        av_packet_unref(packet);
        av_frame_unref(&frame_);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width_, window_height_, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, frame_.data);
        prev_frame_time_ = getTime();
        frame_counter_++;
        glfwSwapBuffers(window_);  // redraw the frame
    }
    glfwPollEvents();
}

bool OpenGLVideoPlayer::shouldClose() {
    return glfwWindowShouldClose(window_);
}
