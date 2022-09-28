#!/vendor/bin/sh
# F2FS
echo "--- F2FS status---"
cat "/sys/kernel/debug/f2fs/status"
echo "\n--- F2FS fsck ---"
cat "/dev/fscklogs/fsck"
echo "\n--- F2FS - fsck time (ms) ---"
getprop ro.boottime.init.fsck.data
echo "\n--- F2FS - checkpoint=disable time (ms) ---"
getprop ro.boottime.init.mount.data
