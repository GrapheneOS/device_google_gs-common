ifneq ($(TARGET_BUILD_VARIANT), user)
  ifeq (,$(filter aosp_%, $(TARGET_PRODUCT))) # Skip aosp target product
    PRODUCT_PACKAGES_DEBUG += \
      Gyotaku

      BOARD_SEPOLICY_DIRS += device/google/gs-common/gyotaku_app/sepolicy/
  endif
endif
