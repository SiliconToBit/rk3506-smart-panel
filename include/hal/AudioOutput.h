#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace hal
{
    /**
     * @class AudioOutput
     * @brief 音频输出模块（基于 ALSA）
     *
     * 负责音频设备打开/关闭、PCM 数据写入、缓冲刷新与错误恢复。
     * 输入数据格式应与解码层约定一致（S16 交错 PCM）。
     * 
     * 该类封装了 ALSA（Advanced Linux Sound Architecture）的底层 API，
     * 提供简化的音频播放接口。支持软件音量控制和设备错误自动恢复。
     * 
     * @note 该类不可拷贝，同一实例一次只能打开一个音频设备
     * @note 使用 Pimpl 模式隐藏 ALSA 内部实现细节
     */
    class AudioOutput
    {
    public:
        /**
         * @brief 构造输出实例
         *
         * 初始化音频输出对象，此时未打开任何设备。
         * 需要调用 open() 后才能开始播放音频。
         */
        AudioOutput();

        /**
         * @brief 析构函数，自动关闭设备
         *
         * 如果当前有打开的设备，会自动调用 close() 关闭并释放 ALSA 资源。
         */
        ~AudioOutput();

        /**
         * @brief 禁止拷贝构造（避免设备句柄重复释放）
         */
        AudioOutput(const AudioOutput&) = delete;
        
        /**
         * @brief 禁止赋值操作（避免资源管理混乱）
         */
        AudioOutput& operator=(const AudioOutput&) = delete;

        /**
         * @brief 打开 ALSA 播放设备
         * @param sampleRate 采样率（Hz），通常使用 44100
         * @param channels 声道数，通常使用 2（立体声）
         * @param errorMsg 输出参数，失败时返回详细错误信息
         * @return true 打开成功并准备好接收数据，false 打开失败
         *
         * 执行以下操作：
         * 1. 打开默认 ALSA 播放设备（default 或 hw:0,0）
         * 2. 配置 PCM 参数（采样率、声道数、采样格式、访问模式）
         * 3. 设置缓冲区大小和周期大小
         * 4. 准备设备进入播放状态
         *
         * @note 如果已打开设备，需要先调用 close()
         * @note 常见失败原因：设备被占用、参数不支持、驱动问题
         * @note 打开后设备处于准备状态，可以开始写入 PCM 数据
         */
        bool open(int sampleRate, int channels, std::string& errorMsg);

        /**
         * @brief 关闭 ALSA 设备并释放资源
         *
         * 停止播放，关闭设备句柄，释放 ALSA 相关资源。
         * 关闭后可以重新调用 open() 打开其他设备或使用不同参数。
         * 
         * @note 如果未打开设备，此操作无效
         * @note 关闭后未播放完的缓冲区数据可能会丢失
         */
        void close();

        /**
         * @brief 判断输出设备是否可用
         * @return true 设备已打开并准备好接收数据，false 未打开或已关闭
         *
         * 可用于检查设备当前状态，避免在未打开设备时调用 write()。
         */
        [[nodiscard]] bool isOpen() const;

        /**
         * @brief 获取 ALSA 缓冲区中尚未播放的帧数
         * @return 延迟帧数，如果设备未打开则返回 0
         *
         * 可用于计算实际播放进度，考虑 ALSA 内部缓冲区的延迟。
         */
        [[nodiscard]] int getDelayFrames() const;

        /**
         * @brief 清空输出缓冲并重新准备设备
         *
         * 丢弃 ALSA 缓冲区中未播放的数据，并重新准备设备。
         * 通常在以下场景调用：
         * - 播放位置跳转（seek）后清空残留数据
         * - 停止播放后清理缓冲区
         * - 设备错误恢复后重新准备
         * 
         * @note 调用后缓冲区中的旧数据会被丢弃
         * @note 如果设备未打开，此操作无效
         */
        void flush();

        /**
         * @brief 写入一段 PCM 数据到 ALSA 设备
         * @param pcmInterleaved 交错 PCM 数据指针（S16 格式）
         * @param frameCount 帧数（每帧包含所有声道的采样，如立体声每帧 2 个采样）
         * @param volume 软件音量（0.0 静音 ~ 1.0 最大音量）
         * @param errorMsg 输出参数，失败时返回详细错误信息
         * @return true 写入成功，false 写入失败（设备错误或未打开）
         *
         * 将解码后的 PCM 数据写入 ALSA 播放缓冲区。
         * 在写入前会应用软件音量控制（线性缩放）。
         * 
         * 数据格式要求：
         * - 采样格式：S16（16位有符号整数，little-endian）
         * - 声道布局：交错（左0, 右0, 左1, 右1, ...）
         * - 采样率和声道数必须与 open() 时配置的参数一致
         *
         * 错误处理：
         * - 如果设备进入错误状态（如 underrun），会尝试自动恢复
         * - 恢复失败时返回 false，并通过 errorMsg 返回错误信息
         * 
         * @note 此函数是阻塞的，如果缓冲区满会等待直到有空间
         * @note frameCount 不应超过设备缓冲区大小，否则可能被截断
         * @note 音量控制是软件实现，不影响系统音量设置
         */
        bool write(const int16_t* pcmInterleaved, int frameCount, float volume, std::string& errorMsg);

    private:
        /**
         * @struct Impl
         * @brief Pimpl 模式实现类，隐藏 ALSA 内部细节
         *
         * 包含 ALSA 设备句柄（snd_pcm_t*）和内部状态，
         * 避免在头文件中暴露 ALSA 依赖（需要包含 alsa/asoundlib.h）。
         * 具体实现在 AudioOutput.cpp 中定义。
         */
        struct Impl;
        
        std::unique_ptr<Impl> m_impl; ///< 指向实现类的智能指针
    };
} // namespace hal
