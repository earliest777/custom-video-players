// Copyright 2022 James T Oswald


extern "C" {
#include<stdio.h>
#include<stdint.h>
#include<pulse/pulseaudio.h>
}

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
           " -ac " + std::to_string(numChannels) + " -f s16le -";
}

// Callbacks ================================================================================================

void paStreamSetWriteCallback(pa_stream *p, size_t nbytes, void* userdata) {
    PulseAudioPlayer* player = static_cast<PulseAudioPlayer*>(userdata);
    int16_t* data;  // Internal pulse audio buffer to write to
    pa_stream_begin_write(p, reinterpret_cast<void**>(&data), &nbytes);
    fread(data, 1, nbytes, player->inputStream);
    if (feof(player->inputStream)) {  // if the audio is over, reset it to the start
        pclose(player->inputStream);
        player->inputStream = popen(player->audioStreamCmd.c_str(), "r");
    }
    pa_stream_write(p, static_cast<void*>(data), nbytes, NULL, 0, PA_SEEK_RELATIVE);
}

void paContextSetStateCallback(pa_context *context, void *userdata) {
    PulseAudioPlayer* player = static_cast<PulseAudioPlayer*>(userdata);
    pa_context_state_t state = pa_context_get_state(context);
    switch (state) {
        case PA_CONTEXT_READY: {
            pa_sample_spec ss;
            ss.format = getPaFormat(player->format);
            ss.channels = player->numChannels;
            ss.rate = player->sampleRate;
            player->outputStream = pa_stream_new(context, "Meme", &ss, NULL);
            player->inputStream = popen(player->audioStreamCmd.c_str(), "r");
            pa_stream_set_write_callback(player->outputStream, &paStreamSetWriteCallback, userdata);
            int error = pa_stream_connect_playback(player->outputStream, NULL, NULL,
                                                   PA_STREAM_NOFLAGS, NULL, NULL);
            printf("Error: %s\n", pa_strerror(error));
            break;
        }
        default:
            break;
    }
}

// Methods ==================================================================================================


PulseAudioPlayer::PulseAudioPlayer(const std::string&& filename, const std::string&& format,
                                   uint8_t channels, uint32_t sampleRate)
:fileName(filename), format(format), numChannels(numChannels), sampleRate(sampleRate),
audioStreamCmd(getAudioStreamCmd(filename, format, numChannels, sampleRate)) {
    mainloop = pa_mainloop_new();
    mainloopapi = pa_mainloop_get_api(mainloop);
    const std::string connectionName = filename + "PulseAudioPlayer";
    context = pa_context_new(mainloopapi, connectionName.c_str());
    int error = pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
    if (error != pa_error_code::PA_OK)
        throw std::runtime_error("PulseAudioPlayer Error: could not connect to the Pulse Audio Server");
    pa_context_set_state_callback(context, &paContextSetStateCallback, this);
}

PulseAudioPlayer::~PulseAudioPlayer() {
    pclose(inputStream);
    pa_stream_disconnect(outputStream);
    pa_mainloop_free(mainloop);
    // Do not need to free mainloopapi, see
    // https://freedesktop.org/software/pulseaudio/doxygen/mainloop_8h.html#a9e5b510dabb4eb1a01645c4db65b9ddb
}


void PulseAudioPlayer::update() {
    pa_mainloop_iterate(mainloop, 1, NULL);
}


