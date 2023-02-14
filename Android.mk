LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cpp .cc
LOCAL_MODULE    := ReSAMP
LOCAL_SRC_FILES := main.cpp samp.cpp mod/logger.cpp mod/config.cpp
FILE_LIST += $(wildcard $(LOCAL_PATH)/game/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/patch/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/pools/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/raknet/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/sampnet/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/ui/*.cpp)
LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_CFLAGS += -O2 -mfloat-abi=softfp -DNDEBUG -std=c++14 -Wno-deprecated-register
LOCAL_C_INCLUDES += ./include
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)
