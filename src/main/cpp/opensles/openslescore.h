
#ifndef __OPENSLESCORE__
#define __OPENSLESCORE__

#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>

#define RECORDER_FRAMES 2048

class OpenslesCore {

public:
    OpenslesCore();
    ~OpenslesCore();
    SLresult createEngine();
    SLresult createAudioRecord(SLuint32 numChannels, SLuint32 samplesPerSec , SLuint32 bitsPerSample);
    SLresult start();
    SLresult stop();
    SLresult release();
    void setAudioCaptureCallback(void (*callback)(int8_t *data, uint32_t len, void *ctx), void *ctx);
    bool isInitSuccess();
    bool isRecordering();

private:

    static void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

    void (*_callback)(int8_t *data, uint32_t len, void *ctx);
    void *_ctx;

    pthread_mutex_t _mutex;
    SLObjectItf engine_object;
    SLEngineItf engine_engine;
    SLObjectItf recorder_object;
    SLRecordItf recorder_recoder;
    SLAndroidSimpleBufferQueueItf recorder_buffer_queue;
    int8_t *pcm_data;
    bool recodering;
    bool init_success;
    uint32_t recorder_size;
};


#endif
