#!/vendor/bin/sh
path="/sys/devices/virtual/goog_touch_interface/gti.0"

echo "------ Force Touch Active ------"
echo 1 > $path/force_active

echo "------ Touch Firmware Version ------"
cat $path/fw_ver

echo "------ Get Mutual Sensing Data - Baseline ------"
cat $path/ms_base

echo "------ Get Mutual Sensing Data - Delta ------"
cat $path/ms_diff

echo "------ Get Mutual Sensing Data - Raw ------"
cat $path/ms_raw

echo "------ Get Self Sensing Data - Baseline ------"
cat $path/ss_base

echo "------ Get Self Sensing Data - Delta ------"
cat $path/ss_diff

echo "------ Get Self Sensing Data - Raw ------"
cat $path/ss_raw

echo "------ Self Test ------"
cat $path/self_test

echo "------ Disable Force Touch Active ------"
echo 0 > $path/force_active
