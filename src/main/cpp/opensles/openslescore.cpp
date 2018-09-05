
#include "openslescore.h"

#define CHECK_RESULT(result) if(result != SL_RESULT_SUCCESS) return result;

OpenslesCore::OpenslesCore()
: engine_object(nullptr)
, engine_engine(nullptr)
, recorder_object(nullptr)
, recorder_recoder(nullptr)
, recorder_buffer_queue(nullptr)
, recorder_size(0)
, pcm_data(nullptr)
, recodering(false)
, init_success(false)
, _callback(nullptr)
{
    pthread_mutex_init(&_mutex, NULL);
}

SLresult OpenslesCore::createEngine()
{
    SLresult result;
    //设置线程安全模式，防止出现线程过多错误
    SLEngineOption engineOption[] = {
            SL_ENGINEOPTION_THREADSAFE, SL_BOOLEAN_TRUE
    };

    //创建opensl es引擎实例
    result = slCreateEngine(&engine_object, 1, engineOption, 0, NULL, NULL);
    CHECK_RESULT(result);
    result = (*engine_object)->Realize(engine_object, SL_BOOLEAN_FALSE);
    CHECK_RESULT(result);
    result = (*engine_object)->GetInterface(engine_object, SL_IID_ENGINE, &engine_engine);
    CHECK_RESULT(result);

    return SL_RESULT_SUCCESS;
}

SLresult OpenslesCore::createAudioRecord(SLuint32 num_channels, SLuint32 samples_per_sec , SLuint32 bits_per_sample)
{
    SLresult result;

    //检测状态
    if (!engine_object)
        return SL_RESULT_UNKNOWN_ERROR;
    if (init_success)
        return SL_RESULT_SUCCESS;

    //跟进采集格式确定缓存区大小
    if (samples_per_sec == SL_PCMSAMPLEFORMAT_FIXED_8) {
        recorder_size = RECORDER_FRAMES;
    }
    else if (samples_per_sec == SL_PCMSAMPLEFORMAT_FIXED_32) {
        recorder_size = RECORDER_FRAMES * 4;
    }
    else { //default SL_PCMSAMPLEFORMAT_FIXED_16
        recorder_size = RECORDER_FRAMES * 2;
    }
    pcm_data = static_cast<int8_t *>(malloc(sizeof(int8_t) * recorder_size));

    //设置IO设备（麦克风）
    SLDataLocator_IODevice io_device = {
            SL_DATALOCATOR_IODEVICE,
            SL_IODEVICE_AUDIOINPUT,
            SL_DEFAULTDEVICEID_AUDIOINPUT,
            NULL
    };
    SLDataSource data_src = {&io_device, NULL};

    //设置buffer队列
    SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2
    };

    //跟进外部传入参数设置录制规格：声道数、采样频率、采样格式
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, num_channels, samples_per_sec, bits_per_sample, bits_per_sample,
            num_channels == 2 ? (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT) : SL_SPEAKER_FRONT_CENTER,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSink audioSink = {&buffer_queue, &format_pcm};

    //创建录制器
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engine_engine)->CreateAudioRecorder(engine_engine, &recorder_object, &data_src,
                                                &audioSink, 1, id, req);
    CHECK_RESULT(result);
    result = (*recorder_object)->Realize(recorder_object, SL_BOOLEAN_FALSE);
    CHECK_RESULT(result);
    result = (*recorder_object)->GetInterface(recorder_object, SL_IID_RECORD, &recorder_recoder);
    CHECK_RESULT(result);
    result = (*recorder_object)->GetInterface(recorder_object, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorder_buffer_queue);
    CHECK_RESULT(result);
    result = (*recorder_buffer_queue)->RegisterCallback(recorder_buffer_queue, bqRecorderCallback, this);
    CHECK_RESULT(result);

    init_success = true;
    return SL_RESULT_SUCCESS;
}

SLresult OpenslesCore::start()
{
    if (!init_success || recodering)
        return SL_RESULT_UNKNOWN_ERROR;
    SLresult result;
    recodering = true;
    //设置录制器为录制状态
    result = (*recorder_recoder)->SetRecordState(recorder_recoder, SL_RECORDSTATE_RECORDING);
    CHECK_RESULT(result);
    //在设置完录制状态后一定需要先Enqueue一次，这样的话才会开始采集回调
    result = (*recorder_buffer_queue)->Enqueue(recorder_buffer_queue, pcm_data, recorder_size);
    CHECK_RESULT(result);
    return SL_RESULT_SUCCESS;
}

SLresult OpenslesCore::stop()
{
    pthread_mutex_lock(&_mutex);
    if (!recodering)
        return SL_RESULT_UNKNOWN_ERROR;
    recodering = false;
    //设置录制器为停止状态
    (*recorder_recoder)->SetRecordState(recorder_recoder, SL_RECORDSTATE_STOPPED);
    (*recorder_buffer_queue)->RegisterCallback(recorder_buffer_queue, NULL, NULL);
    pthread_mutex_unlock(&_mutex);
    return SL_RESULT_SUCCESS;
}


void OpenslesCore::setAudioCaptureCallback(void (*callback)(int8_t *, uint32_t, void *), void *ctx)
{
    _callback = callback;
    _ctx = ctx;
}

/**
 * 返回包括已经初始化完毕或者正在录制状态
 */
bool OpenslesCore::isInitSuccess()
{
    return init_success;
}

/**
 * 返回正在录制状态
 */
bool OpenslesCore::isRecordering()
{
    if (!init_success)
        return false;
    else
        return recodering;
}

SLresult OpenslesCore::release()
{
    if (recodering)
        return SL_RESULT_UNKNOWN_ERROR;
    init_success = false;
    //释放opensl es资源
    if (recorder_buffer_queue)
        (*recorder_buffer_queue)->Clear(recorder_buffer_queue);
    if (recorder_object)
        (*recorder_object)->Destroy(recorder_object);
    if (engine_object)
        (*engine_object)->Destroy(engine_object);
    //释放缓存区
    free(pcm_data);
    recorder_size = 0;
    //清理回调
    _callback = nullptr;
    _ctx = nullptr;
    return SL_RESULT_SUCCESS;
}


OpenslesCore::~OpenslesCore()
{
    pthread_mutex_destroy(&_mutex);
}

void OpenslesCore::bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    OpenslesCore *ctx = static_cast<OpenslesCore *>(context);
    pthread_mutex_lock(&ctx->_mutex);
    //如果当前是录制状态
    if (ctx->recodering) {
        if (ctx->_callback)
            ctx->_callback(ctx->pcm_data, ctx->recorder_size, ctx->_ctx);
        //调用这个函数说明callback已经处理完毕，可以拿回缓存区存放下一次pcm数据了
        (*ctx->recorder_buffer_queue)->Enqueue(ctx->recorder_buffer_queue, ctx->pcm_data, ctx->recorder_size);
    }
    pthread_mutex_unlock(&ctx->_mutex);
}