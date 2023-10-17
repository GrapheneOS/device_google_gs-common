ifeq (,$(filter factory_%,$(TARGET_PRODUCT)))
PRODUCT_PACKAGES += battery_mitigation
endif
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
ifeq (,$(filter factory_%,$(TARGET_PRODUCT)))
PRODUCT_PACKAGES += BrownoutDetection
endif
endif

BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/battery_mitigation/sepolicy/vendor
SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += device/google/gs-common/battery_mitigation/sepolicy/system_ext/private
SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += device/google/gs-common/battery_mitigation/sepolicy/system_ext/public
PRODUCT_SOONG_NAMESPACES += device/google/gs-common/battery_mitigation
