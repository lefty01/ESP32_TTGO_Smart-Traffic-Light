# ESP32_TTGO_Smart-Traffic-Light
arduino sample sketch for ESP32 (here: LILYGO TTGO-T w. color oled) to control three 8x8 RGB-Led matrices (WS2812B)

# Current Hardware:
* MCU Espressif ESP32 LILYGO TTGO-T Color Display (https://www.banggood.com/de/LILYGO-TTGO-T-Display-ESP32-CP2104-WiFi-bluetooth-Module-1_14-Inch-LCD-Development-Board-p-1522925.html)
* 3 x 8*8 RGB-Led Matrix (WS2812B)
* 3 Input Buttons (push-button w/external pull-up), 1 rotary encoder switch (w/push-button)

## LED Martix Arrangement
                  Din
     _ _ _ _ _ _ _ _
    |_|_|_|_|_|_|_|_| 0
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_| 7
     _ _ _ _ _ _ _ _
    |_|_|_|_|_|_|_|_|64
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|71
     _ _ _ _ _ _ _ _
    |_|_|_|_|_|_|_|_|128
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|135


# Features / Ideas


