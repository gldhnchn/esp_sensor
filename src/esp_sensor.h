#ifndef esp_sensor_h
#define esp_sensor_h

/* ++++++ */
/* config */

/* Max Min Values */
#define TEMPERATURE_MAX 50
#define TEMPERATURE_MIN -20
#define TEMPERATURE_MAX_DIFF 5
#define HUMIDITY_MAX 100
#define HUMIDITY_MIN 20
#define HUMIDITY_MAX_DIFF 5
#define PRESSURE_MAX 150000
#define PRESSURE_MIN 50000
#define PRESSURE_MAX_DIFF 50
#define CO2_MAX 5000
#define CO2_MIN 100
#define CO2_MAX_DIFF 1000

/* Timing */
#define CYCLE_TIME_IN_S 60
#define TIMEOUT_FOR_WATCHDOG_IN_MS 1000*60*5
#define TIME_BEFORE_SLEEP_IN_MS 5000

/* EEPROM */
#define EEPROM_SIZE 16
#define EEPROM_ADDR_TEMPERATURE 0
#define EEPROM_ADDR_HUMIDITY 4
#define EEPROM_ADDR_CO2 8
#define EEPROM_ADDR_PRESSURE 12

/* MQTT */
#define MQTT_HOST "example.net"  // CHANGE: IP of MQTT Broker
#define MQTT_PORT 8883  // CHANGE: Port of MQTT Broker
#define MQTT_TOPIC "home/"  // CHANGE: MQTT Topic to publish the CO2 value to
#define MQTT_USER "esp32"
#define MQTT_PASWORD "password"
#define MQTT_TIMEOUT_IN_MS 10000

/* Sensor Pins */
#define MH_Z19B_PWM_PIN 4

/* RGB LEDs */
//choose one LED type
//#define LED_TYPE_APA102
#define LED_TYPE_NEOPIXEL
#define LED_CLOCK_PIN 18 //only used for spi leds like APA102
#define LED_DATA_PIN 19
#define NUM_LEDS 1
#define LED_OVERALL_BRIGHTNESS 20 //scale 0 to 255
#define ONBOARDLED_PIN 2

/* WIFI */
#define WIFI_SSID "wlanssid"
#define WIWI_PASSWORD "wlanpassword"
#define OTA_FIRWAMRE_MANIFEST_URL "https://example.net/esp32fota.json"

/* ++++++++ */
/* includes */
#include <Arduino.h>
#include <esp32fota.h>
#include <esp_log.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <SHT21.h>
#include <float.h> 
#include <limits.h>
#include <MHZ19.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>
#include "misc.h"
#include "wifi.h"
#include "root_ca.h"

/* git version */
#if __has_include("git_info.h")
#include "git_info.h"
#else
#define GIT_TAG "0.0.1"
#endif

/* function declerations */
void setup();
void loop();
#endif // esp_sensor_h
