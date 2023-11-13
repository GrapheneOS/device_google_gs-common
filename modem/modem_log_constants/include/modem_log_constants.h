#pragma once

#include <string_view>

namespace pixel_modem::logging {

// Modem related Android System Properties

// Controls triggering `modem_logging_start` and `modem_logging_stop`.
inline constexpr std::string_view kModemLoggingEnabledProperty =
    "vendor.sys.modem.logging.enable";
// Signals the current modem logging state. This will be set to
// `vendor.sys.modem.logging.enable` when `modem_log_start` or `modem_log_stop`
// terminates.
inline constexpr std::string_view kModemLoggingStatusProperty =
    "vendor.sys.modem.logging.status";
// Int which specifies how many files to include in the bugreport.
inline constexpr std::string_view kModemLoggingNumberBugreportProperty =
    "persist.vendor.sys.modem.logging.br_num";
// Signals the current location that is being logged to. This can be used to
// determine the logging type.
inline constexpr std::string_view kModemLoggingPathProperty =
    "vendor.sys.modem.logging.log_path";
inline constexpr std::string_view kModemLoggingLogCountProperty =
    "vendor.sys.modem.logging.log_count";
inline constexpr std::string_view kModemLoggingLogPath =
    "vendor.sys.modem.logging.log_path";

// Bugreport constants
inline constexpr int kDefaultBugreportNumberFiles = 100;
inline constexpr std::string_view kModemAlwaysOnLogDirectory =
    "/data/vendor/radio/logs/always-on";
inline constexpr std::string_view kModemLogPrefix = "sbuff_";

}  // namespace pixel_modem::logging
