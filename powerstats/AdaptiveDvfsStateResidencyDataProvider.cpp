/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "AdaptiveDvfsStateResidencyDataProvider.h"

#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>

#include <string>
#include <utility>

using android::base::Split;
using android::base::Trim;

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace stats {

AdaptiveDvfsStateResidencyDataProvider::AdaptiveDvfsStateResidencyDataProvider(
        std::string path,
        uint64_t clockRate,
        std::string powerEntityName,
        std::string freqPath)
    : DvfsStateResidencyDataProvider(path, clockRate, {}) {
    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(freqPath.c_str(), "r"), fclose);
    if (!fp) {
        PLOG(ERROR) << __func__ << ":Failed to open file " << freqPath;
        return;
    }

    size_t len = 0;
    char *line = nullptr;
    std::string suffix = "MHz";
    std::vector<std::pair<std::string, std::string>> states = {};
    std::vector<std::string> parts;
    std::string freqStr;

    while (getline(&line, &len, fp.get()) != -1) {
        parts = Split(line, " ");
        if (parts.size() > 0) {
            freqStr = Trim(parts[0]);
            states.push_back(std::make_pair(
                    freqStr.substr(0, freqStr.length() - 3) + suffix,
                    freqStr));
        }
    }

    mPowerEntities.push_back({powerEntityName, std::move(states)});
}

bool AdaptiveDvfsStateResidencyDataProvider::getStateResidencies(
        std::unordered_map<std::string, std::vector<StateResidency>> *residencies) {
    return DvfsStateResidencyDataProvider::getStateResidencies(residencies);
}

std::unordered_map<std::string, std::vector<State>>
        AdaptiveDvfsStateResidencyDataProvider::getInfo() {
    return DvfsStateResidencyDataProvider::getInfo();
}

}  // namespace stats
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
