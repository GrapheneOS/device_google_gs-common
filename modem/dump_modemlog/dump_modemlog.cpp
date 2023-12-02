/*
 * Copyright 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <android-base/properties.h>
#include <dump/pixel_dump.h>

#include "android_property_manager_impl.h"
#include "dumper.h"
#include "modem_log_dumper.h"

namespace pixel_modem::logging {

/**
 * @brief Implementation of Dumper that directly forwards to their corresponding
 * dumpstate methods.
 */
class DumperImpl : public Dumper {
 public:
  void DumpLogs(const LogDumpInfo& log_dump_info) override {
    dumpLogs(log_dump_info.src_dir.data(), log_dump_info.dest_dir.data(),
             log_dump_info.limit, log_dump_info.prefix.data());
  }
  void CopyFile(const FileCopyInfo& file_copy_info) override {
    copyFile(file_copy_info.src_dir.data(), file_copy_info.dest_dir.data());
  }
};

}  // namespace pixel_modem::logging

int main() {
  pixel_modem::logging::DumperImpl dumper_impl;
  pixel_modem::AndroidPropertyManagerImpl android_property_manager_impl;
  pixel_modem::logging::ModemLogDumper modem_log_dumper(
      dumper_impl, android_property_manager_impl);

  modem_log_dumper.DumpModemLogs();
  return 0;
}
