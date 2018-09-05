
#ifndef __BASE_RECORD__
#define __BASE_RECORD__

#include <cstdint>

class BaseRecord {

public:
    virtual int init(uint32_t sample_rate, uint16_t channel_config, uint16_t audio_format) = 0;
    virtual int getState() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void release() = 0;
    virtual void setAudioCaptureCallback(void (*callback)(int8_t *data, uint32_t len, void *ctx), void *ctx) = 0;

    BaseRecord() {}
};

#endif
