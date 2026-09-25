/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "hal.h"
#include <mooncake_log.h>
#include <cstdio>
#include <memory>
#include <ota.h>

static const std::string_view _tag = "HAL-OTA";

bool Hal::updateFirmware(std::function<void(std::string_view)> onLog)
{
    onLog("Checking firmware updates...");

    Ota ota;
    esp_err_t err = ota.CheckVersion();
    if (err != ESP_OK) {
        mclog::tagError(_tag, "failed to check firmware version: {}", esp_err_to_name(err));
        onLog("Failed to check firmware updates");
        return false;
    }

    if (!ota.HasNewVersion()) {
        ota.MarkCurrentVersionValid();
        mclog::tagInfo(_tag, "no new firmware version available");
        onLog("Already up to date");
        return true;
    }

    // ZeroScope presence: CheckNewVersion may still see a version field, but
    // UpgradeFirmware stays closed so the body never flashes a foreign image.
    mclog::tagWarn(_tag, "firmware upgrade disabled");
    onLog("Firmware upgrades are disabled");
    ota.MarkCurrentVersionValid();
    return true;
}
