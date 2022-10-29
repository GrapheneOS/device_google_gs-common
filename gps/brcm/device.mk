BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gps/brcm/sepolicy
PRODUCT_COPY_FILES += \
	device/google/gs-common/gps/brcm/firmware/SensorHub.patch:$(TARGET_COPY_OUT_VENDOR)/firmware/SensorHub.patch

PRODUCT_PACKAGES += \
	android.hardware.gnss@2.1-impl-google \
	gps.default \
	flp.default \
	gpsd \
	lhd \
	scd \
	android.hardware.gnss@2.1-service-brcm \
	android.hardware.location.gps.prebuilt.xml

PRODUCT_PACKAGES_DEBUG += \
	init.gps_log.rc
