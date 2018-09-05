package com.net168.audiorecord.audiorecord;

import android.media.AudioFormat;

import com.net168.audiorecord.AudioCapture;
import com.net168.audiorecord.BaseRecord;
import com.net168.audiorecord.OnAudioCaptureCallback;
import com.orhanobut.logger.Logger;

import java.nio.ByteBuffer;

/**
 * 基于AudioRecord封装的录制api类
 * 统一封装了AudioRecordCore的录制功能
 * java层：提供了统一的对外接口api
 * native层：提供了audiorecord.cpp的封装实现
 */

public class AudioRecordRecord implements BaseRecord {

    private AudioRecordCore mCore;

    public AudioRecordRecord() {
    }

    @Override
    public boolean init(int sampleRate, int channelConfig, int audioFormat) {
        //防止多次初始化操作
        if (mCore != null) {
            Logger.w("AudioRecordCore already init");
            return true;
        }
        mCore = new AudioRecordCore();
        boolean result = mCore.createRecord(getAudioRecordSampleRate(sampleRate), getAudioRecordChannelConfig(channelConfig), getAudioRecordAudioFormat(audioFormat));
        //如果createRecord不成功，认为初始化失败
        if (!result) {
            Logger.e("AudioRecordCore create record error");
            mCore.releaseRecord();
            mCore = null;
        }
        return result;
    }

    @Override
    public void start() {
        if (mCore == null) {
            Logger.e("AudioRecordCore not init");
            return;
        }
        mCore.startRecord();
    }

    @Override
    public void stop() {
        if (mCore == null) {
            Logger.e("AudioRecordCore not init");
            return;
        }
        mCore.stopRecord();
    }

    @Override
    public void release() {
        if (mCore == null) {
            Logger.e("AudioRecordCore not init");
            return;
        }
        mCore.releaseRecord();
    }

    @Override
    public void setAudioCaptureCallback(OnAudioCaptureCallback callback) {
        if (mCore == null) {
            Logger.e("AudioRecordCore not init");
            return;
        }
        mCore.setOnAudioCaptureCallback(callback);
    }

    @Override
    public int getState() {
        int state = STATE_UNINIT;
        if (mCore == null)
            state = STATE_UNINIT;
        else if (mCore.isRecording())
            state = STATE_RECORDING;
        else if (mCore.isInitSuccess())
            state = STATE_IDLE;
        return state;
    }

    /**
     * 将AudioCapture的采样频率转为AudioRecord支持的采样频率格式
     */
    private static int getAudioRecordSampleRate(final int sampleRate) {
        if (sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_8 || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_11_025
                || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_12 || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_16
                || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_22_05 || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_24
                || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_32 || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_44_1
                || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_48 || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_64
                || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_82 || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_96
                || sampleRate == AudioCapture.AUDIO_SAMPLE_RATE_192)
            return sampleRate;
        else
            return AudioCapture.AUDIO_SAMPLE_RATE_16;
    }

    /**
     * 将AudioCapture的声道配置转为AudioRecord支持的声道配置格式
     */
    private static int getAudioRecordChannelConfig(final int channelConfig) {
        int result;
        switch (channelConfig) {
            case AudioCapture.AUDIO_CHANNEL_MONO:
                result = AudioFormat.CHANNEL_IN_MONO;
                break;
            case AudioCapture.AUDIO_CHANNEL_STEREO:
                result = AudioFormat.CHANNEL_IN_STEREO;
                break;
            default:
                result = AudioFormat.CHANNEL_IN_MONO;
        }
        return result;
    }

    /**
     * 将AudioCapture的采样格式转为AudioRecord支持的采样格式
     */
    private static int getAudioRecordAudioFormat(final int audioFormat) {
        int result;
        switch (audioFormat) {
            case AudioCapture.AUDIO_FORMAT_PCM_8BIT:
                result = AudioFormat.ENCODING_PCM_8BIT;
                break;
            case AudioCapture.AUDIO_FORMAT_PCM_16BIT:
                result = AudioFormat.ENCODING_PCM_16BIT;
                break;
            case AudioCapture.AUDIO_FORMAT_PCM_FLOAT:
                result = AudioFormat.ENCODING_PCM_FLOAT;
                break;
            default:
                result = AudioFormat.ENCODING_PCM_16BIT;
        }
        return result;
    }


    /************************************ native 层封装 *****************************************/
    private long mInstance;        //native层 audiorecord 实例的引用内存地址
    private ByteBuffer mBuffer;     //NIO:提供高效的java与jni的内存数据交换

    /**
     * 仅提供jni调用，初始化一个实例
     */
    private AudioRecordRecord(long instance) {
        mInstance = instance;
    }

    /**
     * 仅提供jni调用，设置一个PCM数据回调，后续会将数据发送会native
     * @param run native是否需要设置监听
     */
    private void setNativeCallback(boolean run) {
        if (run) {
            setAudioCaptureCallback(new OnAudioCaptureCallback() {
                @Override
                public void onPCMDataAvailable(byte[] data, int size) {
                    if (mBuffer == null) {
                        //由于是native与java的内存交换，所以不能使用
                        //ByteBuffer.wrap()生成buffer，具体请参考DirectByteBuffer与HeapByteBuffer的区别
                        mBuffer = ByteBuffer.allocateDirect(mCore.getMaxBufferSize());
                    }
                    //由于mBuffer持续复用，所以这里回复到正常状态
                    mBuffer.clear();
                    //将数据导入mBuffer中，注意size不一定等于data.length
                    //不能直接mBuffer.put(data),会有脏数据
                    mBuffer.put(data, 0 ,size);
                    //将写入状态变更为读取状态
                    mBuffer.flip();
                    //将数据发送到Native层业务
                    sendDataToNative(mInstance, mBuffer);
                }
            });
        }
        else {
            //native层取消pcm回调
            setAudioCaptureCallback(null);
        }
    }

    /**
     * 将ByteBuffer发送回native层
     */
    private native void sendDataToNative(long instance, ByteBuffer data);

}