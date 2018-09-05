
#ifndef __OPENSLES_RECORD__
#define __OPENSLES_RECORD__

#include <opensles/openslescore.h>
#include <baserecord.h>
#include <audiocapture.h>
#include <base/jvm.h>
#include <base/log.h>

typedef struct _OpenslJniCallback {
    jmethodID method_data_callback;
    jobject java_object;
} OpenslJniCallback;

class OpenslesRecord : public BaseRecord {

public:
    OpenslesRecord();
    ~OpenslesRecord();
    int init(uint32_t sample_rate, uint16_t channel_config, uint16_t audio_format);
    int getState();
    void start();
    void stop();
    void release();
    void setAudioCaptureCallback(void (*callback)(int8_t *data, uint32_t len, void *ctx), void *ctx);

    static void onDataRead(int8_t *data, uint32_t len, void *ctx);

    OpenslJniCallback *jniData;
private:

    static SLresult getSampleRate(uint32_t sample_rate);
    static SLresult getChannelNum(uint16_t channel_config);
    static SLresult getAudioFormat(uint16_t audio_format);

    OpenslesCore *core;

};


#endif


