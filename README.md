# 功能简介
提供了基于AudioRecord和OpenSL ES两种框架的采集功能的统一封装。  
可以指定采集的频率（例如44.1khz）、采集的声道数（例如单声道MONO）、采集的格式（例如16bit）。  
输出PCM原始数据，基于Lib库的组合实现，这个Lib不提供编码功能。  
验证PCM数据可以通过工具Audacity等查看声波图或者播放。  

提供了基于JAVA层和基于Native层的两套使用API。  
由于需要提供so库在cmake或者ndk build进行链接，所以没有提供JCenter库版本。  

# 接入方法
进入你的主工程目录下，clone这个lib
```
git clone https://github.com/net168/AudioRecorderLib.git
```
## 仅需要在java层api使用
在项目settings.gradle加入（AudioRecorderLib取决你lib的文件名，默认AudioRecorderLib）
```
include ':AudioRecorderLib'
```
在主工程build.gradle中加入
```
implementation project(':AudioRecorderLib')
```

## 如果还需要在native层使用（cmake方式）
在前面java层引入的基础上，还需要处理如下  
修改你的主工程CMakeList.txt文件，添加如下内容  
```
#设置lib工程在主工程的项目位置
set(audiorecord_DIR ${CMAKE_SOURCE_DIR}/../AudioRecorderLib)
#加载lib需要暴露的头文件
include_directories(${audiorecord_DIR}/src/main/cpp/include)
#导入lib工程的so库
add_library(audiorecord SHARED IMPORTED)
set_target_properties(audiorecord PROPERTIES IMPORTED_LOCATION
                      ${audiorecord_DIR}/libs/${ANDROID_ABI}/libaudiorecord.so)
#将lib的so连接到你的native开发工程中
target_link_libraries(
                       your_native-lib
                       audiorecord
                     )
```

# Java层的使用方法
```
//开启录音
mAudioCapture = new AudioCapture(AudioCapture.AUDIO_CAPTURE_TYPE_OPENSLES, AudioCapture.AUDIO_SAMPLE_RATE_44_1,
                AudioCapture.AUDIO_CHANNEL_MONO, AudioCapture.AUDIO_FORMAT_PCM_16BIT);
        if (mAudioCapture.getState() == AudioCapture.STATE_IDLE) {
            mAudioCapture.setAudioCaptureCallback(new OnAudioCaptureCallback());
            mAudioCapture.startRecording();
        }
        else {
            mAudioCapture.releaseRecording();
        }
//关闭录音
mAudioCapture.stopRecording();
mAudioCapture.releaseRecording();
//PCM数据回调
public void onPCMDataAvailable(byte[] data, int size) {
  //TODO
}
```

# Native层的使用方法
```
//包含头文件
#include <audiocapture.h>
//开始录制
capture = new AudioCapture(AUDIO_CAPTURE_TYPE_OPENSLES, AUDIO_SAMPLE_RATE_44_1, AUDIO_CHANNEL_MONO, AUDIO_FORMAT_PCM_16BIT);
if (capture->getState() == STATE_IDEL)
{
   capture->setAudioCaptureCallback(readData, this);
   capture->startRecording();
}
else
{
   capture->releaseRecording();
   delete capture;
   capture = nullptr;
}
//结束录制
capture->stopRecording();
capture->releaseRecording();
delete capture;
capture = nullptr;
//PCM数据回调
void static readData(int8_t *data, uint32_t size, void *ctx)
{
  //TODO
}
```

# 接入工程示例
请参考 [AndroidAudioRecordDemo](https://github.com/net168/AndroidAudioRecordDemo)
