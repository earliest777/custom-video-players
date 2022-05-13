

#include<string>
#include<unordered_map>

extern "C"{
#include<pulse/pulseaudio.h>
}

class PulseAudioPlayer{
    private:
        //Maps valid ffmpeg audio formats to pulse audio format enums
        using FormatLookupTable = std::unordered_map<std::string, pa_sample_format_t>;
        const static FormatLookupTable formatMap;

        const unsigned int channels, sampleRate;
        const std::string fileName, audioStreamCmd;
        pa_mainloop* mainloop;
        pa_mainloop_api* mainloopapi;
        pa_context* context;
    public:
        PulseAudioPlayer();
        PulseAudioPlayer(std::string filename, std::string format, uint8_t channels = 2, uint32_t sampleRate = 44100);
        ~PulseAudioPlayer();
        void update();
};