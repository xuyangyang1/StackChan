
## Build

### Fetch Dependencies

```bash
python3 ./fetch_repos.py
```

### Tool Chains

[ESP-IDF v5.5.4](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/index.html)

### Build

```bash
idf.py build
```

### Host-side tests

The motion coordinate helpers can be tested without ESP-IDF hardware:

```bash
python3 tests/test_presence_ota_access.py
cmake -S tests -B build-host-tests
cmake --build build-host-tests
ctest --test-dir build-host-tests --output-on-failure
```

### Flash

```bash
idf.py flash
```

## ZeroScope presence (zeroscope-presence)

This branch points Xiaozhi `CONFIG_OTA_URL` at ZeroScope Channel Presence OTA and keeps `CheckNewVersion` so the device can discover the local WebSocket. `UpgradeFirmware` / `Hal::updateFirmware` will not flash a foreign image. NVS `wifi.ota_url` values that still name the official cloud are ignored.

```text
https://zeroscope.cn/hr-atbot/channel/presence/ota
```

Erase NVS before first flash if the device previously used the official OTA. Do not put cloud keys, tenant IDs, or official OTA hosts into `sdkconfig`. World App / wake-word changes stay out of this branch.
