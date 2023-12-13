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

const char* logbuffer_paths[][2] = {
    {"DSIM0", "/dev/logbuffer_dsim0"},
    {"DSIM1", "/dev/logbuffer_dsim1"},
};

int main() {
    for (auto &logbuffer_path : logbuffer_paths) {
        if(!access(logbuffer_path[1], R_OK)) {
            dumpFileContent(logbuffer_path[0], logbuffer_path[1]);
        }
    }

    return 0;
}