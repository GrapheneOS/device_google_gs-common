#!/vendor/bin/sh
path=`getprop ro.vendor.touch.dump.sys`

echo "Enable Force Touch Active"
echo 21 > $path/force_active

echo "Touch Firmware Information"
cat $path/info

echo "Get Raw Data - Delta"
echo 12 > $path/get_raw_data && cat $path/get_raw_data

echo "Get Raw Data - Raw"
echo 13 > $path/get_raw_data && cat $path/get_raw_data

echo "Get Raw Data - Baseline"
echo 14 > $path/get_raw_data && cat $path/get_raw_data

echo "Disable Force Touch Active"
echo 20 > $path/force_active

