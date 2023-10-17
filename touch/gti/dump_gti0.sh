#!/vendor/bin/sh
path="/sys/devices/virtual/goog_touch_interface/gti.0"
procfs_path="/proc/goog_touch_interface/gti.0"

if [[ -d "$procfs_path" ]]; then
heatmap_path=$procfs_path
else
heatmap_path=$path
fi

echo "------ Force Touch Active ------"
result=$( cat "$path/force_active" 2>&1 )
if [ $? -eq 0 ]; then
    state=$( echo "$result" |cut -d " " -f 2 )
    if [ "$state" = "locked" ]; then
        echo "The force_active is already locked!"
    else
        echo 1 > $path/force_active
        if [ $? -ne 0  ]; then
            echo "Failed to active touch device"
            exit 1
        fi
    fi
else
    if [[ $result == *Operation\ not\ supported* ]]; then
        echo "force_active is not support, skip it"
    else
        echo "Failed to read the state of force_force"
        exit 1
    fi
fi

echo "------ Touch Firmware Version ------"
cat $path/fw_ver

echo "------ Panel ID ------"
cat $path/panel_id

echo "------ Offload ID ------"
cat $path/offload_id

echo "------ Get Mutual Sensing Data - Baseline ------"
cat $heatmap_path/ms_base

echo "------ Get Mutual Sensing Data - Delta ------"
cat $heatmap_path/ms_diff

echo "------ Get Mutual Sensing Data - Raw ------"
cat $heatmap_path/ms_raw

echo "------ Get Self Sensing Data - Baseline ------"
cat $heatmap_path/ss_base

echo "------ Get Self Sensing Data - Delta ------"
cat $heatmap_path/ss_diff

echo "------ Get Self Sensing Data - Raw ------"
cat $heatmap_path/ss_raw

echo "------ Self Test ------"
cat $path/self_test

if [[ -f "${procfs_path}/dump" ]]; then
  echo "------ Dump ------"
  cat ${procfs_path}/dump
fi

echo "------ Disable Force Touch Active ------"
echo 0 > $path/force_active
