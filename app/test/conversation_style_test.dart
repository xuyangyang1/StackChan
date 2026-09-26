/*
SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
SPDX-License-Identifier: MIT
*/

import 'package:flutter_test/flutter_test.dart';
import 'package:stack_chan/util/conversation_style.dart';

void main() {
  test('natural conversation preset separates chat from HR queries', () {
    expect(naturalConversationCharacter, contains('闲聊'));
    expect(naturalConversationCharacter, contains('只有用户明确询问'));
    expect(naturalConversationCharacter, contains('不超过 30 个汉字'));
    expect(naturalConversationCharacter, contains('实际匹配到多个人'));
    expect(naturalConversationCharacter, contains('不用 Markdown'));
  });

  test('selects a localized preset', () {
    expect(naturalConversationCharacterForLanguage('zh-CN'), contains('闲聊'));
    expect(
      naturalConversationCharacterForLanguage('en'),
      contains('small talk'),
    );
    expect(
      naturalConversationCharacterForLanguage(null),
      naturalConversationCharacterEnglish.trim(),
    );
  });
}
