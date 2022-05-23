// Copyright 2022 James T Oswald


extern "C" {
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
}

#include<stdexcept>


int main() {
    av_log_set_callback(AV_LOG_QUIET);
    AVFormatContext* format_context_ = nullptr;
    int error = avformat_open_input(&format_context_, "../assets/sample.mp4", nullptr, nullptr);
    if (error != 0)
        throw std::runtime_error("Failed to open avformat");

    AVStream* video_stream = nullptr;
    for (int i = 0; i < format_context_->nb_streams; i++) {
        video_stream = format_context_->streams[i];
        if (video_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            break;
        }
    }

    AVCodec* decoder_ = avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (!decoder_)
        throw std::runtime_error("Could not find the decoder");
    AVCodecContext* decoder_context_ = avcodec_alloc_context3(decoder_);
    avcodec_parameters_to_context(decoder_context_, video_stream->codecpar);
    if (avcodec_open2(decoder_context_, decoder_, nullptr))
        throw std::runtime_error("Could not open the decoder");


    AVFrame* frame_ = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    while (true) {
        av_read_frame(format_context_, packet);
        avcodec_send_packet(decoder_context_, packet);
        if (avcodec_receive_frame(decoder_context_, frame_) == 0) {
            break;
        }
    }
    av_packet_free(&packet);
    av_frame_free(&frame_);
}
