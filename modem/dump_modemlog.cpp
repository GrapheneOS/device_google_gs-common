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
#include <dump/pixel_dump.h>

int main() {
    dumpLogs("/data/vendor/radio/extended_logs", "/data/vendor/radio/logs/always-on/all_logs", 20, "extended_log_");
    copyFile("/mnt/vendor/efs/nv_normal.bin", "/data/vendor/radio/logs/always-on/all_logs/nv_normal.bin");
    copyFile("/mnt/vendor/efs/nv_protected.bin", "/data/vendor/radio/logs/always-on/all_logs/nv_protected.bin");
    return 0;
}
