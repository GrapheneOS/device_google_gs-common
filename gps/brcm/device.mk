BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gps/brcm/sepolicy

PRODUCT_SOONG_NAMESPACES += vendor/broadcom/gps/bcm47765

SOONG_CONFIG_NAMESPACES += gpssdk
SOONG_CONFIG_gpssdk += sdkv1
SOONG_CONFIG_gpssdk_sdkv1 ?= false

SOONG_CONFIG_NAMESPACES += gpssdk
SOONG_CONFIG_gpssdk += gpsmcuversion
SOONG_CONFIG_gpssdk_gpsmcuversion ?= gpsv2_$(TARGET_BUILD_VARIANT)

PRODUCT_PACKAGES += \
	bcm47765_gps_package \
	sitril-gps \
	android.hardware.location.gps.prebuilt.xml

PRODUCT_PACKAGES_DEBUG += \
	init.gps_log.rc
