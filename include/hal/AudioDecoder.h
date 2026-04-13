#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hal
{
    /**
     * @class AudioDecoder
     * @brief 音频解码与重采样模块（FFmpeg）
     *
     * 负责打开音频文件、解码压缩帧并统一输出为 PCM（S16/44.1kHz/双声道）。
     * 该类只处理“文件 -> PCM”数据路径，不涉及音频设备写入。
     */
    class AudioDecoder
    {
    public:
        /// 统一输出声道数（立体声）
        static constexpr int kOutputChannels = 2;
        /// 统一输出采样率（Hz）
        static constexpr int kOutputSampleRate = 44100;

        /**
         * @brief 构造解码器实例
         */
        AudioDecoder();

        /**
         * @brief 析构函数，自动释放解码相关资源
         */
        ~AudioDecoder();

        AudioDecoder(const AudioDecoder&) = delete;
        AudioDecoder& operator=(const AudioDecoder&) = delete;

        /**
         * @brief 打开音频文件并初始化解码器/重采样器
         * @param filePath 音频文件路径
         * @return true 打开成功，false 打开或初始化失败
         */
        bool open(const std::string& filePath);

        /**
         * @brief 关闭当前文件并释放内部资源
         */
        void close();

        /**
         * @brief 跳转到指定播放位置
         * @param positionSeconds 目标时间（秒）
         * @return true 跳转成功，false 失败
         */
        bool seek(double positionSeconds);

        /**
         * @brief 在 seek 后刷新解码器内部缓存
         */
        void flushAfterSeek();

        /**
         * @brief 判断解码器是否已处于可用状态
         */
        [[nodiscard]] bool isOpen() const;

        /**
         * @brief 获取音频总时长（秒）
         * @return 有效时长或空值
         */
        [[nodiscard]] std::optional<double> getDurationSeconds() const;

        /**
         * @brief 读取一块已解码 PCM 数据
         * @param pcmInterleaved 输出 PCM 数据（S16，交错双声道）
         * @param ptsSeconds 输出帧对应时间戳（秒）
         * @param errorMsg 失败时返回错误信息
         * @return true 成功读取一块数据；false 表示 EOF 或错误
         */
        bool readNextPcm(std::vector<int16_t>& pcmInterleaved, double& ptsSeconds, std::string& errorMsg);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
} // namespace hal
