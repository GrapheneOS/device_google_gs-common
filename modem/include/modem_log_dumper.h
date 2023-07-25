#pragma once

#include "android_property_manager.h"
#include "dumper.h"

namespace modem {
namespace logging {

/**
 * @brief Responsible for dumping all relevant modem logs.
 */
class ModemLogDumper {
 public:
  ModemLogDumper(Dumper& dumper,
                 AndroidPropertyManager& android_property_manager)
      : dumper_(dumper), android_property_manager_(android_property_manager){};

  /**
   * @brief Dumps modem related logs and persistent files to bugreport.
   *
   * If PILOT and On Demand Logging are both not enabled, this method will
   * attempt to stop modem logging, copy over the logs, and then restart so that
   * the original logging enabled / disabled state is preserved. Additionally,
   * all directories specified in `kLogDumpInfo` and all files in
   * `kFileCopyInfo` will be included.
   */
  void DumpModemLogs();

 private:
  /**
   * @brief Checks modem logging status property to assert if logging is
   * running or not.
   */
  bool isModemLoggingRunning();

  /**
   * @brief Checks if On Demand Logging or PILOT Logging is enabled.
   *
   * If either of them are enabled, then the `log_path` property will no longer
   * point to the always on logging directory.
   */
  bool allowedToStopModemLogging();

  /**
   * @brief Stops modem logging.
   *
   * This sets the modem logging property which in turn triggers
   * modem_logging_control's modem_logging_stop service. Modem logging isn't
   * guaranteed to have stopped after this call, so it's necessary to poll the
   * status property to ensure it's stopped before proceeding.
   */
  void stopModemLogging();

  /**
   * @brief Polls modem logging status property to ensure modem logging has
   * stopped.
   *
   * Even after the property is confirmed to be false, it will continue to
   * sleep for a second to ensure that the modem_logging_stop service has exited
   * properly.
   */
  void waitForStopModemLogging();

  /**
   * @brief Starts modem logging.
   *
   * This sets the modem logging property which in turn triggers
   * modem_logging_control's modem_logging_start service. Modem logging isn't
   * guaranteed to have started after this call, so it's necessary to poll the
   * status property to ensure it's started before proceeding to guarantee
   * success.
   */
  void startModemLogging();

 private:
  Dumper& dumper_;
  AndroidPropertyManager& android_property_manager_;
};

}  // namespace logging
}  // namespace modem
