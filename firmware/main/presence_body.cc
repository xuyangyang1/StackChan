#include "presence_body.h"

#include <stdio.h>
#include <string.h>

static int subscribe_only = 1;
static int publish_audio = 0;
static int publish_video = 0;
static int room_tts = 0;
static int listen_open = 0;
static int beep = 0;
static int in_room = 0;
static char last_script[32];
static char last_outbound[256];
static char last_window[64];

// Factory entries disabled in published firmware:
// World App, App Center, Home Assistant, HA MCP, ESP-NOW,
// device weather, device joke, device encyclopedia.

static void clear_room_flags(void) {
    subscribe_only = 1;
    publish_audio = 0;
    publish_video = 0;
    room_tts = 0;
    in_room = 0;
}

static const char *find_key(const char *json, const char *key) {
    if (!json || !key) {
        return 0;
    }
    char needle[80];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *found = strstr(json, needle);
    if (!found) {
        return 0;
    }
    found += strlen(needle);
    while (*found == ' ' || *found == ':' || *found == '"') {
        if (*found == '"') {
            return found + 1;
        }
        found++;
    }
    return found;
}

static int extract_string(const char *json, const char *key, char *out, size_t out_len) {
    const char *value = find_key(json, key);
    if (!value || !out || out_len == 0) {
        return 0;
    }
    size_t i = 0;
    while (value[i] && value[i] != '"' && i + 1 < out_len) {
        out[i] = value[i];
        i++;
    }
    out[i] = '\0';
    return i > 0;
}

static int allowed_express(const char *script_id) {
    return script_id && (
        strcmp(script_id, "greeting") == 0
        || strcmp(script_id, "thinking") == 0
        || strcmp(script_id, "confirm_success") == 0
        || strcmp(script_id, "fail_apology") == 0);
}

void presence_body_init(void) {
    clear_room_flags();
    listen_open = 0;
    beep = 0;
    last_script[0] = '\0';
    last_outbound[0] = '\0';
    last_window[0] = '\0';
}

void handle_wake(const char *reason, const char *spoken_hint) {
    (void)reason;
    (void)spoken_hint;
    // Short beep then open listen. Wake word remains hai xiao ling / 嗨，小零.
    beep = 1;
    listen_open = 1;
}

void handle_express(const char *script_id) {
    if (!allowed_express(script_id)) {
        return;
    }
    snprintf(last_script, sizeof(last_script), "%s", script_id);
}

void presence_send_snapshot_frame(const char *window_id) {
    if (!window_id || window_id[0] == '\0') {
        last_outbound[0] = '\0';
        return;
    }
    snprintf(last_window, sizeof(last_window), "%s", window_id);
    snprintf(last_outbound, sizeof(last_outbound),
        "{\"type\":\"snapshot.frame\",\"windowId\":\"%s\"}", window_id);
}

void handle_snapshot_request(const char *window_id, const char *purpose) {
    (void)purpose;
    if (!window_id || window_id[0] == '\0') {
        return;
    }
    presence_send_snapshot_frame(window_id);
}

void handle_listen_only(const char *action, const char *room_ref, int join_subscribe_only) {
    (void)room_ref;
    (void)join_subscribe_only;
    if (action && strcmp(action, "leave") == 0) {
        clear_room_flags();
        return;
    }
    subscribe_only = 1;
    publish_audio = 0;
    publish_video = 0;
    room_tts = 0;
    in_room = 1;
}

int presence_handle_json(const char *json) {
    if (!json || json[0] == '\0') {
        return -1;
    }
    if (strstr(json, "scriptUrl")) {
        return -1;
    }
    char type[48];
    if (!extract_string(json, "type", type, sizeof(type))) {
        return -1;
    }
    if (strcmp(type, "wake") == 0) {
        char reason[32] = {0};
        char hint[64] = {0};
        extract_string(json, "reason", reason, sizeof(reason));
        extract_string(json, "spokenHint", hint, sizeof(hint));
        handle_wake(reason, hint);
        return 0;
    }
    if (strcmp(type, "express") == 0) {
        char script_id[32] = {0};
        extract_string(json, "scriptId", script_id, sizeof(script_id));
        if (!allowed_express(script_id)) {
            return -1;
        }
        handle_express(script_id);
        return 0;
    }
    if (strcmp(type, "snapshot.request") == 0) {
        char window_id[64] = {0};
        char purpose[32] = {0};
        extract_string(json, "windowId", window_id, sizeof(window_id));
        extract_string(json, "purpose", purpose, sizeof(purpose));
        if (window_id[0] == '\0') {
            return -1;
        }
        handle_snapshot_request(window_id, purpose);
        return 0;
    }
    if (strcmp(type, "listen_only") == 0) {
        char action[16] = {0};
        char room_ref[64] = {0};
        extract_string(json, "action", action, sizeof(action));
        extract_string(json, "roomRef", room_ref, sizeof(room_ref));
        handle_listen_only(action, room_ref, 1);
        return 0;
    }
    return -1;
}

void presence_on_disconnect(void) {
    handle_listen_only("leave", 0, 1);
}

int presence_publish_audio(void) {
    return publish_audio;
}

int presence_publish_video(void) {
    return publish_video;
}

int presence_room_tts(void) {
    return room_tts;
}

int presence_listen_open(void) {
    return listen_open;
}

int presence_beep(void) {
    return beep;
}

int presence_in_room(void) {
    return in_room;
}

const char *presence_last_script(void) {
    return last_script[0] ? last_script : 0;
}

const char *presence_last_outbound(void) {
    return last_outbound[0] ? last_outbound : 0;
}

static int same_name(const char *left, const char *right) {
    return left && right && strcmp(left, right) == 0;
}

int presence_install_app(const char *name) {
    if (same_name(name, "AI.AGENT") || same_name(name, "SETUP")) {
        return 1;
    }
    (void)name;
    return 0;
}

int presence_setup_section_allowed(const char *section) {
    return same_name(section, "无线网络")
        || same_name(section, "设备")
        || same_name(section, "硬件检测")
        || same_name(section, "固件");
}

int presence_setup_item_allowed(const char *item) {
    if (same_name(item, "检查更新")
        || same_name(item, "Check for Updates")
        || same_name(item, "解绑并重置")
        || same_name(item, "Unbind & Reset")) {
        return 0;
    }
    return item && item[0] != '\0';
}

const char *presence_launcher_name(void) {
    return "小零机器人";
}

const char *presence_launcher_icon(void) {
    return "icon_xiaoling.png";
}

const char *presence_home_asset(void) {
    return "xiaoling_robot_home.png";
}

const char *presence_home_title(void) {
    return "小零机器人";
}

const char *presence_home_subtitle(void) {
    return "AI 助手";
}

int presence_screen_width(void) {
    return 320;
}

int presence_screen_height(void) {
    return 240;
}

int presence_home_card_width(void) {
    return 320;
}

int presence_home_card_height(void) {
    return 240;
}

int presence_home_card_offset_y(void) {
    return 0;
}

int presence_home_arrow_width(void) {
    return 28;
}

int presence_home_arrow_height(void) {
    return 64;
}

int presence_home_arrow_offset_x(void) {
    return 146;
}

int presence_home_dot_size(void) {
    return 4;
}

int presence_home_dot_active(void) {
    return 7;
}

int presence_home_dot_offset_y(void) {
    return 108;
}

unsigned presence_home_theme_color(void) {
    return 0x2CEFF1;
}

const char *presence_settings_asset(void) {
    return "settings_home.png";
}

const char *presence_settings_title(void) {
    return "设置";
}

const char *presence_settings_subtitle(void) {
    return "系统设置";
}

unsigned presence_settings_theme_color(void) {
    return 0x016ADF;
}

int presence_launcher_card(const char *name) {
    return same_name(name, presence_home_title()) || same_name(name, presence_settings_title());
}

const char *presence_setup_look_for_me(void) {
    return "请在商户端找到我\n开始设置";
}

const char *presence_setup_app_title(void) {
    return "配网设置";
}

const char *presence_setup_wifi_label(void) { return "连接无线网"; }
const char *presence_setup_brightness_label(void) { return "屏幕亮度"; }
const char *presence_setup_volume_label(void) { return "扬声器音量"; }
const char *presence_setup_timezone_label(void) { return "时区"; }
const char *presence_setup_servo_label(void) { return "舵机校准"; }
const char *presence_setup_mic_label(void) { return "麦克风测试"; }
const char *presence_setup_light_label(void) { return "灯带测试"; }
const char *presence_setup_version_label(void) { return "固件版本"; }
const char *presence_setup_confirm_label(void) { return "确定"; }

const char *presence_setup_welcome(void) {
    return "欢迎使用小零机器人\n开始设置";
}

const char *presence_setup_wifi_intro(void) {
    return "先给设备连上 Wi-Fi\n再到商户端填写设备 ID";
}

const char *presence_activation_title(void) {
    return "小零机器人";
}

const char *presence_activation_speech(void) {
    return "请到商户端绑定\n填写设备 ID";
}

const char *presence_display_speech(const char *content) {
    if (!content || content[0] == '\0') {
        return content ? content : "";
    }
    if (strstr(content, "mobile app")
        || strstr(content, "Please bind")
        || strstr(content, "Look for me")
        || strstr(content, "桌上小零")
        || strstr(content, "生成配对码")
        || strstr(content, "StackChan World")) {
        return presence_activation_speech();
    }
    return content;
}

int presence_speech_max_width(void) {
    return 320;
}

int presence_speech_bubble_height(void) {
    return 48;
}

int presence_speech_wrap(void) {
    return 0;
}

int presence_speech_scroll(void) {
    return 1;
}

int presence_speech_radius(void) {
    return 0;
}

int presence_speech_pad_x(void) {
    return 12;
}

int presence_speech_pad_y(void) {
    return 6;
}

int presence_speech_container_y(void) {
    return 0;
}

int presence_speech_container_h(void) {
    return 48;
}

int presence_speech_show_arrow(void) {
    return 0;
}

int presence_speech_fixed_card(void) {
    return 1;
}
