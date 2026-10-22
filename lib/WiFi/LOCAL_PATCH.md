# Local WiFi scan compatibility fix

Vendored from official espressif/arduino-esp32 2.0.16 libraries/WiFi (framework package 3.20016.0). Original LGPL-2.1-or-later notices retained.

Only functional change: WiFiScan.cpp initializes wifi_scan_config_t to zero and sets home_chan_dwell_time=30ms. The bundled ESP-IDF struct contains this field but the original wrapper leaves it uninitialized. This can pass indeterminate parameters to esp_wifi_scan_start.

Project-local library wins PlatformIO dependency lookup; global toolchain files are unchanged. Source build and host regression cover this file. Real RF behavior remains unverified.
