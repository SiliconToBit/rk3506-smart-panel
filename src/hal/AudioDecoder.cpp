#include "hal/AudioDecoder.h"

#include <cstring>
#include <memory>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

namespace
{
    template <typename T> struct FFmpegDeleter;

    template <> struct FFmpegDeleter<AVFormatContext>
    {
        void operator()(AVFormatContext* ptr) const noexcept
        {
            if (ptr != nullptr)
            {
                avformat_close_input(&ptr);
            }
        }
    };

    template <> struct FFmpegDeleter<AVCodecContext>
    {
        void operator()(AVCodecContext* ptr) const noexcept
        {
            if (ptr != nullptr)
            {
                avcodec_free_context(&ptr);
            }
        }
    };

    template <> struct FFmpegDeleter<SwrContext>
    {
        void operator()(SwrContext* ptr) const noexcept
        {
            if (ptr != nullptr)
            {
                swr_free(&ptr);
            }
        }
    };

    template <typename T> using FFmpegPtr = std::unique_ptr<T, FFmpegDeleter<T>>;

    struct PacketDeleter
    {
        void operator()(AVPacket* ptr) const noexcept
        {
            if (ptr != nullptr)
            {
                av_packet_free(&ptr);
            }
        }
    };

    struct FrameDeleter
    {
        void operator()(AVFrame* ptr) const noexcept
        {
            if (ptr != nullptr)
            {
                av_frame_free(&ptr);
            }
        }
    };
} // namespace

namespace hal
{
    struct AudioDecoder::Impl
    {
        FFmpegPtr<AVFormatContext> formatCtx{nullptr};
        FFmpegPtr<AVCodecContext> codecCtx{nullptr};
        FFmpegPtr<SwrContext> swrCtx{nullptr};

        std::unique_ptr<AVPacket, PacketDeleter> packet{av_packet_alloc()};
        std::unique_ptr<AVFrame, FrameDeleter> frame{av_frame_alloc()};

        int audioStreamIndex{-1};
        bool drainSent{false};
    };

    AudioDecoder::AudioDecoder()
        : m_impl(std::make_unique<Impl>())
    {
    }

    AudioDecoder::~AudioDecoder()
    {
        close();
    }

    bool AudioDecoder::open(const std::string& filePath)
    {
        close();

        AVFormatContext* rawFmtCtx = nullptr;
        if (avformat_open_input(&rawFmtCtx, filePath.c_str(), nullptr, nullptr) != 0)
        {
            return false;
        }
        m_impl->formatCtx.reset(rawFmtCtx);

        if (avformat_find_stream_info(m_impl->formatCtx.get(), nullptr) < 0)
        {
            close();
            return false;
        }

        m_impl->audioStreamIndex = -1;
        for (unsigned int i = 0; i < m_impl->formatCtx->nb_streams; ++i)
        {
            if (m_impl->formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                m_impl->audioStreamIndex = static_cast<int>(i);
                break;
            }
        }

        if (m_impl->audioStreamIndex < 0)
        {
            close();
            return false;
        }

        AVCodecParameters* codecParams = m_impl->formatCtx->streams[m_impl->audioStreamIndex]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (codec == nullptr)
        {
            close();
            return false;
        }

        AVCodecContext* rawCodecCtx = avcodec_alloc_context3(codec);
        if (rawCodecCtx == nullptr)
        {
            close();
            return false;
        }
        m_impl->codecCtx.reset(rawCodecCtx);

        if (avcodec_parameters_to_context(m_impl->codecCtx.get(), codecParams) < 0)
        {
            close();
            return false;
        }
        if (avcodec_open2(m_impl->codecCtx.get(), codec, nullptr) < 0)
        {
            close();
            return false;
        }

        SwrContext* rawSwrCtx = swr_alloc();
        if (rawSwrCtx == nullptr)
        {
            close();
            return false;
        }
        m_impl->swrCtx.reset(rawSwrCtx);

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100)
        av_opt_set_chlayout(m_impl->swrCtx.get(), "in_chlayout", &m_impl->codecCtx->ch_layout, 0);
#else
        uint64_t inChannelLayout = m_impl->codecCtx->channel_layout;
        if (inChannelLayout == 0)
        {
            inChannelLayout = av_get_default_channel_layout(m_impl->codecCtx->channels);
        }
        av_opt_set_channel_layout(m_impl->swrCtx.get(), "in_channel_layout", inChannelLayout, 0);
#endif
        av_opt_set_int(m_impl->swrCtx.get(), "in_sample_rate", m_impl->codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(m_impl->swrCtx.get(), "in_sample_fmt", m_impl->codecCtx->sample_fmt, 0);

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100)
        AVChannelLayout outLayout;
        av_channel_layout_default(&outLayout, kOutputChannels);
        av_opt_set_chlayout(m_impl->swrCtx.get(), "out_chlayout", &outLayout, 0);
#else
        av_opt_set_channel_layout(m_impl->swrCtx.get(), "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
#endif
        av_opt_set_int(m_impl->swrCtx.get(), "out_sample_rate", kOutputSampleRate, 0);
        av_opt_set_sample_fmt(m_impl->swrCtx.get(), "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

        if (swr_init(m_impl->swrCtx.get()) < 0)
        {
            close();
            return false;
        }

        m_impl->drainSent = false;
        return true;
    }

    void AudioDecoder::close()
    {
        m_impl->swrCtx.reset();
        m_impl->codecCtx.reset();
        m_impl->formatCtx.reset();
        m_impl->audioStreamIndex = -1;
        m_impl->drainSent = false;
    }

    bool AudioDecoder::seek(double positionSeconds)
    {
        if (!isOpen())
        {
            return false;
        }

        AVStream* stream = m_impl->formatCtx->streams[m_impl->audioStreamIndex];
        int64_t timestamp = static_cast<int64_t>(positionSeconds / av_q2d(stream->time_base));

        if (av_seek_frame(m_impl->formatCtx.get(), m_impl->audioStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD) < 0)
        {
            return false;
        }

        flushAfterSeek();
        return true;
    }

    void AudioDecoder::flushAfterSeek()
    {
        if (!m_impl->codecCtx)
        {
            return;
        }

        avcodec_flush_buffers(m_impl->codecCtx.get());
        if (m_impl->packet)
        {
            av_packet_unref(m_impl->packet.get());
        }
        if (m_impl->frame)
        {
            av_frame_unref(m_impl->frame.get());
        }
        m_impl->drainSent = false;
    }

    bool AudioDecoder::isOpen() const
    {
        return m_impl->formatCtx != nullptr && m_impl->codecCtx != nullptr && m_impl->swrCtx != nullptr &&
               m_impl->audioStreamIndex >= 0 && m_impl->packet != nullptr && m_impl->frame != nullptr;
    }

    std::optional<double> AudioDecoder::getDurationSeconds() const
    {
        if (!isOpen())
        {
            return std::nullopt;
        }

        AVStream* stream = m_impl->formatCtx->streams[m_impl->audioStreamIndex];
        if (stream->duration != AV_NOPTS_VALUE)
        {
            return stream->duration * av_q2d(stream->time_base);
        }

        if (m_impl->formatCtx->duration != AV_NOPTS_VALUE)
        {
            return static_cast<double>(m_impl->formatCtx->duration) / AV_TIME_BASE;
        }

        return std::nullopt;
    }

    bool AudioDecoder::readNextPcm(std::vector<int16_t>& pcmInterleaved, double& ptsSeconds, std::string& errorMsg)
    {
        errorMsg.clear();
        if (!isOpen())
        {
            errorMsg = "Decoder is not open";
            return false;
        }

        while (true)
        {
            // 步骤 A：尝试从解码器获取已解码帧
            const int receiveRet = avcodec_receive_frame(m_impl->codecCtx.get(), m_impl->frame.get());
            if (receiveRet == 0)
            {
                AVFrame* frame = m_impl->frame.get();
                const int maxOutSamples = swr_get_out_samples(m_impl->swrCtx.get(), frame->nb_samples);
                if (maxOutSamples <= 0)
                {
                    av_frame_unref(frame);
                    continue;
                }

                std::vector<uint8_t> byteBuffer(static_cast<size_t>(maxOutSamples) * kOutputChannels * sizeof(int16_t));
                uint8_t* outPtr = byteBuffer.data();
                const uint8_t** inputData = const_cast<const uint8_t**>(frame->extended_data);
                const int outSamples =
                    swr_convert(m_impl->swrCtx.get(), &outPtr, maxOutSamples, inputData, frame->nb_samples);
                if (outSamples <= 0)
                {
                    av_frame_unref(frame);
                    continue;
                }

                const size_t outCount = static_cast<size_t>(outSamples) * kOutputChannels;
                pcmInterleaved.resize(outCount);
                std::memcpy(pcmInterleaved.data(), byteBuffer.data(), outCount * sizeof(int16_t));

                if (frame->pts != AV_NOPTS_VALUE)
                {
                    AVStream* stream = m_impl->formatCtx->streams[m_impl->audioStreamIndex];
                    ptsSeconds = frame->pts * av_q2d(stream->time_base);
                }

                av_frame_unref(frame);
                return true;
            }

            if (receiveRet == AVERROR_EOF)
            {
                // 解码器内部缓冲已全部排空
                return false;
            }

            if (receiveRet != AVERROR(EAGAIN))
            {
                char errBuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(receiveRet, errBuf, sizeof(errBuf));
                errorMsg = "avcodec_receive_frame failed: " + std::string(errBuf);
                return false;
            }

            // receive_frame 返回 EAGAIN：解码器需要更多输入数据
            if (m_impl->drainSent)
            {
                // drain 已发送但解码器仍要求更多数据 → 确认无残留帧
                return false;
            }

            // 步骤 B：读取下一个数据包并发送给解码器
            const int readRet = av_read_frame(m_impl->formatCtx.get(), m_impl->packet.get());
            if (readRet < 0)
            {
                // 文件已读完，发送 drain 信号
                const int drainRet = avcodec_send_packet(m_impl->codecCtx.get(), nullptr);
                if (drainRet < 0 && drainRet != AVERROR(EAGAIN))
                {
                    char errBuf[AV_ERROR_MAX_STRING_SIZE];
                    av_strerror(drainRet, errBuf, sizeof(errBuf));
                    errorMsg = "Failed to drain decoder: " + std::string(errBuf);
                    return false;
                }
                m_impl->drainSent = true;
                continue;
            }

            if (m_impl->packet->stream_index != m_impl->audioStreamIndex)
            {
                av_packet_unref(m_impl->packet.get());
                continue;
            }

            const int sendRet = avcodec_send_packet(m_impl->codecCtx.get(), m_impl->packet.get());
            av_packet_unref(m_impl->packet.get());
            if (sendRet == AVERROR(EAGAIN))
            {
                // 解码器内部队列满，需要输出帧，继续尝试 receive_frame
                continue;
            }

            if (sendRet < 0)
            {
                char errBuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(sendRet, errBuf, sizeof(errBuf));
                errorMsg = "avcodec_send_packet failed: " + std::string(errBuf);
                return false;
            }
        }
    }
} // namespace hal
