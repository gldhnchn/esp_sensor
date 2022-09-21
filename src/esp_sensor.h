#ifndef esp_sensor_h
#define esp_sensor_h

/* ++++++ */
/* config */

/* RGB LEDs */
//choose one LED type
//#define LED_TYPE_APA102
#define LED_TYPE_NEOPIXEL
#define LED_CLOCK_PIN 18 //only used for spi leds like APA102
#define LED_DATA_PIN 19
#define NUM_LEDS 4
#define LED_OVERALL_BRIGHTNESS 20 //scale 0 to 255
//FASTLED_ESP32_I2S is defined to tell FastLED to use I2S driver instead of RMT. Reference for this option in FastLED/platforms/esp/32/clockless_i2s_esp32.h. There is a conflict when using RMT with both LED and IR, although it should be possible according to the options in FastLED/platforms/esp/32/clockless_rmt_esp32.h. Maybe there is a connection to this issue: https://github.com/FastLED/FastLED/issues/835
#define FASTLED_ESP32_I2S true
#define ONBOARDLED_PIN 2

/* OTA */
#define OTA_WIFI_SSID "wlanssid"
#define OTA_WIWI_PASSWORD "wlanpassword"
#define EEPROM_ADDR_OTA 0
#define EEPROM_SIZE 1

/* ++++++++ */
/* includes */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include "misc.h"
#include <FastLED.h>
#include <esp_log.h>
#include "ota.h"
#include <EEPROM.h>

/* git version */
#if __has_include("git_info.h")
#include "git_info.h"
#else
#define GIT_TAG "v0.1.3"
#endif

/* global variables*/
extern TaskHandle_t xHandle_handle_ir;
extern Led led;
extern CRGB leds[NUM_LEDS];

/* function declerations */
void setup();
void loop();
void create_tasks();
#endif // esp_sensor_h
