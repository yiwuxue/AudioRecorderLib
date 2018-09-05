
#ifndef __JVM__
#define __JVM__

#include <jni.h>

static JavaVM *g_jvm;

void initGlobalJvm(JavaVM *jvm);
JNIEnv *getEnv();
JNIEnv *getEnv(bool *);
int detatchEnv();

#endif
