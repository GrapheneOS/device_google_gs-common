#!/vendor/bin/sh

WAKEUP_EVENTS_FILE=/sys/devices/platform/cpif/wakeup_events
CPIF_LOGBUFFER=/dev/logbuffer_cpif
PCIE_EVENT_STATS=/sys/devices/platform/cpif/modem/pcie_event_stats

echo "------ Modem Stat ------"
cat /data/vendor/modem_stat/debug.txt

echo "\n------ Modem SSR history ------"
for f in $(ls /data/vendor/ssrdump/crashinfo_modem*); do
  echo $f
  cat $f
done

echo "\n------ RFSD error log ------"
for f in $(ls /data/vendor/log/rfsd/rfslog_*); do
  echo $f
  cat $f
done

if [ -e $WAKEUP_EVENTS_FILE ]
then
  echo "\n------ Wakeup event counts ------"
  echo $WAKEUP_EVENTS_FILE
  cat $WAKEUP_EVENTS_FILE
fi

if [ -e $CPIF_LOGBUFFER ]
then
  echo "\n------ CPIF Logbuffer ------"
  echo $CPIF_LOGBUFFER
  cat $CPIF_LOGBUFFER
fi

if [ -e $PCIE_EVENT_STATS ]
then
  echo "\n------ PCIe event stats ------"
  echo $PCIE_EVENT_STATS
  cat $PCIE_EVENT_STATS
fi
