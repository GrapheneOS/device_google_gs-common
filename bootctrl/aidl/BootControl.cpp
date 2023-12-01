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

#define LOG_TAG "bootcontrolhal"

#include "BootControl.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <bootloader_message/bootloader_message.h>
#include <cutils/properties.h>
#include <libboot_control/libboot_control.h>
#include <log/log.h>
#include <trusty/tipc.h>

#include "DevInfo.h"
#include "GptUtils.h"

using HIDLMergeStatus = ::android::bootable::BootControl::MergeStatus;
using ndk::ScopedAStatus;

using android::bootable::GetMiscVirtualAbMergeStatus;
using android::bootable::InitMiscVirtualAbMessageIfNeeded;
using android::bootable::SetMiscVirtualAbMergeStatus;

namespace aidl::android::hardware::boot {

namespace {

// clang-format off

#define BOOT_A_PATH     "/dev/block/by-name/boot_a"
#define BOOT_B_PATH     "/dev/block/by-name/boot_b"
#define DEVINFO_PATH    "/dev/block/by-name/devinfo"

// slot flags
#define AB_ATTR_PRIORITY_SHIFT      52
#define AB_ATTR_PRIORITY_MASK       (3UL << AB_ATTR_PRIORITY_SHIFT)
#define AB_ATTR_ACTIVE_SHIFT        54
#define AB_ATTR_ACTIVE              (1UL << AB_ATTR_ACTIVE_SHIFT)
#define AB_ATTR_RETRY_COUNT_SHIFT   (55)
#define AB_ATTR_RETRY_COUNT_MASK    (7UL << AB_ATTR_RETRY_COUNT_SHIFT)
#define AB_ATTR_SUCCESSFUL          (1UL << 58)
#define AB_ATTR_UNBOOTABLE          (1UL << 59)

#define AB_ATTR_MAX_PRIORITY        3UL
#define AB_ATTR_MAX_RETRY_COUNT     3UL

// clang-format on

static std::string getDevPath(int32_t in_slot) {
    char real_path[PATH_MAX];

    const char *path = in_slot == 0 ? BOOT_A_PATH : BOOT_B_PATH;

    int ret = readlink(path, real_path, sizeof real_path);
    if (ret < 0) {
        ALOGE("readlink failed for boot device %s\n", strerror(errno));
        return std::string();
    }

    std::string dp(real_path);
    // extract /dev/sda.. part
    return dp.substr(0, sizeof "/dev/block/sdX" - 1);
}

static bool isSlotFlagSet(int32_t in_slot, uint64_t flag) {
    std::string dev_path = getDevPath(in_slot);
    if (dev_path.empty()) {
        ALOGI("Could not get device path for slot %d\n", in_slot);
        return false;
    }

    GptUtils gpt(dev_path);
    if (gpt.Load()) {
        ALOGI("failed to load gpt data\n");
        return false;
    }

    gpt_entry *e = gpt.GetPartitionEntry(in_slot ? "boot_b" : "boot_a");
    if (e == nullptr) {
        ALOGI("failed to get gpt entry\n");
        return false;
    }

    return !!(e->attr & flag);
}

static bool setSlotFlag(int32_t in_slot, uint64_t flag) {
    std::string dev_path = getDevPath(in_slot);
    if (dev_path.empty()) {
        ALOGI("Could not get device path for slot %d\n", in_slot);
        return false;
    }

    GptUtils gpt(dev_path);
    if (gpt.Load()) {
        ALOGI("failed to load gpt data\n");
        return false;
    }

    gpt_entry *e = gpt.GetPartitionEntry(in_slot ? "boot_b" : "boot_a");
    if (e == nullptr) {
        ALOGI("failed to get gpt entry\n");
        return false;
    }

    e->attr |= flag;
    gpt.Sync();

    return true;
}

static bool is_devinfo_valid;
static bool is_devinfo_initialized;
static std::mutex devinfo_lock;
static devinfo_t devinfo;

static bool isDevInfoValid() {
    const std::lock_guard<std::mutex> lock(devinfo_lock);

    if (is_devinfo_initialized) {
        return is_devinfo_valid;
    }

    is_devinfo_initialized = true;

    ::android::base::unique_fd fd(open(DEVINFO_PATH, O_RDONLY));
    ::android::base::ReadFully(fd, &devinfo, sizeof devinfo);

    if (devinfo.magic != DEVINFO_MAGIC) {
        return is_devinfo_valid;
    }

    uint32_t version = ((uint32_t)devinfo.ver_major << 16) | devinfo.ver_minor;
    // only version 3.3+ supports A/B data
    if (version >= 0x0003'0003) {
        is_devinfo_valid = true;
    }

    return is_devinfo_valid;
}

static bool DevInfoSync() {
    if (!isDevInfoValid()) {
        return false;
    }

    ::android::base::unique_fd fd(open(DEVINFO_PATH, O_WRONLY | O_DSYNC));
    return ::android::base::WriteFully(fd, &devinfo, sizeof devinfo);
}

static void DevInfoInitSlot(devinfo_ab_slot_data_t &slot_data) {
    slot_data.retry_count = AB_ATTR_MAX_RETRY_COUNT;
    slot_data.unbootable = 0;
    slot_data.successful = 0;
    slot_data.active = 1;
    slot_data.fastboot_ok = 0;
}

static int blow_otp_AR(bool secure) {
    static const char *dev_name = "/dev/trusty-ipc-dev0";
    static const char *otp_name = "com.android.trusty.otp_manager.tidl";
    int fd = 1, ret = 0;
    uint32_t cmd = secure? OTP_CMD_write_antirbk_secure_ap : OTP_CMD_write_antirbk_non_secure_ap;
    fd = tipc_connect(dev_name, otp_name);
    if (fd < 0) {
        ALOGI("Failed to connect to OTP_MGR ns TA - is it missing?\n");
        ret = -1;
        return ret;
    }

    struct otp_mgr_req_base req = {
        .command = cmd,
        .resp_payload_size = 0,
    };
    struct iovec iov[] = {
        {
            .iov_base = &req,
            .iov_len = sizeof(req),
        },
    };

    size_t rc = tipc_send(fd, iov, 1, NULL, 0);
    if (rc != sizeof(req)) {
        ALOGI("Send fail! %zx\n", rc);
        return rc;
    }

    struct otp_mgr_rsp_base resp;
    rc = read(fd, &resp, sizeof(resp));
    if (rc < 0) {
        ALOGI("Read fail! %zx\n", rc);
        return rc;
    }

    if (rc < sizeof(resp)) {
        ALOGI("Not enough data! %zx\n", rc);
        return -EIO;
    }

    if (resp.command != (cmd | OTP_RESP_BIT)) {
        ALOGI("Wrong command! %x\n", resp.command);
        return -EINVAL;
    }

    if (resp.result != 0) {
        fprintf(stderr, "AR writing error! %x\n", resp.result);
        return -EINVAL;
    }

    tipc_close(fd);
    return 0;
}

static bool blowAR() {
    int ret = blow_otp_AR(true);
    if (ret) {
        ALOGI("Blow secure anti-rollback OTP failed");
        return false;
    }

    ret = blow_otp_AR(false);
    if (ret) {
        ALOGI("Blow non-secure anti-rollback OTP failed");
        return false;
    }

    return true;
}

static constexpr MergeStatus ToAIDLMergeStatus(HIDLMergeStatus status) {
    switch (status) {
        case HIDLMergeStatus::NONE:
            return MergeStatus::NONE;
        case HIDLMergeStatus::UNKNOWN:
            return MergeStatus::UNKNOWN;
        case HIDLMergeStatus::SNAPSHOTTED:
            return MergeStatus::SNAPSHOTTED;
        case HIDLMergeStatus::MERGING:
            return MergeStatus::MERGING;
        case HIDLMergeStatus::CANCELLED:
            return MergeStatus::CANCELLED;
    }
}

static constexpr HIDLMergeStatus ToHIDLMergeStatus(MergeStatus status) {
    switch (status) {
        case MergeStatus::NONE:
            return HIDLMergeStatus::NONE;
        case MergeStatus::UNKNOWN:
            return HIDLMergeStatus::UNKNOWN;
        case MergeStatus::SNAPSHOTTED:
            return HIDLMergeStatus::SNAPSHOTTED;
        case MergeStatus::MERGING:
            return HIDLMergeStatus::MERGING;
        case MergeStatus::CANCELLED:
            return HIDLMergeStatus::CANCELLED;
    }
}

}  // namespace

BootControl::BootControl() {
    CHECK(InitMiscVirtualAbMessageIfNeeded());
}

ScopedAStatus BootControl::getActiveBootSlot(int32_t* _aidl_return) {
    int32_t slots = 0;
    getNumberSlots(&slots);
    if (slots == 0) {
        *_aidl_return = 0;
        return ScopedAStatus::ok();
    }

    if (isDevInfoValid()) {
        *_aidl_return = devinfo.ab_data.slots[1].active ? 1 : 0;
        return ScopedAStatus::ok();
    }
    *_aidl_return = isSlotFlagSet(1, AB_ATTR_ACTIVE) ? 1 : 0;
    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::getCurrentSlot(int32_t* _aidl_return) {
    char suffix[PROPERTY_VALUE_MAX];
    property_get("ro.boot.slot_suffix", suffix, "_a");
    *_aidl_return = std::string(suffix) == "_b" ? 1 : 0;
    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::getNumberSlots(int32_t* _aidl_return) {
    int32_t slots = 0;

    if (access(BOOT_A_PATH, F_OK) == 0)
        slots++;

    if (access(BOOT_B_PATH, F_OK) == 0)
        slots++;

    *_aidl_return = slots;
    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::getSnapshotMergeStatus(MergeStatus* _aidl_return) {
    HIDLMergeStatus status;
    int32_t current_slot = 0;
    getCurrentSlot(&current_slot);
    if (!GetMiscVirtualAbMergeStatus(current_slot, &status)) {
        *_aidl_return = MergeStatus::UNKNOWN;
        return ScopedAStatus::ok();
    }
    *_aidl_return = ToAIDLMergeStatus(status);
    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::getSuffix(int32_t in_slot, std::string* _aidl_return) {
    *_aidl_return = in_slot == 0 ? "_a" : in_slot == 1 ? "_b" : "";
    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::isSlotBootable(int32_t in_slot, bool* _aidl_return) {
    int32_t slots = 0;
    getNumberSlots(&slots);
    if (slots == 0) {
        *_aidl_return = false;
        return ScopedAStatus::ok();
    }
    if (in_slot >= slots)
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                INVALID_SLOT, (std::string("Invalid slot ") + std::to_string(in_slot)).c_str());

    bool unbootable;
    if (isDevInfoValid()) {
        auto &slot_data = devinfo.ab_data.slots[in_slot];
        unbootable = !!slot_data.unbootable;
    } else {
        unbootable = isSlotFlagSet(in_slot, AB_ATTR_UNBOOTABLE);
    }

    *_aidl_return = unbootable ? false: true;
    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::isSlotMarkedSuccessful(int32_t in_slot, bool* _aidl_return) {
    int32_t slots = 0;
    getNumberSlots(&slots);
    if (slots == 0) {
        // just return true so that we don't we another call trying to mark it as successful
        // when there is no slots
        *_aidl_return = true;
        return ScopedAStatus::ok();
    }
    if (in_slot >= slots)
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                INVALID_SLOT, (std::string("Invalid slot ") + std::to_string(in_slot)).c_str());

    bool successful;
    if (isDevInfoValid()) {
        auto &slot_data = devinfo.ab_data.slots[in_slot];
        successful = !!slot_data.successful;
    } else {
        successful = isSlotFlagSet(in_slot, AB_ATTR_SUCCESSFUL);
    }

    *_aidl_return = successful ? true : false;
    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::markBootSuccessful() {
    int32_t slots = 0;
    getNumberSlots(&slots);
    if (slots == 0) {
        // no slots, just return true otherwise Android keeps trying
        return ScopedAStatus::ok();
    }

    bool ret;
    int32_t current_slot = 0;
    getCurrentSlot(&current_slot);
    if (isDevInfoValid()) {
        auto const slot = current_slot;
        devinfo.ab_data.slots[slot].successful = 1;
        ret = DevInfoSync();
    } else {
        ret = setSlotFlag(current_slot, AB_ATTR_SUCCESSFUL);
    }

    if (!ret) {
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(COMMAND_FAILED,
                                                                  "Failed to set successful flag");
    }

    if (!blowAR()) {
        ALOGE("Failed to blow anti-rollback counter");
        // Ignore the error, since ABL will re-trigger it on reboot
    }

    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::setActiveBootSlot(int32_t in_slot) {
    if (in_slot >= 2) {
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                INVALID_SLOT, (std::string("Invalid slot ") + std::to_string(in_slot)).c_str());
    }

    if (isDevInfoValid()) {
        auto &active_slot_data = devinfo.ab_data.slots[in_slot];
        auto &inactive_slot_data = devinfo.ab_data.slots[!in_slot];

        inactive_slot_data.active = 0;
        DevInfoInitSlot(active_slot_data);

        if (!DevInfoSync()) {
            return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                    COMMAND_FAILED, "Could not update DevInfo data");
        }
    } else {
        std::string dev_path = getDevPath(in_slot);
        if (dev_path.empty()) {
            return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                    COMMAND_FAILED, "Could not get device path for slot");
        }

        GptUtils gpt(dev_path);
        if (gpt.Load()) {
            return ScopedAStatus::fromServiceSpecificErrorWithMessage(COMMAND_FAILED,
                                                                      "failed to load gpt data");
        }

        gpt_entry *active_entry = gpt.GetPartitionEntry(in_slot == 0 ? "boot_a" : "boot_b");
        gpt_entry *inactive_entry = gpt.GetPartitionEntry(in_slot == 0 ? "boot_b" : "boot_a");
        if (active_entry == nullptr || inactive_entry == nullptr) {
            return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                    COMMAND_FAILED, "failed to get entries for boot partitions");
        }

        ALOGV("slot active attributes %lx\n", active_entry->attr);
        ALOGV("slot inactive attributes %lx\n", inactive_entry->attr);

        // update attributes for active and inactive
        inactive_entry->attr &= ~AB_ATTR_ACTIVE;
        active_entry->attr = AB_ATTR_ACTIVE | (AB_ATTR_MAX_PRIORITY << AB_ATTR_PRIORITY_SHIFT) |
                             (AB_ATTR_MAX_RETRY_COUNT << AB_ATTR_RETRY_COUNT_SHIFT);
    }

    char boot_dev[PROPERTY_VALUE_MAX];
    property_get("ro.boot.bootdevice", boot_dev, "");
    if (boot_dev[0] == '\0') {
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                COMMAND_FAILED, "invalid ro.boot.bootdevice prop");
    }

    std::string boot_lun_path =
            std::string("/sys/devices/platform/") + boot_dev + "/pixel/boot_lun_enabled";
    int fd = open(boot_lun_path.c_str(), O_RDWR | O_DSYNC);
    if (fd < 0) {
        // Try old path for kernels < 5.4
        // TODO: remove once kernel 4.19 support is deprecated
        std::string boot_lun_path =
                std::string("/sys/devices/platform/") + boot_dev + "/attributes/boot_lun_enabled";
        fd = open(boot_lun_path.c_str(), O_RDWR | O_DSYNC);
        if (fd < 0) {
            return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                    COMMAND_FAILED, "failed to open ufs attr boot_lun_enabled");
        }
    }

    //
    // bBootLunEn
    // 0x1  => Boot LU A = enabled, Boot LU B = disable
    // 0x2  => Boot LU A = disable, Boot LU B = enabled
    //
    int ret = ::android::base::WriteStringToFd(in_slot == 0 ? "1" : "2", fd);
    close(fd);
    if (ret < 0) {
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                COMMAND_FAILED, "faied to write boot_lun_enabled attribute");
    }

    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::setSlotAsUnbootable(int32_t in_slot) {
    if (in_slot >= 2)
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                INVALID_SLOT, (std::string("Invalid slot ") + std::to_string(in_slot)).c_str());

    if (isDevInfoValid()) {
        auto &slot_data = devinfo.ab_data.slots[in_slot];
        slot_data.unbootable = 1;
        if (!DevInfoSync()) {
            return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                    COMMAND_FAILED, "Could not update DevInfo data");
        }
    } else {
        std::string dev_path = getDevPath(in_slot);
        if (dev_path.empty()) {
            return ScopedAStatus::fromServiceSpecificErrorWithMessage(
                    COMMAND_FAILED, "Could not get device path for slot");
        }

        GptUtils gpt(dev_path);
        gpt.Load();

        gpt_entry *e = gpt.GetPartitionEntry(in_slot ? "boot_b" : "boot_a");
        e->attr |= AB_ATTR_UNBOOTABLE;

        gpt.Sync();
    }

    return ScopedAStatus::ok();
}

ScopedAStatus BootControl::setSnapshotMergeStatus(MergeStatus in_status) {
    int32_t current_slot = 0;
    getCurrentSlot(&current_slot);
    if (!SetMiscVirtualAbMergeStatus(current_slot, ToHIDLMergeStatus(in_status)))
        return ScopedAStatus::fromServiceSpecificErrorWithMessage(COMMAND_FAILED,
                                                                  "Operation failed");
    return ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::boot
