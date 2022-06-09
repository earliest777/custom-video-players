// Copyright 2022 James T Oswald

#include"OpenGLVideoPlayer.hpp"
#include"pulseAudioPlayer.hpp"

int main(int argc, char** argv) {
    std::string video_path;
    if (argc <= 1) {
        video_path = "assets/sample.mp4";
    } else if (argc == 2) {
        video_path = argv[1];
    } else {
        printf("usage: Overhead.exe [MP4 FilePath]\n");
        exit(0);
    }

    OpenGLVideoPlayer videoPlayer(video_path);
    PulseAudioPlayer audioPlayer(video_path);

    while (!videoPlayer.shouldClose()) {
        videoPlayer.update();
        audioPlayer.update();
    }
}
