
#include "audiorecord.h"

/**
 * 提供了java层AudioRecordRecord.java的映射实现
 * 封装了统一的native对外api
 */

AudioRecord::AudioRecord()
: _callback(nullptr)
, _ctx(nullptr)
{
    JNIEnv *env = getEnv();
    //获取java映射的class字节
    class_audio = env->FindClass("com/net168/audiorecord/audiorecord/AudioRecordRecord");
    //获取各个反射方法
    method_start = env->GetMethodID(class_audio, "start", "()V");
    method_stop = env->GetMethodID(class_audio, "stop", "()V");
    method_release = env->GetMethodID(class_audio, "release", "()V");
    method_get_state = env->GetMethodID(class_audio, "getState", "()I");
    method_init = env->GetMethodID(class_audio, "init", "(III)Z");
    method_set_native_callback = env->GetMethodID(class_audio, "setNativeCallback", "(Z)V");
    method_object = env->GetMethodID(class_audio, "<init>", "(J)V");
}

int AudioRecord::init(uint32_t sample_rate, uint16_t channel_config, uint16_t audio_format)
{
    JNIEnv *env = getEnv();
    //由于object_audio需要在采集线程使用到，所以需要声明GlobalRef引用
    object_audio = env->NewGlobalRef(env->NewObject(class_audio, method_object, reinterpret_cast<intptr_t>(this)));
    return env->CallBooleanMethod(object_audio, method_init, sample_rate, channel_config, audio_format) ? 0 : -1;
}

int AudioRecord::getState()
{
    return getEnv()->CallIntMethod(object_audio, method_get_state);
}

void AudioRecord::start()
{
    getEnv()->CallVoidMethod(object_audio, method_start);
    //设置native pcm数据回调，将会调用 Java_com_net168_audiorecord_audiorecord_AudioRecordRecord_sendDataToNative 方法
    getEnv()->CallVoidMethod(object_audio, method_set_native_callback, true);
}

void AudioRecord::stop()
{
    //取消native pcm数据回调
    getEnv()->CallVoidMethod(object_audio, method_set_native_callback, false);
    getEnv()->CallVoidMethod(object_audio, method_stop);
}

void AudioRecord::release()
{
    getEnv()->CallVoidMethod(object_audio, method_release);
}

void AudioRecord::setAudioCaptureCallback(
        void (*callback)(int8_t *, uint32_t, void *), void *ctx)
{
    _callback = callback;
    _ctx = ctx;
}

AudioRecord::~AudioRecord()
{
    getEnv()->DeleteGlobalRef(reinterpret_cast<jobject>(object_audio));
    object_audio = nullptr;
    _callback = nullptr;
    _ctx = nullptr;
}

void AudioRecord::readData(int8_t *data, uint32_t size)
{
    /**
     * 如果有设置回调监听并且存在缓存数据则回调
     */
    if (_callback && data && size >= 0)
    {
        _callback(data, size, _ctx);
    }
}


extern "C" JNIEXPORT void
JNICALL
Java_com_net168_audiorecord_audiorecord_AudioRecordRecord_sendDataToNative(JNIEnv *env ,jobject thiz, jlong instance, jobject data) {
    //获取NIO buffer数据的有效数据size
    uint32_t size = static_cast<uint32_t>(env->GetDirectBufferCapacity(data));
    //获取NIO buffer的缓存数据指针
    int8_t *buf = static_cast<int8_t *>(env->GetDirectBufferAddress(data));
    AudioRecord *audioRecord = reinterpret_cast<AudioRecord *>(instance);
    audioRecord->readData(buf, size);
}