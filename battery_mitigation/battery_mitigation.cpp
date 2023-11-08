/*
 * Copyright (C) 2022 The Android Open Source Project
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

#define LOG_TAG "battery-mitigation"

#include <android/binder_process.h>
#include <battery_mitigation/BatteryMitigation.h>
#include <battery_mitigation/BatteryMitigationService.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

#define COUNT_LIMIT 10

using android::hardware::google::pixel::BatteryMitigation;
using android::hardware::google::pixel::BatteryMitigationService;
using android::hardware::google::pixel::MitigationConfig;

android::sp<BatteryMitigation> bmSp;
android::sp<BatteryMitigationService> batteryMitigationService;

const struct MitigationConfig::Config cfg = {
    .SystemPath = {
        "/dev/thermal/tz-by-name/batoilo/temp",
        "/dev/thermal/tz-by-name/smpl_gm/temp",
        "/dev/thermal/tz-by-name/soc/temp",
        "/dev/thermal/tz-by-name/vdroop1/temp",
        "/dev/thermal/tz-by-name/vdroop2/temp",
        "/dev/thermal/tz-by-name/ocp_gpu/temp",
        "/dev/thermal/tz-by-name/ocp_tpu/temp",
        "/dev/thermal/tz-by-name/soft_ocp_cpu2/temp",
        "/dev/thermal/tz-by-name/soft_ocp_cpu1/temp",
        "/dev/thermal/tz-by-name/battery/temp",
        "/dev/thermal/tz-by-name/battery_cycle/temp",
        "/sys/bus/iio/devices/iio:device0/lpf_power",
        "/sys/bus/iio/devices/iio:device1/lpf_power",
        "/dev/thermal/cdev-by-name/thermal-cpufreq-2/cur_state",
        "/dev/thermal/cdev-by-name/thermal-cpufreq-1/cur_state",
        "/dev/thermal/cdev-by-name/thermal-gpufreq-0/cur_state",
        "/dev/thermal/cdev-by-name/tpu_cooling/cur_state",
        "/dev/thermal/cdev-by-name/CAM/cur_state",
        "/dev/thermal/cdev-by-name/DISP/cur_state",
        "/dev/thermal/cdev-by-name/gxp-cooling/cur_state",
        "/sys/class/power_supply/battery/voltage_now",
        "/sys/class/power_supply/battery/current_now",
    },
    .FilteredZones = {
        "batoilo",
        "vdroop1",
        "vdroop2",
        "smpl_gm",
    },
    .SystemName = {
        "batoilo", "smpl_gm", "soc", "vdroop1", "vdroop2", "ocp_gpu",
        "ocp_tpu", "soft_ocp_cpu2", "soft_ocp_cpu1", "battery", "battery_cycle",
        "main", "sub", "CPU2", "CPU1", "GPU", "TPU", "CAM", "DISP", "NPU",
        "voltage_now", "current_now",
    },
    .LogFilePath = "/data/vendor/mitigation/thismeal.txt",
    .TimestampFormat = "%Y-%m-%d %H:%M:%S",
};

const struct MitigationConfig::EventThreadConfig eventThreadCfg = {
    .NumericSysfsStatPaths = {
        {"cpu0_freq", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"},
        {"cpu1_freq", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq"},
        {"cpu2_freq", "/sys/devices/system/cpu/cpu2/cpufreq/scaling_cur_freq"},
        {"gpu_freq", "/sys/devices/platform/1f000000.mali/cur_freq"},
        {"battery_temp", "/dev/thermal/tz-by-name/battery/temp"},
        {"battery_cycle", "/dev/thermal/tz-by-name/battery_cycle/temp"},
        {"voltage_now", "/sys/class/power_supply/battery/voltage_now"},
        {"current_now", "/sys/class/power_supply/battery/current_now"},
    },
    .NumericSysfsStatDirs = {
        {"last_triggered_mode", "/sys/devices/virtual/pmic/mitigation/last_triggered_mode/"},
    },
    .TriggeredIdxPath = "/sys/devices/virtual/pmic/mitigation/br_stats/triggered_idx",
    .BrownoutStatsPath = "/sys/devices/virtual/pmic/mitigation/br_stats/stats",
    .StoringPath = "/data/vendor/mitigation/thismeal.bin",
    .ParsedThismealPath = "/data/vendor/mitigation/thismeal.txt",
    .ParsedLastmealPath = "/data/vendor/mitigation/lastmeal.txt",
    .ParsedLastmealCSVPath = "/data/vendor/mitigation/lastmeal.csv",
    .FvpStatsPath = "/sys/devices/platform/acpm_stats/fvp_stats",
    .PmicCommon = {
                /* Main Pmic */
                {
                    .OdpmDir = "/sys/bus/iio/devices/iio:device0",
                    .OdpmEnabledRailsPath = "/sys/bus/iio/devices/iio:device0/enabled_rails",
                    .PmicNamePath = "/sys/bus/iio/devices/iio:device0/name",
                },
                /* Sub Pmic */
                {
                    .OdpmDir = "/sys/bus/iio/devices/iio:device1",
                    .OdpmEnabledRailsPath = "/sys/bus/iio/devices/iio:device1/enabled_rails",
                    .PmicNamePath = "/sys/bus/iio/devices/iio:device1/name",
                },
    },
    .PlatformSpecific = {
                /* MIN_SUPPORTED_PLATFORM */
                {
                    .MainPmicName = "s2mpg10-odpm\n",
                    .SubPmicName = "s2mpg11-odpm\n",
                    .PcieModemPath = "/sys/devices/platform/11920000.pcie/power_stats",
                    .PcieWifiPath = "/sys/devices/platform/14520000.pcie/power_stats",
                },
                {
                    .MainPmicName = "s2mpg12-odpm\n",
                    .SubPmicName = "s2mpg13-odpm\n",
                    .PcieModemPath = "/sys/devices/platform/11920000.pcie/power_stats",
                    .PcieWifiPath = "/sys/devices/platform/14520000.pcie/power_stats",
                },
                {
                    .MainPmicName = "s2mpg14-odpm\n",
                    .SubPmicName = "s2mpg15-odpm\n",
                    .PcieModemPath = "/sys/devices/platform/12100000.pcie/power_stats",
                    .PcieWifiPath = "/sys/devices/platform/13120000.pcie/power_stats",
                },
                /* MAX_SUPPORTED_PLATFORM */
                {
                    .MainPmicName = "s2mpg14-odpm\n",
                    .SubPmicName = "s2mpg15-odpm\n",
                    .PcieModemPath = "/sys/devices/platform/12100000.pcie/power_stats",
                    .PcieWifiPath = "/sys/devices/platform/13120000.pcie/power_stats",
                },

    },
};

const char kReadyFilePath[] = "/sys/devices/virtual/pmic/mitigation/instruction/ready";
const char kReadyProperty[] = "vendor.brownout.mitigation.ready";
const char kLastMealPath[] = "/data/vendor/mitigation/lastmeal.txt";
const char kBRRequestedProperty[] = "vendor.brownout_reason";
const char kLastMealProperty[] = "vendor.brownout.br.feasible";
const char kCDTProperty[] = "ro.boot.cdt_hwid";
const std::regex kTimestampRegex("^\\S+\\s[0-9]+:[0-9]+:[0-9]+\\S+$");

std::string GetSystemProperty(std::string property) {
    char value[PROP_VALUE_MAX];
    __system_property_get(property.c_str(), value);
    return std::string(value);
}

int main(int argc, char **argv) {
    std::string cdt = GetSystemProperty(kCDTProperty);
    int platformNum  = atoi(cdt.substr(5, 1).c_str());
    batteryMitigationService = new BatteryMitigationService(eventThreadCfg,
                                                            platformNum);
    if (!batteryMitigationService) {
        return 0;
    }
    bool platformSupported = batteryMitigationService->isPlatformSupported();
    bool brownoutStatsBinarySupported = batteryMitigationService->isBrownoutStatsBinarySupported();
    if (argc == 2) {
        if(strcmp(argv[1], "-d") == 0 &&
           brownoutStatsBinarySupported &&
           platformSupported) {
            /* Create thismeal.txt from thismeal.bin */
            batteryMitigationService->genParsedMeal(eventThreadCfg.ParsedThismealPath);
        }
        return 0;
    }

    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    auto batteryMitigationStartTime = std::chrono::system_clock::now();
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    ABinderProcess_startThreadPool();

    bool mitigationLogTimeValid;
    std::string reason = android::base::GetProperty(kBRRequestedProperty, "");
    if (brownoutStatsBinarySupported) {
        /* Create lastmeal.txt if the dump time in thismeal.bin are valid */
        mitigationLogTimeValid = batteryMitigationService->isTimeValid(eventThreadCfg.StoringPath,
                                                                       batteryMitigationStartTime);
        if (!reason.empty() && mitigationLogTimeValid &&
            batteryMitigationService->genParsedMeal(eventThreadCfg.ParsedLastmealPath) &&
            batteryMitigationService->genLastmealCSV(eventThreadCfg.ParsedLastmealCSVPath)) {
            android::base::SetProperty(kLastMealProperty, "1");
        }
        /* Start BrownoutEventThread to poll brownout event from kernel */
        batteryMitigationService->startBrownoutEventThread();
    } else{
        bmSp = new BatteryMitigation(cfg);
        if (!bmSp) {
            return 0;
        }
        mitigationLogTimeValid = bmSp->isMitigationLogTimeValid(batteryMitigationStartTime,
                                                                cfg.LogFilePath,
                                                                cfg.TimestampFormat,
                                                                kTimestampRegex);
        if (!reason.empty() && mitigationLogTimeValid) {
            std::ifstream src(cfg.LogFilePath, std::ios::in);
            std::ofstream dst(kLastMealPath, std::ios::out);
            dst << src.rdbuf();
            android::base::SetProperty(kLastMealProperty, "1");
        }
    }

    bool isBatteryMitigationReady = false;
    std::string ready_str;
    int val = 0;
    for (int i = 0; i < COUNT_LIMIT; i++) {
        if (!android::base::ReadFileToString(kReadyFilePath, &ready_str)) {
            continue;
        }
        ready_str = android::base::Trim(ready_str);
        if (!android::base::ParseInt(ready_str, &val)) {
            continue;
        }
        if (val == 1) {
            isBatteryMitigationReady = true;
            break;
        }
    }
    if (isBatteryMitigationReady) {
        android::base::SetProperty(kReadyProperty, "1");
    }
    while (true) {
        pause();
    }
    return 0;
}
