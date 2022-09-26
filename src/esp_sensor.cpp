#include "esp_sensor.h"

Led led(ONBOARDLED_PIN);
CRGB leds[NUM_LEDS];

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
	ESP_LOGI(logtag, "*******************************************");

	esp32FOTA esp32fota("esp32-fota-http", GIT_TAG, false);
	FOTAConfig_t config = esp32fota.getConfig();
	config.name = "esp32-fota-http";
	config.manifest_url = "https://raspberrypi.nee1974kl4pqrpa7.myfritz.net/esp32fota.json";
	config.sem = SemverClass( GIT_TAG );
	config.check_sig = false;
	config.unsafe = false;
	CryptoMemAsset *MyRootCA = new CryptoMemAsset("Certificates Chain", root_ca, strlen(root_ca)+1 );
	config.root_ca = MyRootCA;
	esp32fota.setConfig(config);

	wifi_init();
	wifi_connect(WIFI_SSID, WIWI_PASSWORD);

/*
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
*/

	led.blinks();

	bool updatedNeeded = esp32fota.execHTTPcheck();
	if (updatedNeeded)
	{
		ESP_LOGI(logtag, "Update available");
		esp32fota.execOTA();
	}
	else
	{
		ESP_LOGI(logtag, "No updated needed");
	}
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	ESP_LOGI(logtag, "Going to sleep");
	ESP.deepSleep(10 * 1000000);
}

// This is never reached since ESP.deepSleep is called before.
void loop()
{
	
	vTaskDelay(portMAX_DELAY); /*wait as much as possible ... */
}
