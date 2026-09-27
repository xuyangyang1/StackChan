/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "workers.h"
#include <presence_body.h>
#include <stackchan/stackchan.h>

LV_FONT_DECLARE(font_setup_14);
#include <apps/common/toast/toast.h>
#include <mooncake_log.h>
#include <assets/assets.h>
#include <hal/hal.h>
#include <vector>
#include <string>

using namespace smooth_ui_toolkit::lvgl_cpp;
using namespace setup_workers;

static std::string _tag = "Setup-System";

struct TimezoneOption_t {
    std::string name;
    std::string tz_posix;
};

static const std::vector<TimezoneOption_t> _timezone_list = {
    {"贝克岛 UTC-12", "BIT12"},  {"中途岛 UTC-11", "SST11"}, {"檀香山 UTC-10", "HST10"},
    {"阿拉斯加 UTC-9", "AKST9"}, {"洛杉矶 UTC-8", "PST8"},   {"丹佛 UTC-7", "MST7"},
    {"芝加哥 UTC-6", "CST6"},    {"纽约 UTC-5", "EST5"},     {"哈利法克斯 UTC-4", "AST4"},
    {"圣保罗 UTC-3", "BRT3"},    {"南乔治亚 UTC-2", "GST2"}, {"亚速尔 UTC-1", "AZOT1"},
    {"伦敦 UTC+0", "GMT0"},      {"柏林 UTC+1", "CET-1"},    {"开罗 UTC+2", "EET-2"},
    {"莫斯科 UTC+3", "MSK-3"},   {"迪拜 UTC+4", "GST-4"},    {"卡拉奇 UTC+5", "PKT-5"},
    {"达卡 UTC+6", "BST-6"},     {"曼谷 UTC+7", "ICT-7"},    {"北京 UTC+8", "CST-8"},
    {"东京 UTC+9", "JST-9"},     {"悉尼 UTC+10", "AEST-10"}, {"努美阿 UTC+11", "SBT-11"},
    {"奥克兰 UTC+12", "NZST-12"}, {"斐济 UTC+13", "FJT-13"}, {"莱恩群岛 UTC+14", "LINT-14"}};

VolumeSetupWorker::VolumeSetupWorker()
{
    mclog::info("VolumeSetupWorker start");

    for (int volume = 0; volume <= 100; volume += 5) {
        _volume_levels.push_back(volume);
    }

    uint8_t current_volume = GetHAL().getSpeakerVolume();
    _original_volume       = current_volume;
    int current_index      = _volume_levels.size() - 1;
    for (size_t i = 0; i < _volume_levels.size(); i++) {
        if (_volume_levels[i] >= current_volume) {
            current_index = static_cast<int>(i);
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

    _label_volume = std::make_unique<Label>(*_panel);
    _label_volume->setText(fmt::format("{}  {}%", presence_setup_volume_label(), _volume_levels[current_index]));
    _label_volume->setTextFont(&font_setup_14);
    _label_volume->setTextColor(lv_color_hex(0x26206A));
    _label_volume->setWidth(280);
    _label_volume->setLongMode(LV_LABEL_LONG_MODE_CLIP);
    _label_volume->setTextAlign(LV_TEXT_ALIGN_CENTER);
    _label_volume->align(LV_ALIGN_CENTER, 0, -62);

    _slider = std::make_unique<Slider>(*_panel);
    _slider->align(LV_ALIGN_CENTER, 0, -8);
    _slider->setRange(0, _volume_levels.size() - 1);
    _slider->setSize(240, 14);
    _slider->setBgColor(lv_color_hex(0x615B9E), LV_PART_KNOB);
    _slider->setBgColor(lv_color_hex(0x615B9E), LV_PART_INDICATOR);
    _slider->setBgColor(lv_color_hex(0xB8D3FD), LV_PART_MAIN);
    _slider->setBgOpa(255);
    _slider->setValue(current_index);
    _slider->onValueChanged().connect([this](int32_t value) {
        _label_volume->setText(fmt::format("{}  {}%", presence_setup_volume_label(), _volume_levels[value]));
        _target_volume = _volume_levels[value];
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

VolumeSetupWorker::~VolumeSetupWorker()
{
    if (_confirmed) {
        auto volume = _volume_levels[_slider->getValue()];
        mclog::tagInfo(_tag, "final volume: {}", volume);
        GetHAL().setSpeakerVolume(volume, true);
        return;
    }

    mclog::tagInfo(_tag, "volume change cancelled, restore: {}", _original_volume);
    GetHAL().setSpeakerVolume(_original_volume, false);
}

void VolumeSetupWorker::update()
{
    if (_target_volume != -1) {
        GetHAL().setSpeakerVolume(_target_volume, false);
        _target_volume = -1;
    }
}

TimezoneWorker::TimezoneWorker()
{
    _panel = std::make_unique<uitk::lvgl_cpp::Container>(lv_screen_active());
    _panel->setSize(320, 240);
    _panel->setPos(0, 0);
    _panel->setPadding(0, 0, 0, 0);
    _panel->setBgColor(lv_color_hex(0xEDF4FF));
    _panel->setBorderWidth(0);
    _panel->setRadius(0);

    _label = std::make_unique<uitk::lvgl_cpp::Label>(_panel->get());
    _label->setText(presence_setup_timezone_label());
    _label->setTextFont(&font_setup_14);
    _label->setTextColor(lv_color_hex(0x26206A));
    _label->align(LV_ALIGN_TOP_MID, 0, 8);

    // Timezone list
    std::string options;
    for (const auto& tz : _timezone_list) {
        options += tz.name + "\n";
    }
    // Remove last newline
    if (!options.empty()) {
        options.pop_back();
    }

    _roller = std::make_unique<uitk::lvgl_cpp::Roller>(_panel->get());
    _roller->setSize(250, 148);
    _roller->setOptions(options.c_str());
    _roller->align(LV_ALIGN_CENTER, 0, 4);
    _roller->setTextFont(&font_setup_14);
    _roller->setTextColor(lv_color_hex(0x26206A));
    _roller->setBgColor(lv_color_hex(0xB8D3FD));
    _roller->setRadius(18);
    _roller->setShadowWidth(0);
    _roller->setBorderWidth(0);
    _roller->setBgColor(lv_color_hex(0x615B9E), LV_PART_SELECTED);

    // Set current selection
    std::string current_tz = GetHAL().getTimezone();
    int utc0_index         = -1;
    for (size_t i = 0; i < _timezone_list.size(); ++i) {
        if (current_tz == _timezone_list[i].tz_posix) {
            _roller->setSelected(i, LV_ANIM_OFF);
            utc0_index = -1;
            break;
        } else if (_timezone_list[i].tz_posix == "CST-8") {
            utc0_index = i;
        }
    }

    if (utc0_index >= 0) {
        _roller->setSelected(utc0_index, LV_ANIM_OFF);
    }

    _btn_confirm = std::make_unique<uitk::lvgl_cpp::Button>(_panel->get());
    _btn_confirm->label().setText(presence_setup_confirm_label());
    _btn_confirm->label().setTextFont(&font_setup_14);
    _btn_confirm->label().setTextColor(lv_color_hex(0xFFFFFF));
    _btn_confirm->label().setWidth(80);
    _btn_confirm->label().setLongMode(LV_LABEL_LONG_MODE_CLIP);
    _btn_confirm->label().setTextAlign(LV_TEXT_ALIGN_CENTER);
    _btn_confirm->setSize(120, 36);
    _btn_confirm->align(LV_ALIGN_BOTTOM_MID, 0, -8);
    _btn_confirm->onClick().connect([&]() { _confirm_flag = true; });
    _btn_confirm->setRadius(18);
    _btn_confirm->setShadowWidth(0);
    _btn_confirm->setBgColor(lv_color_hex(0x615B9E));
}

TimezoneWorker::~TimezoneWorker()
{
}

void TimezoneWorker::update()
{
    if (_confirm_flag) {
        _confirm_flag = false;

        uint16_t selected_id = _roller->getSelected();
        if (selected_id < _timezone_list.size()) {
            const auto& selected_option = _timezone_list[selected_id];
            GetHAL().setTimezone(selected_option.tz_posix);

            view::pop_a_toast("Timezone Set", view::ToastType::Success);
            mclog::tagInfo(_tag, "timezone set to: {}", selected_option.name);
        }

        _is_done = true;
    }
}

FactoryResetWorker::FactoryResetWorker(std::function<void()> beforeResetAction)
{
    _before_reset_action = std::move(beforeResetAction);

    _panel = std::make_unique<uitk::lvgl_cpp::Container>(lv_screen_active());
    _panel->setPadding(0, 0, 0, 0);
    _panel->setBgColor(lv_color_hex(0xEDF4FF));
    _panel->align(LV_ALIGN_CENTER, 0, 0);
    _panel->setBorderWidth(0);
    _panel->setSize(320, 240);
    _panel->setRadius(0);

    // Title
    _label_title = std::make_unique<uitk::lvgl_cpp::Label>(_panel->get());
    _label_title->setText("Factory Reset");
    _label_title->setTextFont(&lv_font_montserrat_24);
    _label_title->setTextColor(lv_color_hex(0x26206A));
    _label_title->align(LV_ALIGN_CENTER, 0, -80);

    // Info
    _label_info = std::make_unique<uitk::lvgl_cpp::Label>(_panel->get());
    _label_info->setTextFont(&lv_font_montserrat_16);
    _label_info->setTextColor(lv_color_hex(0x26206A));
    _label_info->align(LV_ALIGN_CENTER, 0, -20);
    _label_info->setTextAlign(LV_TEXT_ALIGN_CENTER);
    _label_info->setWidth(280);

    // Cancel Button
    _btn_cancel = std::make_unique<uitk::lvgl_cpp::Button>(_panel->get());
    apply_button_common_style(*_btn_cancel);
    _btn_cancel->align(LV_ALIGN_CENTER, -72, 60);
    _btn_cancel->setSize(112, 48);
    _btn_cancel->label().setText("Cancel");
    _btn_cancel->label().setTextFont(&lv_font_montserrat_20);
    _btn_cancel->onClick().connect([this]() { _cancel_flag = true; });

    // Confirm Button
    _btn_confirm = std::make_unique<uitk::lvgl_cpp::Button>(_panel->get());
    apply_button_common_style(*_btn_confirm);
    _btn_confirm->align(LV_ALIGN_CENTER, 72, 60);
    _btn_confirm->setSize(112, 48);
    _btn_confirm->label().setText("Confirm");
    _btn_confirm->label().setTextFont(&lv_font_montserrat_20);
    _btn_confirm->onClick().connect([this]() { _confirm_flag = true; });

    update_ui();
}

FactoryResetWorker::~FactoryResetWorker()
{
}

void FactoryResetWorker::update()
{
    if (_cancel_flag) {
        _is_done = true;
        return;
    }

    if (_confirm_flag) {
        _confirm_flag = false;
        _confirm_count++;

        if (_confirm_count >= 3) {
            mclog::tagInfo(_tag, "factory reset triggered");

            _btn_cancel.reset();
            _btn_confirm.reset();
            _label_title.reset();

            _label_info->setText("Factory Resetting...\nDo not turn off power.");
            _label_info->align(LV_ALIGN_CENTER, 0, 0);

            if (_before_reset_action) {
                _before_reset_action();
            }

            GetHAL().lvglUnlock();
            GetHAL().delay(200);
            GetHAL().factoryReset();

            while (1) {
                GetHAL().delay(200);
            }

        } else {
            update_ui();
        }
    }
}

void FactoryResetWorker::update_ui()
{
    if (_confirm_count == 0) {
        _label_info->setText("Reset all settings to factory default?\nThis cannot be undone.");
        _btn_confirm->label().setText("Reset");
        _btn_confirm->setBgColor(lv_color_hex(0xFFB8B8));
    } else if (_confirm_count == 1) {
        _label_info->setText("Are you absolutely sure?\nAll user data will be lost!");
        _btn_confirm->label().setText("Yes");
        _btn_confirm->setBgColor(lv_color_hex(0xFF8888));
    } else if (_confirm_count == 2) {
        _label_info->setText("Last Warning!\nPress Confirm to erase everything.");
        _btn_confirm->label().setText("Confirm");
        _btn_confirm->setBgColor(lv_color_hex(0xFF4444));
    }
}
