
#ifndef __AUDIO_CAPTURE__
#define __AUDIO_CAPTURE__

#include <cstdint>
#include <baserecord.h>
#include <jni.h>

#define RESULT_CODE_OK        0;
#define RESULT_CODE_ERR       -1;

/**
 * 采集内核
 */
#define AUDIO_CAPTURE_TYPE_OPENSLES        0x001
#define AUDIO_CAPTURE_TYPE_AUDIORECORD     0x002

/**
 * 声道配置
 */
#define AUDIO_CHANNEL_MONO                 0x201
#define AUDIO_CHANNEL_STEREO               0x202

/**
 * 采集格式
 */
#define AUDIO_FORMAT_PCM_16BIT            0x301
#define AUDIO_FORMAT_PCM_8BIT             0x302
#define AUDIO_FORMAT_PCM_FLOAT            0x303

/**
 * 采集频率
 */
#define AUDIO_SAMPLE_RATE_8               8000
#define AUDIO_SAMPLE_RATE_11_025          11025
#define AUDIO_SAMPLE_RATE_12              12000
#define AUDIO_SAMPLE_RATE_16              16000
#define AUDIO_SAMPLE_RATE_22_05           22050
#define AUDIO_SAMPLE_RATE_24              24000
#define AUDIO_SAMPLE_RATE_32              32000
#define AUDIO_SAMPLE_RATE_44_1            44100
#define AUDIO_SAMPLE_RATE_48              48000
#define AUDIO_SAMPLE_RATE_64              64000
#define AUDIO_SAMPLE_RATE_82              82000
#define AUDIO_SAMPLE_RATE_96              96000
#define AUDIO_SAMPLE_RATE_192             192000

#define STATE_UNINIT        0
#define STATE_IDEL          1
#define STATE_RECORDING     2

/**
 * 对外统一接口
 * sample:
 * capture = new AudioCapture(AUDIO_CAPTURE_TYPE_AUDIORECORD, AUDIO_SAMPLE_RATE_44_1, AUDIO_CHANNEL_MONO, AUDIO_FORMAT_PCM_16BIT);
 * if (capture->getState() == STATE_IDEL)
 * {
 *     capture->setAudioCaptureCallback(readData, this);
 *     capture->startRecording();
 * }
 * else
 * {
 *     capture->releaseRecording();
 *     delete capture;
 *     capture = nullptr;
 * }
 *
 *
 * capture->stopRecording();
 * capture->releaseRecording();
 * delete capture;
 * capture = nullptr;
 */

class AudioCapture {
public:
    /**
     * 构造一个采集实例
     * @param capture_type    采集内核 支持audiorecord 和 opensl es, 参数参见 audiocaptrue.h定义
     * @param sample_rate     采集频率，参数参见 audiocaptrue.h定义
     * @param channel_config  采集声道配置，参数参见 audiocaptrue.h定义
     * @param audio_format    采集格式，参数参见 audiocaptrue.h定义
     */
    AudioCapture(uint16_t capture_type, uint32_t sample_rate, uint16_t channel_config, uint16_t audio_format);
    /**
     * 录制器当前状态
     * @return  状态，参数参见 audiocaptrue.h定义
     */
    int getState();
    /**
     * 开始采集器录制工作
     */
    void startRecording();
    /**
     * 暂停采集器录制工作
     */
    void stopRecording();
    /**
     * 释放采集器资源
     */
    void releaseRecording();
    /**
     * 设置采集器PCM数据回调
     * @param callback  回调函数
     * @param ctx       回调上下文
     */
    void setAudioCaptureCallback(void (*callback)(int8_t *data, uint32_t len, void *ctx), void *ctx);

    ~AudioCapture();

private:
    BaseRecord *_record_impl;
    uint16_t _capture_type;
};

#endif

