PRODUCT_PACKAGES += \
	android.hardware.boot-service.default-pixel \
	android.hardware.boot-service.default_recovery-pixel

PRODUCT_SOONG_NAMESPACES += device/google/gs-common/bootctrl/aidl
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/bootctrl/sepolicy/aidl
