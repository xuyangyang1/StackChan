/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "workers.h"
#include <presence_body.h>
#include <mooncake_log.h>
#include <hal/hal.h>

LV_FONT_DECLARE(font_setup_14);
#include <cstdint>
#include <vector>

using namespace smooth_ui_toolkit::lvgl_cpp;
using namespace setup_workers;

static std::string _tag                        = "Setup-Display";
static std::vector<uint8_t> _brightness_levels = {1, 15, 30, 45, 60, 75, 90, 100};

BrightnessSetupWorker::BrightnessSetupWorker()
{
    mclog::info("BrightnessSetupWorker start");

    uint8_t current_brightness = GetHAL().getBackLightBrightness();
    _original_brightness       = current_brightness;
    int current_index          = 0;
    for (size_t i = 0; i < _brightness_levels.size(); i++) {
        if (_brightness_levels[i] >= current_brightness) {
            current_index = i;
            break;
        }
    }

    _panel = std::make_unique<Container>(lv_screen_active());
    _panel->setSize(320, 240);
    _panel->setPos(0, 0);
    _panel->setBgColor(lv_color_hex(0xEDF4FF));
    _panel->setBorderWidth(0);
    _panel->setRadius(0);
    _panel->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

    _label_brightness = std::make_unique<Label>(*_panel);
    _label_brightness->setText(fmt::format("{}  {}%", presence_setup_brightness_label(), _brightness_levels[current_index]));
    _label_brightness->setTextFont(&font_setup_14);
    _label_brightness->setTextColor(lv_color_hex(0x26206A));
    _label_brightness->setWidth(280);
    _label_brightness->setLongMode(LV_LABEL_LONG_MODE_CLIP);
    _label_brightness->setTextAlign(LV_TEXT_ALIGN_CENTER);
    _label_brightness->align(LV_ALIGN_CENTER, 0, -62);

    _slider = std::make_unique<Slider>(*_panel);
    _slider->align(LV_ALIGN_CENTER, 0, -8);
    _slider->setRange(0, _brightness_levels.size() - 1);
    _slider->setSize(240, 14);
    _slider->setBgColor(lv_color_hex(0x615B9E), LV_PART_KNOB);
    _slider->setBgColor(lv_color_hex(0x615B9E), LV_PART_INDICATOR);
    _slider->setBgColor(lv_color_hex(0xB8D3FD), LV_PART_MAIN);
    _slider->setBgOpa(255);
    _slider->setValue(current_index);
    _slider->onValueChanged().connect([this](int32_t value) {
        _label_brightness->setText(
            fmt::format("{}  {}%", presence_setup_brightness_label(), _brightness_levels[value]));
        _target_brightness = _brightness_levels[value];
    });

    _btn_confirm = std::make_unique<Button>(*_panel);
    apply_button_common_style(*_btn_confirm);
    _btn_confirm->align(LV_ALIGN_CENTER, 0, 58);
    _btn_confirm->setSize(148, 40);
    _btn_confirm->label().setText(presence_setup_confirm_label());
    _btn_confirm->label().setTextFont(&font_setup_14);
    _btn_confirm->label().setWidth(120);
    _btn_confirm->label().setLongMode(LV_LABEL_LONG_MODE_CLIP);
    _btn_confirm->label().setTextAlign(LV_TEXT_ALIGN_CENTER);
    _btn_confirm->onClick().connect([this]() {
        _confirmed = true;
        _is_done   = true;
    });
}

void BrightnessSetupWorker::update()
{
    if (_target_brightness != -1) {
        GetHAL().setBackLightBrightness(_target_brightness, false);
        _target_brightness = -1;
    }
}

BrightnessSetupWorker::~BrightnessSetupWorker()
{
    if (_confirmed) {
        auto brightness = _brightness_levels[_slider->getValue()];
        mclog::tagInfo(_tag, "final brightness: {}", brightness);
        GetHAL().setBackLightBrightness(brightness, true);
        return;
    }

    mclog::tagInfo(_tag, "brightness change cancelled, restore: {}", _original_brightness);
    GetHAL().setBackLightBrightness(_original_brightness, false);
}
