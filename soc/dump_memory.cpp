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

#include <android-base/file.h>
#include <stdio.h>
#include <log/log.h>

char* concat(char* result, const char* one, const char* two){
  strcpy(result, one);
  strcat(result, two);
  return result;
}

void iterate(const char* path){
    dirent *entry, *entry2;
    char result[100], base[100];

    std::unique_ptr<DIR, decltype(&closedir)> ion(opendir(path), closedir);
    if (!ion) {
        ALOGE("Fail To Open Dir %s", path);
        return;
    }
    while ((entry = readdir(ion.get())) != nullptr) {
        if(entry->d_name[0] == '.') {
            continue;
        }
        strcpy(base, path);
        strcat(base, entry->d_name);
        strcat(base, "/");
        std::unique_ptr<DIR, decltype(&closedir)> ion2(opendir(base), closedir);
        if (!ion2) {
            ALOGE("Fail To Open Dir %s\n", base);
            return;
        }
        while ((entry2 = readdir(ion2.get())) != nullptr) {
            if(entry2->d_name[0] == '.') {
                continue;
            }
            dumpFileContent(entry2->d_name, concat(result, base, entry2->d_name));
        }
    }
    return;
}

// Dump memory.
int main() {
    dirent *entry;
    char result[100];

    printf("------ ION HEAPS  ------\n");
    iterate("/d/ion/");

    dumpFileContent("dmabuf info", "/d/dma_buf/bufinfo");
    dumpFileContent("Page Pinner - longterm pin", "/sys/kernel/debug/page_pinner/buffer");

    printf("------ CMA info  ------\n");
    std::unique_ptr<DIR, decltype(&closedir)> cmadebug(opendir("/sys/kernel/debug/cma/"), closedir);
    if (!cmadebug) {
        ALOGE("Fail To Open Dir /sys/kernel/debug/cma/");
    } else {
        while ((entry = readdir(cmadebug.get())) != nullptr) {
            if(entry->d_name[0] == '.') {
                continue;
            }
            dumpFileContent("count", concat(result, concat(result, "/sys/kernel/debug/cma/", entry->d_name), "/count"));
            dumpFileContent("used", concat(result, concat(result, "/sys/kernel/debug/cma/", entry->d_name), "/used"));
            dumpFileContent("bitmap", concat(result, concat(result, "/sys/kernel/debug/cma/", entry->d_name), "/bitmap"));
        }
    }

    printf("------ Pixel CMA stat  ------\n");
    iterate("/sys/kernel/pixel_stat/mm/cma/");

    dumpFileContent("Pixel Trace", "/sys/kernel/tracing/instances/pixel/trace");
    return 0;
}

