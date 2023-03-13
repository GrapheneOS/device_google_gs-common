DEVICE_MANIFEST_FILE += device/google/gs-common/audio/aidl/manifest.xml

# Audio HALs
PRODUCT_PACKAGES += \
    android.hardware.audio.service-aidl.aoc

BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/audio/sepolicy/aidl

include device/google/gs-common/audio/common.mk

DEVICE_PRODUCT_COMPATIBILITY_MATRIX_FILE += device/google/gs-common/audio/aidl/device_framework_matrix_product.xml

PRODUCT_PROPERTY_OVERRIDES += \
       vendor.audio_hal.aidl.enable=true
