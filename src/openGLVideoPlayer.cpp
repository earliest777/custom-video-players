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

// Callbacks ==============================================================================================

void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// Private Helper Methods =================================================================================
void OpenGLVideoPlayer::glfwSetup() {
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
    window_ = glfwCreateWindow(800 , 600, "OpenGLVideoPlayer", NULL, NULL);
    if (window_ == nullptr) {
        const char* errorMessage;
        glfwGetError(&errorMessage);
        throw std::runtime_error("Could not create window: " + std::string(errorMessage));
    }
    glfwMakeContextCurrent(window_);
    // glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetKeyCallback(window_, onKeyPress);

    // Set the window width and height
    int* width_ptr = reinterpret_cast<int*>(&window_width_);
    int* height_ptr = reinterpret_cast<int*>(&window_height_);
    glfwGetWindowSize(window_, width_ptr, height_ptr);
}

void OpenGLVideoPlayer::libavSetup() {
    av_log_set_level(AV_LOG_QUIET);

    // Set up the Format Context
    format_context_ = nullptr;
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

    decoder_ = avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (!decoder_)
        throw std::runtime_error("Could not find the decoder");
    AVCodecContext* decoder_context_ = avcodec_alloc_context3(decoder_);
    avcodec_parameters_to_context(decoder_context_, video_stream->codecpar);
    if (avcodec_open2(decoder_context_, decoder_, nullptr))
        throw std::runtime_error("Could not open the decoder");

    packet_ = av_packet_alloc();  // A packet
    frame_ = av_frame_alloc();  // The completed frame
}

extern const char* vertexShader;
extern const char* fragmentShader;

void OpenGLVideoPlayer::openglSetup() {
    GLenum glewInitError = glewInit();
    if (glewInitError != GLEW_OK) {
        const char* errorCString = reinterpret_cast<const char*>(glewGetErrorString(glewInitError));
        std::string errorString = std::string(errorCString);
        throw std::runtime_error("Could not create window: " + errorString);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Compile the shader program and set the texture location
    uint32_t vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (char const* const*)&vertexShader, NULL);
    glCompileShader(vertShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        printf("vert failed to compile with error:\n%s", infoLog);
        exit(1);
    }
    uint32_t fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (char const* const*)&fragmentShader, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        printf("frag failed to compile with error:\n%s", infoLog);
        exit(1);
    }
    uint32_t shader_program = glCreateProgram();
    glAttachShader(shader_program, vertShader);
    glAttachShader(shader_program, fragShader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        printf("failed to link program with error:\n%s", infoLog);
        exit(1);
    }

    glUseProgram(shader_program);
    GLint YLocation = glGetUniformLocation(shader_program, "Y");
    glUniform1i(YLocation, 0);
    GLint ULocation = glGetUniformLocation(shader_program, "U");
    glUniform1i(ULocation, 1);
    GLint VLocation = glGetUniformLocation(shader_program, "V");
    glUniform1i(VLocation, 2);

    // You have to bind a VAO to use glDrawArrays or everything breaks
    // Even if the VAO is just empty
    uint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /*
        Textures are stored in the same order as YUV planes are in the frame
        textures[0] is the Y plane, textures[1] is the U plane and textures[2] is the V plane
    */
    unsigned int textures[3];
    glGenTextures(3, textures);
}

// Public Methods ===========================================================================================

OpenGLVideoPlayer::OpenGLVideoPlayer(std::string video_path) {
    glfwSetup();
    libavSetup();
    openglSetup();
}

OpenGLVideoPlayer::~OpenGLVideoPlayer() {
    glfwTerminate();
    avformat_close_input(&format_context_);
}

void OpenGLVideoPlayer::update() {
    av_read_frame(format_context_, packet_);
    if (packet_->buf == NULL) {
        // break;
        // Restart the video if its over
        av_seek_frame(format_context_, -1, 0, 0);
        av_read_frame(format_context_, packet_);
    }
    avcodec_send_packet(decoder_context_, packet_);
    if (avcodec_receive_frame(decoder_context_, frame_) == 0) {
        // Bind and write the Y component
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, textures_[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame_->width, frame_->height, 0,
            GL_RED, GL_UNSIGNED_BYTE, frame_->data[0]);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Bind and write the U component
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, textures_[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame_->width/2, frame_->height/2, 0,
            GL_RED, GL_UNSIGNED_BYTE, frame_->data[1]);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Bind and write the V component
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, textures_[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame_->width/2, frame_->height/2, 0,
            GL_RED, GL_UNSIGNED_BYTE, frame_->data[2]);
        glGenerateMipmap(GL_TEXTURE_2D);


        // Convert the texture to a frame buffer?
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

bool OpenGLVideoPlayer::shouldClose() {
    return glfwWindowShouldClose(window_);
}
