package com.net168.audiorecord;

import com.net168.audiorecord.audiorecord.AudioRecordRecord;
import com.net168.audiorecord.opensles.OpenSLESRecord;

/**
 * 对外统一接口
 * sample:
 * mAudioCapture = new AudioCapture(AudioCapture.AUDIO_CAPTURE_TYPE_AUDIORECORD, AudioCapture.AUDIO_SAMPLE_RATE_44_1,
 * AudioCapture.AUDIO_CHANNEL_MONO, AudioCapture.AUDIO_FORMAT_PCM_16BIT);
 * if (mAudioCapture.getState() == AudioCapture.STATE_IDLE) {
 *      mAudioCapture.setAudioCaptureCallback(new OnAudioCaptureCallback());
 *      mAudioCapture.startRecording();
 * }
 * else {
 *      mAudioCapture.releaseRecording();
 * }
 *
 * mAudioCapture.stopRecording();
 * mAudioCapture.releaseRecording();
 */


public class AudioCapture {

    /**
     * 录制内核类型，支持OpenSL ES 和 AudioRecord
     */
    public static final int AUDIO_CAPTURE_TYPE_OPENSLES =       0x001;
    public static final int AUDIO_CAPTURE_TYPE_AUDIORECORD =    0x002;

    /**
     * 音频声道类型  MONO-单声道   STEREO-双声道
     */
    public static final int AUDIO_CHANNEL_MONO =                0x201;
    public static final int AUDIO_CHANNEL_STEREO =              0x202;

    /**
     * 采样格式  16bit为android最通用的采样格式
     */
    public static final int AUDIO_FORMAT_PCM_16BIT =           0x301;
    public static final int AUDIO_FORMAT_PCM_8BIT =            0x302;
    public static final int AUDIO_FORMAT_PCM_FLOAT =           0x303;

    /**
     * 采样频率
     */
    public static final int AUDIO_SAMPLE_RATE_8 =              8000;
    public static final int AUDIO_SAMPLE_RATE_11_025 =         11025;
    public static final int AUDIO_SAMPLE_RATE_12 =             12000;
    public static final int AUDIO_SAMPLE_RATE_16 =             16000;
    public static final int AUDIO_SAMPLE_RATE_22_05 =          22050;
    public static final int AUDIO_SAMPLE_RATE_24 =             24000;
    public static final int AUDIO_SAMPLE_RATE_32 =             32000;
    public static final int AUDIO_SAMPLE_RATE_44_1 =           44100;
    public static final int AUDIO_SAMPLE_RATE_48 =             48000;
    public static final int AUDIO_SAMPLE_RATE_64 =             64000;
    public static final int AUDIO_SAMPLE_RATE_82 =             82000;
    public static final int AUDIO_SAMPLE_RATE_96 =             96000;
    public static final int AUDIO_SAMPLE_RATE_192 =            192000;

    /**
     * 录制器状态
     */
    public static final int STATE_UNINIT =       BaseRecord.STATE_UNINIT;
    public static final int STATE_IDLE =         BaseRecord.STATE_IDLE;
    public static final int STATE_RECORDING =    BaseRecord.STATE_RECORDING;

    private BaseRecord mAudioRecord;

    public AudioCapture(final int captureType, final int sampleRate, final int channelConfig,
                        final int audioFormat) {
        if (captureType != AUDIO_CAPTURE_TYPE_OPENSLES && captureType != AUDIO_CAPTURE_TYPE_AUDIORECORD)
            throw new IllegalStateException("the captureType value no support");
        mAudioRecord = captureType == AUDIO_CAPTURE_TYPE_AUDIORECORD ? new AudioRecordRecord() : new OpenSLESRecord();
        mAudioRecord.init(sampleRate, channelConfig, audioFormat);
    }

    public void setAudioCaptureCallback(OnAudioCaptureCallback callback) {
        mAudioRecord.setAudioCaptureCallback(callback);
    }

    public int getState() {
        return mAudioRecord.getState();
    }

    public void startRecording() {
        mAudioRecord.start();
    }

    public void stopRecording() {
        mAudioRecord.stop();
    }

    public void releaseRecording() {
        mAudioRecord.release();
    }

}
