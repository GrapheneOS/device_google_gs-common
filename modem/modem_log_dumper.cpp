#include "modem_log_dumper.h"

#include <log/log.h>

#include "dumper.h"
#include "modem_log_constants.h"

namespace modem {
namespace logging {

void ModemLogDumper::DumpModemLogs() {
  bool shouldRestartModemLogging =
      allowedToStopModemLogging() && isModemLoggingRunning();
  int maxFileNum = android_property_manager_.GetIntProperty(
      kModemLoggingNumberBugreportProperty.data(),
      kDefaultBugreportNumberFiles);

  if (shouldRestartModemLogging) {
    // If modem logging is running at time of bugreport, it needs to be stopped
    // to ensure that the most recent logs are included in the bugreport. If
    // this command fails, only older log files will be included, as seen in
    // b/289435256.
    stopModemLogging();
    waitForStopModemLogging();
  } else {
    ALOGD("modem logging is not running\n");
  }

  dumper_.DumpLogs({kModemAlwaysOnLogDirectory, kBugreportPackingDirectory,
                    maxFileNum, kModemLogPrefix});

  if (shouldRestartModemLogging) {
    startModemLogging();
  }

  for (const LogDumpInfo& log_dump_info : kLogDumpInfo) {
    dumper_.DumpLogs(log_dump_info);
  }

  for (const FileCopyInfo& file_copy_info : kFileCopyInfo) {
    dumper_.CopyFile(file_copy_info);
  }
};

bool ModemLogDumper::isModemLoggingRunning() {
  return android_property_manager_.GetBoolProperty(
      kModemLoggingStatusProperty.data(), false);
}

bool ModemLogDumper::allowedToStopModemLogging() {
  return android_property_manager_.GetProperty(kModemLoggingPathProperty.data(),
                                               /*default_value=*/"") ==
         kModemAlwaysOnLogDirectory;
}

void ModemLogDumper::stopModemLogging() {
  android_property_manager_.SetProperty(kModemLoggingEnabledProperty.data(),
                                        "false");
  ALOGD("Stopping modem logging...\n");
}

void ModemLogDumper::waitForStopModemLogging() {
  // TODO(b/289582966) improve stop logging mechanism to not use sleep
  for (int i = 0; i < 15; i++) {
    if (!isModemLoggingRunning()) {
      ALOGD("modem logging stopped\n");
      sleep(1);
      break;
    }
    sleep(1);
  }
}

void ModemLogDumper::startModemLogging() {
  ALOGD("Restarting modem logging...\n");
  android_property_manager_.SetProperty(kModemLoggingEnabledProperty.data(),
                                        "true");
}
}  // namespace logging
}  // namespace modem
