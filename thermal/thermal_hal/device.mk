PRODUCT_PACKAGES += android.hardware.thermal-service.pixel

# Thermal utils
PRODUCT_PACKAGES += thermal_symlinks

# Thermal logd
PRODUCT_PACKAGES_DEBUG += thermal_logd
PRODUCT_PACKAGES_ENG += thermal_logd

BOARD_SEPOLICY_DIRS += device/google/gs-common/thermal/sepolicy/thermal_hal
