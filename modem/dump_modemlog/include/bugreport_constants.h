#pragma once

#include <string_view>

#include "dumper.h"

namespace pixel_modem::logging {

inline constexpr std::string_view kBugreportPackingDirectory =
    "/data/vendor/radio/logs/always-on/all_logs";

inline constexpr LogDumpInfo kLogDumpInfo[] = {
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

constexpr FileCopyInfo kFileCopyInfo[] = {
    {.src_dir = "/mnt/vendor/efs/nv_normal.bin",
     .dest_dir = "/data/vendor/radio/logs/always-on/all_logs/nv_normal.bin"},
    {.src_dir = "/mnt/vendor/efs/nv_protected.bin",
     .dest_dir =
         "/data/vendor/radio/logs/always-on/all_logs/nv_protected.bin"}};

}  // namespace pixel_modem::logging
