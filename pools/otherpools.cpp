#include "otherpools.h"
#include <mod/logger.h>
#include <jni.h>

JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_multiTouchEvent(JNIEnv* env, jobject* thiz)
{
    logger->Info("multiTouchEvent");
    
    return (jboolean)true;
}
