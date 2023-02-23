BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gps/brcm/sepolicy

PRODUCT_SOONG_NAMESPACES += vendor/broadcom/gps/bcm47765

PRODUCT_PACKAGES += \
	bcm47765_gps_package_v2 \
	android.hardware.location.gps.prebuilt.xml

PRODUCT_PACKAGES_DEBUG += \
	init.gps_log.rc
