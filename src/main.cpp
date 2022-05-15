// Copyright 2022 James T Oswald

#include"video.hpp"
#include"audio.hpp"

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

    X11VideoPlayer videoPlayer(video_path);
    PulseAudioPlayer audioPlayer(video_path);

    while (true) {
        videoPlayer.update();
        //audioPlayer.update();
    }
}
