ifeq (,$(filter factory_%,$(TARGET_PRODUCT)))
PRODUCT_PACKAGES += battery_mitigation
endif
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
ifeq (,$(filter factory_%,$(TARGET_PRODUCT)))
PRODUCT_PACKAGES += BrownoutDetection
endif
endif

BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/battery_mitigation/sepolicy
PRODUCT_SOONG_NAMESPACES += device/google/gs-common/battery_mitigation
