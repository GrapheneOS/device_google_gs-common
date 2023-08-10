//
// Copyright (C) 2023 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//
// UmfwStat dump tool.
//

#include <dump/pixel_dump.h>

int main() {
  // Set stdout to be unbuffered.
  setbuf(stdout, NULL);

  // Dump AoC statistics.
  // TODO(b/272300997): Add timeouts for requests.
  runCommand("AoC A32 statistics",
             "timeout 5 /vendor/bin/umfw_stat_tool -s aoc:1 get_all");
  runCommand("AoC FF1 statistics",
             "timeout 5 /vendor/bin/umfw_stat_tool -s aoc:2 get_all");
  runCommand("AoC HF0 statistics",
             "timeout 5 /vendor/bin/umfw_stat_tool -s aoc:3 get_all");
  runCommand("AoC HF1 statistics",
             "timeout 5 /vendor/bin/umfw_stat_tool -s aoc:4 get_all");

  return 0;
}

