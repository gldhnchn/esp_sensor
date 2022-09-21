#include "esp_sensor.h"

TaskHandle_t xHandle_handle_ir;
Led led(ONBOARDLED_PIN);
CRGB leds[NUM_LEDS];
bool ota_flag = false;

/**
 * @brief arduino setup
 * This is the arduino setup function. It is run once at startup.
 * Inits are done here and RTOS tasks are started here.
 */
void setup()
{
    const char *logtag = "setup";
    esp_log_level_set("*", ESP_LOG_DEBUG);

    Serial.begin(115200);

    ESP_LOGI(logtag, "*******************************************");
    ESP_LOGI(logtag, "Hello, this is ESP Sensor.");
    ESP_LOGI(logtag, "Firmware version: %s", GIT_TAG);
    ESP_LOGI(logtag, "*******************************************");

    ESP_LOGD(logtag, "Begin EEPROM");
    EEPROM.begin(EEPROM_SIZE);
    ESP_LOGD(logtag, "Read OTA flag");
    ota_flag = EEPROM.read(EEPROM_ADDR_OTA);
    ESP_LOGD(logtag, "OTA flag: %i", ota_flag);
    if (ota_flag)
    {
        ESP_LOGI(logtag, "Entering OTA mode");
        ESP_LOGI(logtag, "Setting OTA flag to 0");
        EEPROM.write(EEPROM_ADDR_OTA, 0);
        EEPROM.commit();
        init_ota(OTA_WIFI_SSID, OTA_WIWI_PASSWORD);
    }
    else
    {
        ESP_LOGD(logtag, "Entering normal mode, starting init...");
        
//init fast LED strip
#if defined LED_TYPE_NEOPIXEL && defined LED_TYPE_APA102
#error "Please specify only one LED type"
#elif defined LED_TYPE_NEOPIXEL
        FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
#elif defined LED_TYPE_APA102
        FastLED.addLeds<APA102, LED_DATA_PIN, LED_CLOCK_PIN, BGR>(leds, NUM_LEDS);
#else
#error "Please specify one LED type"
#endif
        FastLED.setBrightness(LED_OVERALL_BRIGHTNESS);
        leds[LED_INDEX_BT].setColorCode(COLOR_BT_CONNECTION_OFF);
        FastLED.show();
        //init trigger and ir handling
        create_tasks();
        FastLED.show();
        //blink for telling that setup is done
        ESP_LOGD(logtag, "Init finished: blink LED");
        led.blinks();
    }
}

/**
 * @brief arduino loop
 * This is the arduino loop function. It is not used, instead RTOS tasks are used.
 * That's why the loop task is delayed max. When OTA flag is set to active during boot, this loop is used for handling OTA.
 * 
 */
void loop()
{
    if (ota_flag)
        handle_ota();
    else
        vTaskDelay(portMAX_DELAY); /*wait as much as possible ... */
}

/**
 * @brief Create all tasks object
 * This function creates all RTOS tasks. 
 */
void create_tasks()
{
     xTaskCreate(
        handle_player_status,         /* Task function. */
        "handle_player_status",       /* name of task. */
        2048,                         /* Stack size of task */
        NULL,                         /* parameter of the task */
        1,                            /* priority of the task */
        &xHandle_handle_player_status /* Task handle to keep track of created task */
    );
}
