
#include "openslesrecord.h"

#define CHECK_CTX(ctx)  if (ctx == 0) return;

OpenslesRecord::OpenslesRecord()
: core(nullptr)
, jniData(nullptr)
{
    core = new OpenslesCore();
}

int OpenslesRecord::init(uint32_t sample_rate, uint16_t channel_config, uint16_t audio_format)
{
    SLresult result;
    //初始化opensl es内核结构
    result = core->createEngine();
    if (result != SL_RESULT_SUCCESS)
        goto error;
    result = core->createAudioRecord(getChannelNum(channel_config), getSampleRate(sample_rate), getAudioFormat(audio_format));
    if (result != SL_RESULT_SUCCESS)
        goto error;
    return 0;

error:
    core->release();
    delete core;
    core = nullptr;
    return -1;
}

int OpenslesRecord::getState()
{
    if (!core)
        return STATE_UNINIT;
    if (core->isRecordering())
        return STATE_RECORDING;
    else if (core->isInitSuccess())
        return STATE_IDEL;
    else
        return STATE_UNINIT;
}

void OpenslesRecord::start()
{
    if (core)
        core->start();
}

void OpenslesRecord::stop()
{
    if (core)
        core->stop();
}

void OpenslesRecord::release()
{
    if (core)
        core->release();
}

void OpenslesRecord::setAudioCaptureCallback
        (void (*callback)(int8_t *data, uint32_t len, void *ctx), void *ctx)
{
    if (core)
        core->setAudioCaptureCallback(callback, ctx);
}



OpenslesRecord::~OpenslesRecord()
{
    delete core;
    core = nullptr;
}

/**
 * 将audiocapture支持的采集频率转换成opensl es支持的采集频率格式
 */
SLresult OpenslesRecord::getSampleRate(uint32_t sample_rate)
{
    if (sample_rate == AUDIO_SAMPLE_RATE_8 || sample_rate == AUDIO_SAMPLE_RATE_11_025 || sample_rate == AUDIO_SAMPLE_RATE_12
            || sample_rate == AUDIO_SAMPLE_RATE_16 || sample_rate == AUDIO_SAMPLE_RATE_22_05 || sample_rate == AUDIO_SAMPLE_RATE_24
            || sample_rate == AUDIO_SAMPLE_RATE_32 || sample_rate == AUDIO_SAMPLE_RATE_44_1 || sample_rate == AUDIO_SAMPLE_RATE_48
            || sample_rate == AUDIO_SAMPLE_RATE_64 || sample_rate == AUDIO_SAMPLE_RATE_82 || sample_rate == AUDIO_SAMPLE_RATE_96
            || sample_rate == AUDIO_SAMPLE_RATE_192)
        return sample_rate*1000;
    else
        return SL_SAMPLINGRATE_16;
}

/**
 * 将audiocapture支持的声道配置转换成opensl es支持的声道数
 */
SLresult OpenslesRecord::getChannelNum(uint16_t channel_config)
{
    if (channel_config == AUDIO_CHANNEL_STEREO)
        return 2;
    else
        return 1;
}

/**
 * 将audiocapture支持的采用格式转换成opensl es支持的采用格式
 */
SLresult OpenslesRecord::getAudioFormat(uint16_t audio_format)
{
    if (audio_format == AUDIO_FORMAT_PCM_8BIT)
        return SL_PCMSAMPLEFORMAT_FIXED_8;
    else if (audio_format == AUDIO_FORMAT_PCM_FLOAT)
        return SL_PCMSAMPLEFORMAT_FIXED_32;
    else
        return SL_PCMSAMPLEFORMAT_FIXED_16;
}


/***************************** 对接封装java层 OpenSLESRecord.java实现 ***************************/

extern "C"
JNIEXPORT jlong JNICALL
Java_com_net168_audiorecord_opensles_OpenSLESRecord__1initRecord(JNIEnv *env, jobject instance,
                                                                 jint sampleRate,
                                                                 jint channelConfig,
                                                                 jint audioFormat) {
    int result;
    OpenslesRecord *record = new OpenslesRecord();
    result = record->init(static_cast<uint32_t>(sampleRate), static_cast<uint16_t>(channelConfig),
                          static_cast<uint16_t>(audioFormat));
    if (result < 0)
    {
        record->release();
        delete record;
        return 0;
    }
    record->jniData = new OpenslJniCallback();
    record->jniData->java_object = env->NewGlobalRef(instance);
    jclass cls = env->GetObjectClass(instance);
    record->jniData->method_data_callback = env->GetMethodID(cls, "onNativeDataRead", "(Ljava/nio/ByteBuffer;)V");
    //将OpenslesRecord实例指针返回java存放
    return reinterpret_cast<intptr_t>(record);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_net168_audiorecord_opensles_OpenSLESRecord__1stop(JNIEnv *env, jobject instance, jlong ctx) {
    CHECK_CTX(ctx);
    OpenslesRecord *record = reinterpret_cast<OpenslesRecord *>(ctx);
    record->stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_net168_audiorecord_opensles_OpenSLESRecord__1start(JNIEnv *env, jobject instance, jlong ctx) {
    CHECK_CTX(ctx);
    OpenslesRecord *record = reinterpret_cast<OpenslesRecord *>(ctx);
    record->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_net168_audiorecord_opensles_OpenSLESRecord__1release(JNIEnv *env, jobject instance, jlong ctx) {
    CHECK_CTX(ctx);
    OpenslesRecord *record = reinterpret_cast<OpenslesRecord *>(ctx);
    record->release();
    delete record;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_net168_audiorecord_opensles_OpenSLESRecord__1getState(JNIEnv *env, jobject instance, jlong ctx) {
    if (ctx == 0)
        return STATE_UNINIT;
    OpenslesRecord *record = reinterpret_cast<OpenslesRecord *>(ctx);
    return record->getState();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_net168_audiorecord_opensles_OpenSLESRecord__1setNativeCallback(JNIEnv *env,
                                                                        jobject instance,
                                                                        jlong ctx,
                                                                        jboolean need) {
    CHECK_CTX(ctx);
    OpenslesRecord *record = reinterpret_cast<OpenslesRecord *>(ctx);
    if (need) //是否需要PCM数据回调
        record->setAudioCaptureCallback(OpenslesRecord::onDataRead, record);
    else
        record->setAudioCaptureCallback(NULL, NULL);
}

void OpenslesRecord::onDataRead(int8_t *data, uint32_t len, void *ctx)
{
    OpenslesRecord *record = static_cast<OpenslesRecord *>(ctx);
    OpenslJniCallback *jniData = record->jniData;
    if (!jniData)
        return;
    bool thread = false; //记录当前是否存在jni环境
    JNIEnv *env = getEnv(&thread);
    //将pcm数据封装成NIO buffer传输到java层
    jobject buffer = env->NewDirectByteBuffer(data, len * sizeof(int8_t));
    env->CallVoidMethod(jniData->java_object, jniData->method_data_callback, buffer);
    if (thread) //如果是新Attach线程则需要Detach
        detatchEnv();
}
