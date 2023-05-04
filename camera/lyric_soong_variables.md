# Lyric Soong variables

## `kernel_version`

Example:
```
$(call soong_config_set,lyric,kernel_version,v515)
```
Linux kernel version on the device. Determines the version of the LWIS userspace
API to use in Lyric.

## `soc`

Example:
```
$(call soong_config_set,lyric,soc,gs101)
```
The codename for the chip used in the device. Currently only Google silicon
is supported.

## `camera_hardware`

Example:
```
$(call soong_config_set,lyric,camera_hardware,oriole)
```
Specifies the set of sensors and peripherals present on the device. Determines
the DeviceContext class that will be used.

## `tuning_product`

Example:
```
$(call soong_config_set,lyric,tuning_product,oriole)
```
Specifies the set of tuning data to use. This may be different than
`camera_hardware` because not all devices have their own tuning data;
development-only devices only have the default tuning for their SoC.

## `target_device`

Example:
```
$(call soong_config_set,google3a_config,target_device,oriole)
```
A mixture of `camera_hardware` and `tuning_product` used by 3A.
