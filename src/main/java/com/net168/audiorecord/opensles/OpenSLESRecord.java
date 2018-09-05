package com.net168.audiorecord.opensles;

import com.net168.audiorecord.BaseRecord;
import com.net168.audiorecord.OnAudioCaptureCallback;
import com.orhanobut.logger.Logger;

import java.nio.ByteBuffer;

/**
 * 基于OpenSL ES封装的录制api类
 * 提供了openslesrecord.cpp的封装实现
 * java层：提供了统一的对外接口api
 */

public class OpenSLESRecord implements BaseRecord {

    private long mNativeInstance = 0;   //native层 openslesrecord 实例的引用内存地址
    private OnAudioCaptureCallback mCallback;
    private byte[] mData;  //从native接收pcm数据缓存区

    @Override
    public boolean init(int sampleRate, int channelConfig, int audioFormat) {
        mNativeInstance = _initRecord(sampleRate, channelConfig, audioFormat);
        if (mNativeInstance == 0) {
            Logger.e("OpenslesRecord creat error");
        }
        return mNativeInstance != 0;
    }

    @Override
    public void start() {
        if (mNativeInstance == 0) {
            Logger.e("OpenslesRecord not exit");
            return;
        }
        _start(mNativeInstance);
    }

    @Override
    public void stop() {
        if (mNativeInstance == 0) {
            Logger.e("OpenslesRecord not exit");
            return;
        }
        _stop(mNativeInstance);
    }

    @Override
    public void release() {
        if (mNativeInstance == 0) {
            Logger.e("OpenslesRecord not exit");
            return;
        }
        _release(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    public int getState() {
        if (mNativeInstance == 0) {
            Logger.e("OpenslesRecord not exit");
            return STATE_UNINIT;
        }
        return _getState(mNativeInstance);
    }

    @Override
    public void setAudioCaptureCallback(OnAudioCaptureCallback callback) {
        if (mNativeInstance == 0) {
            Logger.e("OpenslesRecord not exit");
            return;
        }
        mCallback = callback;
        _setNativeCallback(mNativeInstance, callback != null);
    }

    //forward to openslesrecord.cpp # Java_com_net168_audiorecord_opensles_OpenSLESRecord__1initRecord
    private native long _initRecord(int sampleRate, int channelConfig, int audioFormat);
    //forwar to openslesrecord.cpp # Java_com_net168_audiorecord_opensles_OpenSLESRecord__1stop
    private native void _stop(long instance);
    //forwar to openslesrecord.cpp # Java_com_net168_audiorecord_opensles_OpenSLESRecord__1start
    private native void _start(long instance);
    //forwar to openslesrecord.cpp # Java_com_net168_audiorecord_opensles_OpenSLESRecord__1release
    private native void _release(long instance);
    //forwar to openslesrecord.cpp # Java_com_net168_audiorecord_opensles_OpenSLESRecord__1getState
    private native int _getState(long instance);
    //forwar to openslesrecord.cpp # Java_com_net168_audiorecord_opensles_OpenSLESRecord__1setNativeCallback
    private native void _setNativeCallback(long instance, boolean need);

    /**
     * 提供native层回调使用，传出nio接口的bytebuffer数据
     */
    private void onNativeDataRead(ByteBuffer buffer) {
        if (mCallback != null) {
            //获取返回的size大小，一般来说是固定一个数
            int size = buffer.capacity();
            //如果数据缓存区未初始化或者空间不够存放则重新申请一个缓存空间
            if (mData == null || mData.length < size)
                mData = new byte[size];
            //从NIO获取PCM数据
            buffer.get(mData, 0, size);
            mCallback.onPCMDataAvailable(mData, size);
        }
    }
}
