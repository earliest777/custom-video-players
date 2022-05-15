// Copyright 2022 James T Oswald

#ifndef AUDIO_HPP_
#define AUDIO_HPP_

extern "C" {
#include<pulse/pulseaudio.h>
}

#include<tuple>
#include<string>
#include<unordered_map>

// Called on updates to the Pulse audio server connection
// void paContextSetStateCallback(pa_context *context, void *userdata);

// Called by the audio stream to request more audio data for its internal buffer
// void paStreamSetWriteCallback(pa_stream *p, size_t nbytes, void* userdata);

class PulseAudioPlayer{
 public:
    using StreamInfo = std::tuple<std::string, uint8_t, uint32_t>;
    friend void paContextSetStateCallback(pa_context *context, void *userdata);
    friend void paStreamSetWriteCallback(pa_stream *p, size_t nbytes, void* userdata);
 private:
    // Our internal stream properties
    const unsigned int numChannels_, sampleRate_;
    const std::string fileName_, format_, audioStreamCmd_;

    pa_mainloop* mainloop_;
    pa_mainloop_api* mainloopapi_;
    pa_context* context_;       // The connection to the pulseAudio Server
    FILE* inputStream_;         // The location to read the audio stream from
    pa_stream* outputStream_;   // The output audio stream object

 public:
    PulseAudioPlayer() = delete;
    explicit PulseAudioPlayer(std::string filename);
    PulseAudioPlayer(std::string filename, const StreamInfo& streamInfo);
    PulseAudioPlayer(std::string filename, std::string format,
                     uint8_t channels, uint32_t sampleRate);
    ~PulseAudioPlayer();
    void update();
};

#endif  // AUDIO_HPP_
