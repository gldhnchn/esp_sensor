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
#define NUM_LEDS 1
#define LED_OVERALL_BRIGHTNESS 20 //scale 0 to 255
#define ONBOARDLED_PIN 2

/* OTA */
#define OTA_WIFI_SSID "wlanssid"
#define OTA_WIWI_PASSWORD "wlanpassword"

/* ++++++++ */
/* includes */
#include <Arduino.h>
#include <esp32fota.h>
#include <FastLED.h>
#include <esp_log.h>
#include "misc.h"
#include "wifi.h"
#include "root_ca.h"

/* git version */
#if __has_include("git_info.h")
#include "git_info.h"
#else
#define GIT_TAG "0.0.1"
#endif

/* global variables*/
extern Led led;
extern CRGB leds[NUM_LEDS];

/* function declerations */
void setup();
void loop();
#endif // esp_sensor_h
