LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(BOARD_GPS_MODULE), GNS7560)

LOCAL_SRC_FILES := ../gps.c \
  gps_nxp.c \
	gn_api_call.c \
	gn_main.c \
	GN_GPS_GNB_Patch.c \
	GN_GPS_GNB_Patch_510.c \
	GN_GPS_DataLogs.c
	
LOCAL_MODULE := gps.amlogic
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
#	hardware\libhardware_legacy

LOCAL_LDFLAGS = $(LOCAL_PATH)/GPS_Lib_Static_Cupcake_leader.a

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

include $(BUILD_SHARED_LIBRARY)
endif
