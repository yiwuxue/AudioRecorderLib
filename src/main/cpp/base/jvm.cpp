
#include "jvm.h"

void initGlobalJvm(JavaVM *jvm)
{
    if (!g_jvm)
        g_jvm = jvm;
}

JNIEnv *getEnv()
{
    return getEnv(nullptr);
}

/**
 * 如果其他线程加载了jvm，other_thread为true
 */
JNIEnv *getEnv(bool *other_thread)
{
    JNIEnv *env;
    //如果当前线程已经存在jni环境，直接获得JNIEnv
    if (g_jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        //线程加载到jvm后获取JNIEnv
        g_jvm->AttachCurrentThread(&env, nullptr);
        if (other_thread)
            *other_thread = true;
    }
    return env;
}

int detatchEnv()
{
    return g_jvm->DetachCurrentThread();
}