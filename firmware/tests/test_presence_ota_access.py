#!/usr/bin/env python3
"""Host check: committed defaults discover ZeroScope presence OTA only."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SDKCONFIG = ROOT / "sdkconfig.defaults"
KCONFIG = ROOT / "main/Kconfig.projbuild"
PRESENCE_OTA = "https://zeroscope.cn/hr-atbot/channel/presence/ota"
OFFICIAL_DEFAULT = "https://api.tenclass.net/xiaozhi/ota/"


class PresenceOtaAccessTest(unittest.TestCase):
    def test_sdkconfig_pins_presence_ota(self):
        text = SDKCONFIG.read_text(encoding="utf-8")
        self.assertIn(f'CONFIG_OTA_URL="{PRESENCE_OTA}"', text)
        self.assertNotIn("tenclass.net", text)
        self.assertNotIn("xiaozhi.me", text)

    def test_kconfig_default_is_presence_ota(self):
        text = KCONFIG.read_text(encoding="utf-8")
        self.assertIn(f'default "{PRESENCE_OTA}"', text)
        self.assertNotIn(OFFICIAL_DEFAULT, text)


if __name__ == "__main__":
    unittest.main()
