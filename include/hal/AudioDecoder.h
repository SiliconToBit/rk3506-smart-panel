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
     * @brief 音频解码与重采样模块（基于 FFmpeg）
     *
     * 负责打开音频文件、解码压缩帧并统一输出为 PCM 数据（S16/44.1kHz/双声道）。
     * 该类只处理"文件 -> PCM"数据路径，不涉及音频设备写入。
     *
     * 支持的音频格式包括 MP3、AAC、FLAC、WAV 等 FFmpeg 支持的所有格式。
     * 内部使用 FFmpeg 的 libavformat、libavcodec 和 libswresample 库进行解码和重采样。
     *
     * @note 该类不可拷贝，同一实例一次只能打开一个音频文件
     * @note 输出格式固定为 S16 交错双声道 44.1kHz，便于与 AudioOutput 对接
     */
    class AudioDecoder
    {
    public:
        /// 统一输出声道数（立体声）
        static constexpr int kOutputChannels = 2;

        /// 统一输出采样率（Hz，CD 音质标准）
        static constexpr int kOutputSampleRate = 44100;

        /**
         * @brief 构造解码器实例
         *
         * 初始化解码器内部状态，此时未打开任何音频文件。
         * 需要调用 open() 后才能开始解码。
         */
        AudioDecoder();

        /**
         * @brief 析构函数，自动释放解码相关资源
         *
         * 如果当前有打开的文件，会自动调用 close() 关闭并释放所有 FFmpeg 资源。
         */
        ~AudioDecoder();

        /**
         * @brief 禁止拷贝构造（避免资源重复释放）
         */
        AudioDecoder(const AudioDecoder&) = delete;

        /**
         * @brief 禁止赋值操作（避免资源管理混乱）
         */
        AudioDecoder& operator=(const AudioDecoder&) = delete;

        /**
         * @brief 打开音频文件并初始化解码器/重采样器
         * @param filePath 音频文件路径（支持本地文件路径）
         * @return true 打开并初始化成功，false 打开或初始化失败
         *
         * 执行以下操作：
         * 1. 使用 FFmpeg 打开音频文件并查找音频流
         * 2. 初始化解码器上下文
         * 3. 配置重采样器（将任意格式转换为 S16/44.1kHz/立体声）
         * 4. 读取音频元数据（如时长）
         *
         * @note 如果已打开其他文件，需要先调用 close()
         * @note 失败时可以通过 FFmpeg 错误码获取详细原因
         */
        bool open(const std::string& filePath);

        /**
         * @brief 关闭当前文件并释放内部资源
         *
         * 释放 FFmpeg 相关资源（AVFormatContext、AVCodecContext、SwrContext 等）。
         * 关闭后可以重新调用 open() 打开其他文件。
         *
         * @note 如果未打开文件，此操作无效
         */
        void close();

        /**
         * @brief 跳转到指定播放位置
         * @param positionSeconds 目标时间（秒），从文件开头计算
         * @return true 跳转成功，false 失败（如未打开文件或超出范围）
         *
         * 使用 FFmpeg 的 av_seek_frame() 进行跳转，然后需要调用 flushAfterSeek()
         * 清空解码器缓冲区，避免残留旧数据。
         *
         * @note 跳转后必须调用 flushAfterSeek() 才能继续正确解码
         * @note 某些流媒体文件可能不支持精确跳转
         */
        bool seek(double positionSeconds);

        /**
         * @brief 在 seek 后刷新解码器内部缓存
         *
         * 清空解码器的内部缓冲帧，确保跳转后不会输出旧数据。
         * 必须在 seek() 成功后立即调用，然后再调用 readNextPcm()。
         *
         * @note 如果未调用 seek()，不需要调用此函数
         */
        void flushAfterSeek();

        /**
         * @brief 判断解码器是否已处于可用状态
         * @return true 已成功打开音频文件并准备好解码，false 未打开或已关闭
         *
         * 可用于检查解码器当前是否有效，避免在未打开文件时调用 readNextPcm()。
         */
        [[nodiscard]] bool isOpen() const;

        /**
         * @brief 获取音频总时长（秒）
         * @return 有效时长（秒）或空值（std::nullopt）
         *
         * 从音频文件的元数据中读取总时长。
         * 某些文件（如流媒体或损坏文件）可能无法获取时长。
         *
         * @note 需要在 open() 成功后调用，否则返回空值
         * @note 返回的时长可能不精确，取决于文件元数据质量
         */
        [[nodiscard]] std::optional<double> getDurationSeconds() const;

        /**
         * @brief 读取一块已解码的 PCM 数据
         * @param pcmInterleaved 输出参数，存储解码后的 PCM 数据（S16，交错双声道）
         * @param ptsSeconds 输出参数，当前帧的时间戳（秒），用于播放进度追踪
         * @param errorMsg 输出参数，失败时返回详细错误信息
         * @return true 成功读取一块数据；false 表示到达文件末尾（EOF）或发生错误
         *
         * 从音频文件中解码下一帧数据，并转换为统一的 S16/44.1kHz/立体声格式。
         * 每次调用会读取一个压缩帧，解码后输出 PCM 数据。
         *
         * 数据格式说明：
         * - 采样格式：S16（16位有符号整数）
         * - 声道布局：交错（左声道采样0, 右声道采样0, 左声道采样1, 右声道采样1, ...）
         * - 采样率：44100 Hz
         * - 声道数：2（立体声）
         *
         * @note 返回 false 时应停止调用，检查是 EOF 还是错误
         * @note 此函数是阻塞的，解码耗时取决于音频格式和硬件性能
         * @note pcmInterleaved 会被清空后重新填充，调用前无需手动清空
         */
        bool readNextPcm(std::vector<int16_t>& pcmInterleaved, double& ptsSeconds, std::string& errorMsg);

    private:
        /**
         * @struct Impl
         * @brief Pimpl 模式实现类，隐藏 FFmpeg 内部细节
         *
         * 包含所有 FFmpeg 相关结构体指针和内部状态，避免在头文件中暴露 FFmpeg 依赖。
         * 具体实现在 AudioDecoder.cpp 中定义。
         */
        struct Impl;

        std::unique_ptr<Impl> m_impl; ///< 指向实现类的智能指针
    };
} // namespace hal
