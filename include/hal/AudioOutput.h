#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace hal
{
    /**
     * @class AudioOutput
     * @brief 音频输出模块（ALSA）
     *
     * 负责音频设备打开/关闭、PCM 写入、缓冲刷新与错误恢复。
     * 输入数据格式应与解码层约定一致（S16 交错 PCM）。
     */
    class AudioOutput
    {
    public:
        /**
         * @brief 构造输出实例
         */
        AudioOutput();

        /**
         * @brief 析构函数，自动关闭设备
         */
        ~AudioOutput();

        AudioOutput(const AudioOutput&) = delete;
        AudioOutput& operator=(const AudioOutput&) = delete;

        /**
         * @brief 打开 ALSA 播放设备
         * @param sampleRate 采样率（Hz）
         * @param channels 声道数
         * @param errorMsg 失败时返回错误信息
         * @return true 打开成功，false 失败
         */
        bool open(int sampleRate, int channels, std::string& errorMsg);

        /**
         * @brief 关闭 ALSA 设备并释放资源
         */
        void close();

        /**
         * @brief 判断输出设备是否可用
         */
        [[nodiscard]] bool isOpen() const;

        /**
         * @brief 清空输出缓冲并重新准备设备
         */
        void flush();

        /**
         * @brief 写入一段 PCM 数据
         * @param pcmInterleaved 交错 PCM 数据指针（S16）
         * @param frameCount 帧数（每帧包含所有声道采样）
         * @param volume 软件音量（0.0~1.0）
         * @param errorMsg 失败时返回错误信息
         * @return true 写入成功，false 写入失败
         */
        bool write(const int16_t* pcmInterleaved, int frameCount, float volume, std::string& errorMsg);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
} // namespace hal
