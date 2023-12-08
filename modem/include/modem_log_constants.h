#pragma once
#include <string>

#include "dumper.h"

namespace modem {
namespace logging {

// Modem related Android System Properties

// Controls triggering `modem_logging_start` and `modem_logging_stop`.
inline constexpr static std::string_view kModemLoggingEnabledProperty =
    "vendor.sys.modem.logging.enable";
// Signals the current modem logging state. This will be set to
// `vendor.sys.modem.logging.enable` when `modem_log_start` or `modem_log_stop`
// terminates.
inline constexpr static std::string_view kModemLoggingStatusProperty =
    "vendor.sys.modem.logging.status";
// Int which specifies how many files to include in the bugreport.
inline constexpr static std::string_view kModemLoggingNumberBugreportProperty =
    "persist.vendor.sys.modem.logging.br_num";
// Signals the current location that is being logged to. This can be used to
// determine the logging type.
inline constexpr static std::string_view kModemLoggingPathProperty =
    "vendor.sys.modem.logging.log_path";

// Bugreport constants
inline constexpr static int kDefaultBugreportNumberFiles = 100;
inline constexpr static std::string_view kModemAlwaysOnLogDirectory =
    "/data/vendor/radio/logs/always-on";
inline constexpr static std::string_view kModemLogPrefix = "sbuff_";
inline constexpr static std::string_view kBugreportPackingDirectory =
    "/data/vendor/radio/logs/always-on/all_logs";

inline constexpr static LogDumpInfo kLogDumpInfo[] = {
    {.src_dir = "/data/vendor/radio/extended_logs",
     .dest_dir = kBugreportPackingDirectory,
     .limit = 20,
     .prefix = "extended_log_"},
    {.src_dir = "/data/vendor/radio/sim/",
     .dest_dir = kBugreportPackingDirectory,
     .limit = 1,
     .prefix = "sim_poweron_log_"},
    {.src_dir = "data/vendor/radio/logs/history",
     .dest_dir = kBugreportPackingDirectory,
     .limit = 2,
     .prefix = "Logging"}};

constexpr static FileCopyInfo kFileCopyInfo[] = {
    {.src_dir = "/mnt/vendor/efs/nv_normal.bin",
     .dest_dir = "/data/vendor/radio/logs/always-on/all_logs/nv_normal.bin"},
    {.src_dir = "/mnt/vendor/efs/nv_protected.bin",
     .dest_dir =
         "/data/vendor/radio/logs/always-on/all_logs/nv_protected.bin"}};
}  // namespace logging
}  // namespace modem
