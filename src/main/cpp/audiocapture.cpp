
#include <audiocapture.h>
#include <opensles/openslesrecord.h>
#include <audiorecord/audiorecord.h>
#include <base/jvm.h>


AudioCapture::AudioCapture(uint16_t capture_type, uint32_t sample_rate, uint16_t channel_config, uint16_t audio_format)
    : _capture_type(capture_type)
{
    int result;
    //判断采集内核的类型
    if (capture_type == AUDIO_CAPTURE_TYPE_OPENSLES)
        _record_impl = static_cast<BaseRecord *>(new OpenslesRecord());
    else
        _record_impl = static_cast<BaseRecord *>(new AudioRecord());
    result = _record_impl->init(sample_rate, channel_config, audio_format);
    //如果init，清理内存
    if (result < 0)
    {
        delete _record_impl;
        _record_impl = nullptr;
    }
}

int AudioCapture::getState()
{
    if (!_record_impl)
        return STATE_UNINIT;
    return _record_impl->getState();
}

void AudioCapture::startRecording()
{
    if (_record_impl)
        _record_impl->start();
}

void AudioCapture::stopRecording()
{
    if (_record_impl)
        _record_impl->stop();
}

void AudioCapture::releaseRecording()
{
    if (_record_impl)
        _record_impl->release();
}


void AudioCapture::setAudioCaptureCallback(void (*callback)(int8_t *data, uint32_t len, void *ctx), void *ctx)
{
    if (_record_impl)
        _record_impl->setAudioCaptureCallback(callback, ctx);
}


AudioCapture::~AudioCapture()
{
    delete _record_impl;
    _record_impl = nullptr;
}


extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    JNIEnv *env = nullptr;
    if (jvm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
        return -1;
    //初始化全局jvm，用于提供其他线程的jni环境
    initGlobalJvm(jvm);
    return JNI_VERSION_1_6;
}
