
#ifndef __AUDIO_RECORD__
#define __AUDIO_RECORD__

#include <baserecord.h>
#include <base/jvm.h>


class AudioRecord : public BaseRecord {

public:
    AudioRecord();
    ~AudioRecord();
    int init(uint32_t sample_rate, uint16_t channel_config, uint16_t audio_format);
    int getState();
    void start();
    void stop();
    void release();
    void setAudioCaptureCallback(void (*callback)(int8_t *data, uint32_t len, void *ctx), void *ctx);
    void readData(int8_t *data, uint32_t size);

private:
    jclass class_audio;
    jmethodID method_start;
    jmethodID method_stop;
    jmethodID method_release;
    jmethodID method_get_state;
    jmethodID method_init;
    jmethodID method_set_native_callback;
    jmethodID method_object;
    jobject object_audio;

    void(* _callback)(int8_t *data, uint32_t len, void *ctx);
    void *_ctx;
};

#endif

