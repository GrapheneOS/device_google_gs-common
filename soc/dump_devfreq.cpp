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
#include <unistd.h>

const char* dvfs_paths[][2] = {
    {"MIF DVFS", "/sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/trans_stat"},
    {"INT DVFS", "/sys/devices/platform/17000020.devfreq_int/devfreq/17000020.devfreq_int/trans_stat"},
    {"INTCAM DVFS", "/sys/devices/platform/17000030.devfreq_intcam/devfreq/17000030.devfreq_intcam/trans_stat"},
    {"DISP DVFS", "/sys/devices/platform/17000040.devfreq_disp/devfreq/17000040.devfreq_disp/trans_stat"},
    {"CAM DVFS", "/sys/devices/platform/17000050.devfreq_cam/devfreq/17000050.devfreq_cam/trans_stat"},
    {"TNR DVFS", "/sys/devices/platform/17000060.devfreq_tnr/devfreq/17000060.devfreq_tnr/trans_stat"},
    {"MFC DVFS", "/sys/devices/platform/17000070.devfreq_mfc/devfreq/17000070.devfreq_mfc/trans_stat"},
    {"BO DVFS", "/sys/devices/platform/17000080.devfreq_bo/devfreq/17000080.devfreq_bo/trans_stat"},
    {"BW DVFS", "/sys/devices/platform/17000080.devfreq_bw/devfreq/17000080.devfreq_bw/trans_stat"},
    {"DSU DVFS", "/sys/devices/platform/17000090.devfreq_dsu/devfreq/17000090.devfreq_dsu/trans_stat"},
    {"BCI DVFS", "/sys/devices/platform/170000a0.devfreq_bci/devfreq/170000a0.devfreq_bci/trans_stat"},
    {"BTS stats", "/sys/devices/platform/exynos-bts/bts_stats"}
};
int main() {

    for (auto &dvfs_path : dvfs_paths ) {
        if(!access(dvfs_path[1], R_OK)) {
            dumpFileContent(dvfs_path[0], dvfs_path[1]);
        }
    }
    return 0;
}
