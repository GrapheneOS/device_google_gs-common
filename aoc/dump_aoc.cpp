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
#include <stdio.h>
#include <dump/pixel_dump.h>

// Dump Aoc.
int main() {
    setbuf(stdout, NULL);
    runCommand("AoC Service Status", "timeout 0.1 cat /sys/devices/platform/*.aoc/services");
    runCommand("AoC Restarts", "timeout 0.1 cat /sys/devices/platform/*.aoc/restart_count");
    runCommand("AoC Coredumps", "timeout 0.1 cat /sys/devices/platform/*.aoc/coredump_count");
    runCommand("AoC ring buf wake", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/ring_buffer_wakeup");
    runCommand("AoC host ipc wake", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/host_ipc_wakeup");
    runCommand("AoC usf wake", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/usf_wakeup");
    runCommand("AoC audio wake", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/audio_wakeup");
    runCommand("AoC logging wake", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/logging_wakeup");
    runCommand("AoC hotword wake", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/hotword_wakeup");
    runCommand("AoC memory exception wake", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/memory_exception");
    runCommand("AoC memory votes", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/memory_votes_a32");
    runCommand("AoC memory votes", "timeout 0.1 cat /sys/devices/platform/*.aoc/control/memory_votes_ff1");
    runCommand("clean AoC buffer","echo ' ' > /dev/acd-debug; timeout 0.1 cat /dev/acd-debug");
    runCommand("AoC Heap Stats (A32)", "echo 'dbg heap -c 1' > /dev/acd-debug; timeout 0.1 cat /dev/acd-debug");
    runCommand("AoC Heap Stats (F1)", "echo 'dbg heap -c 2' > /dev/acd-debug; timeout 0.1 cat /dev/acd-debug");
    runCommand("AoC Heap Stats (HF0)", "echo 'dbg heap -c 3' > /dev/acd-debug; timeout 0.1 cat /dev/acd-debug");
    runCommand("AoC Heap Stats (HF1)", "echo 'dbg heap -c 4' > /dev/acd-debug; timeout 0.1 cat /dev/acd-debug");
    runCommand("AoC DVFS (FF1)", "echo 'dbg info -c 2 DVFSFF1' > /dev/acd-debug; timeout 0.1 cat /dev/acd-debug");
    runCommand("AoC Monitor Mode Status", "echo 'monitor_mode status' > /dev/acd-debug; timeout 0.1 cat /dev/acd-debug");
    return 0;
}
