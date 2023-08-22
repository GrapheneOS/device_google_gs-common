ifneq ($(TARGET_BUILD_VARIANT), user)
  PRODUCT_PACKAGES_DEBUG += \
      Gyotaku

  BOARD_SEPOLICY_DIRS += device/google/gs-common/gyotaku_app/sepolicy/
endif
