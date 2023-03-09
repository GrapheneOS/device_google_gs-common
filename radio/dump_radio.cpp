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
#include <android-base/properties.h>
#include <android-base/file.h>

#define RIL_LOG_DIRECTORY "/data/vendor/radio"
#define RIL_LOG_DIRECTORY_PROPERTY "persist.vendor.ril.log.base_dir"
#define RIL_LOG_NUMBER_PROPERTY "persist.vendor.ril.log.num_file"
#define RIL_LOG_PREFIX "rild.log."

int main() {
    std::string rilLogDir = ::android::base::GetProperty(RIL_LOG_DIRECTORY_PROPERTY, RIL_LOG_DIRECTORY);

    int maxFileNum = ::android::base::GetIntProperty(RIL_LOG_NUMBER_PROPERTY, 50);

    const std::string currentLogDir = concatenatePath(rilLogDir.c_str(), "/cur");
    const std::string previousLogDir = concatenatePath(rilLogDir.c_str(), "/prev");
    const std::string currentDestDir = concatenatePath(BUGREPORT_PACKING_DIR, "cur");
    const std::string previousDestDir = concatenatePath(BUGREPORT_PACKING_DIR, "prev");
    if (mkdir(currentDestDir.c_str(), 0777) == -1) {
        printf("Unable to create folder: %s\n", currentDestDir.c_str());
        return 0;
    }
    if (mkdir(previousDestDir.c_str(), 0777) == -1) {
        printf("Unable to create folder: %s\n", previousDestDir.c_str());
        return 0;
    }

    dumpLogs(currentLogDir.c_str(), currentDestDir.c_str(), maxFileNum, RIL_LOG_PREFIX);
    dumpLogs(previousLogDir.c_str(), previousDestDir.c_str(), maxFileNum, RIL_LOG_PREFIX);
    return 0;
}
