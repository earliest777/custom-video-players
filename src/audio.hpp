// Copyright 2022 James T Oswald

#ifndef AUDIO_HPP_
#define AUDIO_HPP_

extern "C" {
#include<pulse/pulseaudio.h>
}

#include<string>
#include<unordered_map>

// Called on updates to the Pulse audio server connection
void paContextSetStateCallback(pa_context *context, void *userdata);

// Called by the audio stream to request more audio data for its internal buffer
void paStreamSetWriteCallback(pa_stream *p, size_t nbytes, void* userdata);

class PulseAudioPlayer{
    friend void paContextSetStateCallback(pa_context *context, void *userdata);
    friend void paStreamSetWriteCallback(pa_stream *p, size_t nbytes, void* userdata);
 private:
    // Our internal logic
    const unsigned int numChannels, sampleRate;
    const std::string fileName, format, audioStreamCmd;

    pa_mainloop* mainloop;
    pa_mainloop_api* mainloopapi;
    pa_context* context;       // The connection to the pulseAudio Server
    FILE* inputStream;         // The location to read the audio stream from
    pa_stream* outputStream;   // The output audio stream object
 public:
    PulseAudioPlayer();
    explicit PulseAudioPlayer(const std::string&& filename);
    PulseAudioPlayer(const std::string&& filename, const std::string&& format,
                     uint8_t channels, uint32_t sampleRate);
    ~PulseAudioPlayer();
    void update();
};

#endif  // AUDIO_HPP_
