PRODUCT_PACKAGES += \
	android.hardware.drm-service.clearkey \

ifdef RELEASE_PACKAGE_WIDEVINE
  PRODUCT_PACKAGES += $(RELEASE_PACKAGE_WIDEVINE)
  PRODUCT_VENDOR_LINKER_CONFIG_FRAGMENTS += \
	vendor/widevine/libwvdrmengine/apex/device/linker.config.json
else
  -include vendor/widevine/libwvdrmengine/apex/device/device.mk
endif