// Copyright 2022 James T Oswald


extern "C" {
#include<pulse/pulseaudio.h>
#include<stdio.h>
#include<stdint.h>
}

#include<tuple>
#include<string>
#include<stdexcept>

#include"audio.hpp"

// Helper Functions =========================================================================================

// Map legal ffmpeg format strings to pulse audio format enums
pa_sample_format_t getPaFormat(std::string ffmpegFormat) {
    static const std::unordered_map<std::string, pa_sample_format_t> formatMap = {
        {"s16le", PA_SAMPLE_S16LE},
        {"s24le", PA_SAMPLE_S24LE},
        {"s32le", PA_SAMPLE_S32LE},
    };
    if (formatMap.find(ffmpegFormat) == formatMap.end()) {
        std::string legalFormats = "";
        for (const auto &[format, _] : formatMap)
            legalFormats += format + "\n";
        throw std::runtime_error("PulseAudioPlayer Error: " + ffmpegFormat +
                                 "is not a valid legal format.\nLegal Formats:" + legalFormats);
    }
    return formatMap.at(ffmpegFormat);
}

std::string getAudioStreamCmd(const std::string& filename, const std::string& format,
                              uint8_t numChannels, uint32_t sampleRate) {
    return "ffmpeg -v error -i " + filename + " -ar " + std::to_string(sampleRate) +
           " -ac " + std::to_string(numChannels) + " -f " + format + " -";
}

// Obtain default stream info with respect to the file we're opening by ffprobing it.
// Still only support s16le as the default audio format
PulseAudioPlayer::StreamInfo defaultStreamInfo(const std::string& filename) {
    // pull defaults from the file's own internal format
    const std::string audioInfoCmd = "ffprobe -v error -show_entries stream=sample_rate,channels"
                                     " -of csv=p=0:s=, " + filename;
    FILE* audio_info_pipe = popen(audioInfoCmd.c_str(), "r");
    uint32_t sample_rate;
    uint8_t num_channels;
    fscanf(audio_info_pipe, "%u,%hhu\n", &sample_rate, &num_channels);
    fflush(audio_info_pipe);
    pclose(audio_info_pipe);
    return std::tie("s16le", num_channels, sample_rate);
}

// Callbacks ================================================================================================

void paStreamSetWriteCallback(pa_stream *p, size_t nbytes, void* userdata) {
    PulseAudioPlayer* player = static_cast<PulseAudioPlayer*>(userdata);
    int16_t* data;  // Internal pulse audio buffer to write to
    pa_stream_begin_write(p, reinterpret_cast<void**>(&data), &nbytes);
    fread(data, 1, nbytes, player->inputStream_);
    if (feof(player->inputStream_)) {  // if the audio is over, reset it to the start
        pclose(player->inputStream_);
        player->inputStream_ = popen(player->audioStreamCmd_.c_str(), "r");
    }
    pa_stream_write(p, static_cast<void*>(data), nbytes, NULL, 0, PA_SEEK_RELATIVE);
}

void paContextSetStateCallback(pa_context *context, void *userdata) {
    PulseAudioPlayer* player = static_cast<PulseAudioPlayer*>(userdata);
    pa_context_state_t state = pa_context_get_state(context);
    switch (state) {
        case PA_CONTEXT_READY: {
            pa_sample_spec ss;
            ss.format = getPaFormat(player->format_);
            ss.channels = player->numChannels_;
            ss.rate = player->sampleRate_;
            player->outputStream_ = pa_stream_new(context, "Meme", &ss, NULL);
            player->inputStream_ = popen(player->audioStreamCmd_.c_str(), "r");
            pa_stream_set_write_callback(player->outputStream_, &paStreamSetWriteCallback, userdata);
            int error = pa_stream_connect_playback(player->outputStream_, NULL, NULL,
                                                   PA_STREAM_NOFLAGS, NULL, NULL);
            printf("Error: %s\n", pa_strerror(error));
            if (error != pa_error_code::PA_OK)
                throw std::runtime_error("PulseAudioPlayer Error:"
                                         "could not connect to the Pulse Audio Server");
            break;
        }
        default:
            break;
    }
}

// Methods ==================================================================================================



PulseAudioPlayer::PulseAudioPlayer(std::string filename, std::string format,
                                   uint8_t channels, uint32_t sampleRate)
:fileName_(filename), format_(format), numChannels_(channels), sampleRate_(sampleRate),
audioStreamCmd_(getAudioStreamCmd(filename, format, channels, sampleRate)) {
    mainloop_ = pa_mainloop_new();
    mainloopapi_ = pa_mainloop_get_api(mainloop_);
    const std::string connectionName = filename + "PulseAudioPlayer";
    context_ = pa_context_new(mainloopapi_, connectionName.c_str());
    inputStream_ = nullptr;
    outputStream_ = nullptr;
    int error = pa_context_connect(context_, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
    if (error != pa_error_code::PA_OK)
        throw std::runtime_error("PulseAudioPlayer Error: could not connect to the Pulse Audio Server");
    pa_context_set_state_callback(context_, &paContextSetStateCallback, this);
}

PulseAudioPlayer::PulseAudioPlayer(std::string filename, const StreamInfo& streamInfo)
:PulseAudioPlayer(filename, std::get<0>(streamInfo), std::get<1>(streamInfo), std::get<2>(streamInfo)) {}

PulseAudioPlayer::PulseAudioPlayer(std::string filename)
:PulseAudioPlayer(filename, defaultStreamInfo(filename)) {}

PulseAudioPlayer::~PulseAudioPlayer() {
    if (inputStream_)
        pclose(inputStream_);
    if (outputStream_)
        pa_stream_disconnect(outputStream_);
    pa_mainloop_free(mainloop_);
    // Do not need to free mainloopapi, see
    // https://freedesktop.org/software/pulseaudio/doxygen/mainloop_8h.html#a9e5b510dabb4eb1a01645c4db65b9ddb
}


void PulseAudioPlayer::update() {
    pa_mainloop_iterate(mainloop_, 1, NULL);
}


