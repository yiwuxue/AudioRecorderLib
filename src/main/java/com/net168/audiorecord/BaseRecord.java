package com.net168.audiorecord;

public interface BaseRecord {

    int STATE_UNINIT =     0;
    int STATE_IDLE =       1;
    int STATE_RECORDING =  2;

    /**
     * 初始化录制器
     * @param sampleRate 采样频率 参数见AudioCapture
     * @param channelConfig 采样声道摸个 参数见AudioCapture
     * @param audioFormat 采样格式 参数见AudioCapture
     * @return 是否初始化成功
     */
    boolean init(final int sampleRate, final int channelConfig, final int audioFormat);

    /**
     * 开始录制器工作
     */
    void start();

    /**
     * 暂停录制器工作
     */
    void stop();

    /**
     * 释放录制器资源，重新start()需要先init()
     */
    void release();

    /**
     * 获取录制器当前状态
     * @return 参看AudioCapture
     */
    int getState();

    /**
     * 设置录制器PCM数据回调
     */
    void setAudioCaptureCallback(OnAudioCaptureCallback callback);

}
