PRODUCT_PACKAGES += \
	android.hardware.drm-service.clearkey \

ifdef RELEASE_PACKAGE_WIDEVINE
  PRODUCT_PACKAGES += $(RELEASE_PACKAGE_WIDEVINE)
  ifneq ($(wildcard vendor/google/dev-keystore),)
    $(call soong_config_set,widevine,use_devkey,true)
  endif
  PRODUCT_VENDOR_LINKER_CONFIG_FRAGMENTS += \
	vendor/widevine/libwvdrmengine/apex/device/linker.config.json
else
  -include vendor/widevine/libwvdrmengine/apex/device/device.mk
endif