/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "AocStateResidencyDataProvider.h"

#include <android-base/logging.h>
#include <chrono>
#include <pthread.h>

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace stats {

struct async_data_t {
    pthread_cond_t *cond;
    pthread_mutex_t *lock;
    std::unordered_map<std::string, std::vector<StateResidency>> *residencies;
    std::unordered_map<std::string,
        std::vector<std::unique_ptr<GenericStateResidencyDataProvider>>> *providers;
};

AocStateResidencyDataProvider::AocStateResidencyDataProvider(std::vector<std::pair<std::string,
        std::string>> ids, std::vector<std::pair<std::string, std::string>> states,
        const uint64_t timeoutMillis) {
    // AoC stats are reported in ticks of 244.140625ns. The transform
    // function converts ticks to milliseconds.
    // 1000000 / 244.140625 = 4096.
    static const uint64_t AOC_CLK = 4096;
    static const uint64_t TIMEOUT_MILLIS = 120;
    std::function<uint64_t(uint64_t)> aocTickToMs = [](uint64_t a) { return a / AOC_CLK; };
    GenericStateResidencyDataProvider::StateResidencyConfig config = {
            .entryCountSupported = true,
            .entryCountPrefix = "Counter:",
            .totalTimeSupported = true,
            .totalTimePrefix = "Cumulative time:",
            .totalTimeTransform = aocTickToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "Time last entered:",
            .lastEntryTransform = aocTickToMs,
    };
    for (const auto &id : ids) {
        for (const auto &state : states) {
            std::vector<std::pair<std::string, std::string>> aocStateHeaders = {
                std::make_pair(state.first, ""),
            };
            std::vector<GenericStateResidencyDataProvider::PowerEntityConfig> cfgs;
            cfgs.emplace_back(generateGenericStateResidencyConfigs(config, aocStateHeaders),
                    id.first, "");
            std::unique_ptr<GenericStateResidencyDataProvider> sdp(
                    new GenericStateResidencyDataProvider(id.second + state.second, cfgs));
            mProviders[id.first].push_back(std::move(sdp));
        }
    }
    mStateCount = states.size();
    mTimeoutMillis = timeoutMillis == 0 ? TIMEOUT_MILLIS : timeoutMillis;
}

void *getStateResidenciesAsync(void *arg) {
    struct async_data_t *async = (struct async_data_t*)arg;
    const auto *originalContainer = async->residencies;

    // States from the same power entity are merged.
    for (const auto &providerList : *async->providers) {
        int32_t stateId = 0;
        std::string curEntity = providerList.first;
        std::vector<StateResidency> stateResidencies;

        // Iterate over each provider in the providerList, appending each of the states
        for (const auto &provider : providerList.second) {
            std::unordered_map<std::string, std::vector<StateResidency>> residency;
            provider->getStateResidencies(&residency);

            // Each provider should only return data for curEntity but checking anyway
            if (residency.find(curEntity) != residency.end()) {
                for (auto &r : residency.at(curEntity)) {
                    /*
                     * Modifying stateId here because we are stitching together infos from
                     * multiple GenericStateResidencyDataProviders. stateId must be modified
                     * to maintain uniqueness for a given entity
                     */
                    r.id = stateId++;
                    stateResidencies.push_back(r);
                }
            }
        }

        if (async->residencies != originalContainer) {
            /*
             * When provider->gretStateResidencies() exceeds timeout expiration, the main thread
             * stops waiting and continue, then the original residency pointer is erased in the
             * main thread when AocStateResidencyDataProvider::getStateResidencies() ends, and new
             * container will be created in the async thread later. In this situation, we ignore
             * delayed residency data and end the async thread.
             */
            return 0;
        }

        async->residencies->emplace(curEntity, stateResidencies);
    }

    pthread_mutex_lock(async->lock);
    pthread_cond_signal(async->cond);
    pthread_mutex_unlock(async->lock);

    return 0;
}

bool AocStateResidencyDataProvider::getStateResidencies(
        std::unordered_map<std::string, std::vector<StateResidency>> *residencies) {
    bool ret = true;
    int condResult = 0;
    pthread_t tid;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    std::unordered_map<std::string, std::vector<StateResidency>> stateResidencies;
    struct timespec start, timeout;
    struct async_data_t async = {
        .cond = &cond,
        .lock = &lock,
        .residencies = &stateResidencies,
        .providers = &mProviders
    };

    pthread_create(&tid, NULL, &getStateResidenciesAsync, (void*)&async);

    clock_gettime(CLOCK_REALTIME, &start);

    uint64_t expirationMillis = mTimeoutMillis * mStateCount;
    timeout.tv_sec = start.tv_sec + expirationMillis / 1000;
    uint64_t nsec = start.tv_nsec + (expirationMillis % 1000) * 1000000;
    if (nsec < 1000000000) {
        timeout.tv_nsec = nsec;
    } else {
        timeout.tv_sec += 1;
        timeout.tv_nsec = nsec - 1000000000;
    }

    pthread_mutex_lock(&lock);
    condResult = pthread_cond_timedwait(&cond, &lock, &timeout);
    pthread_mutex_unlock(&lock);

    if (condResult != 0) {
        if (condResult == ETIMEDOUT) {
            LOG(WARNING) << __func__ << " latency for AoC timeout: " << expirationMillis << " ms";
        } else {
            LOG(ERROR) << "Failed to wait for the condition variable: " << condResult;
        }
        ret = false;
    } else {
        for (const auto &residency : stateResidencies) {
            residencies->emplace(residency.first, residency.second);
        }
    }

    return ret;
}

std::unordered_map<std::string, std::vector<State>> AocStateResidencyDataProvider::getInfo() {
    // States from the same power entity are merged
    std::unordered_map<std::string, std::vector<State>> infos;
    for (const auto &providerList : mProviders) {
        int32_t stateId = 0;
        std::string curEntity = providerList.first;
        std::vector<State> stateInfos;

        // Iterate over each provider in the providerList, appending each of the states
        for (const auto &provider : providerList.second) {
            std::unordered_map<std::string, std::vector<State>> info = provider->getInfo();

            // Each provider should only return data for curEntity but checking anyway
            if (info.find(curEntity) != info.end()) {
                for (auto &i : info.at(curEntity)) {
                    /*
                     * Modifying stateId because we are stitching together infos from
                     * multiple GenericStateResidencyDataProviders. stateId must be modified
                     * to maintain uniqueness for a given entity
                     */
                    i.id = stateId++;
                    stateInfos.push_back(i);
                }
            }
        }

        infos.emplace(curEntity, stateInfos);
    }

    return infos;
}

}  // namespace stats
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
