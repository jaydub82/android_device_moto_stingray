LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

file := $(TARGET_OUT_KEYLAYOUT)/cpcap-key.kl
ALL_PREBUILT += $(file)
$(file) : $(LOCAL_PATH)/cpcap-key.kl | $(ACP)
	$(transform-prebuilt-to-target)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := stingray-keypad.kcm
include $(BUILD_KEY_CHAR_MAP)

include $(CLEAR_VARS)

ifeq ($(TARGET_PREBUILT_KERNEL),)
TARGET_PREBUILT_KERNEL := $(LOCAL_PATH)/kernel
endif

file := $(INSTALLED_KERNEL_TARGET)
ALL_PREBUILT += $(file)
$(file): $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(transform-prebuilt-to-target)

include $(CLEAR_VARS)

target_hw_init_stingray_rc_file := $(TARGET_ROOT_OUT)/init.stingray.rc
target_hw_init_olympus_rc_file := $(TARGET_ROOT_OUT)/init.olympus.rc
target_hw_ueventd_stingray_rc_file := $(TARGET_ROOT_OUT)/ueventd.stingray.rc
target_hw_ueventd_olympus_rc_file := $(TARGET_ROOT_OUT)/ueventd.olympus.rc

$(target_hw_init_stingray_rc_file) : $(LOCAL_PATH)/init.stingray.rc | $(ACP)
	$(transform-prebuilt-to-target)
$(target_hw_init_olympus_rc_file) : $(LOCAL_PATH)/init.stingray.rc | $(ACP)
	$(transform-prebuilt-to-target)
$(target_hw_ueventd_stingray_rc_file) : $(LOCAL_PATH)/ueventd.stingray.rc | $(ACP)
	$(transform-prebuilt-to-target)
$(target_hw_ueventd_olympus_rc_file) : $(LOCAL_PATH)/ueventd.stingray.rc | $(ACP)
	$(transform-prebuilt-to-target)

ALL_PREBUILT += $(target_hw_init_stingray_rc_file) \
                $(target_hw_init_olympus_rc_file) \
                $(target_hw_ueventd_stingray_rc_file) \
                $(target_hw_ueventd_olympus_rc_file)

include $(CLEAR_VARS)

COMMON_DIR := vendor/nvidia/common/

ifeq ($(wildcard $(COMMON_DIR)/TegraBoard.mk),$(COMMON_DIR)/TegraBoard.mk)
include $(COMMON_DIR)/TegraBoard.mk
endif

subdir_makefiles:= \
	$(LOCAL_PATH)/ril/Android.mk \
	$(LOCAL_PATH)/libaudio/Android.mk \
	$(LOCAL_PATH)/taudio/Android.mk

include $(subdir_makefiles)

-include vendor/moto/stingray/AndroidBoardVendor.mk
