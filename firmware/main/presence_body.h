#pragma once

// Slice 8 body client. No tenant, no vendor secrets, no official cloud.

#define PRESENCE_WAKE_WORD "hai xiao ling"
#define PRESENCE_WAKE_WORD_ZH "嗨，小零"

enum presence_express_script {
    PRESENCE_EXPRESS_GREETING,
    PRESENCE_EXPRESS_THINKING,
    PRESENCE_EXPRESS_CONFIRM_SUCCESS,
    PRESENCE_EXPRESS_FAIL_APOLOGY,
    PRESENCE_EXPRESS_REJECTED
};

void presence_body_init(void);
int presence_handle_json(const char *json);
void presence_on_disconnect(void);

void handle_wake(const char *reason, const char *spoken_hint);
void handle_express(const char *script_id);
void handle_snapshot_request(const char *window_id, const char *purpose);
void handle_listen_only(const char *action, const char *room_ref, int subscribe_only);
void presence_send_snapshot_frame(const char *window_id);

int presence_publish_audio(void);
int presence_publish_video(void);
int presence_room_tts(void);
int presence_listen_open(void);
int presence_beep(void);
int presence_in_room(void);
const char *presence_last_script(void);
const char *presence_last_outbound(void);
int presence_install_app(const char *name);
int presence_setup_section_allowed(const char *section);
int presence_setup_item_allowed(const char *item);
const char *presence_launcher_name(void);
const char *presence_launcher_icon(void);
const char *presence_setup_look_for_me(void);
const char *presence_setup_app_title(void);
const char *presence_setup_welcome(void);
const char *presence_setup_wifi_intro(void);
const char *presence_activation_title(void);
const char *presence_activation_speech(void);
const char *presence_display_speech(const char *content);
int presence_speech_max_width(void);
int presence_speech_bubble_height(void);
int presence_speech_wrap(void);
int presence_speech_radius(void);
int presence_speech_pad_x(void);
int presence_speech_pad_y(void);
int presence_speech_container_y(void);
int presence_speech_container_h(void);
int presence_speech_show_arrow(void);
int presence_speech_fixed_card(void);
