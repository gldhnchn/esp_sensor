#include "esp_sensor.h"

/**
 * @brief arduino setup
 * This is the arduino setup function. It is run once at startup.
 */
void setup()
{
	const char *logtag = "setup";
	esp_log_level_set("*", ESP_LOG_VERBOSE);

	Serial.begin(115200);

	ESP_LOGI(logtag, "*******************************************");
	ESP_LOGI(logtag, "Hello, this is ESP Sensor.");
	ESP_LOGI(logtag, "*******************************************");

	esp32FOTA esp32fota("esp32-fota-http", GIT_TAG, false);
	FOTAConfig_t config = esp32fota.getConfig();
	config.name = "esp32-fota-http";
	config.manifest_url = OTA_FIRWAMRE_MANIFEST_URL;
	config.sem = SemverClass( GIT_TAG );
	config.check_sig = false;
	config.unsafe = false;
	CryptoMemAsset *MyRootCA = new CryptoMemAsset("Certificates Chain", root_ca, strlen(root_ca)+1 );
	config.root_ca = MyRootCA;
	esp32fota.setConfig(config);

	wifi_init();
	wifi_connect(WIFI_SSID, WIWI_PASSWORD);

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

	float temperature = FLT_MAX;
	float humidity = FLT_MAX;
	SHT21 sht21;
	ESP_LOGI(logtag, "SHT21 begin");
	sht21.begin();
	ESP_LOGI(logtag, "SHT21 get humidity");
	humidity = sht21.getHumidity();
	ESP_LOGI(logtag, "humidity %f", humidity);
	ESP_LOGI(logtag, "SHT21 get temperature");
	temperature = sht21.getTemperature();
	ESP_LOGI(logtag, "temperature %f", temperature);

	ESP_LOGD(logtag, "Get CO2");
	MHZ19 mhz19_pwm(2, MH_Z19B_PWM_PIN);
	int co2 = mhz19_pwm.getPpmPwm();
	ESP_LOGI(logtag, "co2: %i", co2);

	MQTTClient mqttClient;
	WiFiClientSecure net;
	ESP_LOGD(logtag, "WiFiClient: set CA");
	net.setCACert(root_ca);
	ESP_LOGD(logtag, "MQTT: begin");
	mqttClient.begin(MQTT_HOST, MQTT_PORT, net);
	ESP_LOGI(logtag, "MQTT: connect");
	unsigned long old_time = millis();
	unsigned long new_time = old_time;
	while(!mqttClient.connect(MQTT_USER, MQTT_USER, MQTT_PASWORD) && new_time - old_time < 10000)
	{
		ESP_LOGD(logtag, "MQTT: Waiting for connection...");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		new_time = millis();
	}
	ESP_LOGI(logtag, "MQTT: publish message");
	char payload[50];
	int len = sprintf(payload, "{");
	if(temperature > -20. && temperature < 60.)
		len += sprintf(payload + len, "\"T\": %f, ", temperature);
	if(humidity > 0. && humidity < 100.)
		len += sprintf(payload + len, "\"h\": %f, ", humidity);
	if(co2 > 0 && co2 < 5000)
		len += sprintf(payload + len, "\"co2\": %i, ", co2);
	len += sprintf(payload + len, "\"ID\": \"%s\", \"v\": \"%s\"}",  WiFi.macAddress().c_str(), GIT_TAG);
	if(mqttClient.publish(MQTT_TOPIC, payload))
		ESP_LOGI(logtag, "MQTT: publishing succeeded");
	else
		ESP_LOGE(logtag, "MQTT: publishing failed");

/*
	CRGB leds[NUM_LEDS];
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
	Led led(ONBOARDLED_PIN);
	led.blinks();
	ESP_LOGI(logtag, "Waiting 5 seconds before going to sleep");
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	ESP_LOGI(logtag, "Going to sleep");
	ESP.deepSleep(CYCLE_TIME_IN_S * 1000000);
}

// This is never reached since ESP.deepSleep is called before.
void loop()
{
	vTaskDelay(portMAX_DELAY); /*wait as much as possible ... */
}
