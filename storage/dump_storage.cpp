/*
 * Copyright 2023 The Android Open Source Project
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
#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <android-base/properties.h>
#include <string.h>
#include <stdio.h>
#include <log/log.h>
#include <regex>
#include <fstream>
#include <map>

#define F2FS_FSCK_TIME_PROPERTY "ro.boottime.init.fsck.data"
#define F2FS_MNT_TIME_PROPERTY "ro.boottime.init.mount.data"
#define BOOTDEVICE_PROPERTY "ro.boot.bootdevice"
#define BUILD_TYPE_PROPERTY "ro.build.type"

void read_buffer(int buf_id, int total_len, const char* path)
{
    int i, len;
    system("mkdir -m 0770 -p /data/vendor/storage/");
    auto cmd = android::base::StringPrintf("rm -f /data/vendor/storage/%s\n", path);
    system(cmd.c_str());

    for (i=0;i<=total_len;i+=524288) {
        if (total_len-i < 524288) {
            len = total_len-i;
            printf("/vendor/bin/sg_read_buffer -m 0x1c -i %d -l %d -o %d -r /dev/sg3 >> /data/vendor/storage/%s\n",
                buf_id, len, i, path);
            auto cmd = android::base::StringPrintf(
                "/vendor/bin/sg_read_buffer -m 0x1c -i %d -l %d -o %d -r /dev/sg3 >> /data/vendor/storage/%s\n",
                buf_id, len, i, path);
            system(cmd.c_str());
            break;
        }
        len = 524288;
        printf("/vendor/bin/sg_read_buffer -m 0x1c -i %d -l %d -o %d -r /dev/sg3 >> /data/vendor/storage/%s\n",
            buf_id, len, i, path);
        auto cmd = android::base::StringPrintf(
            "/vendor/bin/sg_read_buffer -m 0x1c -i %d -l %d -o %d -r /dev/sg3 >> /data/vendor/storage/%s\n",
            buf_id, len, i, path);
        system(cmd.c_str());
    }
}

int main() {
    //F2FS
    dumpFileContent("F2FS status", "/sys/kernel/debug/f2fs/status");
    dumpFileContent("F2FS fsck", "/dev/fscklogs/fsck");

    int fsck_time = android::base::GetIntProperty(F2FS_FSCK_TIME_PROPERTY, 0);
    printf("--- F2FS - fsck time (ms) ---\n%d\n\n", fsck_time);
    int mnt_time = android::base::GetIntProperty(F2FS_MNT_TIME_PROPERTY, 0);
    printf("--- F2FS - checkpoint=disable time (ms) ---\n%d\n\n", mnt_time);

    //UFS
    dumpFileContent("UFS model", "/sys/block/sda/device/model");
    dumpFileContent("UFS rev", "/sys/block/sda/device/rev");
    dumpFileContent("UFS size", "/sys/block/sda/size");

    dumpFileContent("UFS Slow IO Read",
                    "/dev/sys/block/bootdevice/slowio_read_cnt");
    dumpFileContent("UFS Slow IO Write",
                    "/dev/sys/block/bootdevice/slowio_write_cnt");
    dumpFileContent("UFS Slow IO Unmap",
                    "/dev/sys/block/bootdevice/slowio_unmap_cnt");
    dumpFileContent("UFS Slow IO Sync",
                    "/dev/sys/block/bootdevice/slowio_sync_cnt");

    printf("------ UFS err_stats ------\n");
    const std::string ufs_err_stats_path(
            "/dev/sys/block/bootdevice/err_stats/");
    std::unique_ptr<DIR, decltype(&closedir)> statdir(
            opendir(ufs_err_stats_path.c_str()), closedir);
    if (statdir) {
        dirent *stat_entry;
        while ((stat_entry = readdir(statdir.get())) != nullptr) {
            std::string ufs_err_stats_path(stat_entry->d_name);
            if (!strcmp(ufs_err_stats_path.c_str(), ".")
                    || !strcmp(ufs_err_stats_path.c_str(), ".."))
                continue;
            std::string bootdevice = android::base::GetProperty(
                    BOOTDEVICE_PROPERTY, "");
            std::string err_stat_path = "/sys/devices/platform/";
            err_stat_path.append(bootdevice.c_str());
            err_stat_path.append("/err_stats/");
            err_stat_path.append(ufs_err_stats_path.c_str());
            std::ifstream err_stat_file(err_stat_path);
            if (err_stat_file.is_open()) {
                std::string err_stat_atom;
                err_stat_file >> err_stat_atom;
                printf("%s:%s\n", ufs_err_stats_path.c_str(),
                       err_stat_atom.c_str());
                err_stat_file.close();
            }
        }
    }

    printf("\n------ UFS io_stats ------\n");
    printf("\t\t%-12s %-12s %-12s %-12s %-12s %-12s\n", "ReadCnt", "ReadBytes",
           "WriteCnt", "WriteBytes", "RWCnt", "RWBytes");
    const std::string ufs_io_stats = "/dev/sys/block/bootdevice/io_stats/";
    const std::string iostats_seq[3] = {"_start", "_complete", "_maxdiff"};
    const std::string iostats[6] = {"rcnt", "rbyte", "wcnt", "wbyte", "rwcnt",
            "rwbyte"};

    for (int i = 0; i < 3; i++) {
        const std::string header[3] = {"Started: \t", "Completed: \t",
                "MaxDiff: \t"};
        printf("%s", header[i].c_str());
        for (int j = 0; j < 6; j++) {
            std::string iostat_path = ufs_io_stats.c_str();
            iostat_path.append(iostats[j].c_str());
            iostat_path.append(iostats_seq[i].c_str());
            std::ifstream io_stats_file(iostat_path.c_str());
            unsigned long io_stat_atom = 0;
            if (io_stats_file.is_open()) {
                io_stats_file >> io_stat_atom;
                printf("%-12lu ", io_stat_atom);
                io_stats_file.close();
            }
        }
        printf("\n");
    }

    printf("\n------ UFS req_stats ------\n");
    printf("\t%-12s %-12s %-12s %-12s %-12s %-12s\n", "All", "Write", "Read",
           "Security", "Flush", "Discard");
    const std::string ufs_req_stats = "/dev/sys/block/bootdevice/req_stats/";
    const std::string reqstats_seq[4] = {"_min", "_max", "_avg", "_sum"};
    const std::string reqstats[6] = {"all", "write", "read", "security",
            "flush", "discard"};

    for (int i = 0; i < 4; i++) {
        const std::string header[4] = {"Min:\t", "Max: \t", "Avg: \t",
                "Count: \t"};
        printf("%s", header[i].c_str());
        for (int j = 0; j < 6; j++) {
            std::string reqstat_path = ufs_req_stats.c_str();
            reqstat_path.append(reqstats[j].c_str());
            reqstat_path.append(reqstats_seq[i].c_str());
            std::ifstream req_stats_file(reqstat_path.c_str());
            unsigned long req_stat_atom = 0;
            if (req_stats_file.is_open()) {
                req_stats_file >> req_stat_atom;
                printf("%-12lu ", req_stat_atom);
                req_stats_file.close();
            }
        }
        printf("\n");
    }

    printf("\n------ UFS health ------\n");
    dumpFileContent(
            "",
            "/dev/sys/block/bootdevice/health_descriptor/life_time_estimation_a");
    dumpFileContent(
            "",
            "/dev/sys/block/bootdevice/health_descriptor/life_time_estimation_b");
    dumpFileContent("", "/dev/sys/block/bootdevice/health_descriptor/eol_info");
    dumpFileContent(
            "",
            "/dev/sys/block/bootdevice/health_descriptor/life_time_estimation_c");

    printf("\n------ UFS error history ------\n");
    std::string build_type = android::base::GetProperty(BUILD_TYPE_PROPERTY, "");
    if (build_type == "userdebug") {
        std::string sg_read_buffer = "/vendor/bin/sg_read_buffer";
        std::ifstream sg_read_buffer_file(sg_read_buffer.c_str());
        if (sg_read_buffer_file.is_open()) {
            const std::string ufs_ver_path(
                "/dev/sys/block/bootdevice/device_descriptor/specification_version");
            std::ifstream ufs_ver_file(ufs_ver_path);
            std::string ufs_ver;
            if (ufs_ver_file.is_open()) {
                ufs_ver_file >> ufs_ver;
                ufs_ver_file.close();
            }
            if (strcmp(ufs_ver.c_str(), "0x0210")) {
                const std::string ufs_brand_path("/sys/block/sda/device/vendor");
                std::ifstream ufs_brand_file(ufs_brand_path);
                std::string ufs_brand;
                if (ufs_brand_file.is_open()) {
                    ufs_brand_file >> ufs_brand;
                    ufs_brand_file.close();
                }

                std::map<std::string, int> const table =
                    {
                    {"MICRON", 0x12C},
                    {"KIOXIA", 0x198},
                    {"SKhynix", 0x1AD},
                    {"SAMSUNG", 0x1CE}
                    };
                auto ufs_vendor_id = table.find(ufs_brand);
                if (ufs_vendor_id != table.end()) {
                    switch(ufs_vendor_id->second){
                      case 0x12C: //MICRON
                        read_buffer(16, 2097152, "micron_10_ufs_err_history.dat");
                        read_buffer(18, 10485760, "micron_12_ufs_err_history.dat");
                        read_buffer(19, 10485760, "micron_13_ufs_err_history.dat");
                        break;
                      case 0x198: //KIOXIA
                        read_buffer(16, 16773120, "kioxia_10_ufs_err_history.dat");
                        read_buffer(17, 2097152, "kioxia_11_ufs_err_history.dat");
                        read_buffer(18, 131072, "kioxia_12_ufs_err_history.dat");
                        break;
                      case 0x1AD: //SKhynix
                        read_buffer(0, 4096, "hynix_00_ufs_err_history.dat");
                        read_buffer(16, 131072, "skhynix_10_ufs_err_history.dat");
                        read_buffer(17, 131072, "skhynix_11_ufs_err_history.dat");
                        read_buffer(18, 131072, "skhynix_12_ufs_err_history.dat");
                        read_buffer(19, 131072, "skhynix_13_ufs_err_history.dat");
                        break;
                      case 0x1CE: //SAMSUNG
                        read_buffer(16, 8404992, "samsung_10_ufs_err_history.dat");
                        break;
                      default:
                        break;
                    }
                }
                sg_read_buffer_file.close();
            }
        } else
          printf("sg_read_buffer does not exist\n");
    }

    return 0;
}
