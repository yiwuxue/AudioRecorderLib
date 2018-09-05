package com.net168.audiorecord;

public interface OnAudioCaptureCallback {
    void onPCMDataAvailable(byte[] data, int size);
}
