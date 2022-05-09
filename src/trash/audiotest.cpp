
#include<string>

extern "C"{
#include<stdio.h>
#include<stdint.h>

#include <pulse/pulseaudio.h>
}

/*
#define sampleSize 2*44100
int16_t sample1[sampleSize] = {0};
int16_t sample2[sampleSize] = {0};
int16_t* s1 = sample1, *s2 = sample2;
bool grabNext = true;

void* LoadAudio(void* _){
    std::string audioStreamCmd = "ffmpeg -v error -i ../assets/mutation.mp3 -ar 44100 -ac 2 -f s16le -";
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
*/


void streamWriteCallback(pa_stream *p, size_t nbytes, void* userdata){
    FILE* audioInputStream = (FILE*)userdata;
    int16_t* data;
    pa_stream_begin_write(p, (void**)&data, &nbytes);
    fread(data, 1, nbytes, audioInputStream);
    pa_stream_write(p, (void*)data, nbytes, NULL, 0, PA_SEEK_RELATIVE);
}

void contextStateCallback(pa_context *context, void *userdata){
    pa_context_state_t state = pa_context_get_state(context);
    switch(state){
        case PA_CONTEXT_READY:{
            pa_sample_spec ss;
            ss.format = PA_SAMPLE_S16NE;
            ss.channels = 2;
            ss.rate = 44100;
            pa_stream* stream = pa_stream_new(context, "Meme", &ss, NULL);
            std::string audioStreamCmd = "ffmpeg -v error -i ../../assets/mutation.mp3 -ar 44100 -ac 2 -f s16le -";
            FILE* audioStream = popen(audioStreamCmd.c_str(), "r");
            pa_stream_set_write_callback(stream, &streamWriteCallback, (void*)audioStream);
            int error = pa_stream_connect_playback(stream, NULL, NULL, PA_STREAM_NOFLAGS, NULL, NULL);
            printf("Error: %s\n", pa_strerror(error));
            break;
        }
        default:
            break;
    }
}

int main(){
    int error;
    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloopapi = pa_mainloop_get_api(mainloop);
    pa_context* ctx = pa_context_new(mainloopapi, "PulseAudio Test");
    error = pa_context_connect(ctx, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
    printf("Error: %s\n", pa_strerror(error));
    pa_context_set_state_callback(ctx, &contextStateCallback, NULL);
    
    while(true){
        pa_mainloop_iterate(mainloop, 1, NULL);
    }
    return 0;
}