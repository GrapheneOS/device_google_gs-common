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
#include "TpuDvfsStateResidencyDataProvider.h"

#include <android-base/logging.h>

static const std::string ENTITY_NAME = "TPU-DVFS";

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace stats {

TpuDvfsStateResidencyDataProvider::TpuDvfsStateResidencyDataProvider(
        const std::string& path,
        std::vector<std::string> frequencies,
        uint64_t clockRate)
    : mPath(path), mFrequencies(std::move(frequencies)), mClockRate(clockRate) {}

bool TpuDvfsStateResidencyDataProvider::getStateResidencies(
        std::unordered_map<std::string, std::vector<StateResidency>> *residencies) {
    // Using FILE* instead of std::ifstream for performance reasons
    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(mPath.c_str(), "r"), fclose);
    if (!fp) {
        PLOG(ERROR) << "Failed to open file " << mPath;
        return false;
    }

    std::vector<StateResidency> stateResidencies;
    for (int i = 0; i < mFrequencies.size(); i++) {
        StateResidency s = {.id = i, .totalTimeInStateMs = 0};
        stateResidencies.push_back(s);
    }

    char *line = nullptr;
    size_t len = 0;
    std::istringstream ssLine;
    std::string split;
    int32_t lineIdx = 0;
    std::vector<int32_t> stateIdxMap;
    int32_t colIdx;
    std::vector<std::string>::const_iterator found;
    while (getline(&line, &len, fp.get()) != -1) {
        ssLine.clear();
        ssLine.str(line);
        colIdx = 0;
        for (std::string split; std::getline(ssLine, split, ' ');) {
            // Skip first column
            if (split.find(':') != std::string::npos)
                continue;

            if (lineIdx == 0) {
                // Build the state index map by first row
                split.erase(split.find_last_not_of(" \n\r\t") + 1);
                found = std::find(mFrequencies.begin(), mFrequencies.end(), split);
                if (found != mFrequencies.end()) {
                    stateIdxMap.push_back(found - mFrequencies.begin());
                } else {
                    PLOG(ERROR) << "TPU frequency " << split << " is not found in " << mPath;
                    stateIdxMap.push_back(0);
                }
            } else {
                // Add up time in frequency per uid
                stateResidencies[stateIdxMap[colIdx]].totalTimeInStateMs +=
                        std::atoll(split.c_str()) / mClockRate;
            }
            colIdx++;
        }
        lineIdx++;
    }

    residencies->emplace(ENTITY_NAME, stateResidencies);
    return true;
}

std::unordered_map<std::string, std::vector<State>> TpuDvfsStateResidencyDataProvider::getInfo() {
    std::vector<State> states;
    for (int32_t id = 0; id < mFrequencies.size(); id++) {
        State s =
            {.id = id, .name = std::to_string(std::atol(mFrequencies[id].c_str()) / 1000) + "MHz"};
        states.push_back(s);
    }

    return {{ENTITY_NAME, states}};
}

}  // namespace stats
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
