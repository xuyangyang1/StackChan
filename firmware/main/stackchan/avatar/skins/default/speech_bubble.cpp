/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "default.h"
#include "presence_body.h"

using namespace uitk;
using namespace uitk::lvgl_cpp;
using namespace stackchan::avatar;

LV_IMAGE_DECLARE(default_bubble_arrow);

DefaultSpeechBubble::DefaultSpeechBubble(lv_obj_t* parent, lv_color_t primaryColor, lv_color_t secondaryColor,
                                         const lv_font_t* font)
{
    _container = std::make_unique<Container>(parent);
    _container->setRadius(0);
    _container->setAlign(LV_ALIGN_CENTER);
    _container->setBorderWidth(0);
    _container->setBgOpa(0);
    _container->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
    _container->setSize(320, presence_speech_container_h());
    _container->setPos(0, presence_speech_container_y());
    _container->setPadding(0, 0, 0, 0);

    _arrow = std::make_unique<Image>(_container->get());
    _arrow->setSrc(&default_bubble_arrow);
    _arrow->setAlign(LV_ALIGN_TOP_MID);
    _arrow->setPos(0, 0);
    _arrow->setImageRecolorOpa(LV_OPA_COVER);
    _arrow->setImageRecolor(primaryColor);
    _arrow->setHidden(!presence_speech_show_arrow());

    _bubble = std::make_unique<Container>(_container->get());
    _bubble->setRadius(presence_speech_radius());
    _bubble->setAlign(LV_ALIGN_CENTER);
    _bubble->setBorderWidth(0);
    _bubble->setBgColor(primaryColor);
    _bubble->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
    _bubble->setSize(presence_speech_max_width(), presence_speech_bubble_height());
    _bubble->setPos(0, presence_speech_show_arrow() ? 8 : 0);
    _bubble->setPadding(presence_speech_pad_y(), presence_speech_pad_y(), presence_speech_pad_x(),
                        presence_speech_pad_x());

    _text = std::make_unique<Label>(_bubble->get());
    _text->setTextColor(secondaryColor);
    _text->setTextFont(font);
    _text->setTextAlign(LV_TEXT_ALIGN_CENTER);
    _text->setAlign(LV_ALIGN_CENTER);
    _text->setPos(0, 0);
    _text->setWidth(presence_speech_max_width() - presence_speech_pad_x() * 2);
    _text->setHeight(presence_speech_bubble_height() - presence_speech_pad_y() * 2);
    _text->setLongMode(presence_speech_wrap() ? LV_LABEL_LONG_MODE_WRAP : LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);

    clearSpeech();
}

DefaultSpeechBubble::~DefaultSpeechBubble()
{
    _text.reset();
    _bubble.reset();
    _arrow.reset();
    _container.reset();
}

void DefaultSpeechBubble::setSpeech(std::string_view text)
{
    if (text.empty()) {
        clearSpeech();
        return;
    }

    _text->setText(text);
    const int pad_x = presence_speech_pad_x();
    const int pad_y = presence_speech_pad_y();
    const int width = presence_speech_fixed_card() ? presence_speech_max_width() : presence_speech_max_width();
    _bubble->setSize(width, presence_speech_bubble_height());
    _bubble->setX(0);
    _text->setWidth(width - pad_x * 2);
    _text->setHeight(presence_speech_bubble_height() - pad_y * 2);
    _text->setTextAlign(LV_TEXT_ALIGN_CENTER);
    _text->setAlign(LV_ALIGN_CENTER);
    setVisible(true);
}

void DefaultSpeechBubble::clearSpeech()
{
    _text->setText("");
    setVisible(false);
}

void DefaultSpeechBubble::setVisible(bool visible)
{
    SpeechBubble::setVisible(visible);

    _container->setHidden(!visible);
}

void DefaultSpeechBubble::setTextFont(void* font)
{
    if (_text && font) {
        _text->setTextFont((lv_font_t*)font);
    }
}
