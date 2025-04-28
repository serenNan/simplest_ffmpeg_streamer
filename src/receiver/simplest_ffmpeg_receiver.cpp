/**
 * 最简单的基于FFmpeg的收流器（接收RTMP）
 * Simplest FFmpeg Receiver (Receive RTMP)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本例子将流媒体数据（以RTMP为例）保存成本地文件。
 * 是使用FFmpeg进行流媒体接收最简单的教程。
 *
 * This example saves streaming media data (Use RTMP as example)
 * as a local file.
 * It's the simplest FFmpeg stream receiver.
 *
 * 修改版：支持命令行参数，适应现代FFmpeg API
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
// Windows
extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
};
#else
// Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#ifdef __cplusplus
};
#endif
#endif

//'1': Use H.264 Bitstream Filter
#define USE_H264BSF 0

void show_usage(char *program_name)
{
    printf("Usage: %s [input_url] [output_file]\n", program_name);
    printf("Example:\n");
    printf("  %s rtmp://live.hkstv.hk.lxdns.com/live/hks output.flv\n", program_name);
    printf("  %s rtmp://localhost/publishlive/livestream output.mp4\n", program_name);
}

int main(int argc, char *argv[])
{
    const AVOutputFormat *ofmt = NULL;
    // Input AVFormatContext and Output AVFormatContext
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int videoindex = -1;
    int frame_index = 0;

    // 处理命令行参数
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))
    {
        show_usage(argv[0]);
        return 0;
    }
    else if (argc < 3)
    {
        const char *default_in_url = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
        const char *default_out_file = "receive.flv";

        printf("No input URL or output file specified, using defaults:\n");
        printf("Input URL: %s\n", default_in_url);
        printf("Output file: %s\n", default_out_file);
        printf("Use --help to see usage information\n\n");

        in_filename = default_in_url;
        out_filename = default_out_file;
    }
    else
    {
        in_filename = argv[1];
        out_filename = argv[2];
    }

    printf("Input URL: %s\n", in_filename);
    printf("Output file: %s\n", out_filename);

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
    // Network
    avformat_network_init();
    // Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        printf("Could not open input URL: %s\n", in_filename);
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        printf("Failed to retrieve input stream information\n");
        goto end;
    }

    for (i = 0; i < ifmt_ctx->nb_streams; i++)
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    // Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);

    if (!ofmt_ctx)
    {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        // Create output AVStream according to input AVStream
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream)
        {
            printf("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        // Copy the codec parameters
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0)
        {
            printf("Failed to copy codec parameters\n");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        {
            // FFmpeg 4.0+ uses AV_CODEC_FLAG_GLOBAL_HEADER instead of CODEC_FLAG_GLOBAL_HEADER
            // But we need to create a codec context first for new FFmpeg versions
            AVCodecContext *codec_ctx;
            const AVCodec *codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            if (codec)
            {
                codec_ctx = avcodec_alloc_context3(codec);
                avcodec_parameters_to_context(codec_ctx, in_stream->codecpar);
                codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
                avcodec_free_context(&codec_ctx);
            }
        }
    }
    // Dump Format------------------
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    // Open output URL
    if (!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output file '%s'\n", out_filename);
            goto end;
        }
    }
    // Write file header
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        printf("Error occurred when opening output file\n");
        goto end;
    }

#if USE_H264BSF
    AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif

    while (1)
    {
        AVStream *in_stream, *out_stream;
        // Get an AVPacket
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        // Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        // Print to Screen
        if (pkt.stream_index == videoindex)
        {
            printf("Receive %8d video frames from input URL\n", frame_index);
            frame_index++;

#if USE_H264BSF
            // 注意：新版FFmpeg中的BitStream Filter API已经更改
            // 如果需要使用，请参考新的API
            av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size,
                                       pkt.data, pkt.size, 0);
#endif
        }
        // ret = av_write_frame(ofmt_ctx, &pkt);
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

        if (ret < 0)
        {
            printf("Error muxing packet\n");
            break;
        }

        av_packet_unref(&pkt);
    }

#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif

    // Write file trailer
    av_write_trailer(ofmt_ctx);
end:
    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred.\n");
        return -1;
    }
    return 0;
}
