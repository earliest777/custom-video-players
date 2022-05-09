
#include<string>

extern "C"{
#include<stdio.h>
#include<stdint.h>

#include <pthread.h>
#include <pulse/simple.h>
#include <pulse/error.h>
}

#define sampleSize 2*44100
int16_t sample1[sampleSize] = {0};
int16_t sample2[sampleSize] = {0};
int16_t* s1 = sample1, *s2 = sample2;
bool grabNext = true;

void* LoadAudio(void* _){
    std::string audioStreamCmd = "ffmpeg -v error -i ../../assets/mutation.mp3 -ar 44100 -ac 2 -f s16le -";
    FILE* f = popen(audioStreamCmd.c_str(), "r");
    while(true){
        if(grabNext){
            grabNext = false;
            fread(s1, 2, sampleSize, f);
        }
    }
    return NULL;
}

void* playAudio(void* _){
    pa_buffer_attr ba;
    ba.maxlength = -1;
    ba.minreq = -1;
    ba.prebuf = -1;
    ba.fragsize = -1;
    ba.tlength = -1;
    pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16NE;
    ss.channels = 2;
    ss.rate = 44100;
    int error = 0;
    pa_simple* s = pa_simple_new(NULL, "AudioTest", PA_STREAM_PLAYBACK, NULL, "AudioTest", &ss, NULL, &ba, &error);
    printf("Error: %s\n", pa_strerror(error));
    while(true){
        pa_simple_write(s, s2, sampleSize, &error);
        pa_simple_drain(s, &error);
        int16_t* temp = s1;
        s1 = s2;
        s2 = temp;
        grabNext = true;
    }
    pa_simple_free(s);
    return NULL;
}

int main(){
    pthread_t audioThread, loaderThread;
    pthread_create(&loaderThread, NULL, &LoadAudio, NULL);
    pthread_create(&audioThread, NULL, &playAudio, NULL);
    pthread_join(audioThread, NULL);
    pthread_join(loaderThread, NULL);
    return 0;
}