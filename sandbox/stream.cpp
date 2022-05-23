// Copyright 2022 James T Oswald

extern "C" {
#include<stdio.h>
#include<stdint.h>

#include <pulse/pulseaudio.h>
}

#include<string>

const std::string audioStreamCmd = "ffmpeg -v error -i ../assets/sound.mp3 -ar 44100 -ac 2 -f s16le -";
FILE* audioInputStream;

void streamWriteCallback(pa_stream *p, size_t nbytes, void* userdata) {
    int16_t* data;
    pa_stream_begin_write(p, (void**)&data, &nbytes);
    fread(data, 1, nbytes, audioInputStream);
    if(feof(audioInputStream)) { //Looping
        pclose(audioInputStream);
        audioInputStream = popen(audioStreamCmd.c_str(), "r");
    }
    pa_stream_write(p, (void*)data, nbytes, NULL, 0, PA_SEEK_RELATIVE);
}

void contextStateCallback(pa_context *context, void *userdata) {
    pa_context_state_t state = pa_context_get_state(context);
    switch (state) {
        case PA_CONTEXT_READY: {
            pa_sample_spec ss;
            ss.format = PA_SAMPLE_S16NE;
            ss.channels = 2;
            ss.rate = 44100;
            pa_stream* stream = pa_stream_new(context, "Meme", &ss, NULL);
            audioInputStream = popen(audioStreamCmd.c_str(), "r");
            pa_stream_set_write_callback(stream, &streamWriteCallback, NULL);
            int error = pa_stream_connect_playback(stream, NULL, NULL, PA_STREAM_NOFLAGS, NULL, NULL);
            printf("Error: %s\n", pa_strerror(error));
            break;
        }
        default:
            break;
    }
}

int main() {
    int error;
    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloopapi = pa_mainloop_get_api(mainloop);
    pa_context* ctx = pa_context_new(mainloopapi, "PulseAudio Test");
    error = pa_context_connect(ctx, "tcp:192.168.128.1", PA_CONTEXT_NOAUTOSPAWN, NULL);
    printf("Error: %s\n", pa_strerror(error));
    pa_context_set_state_callback(ctx, &contextStateCallback, NULL);

    while (true) {
        pa_mainloop_iterate(mainloop, 1, NULL);
    }
    return 0;
}