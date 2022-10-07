#include "esp_sensor.h"

void start_watchdog(void *parameter)
{
	const char *logtag = "watchdog";
	ESP_LOGD(logtag, "Watchdog started. Going to sleep for 5 Minutes.");
	vTaskDelay(TIMEOUT_FOR_WATCHDOG_IN_MS/ portTICK_PERIOD_MS);
	ESP_LOGW(logtag, "Grrrr. Watchdog waking up. ESP haven't slept yet. Wuf Wuf! Forcing reboot");
	ESP.restart();
}

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

	TaskHandle_t xHandle_start_watchdog;
	xTaskCreate(
		start_watchdog,         /* Task function. */
		"start watchdog",       /* name of task. */
		2048,                   /* Stack size of task */
		NULL,                   /* parameter of the task */
		1,                      /* priority of the task */
		&xHandle_start_watchdog /* Task handle to keep track of created task */
	);

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
	float pressure = FLT_MAX;

	Adafruit_BME280 bme; // use I2C interface
	ESP_LOGD(logtag, "Begin BME sensor");
	if (bme.begin(0x76))
	{
		ESP_LOGD(logtag, "Read BME temperature");
		temperature = bme.readTemperature();
		ESP_LOGI(logtag, "Temperature: %f", temperature);
		ESP_LOGD(logtag, "Read BME pressure");
		pressure = bme.readPressure();
		ESP_LOGI(logtag, "Pressure: %f", pressure);
		ESP_LOGD(logtag, "Read BME temperature");
		humidity = bme.readHumidity();
		ESP_LOGD(logtag, "Humidity: %f", humidity);
	}
	else
		ESP_LOGW(logtag, "No BME sensor detected");

	SHT21 sht21;
	ESP_LOGI(logtag, "SHT21 begin");
	sht21.begin();
	ESP_LOGI(logtag, "SHT21 get humidity");
	float humidity_sht = sht21.getHumidity();
	ESP_LOGI(logtag, "humidity %f", humidity_sht);
	if(humidity_sht < HUMIDITY_MAX && humidity_sht > HUMIDITY_MIN)
		humidity = humidity_sht;
	ESP_LOGI(logtag, "SHT21 get temperature");
	float temperature_sht = sht21.getTemperature();
	ESP_LOGI(logtag, "temperature %f", temperature_sht);
	if(temperature_sht < TEMPERATURE_MAX && temperature_sht > TEMPERATURE_MIN)
		temperature = temperature_sht;

	ESP_LOGD(logtag, "Get CO2");
	MHZ19 mhz19_pwm(2, MH_Z19B_PWM_PIN);
	digitalRead(MH_Z19B_PWM_PIN);
	int co2 = mhz19_pwm.getPpmPwm();
	ESP_LOGI(logtag, "co2: %i", co2);

	MQTTClient mqttClient;
	WiFiClientSecure net;
	ESP_LOGD(logtag, "WiFiClient: set CA");
	net.setCACert(root_ca);
	ESP_LOGD(logtag, "MQTT: begin");
	mqttClient.begin(MQTT_HOST, MQTT_PORT, net);
	ESP_LOGI(logtag, "MQTT: connect");
	unsigned long timestamp = millis();
	while(!mqttClient.connect(MQTT_USER, MQTT_USER, MQTT_PASWORD) && millis() - timestamp < MQTT_TIMEOUT_IN_MS)
	{
		ESP_LOGD(logtag, "MQTT: Waiting for connection...");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	char payload[50];
	int len = sprintf(payload, "{");
	if(temperature > TEMPERATURE_MIN && temperature < TEMPERATURE_MAX)
		len += sprintf(payload + len, "\"T\": %f, ", temperature);
	if(humidity > HUMIDITY_MIN && humidity < HUMIDITY_MAX)
		len += sprintf(payload + len, "\"h\": %f, ", humidity);
	if(co2 > CO2_MIN && co2 < CO2_MAX)
		len += sprintf(payload + len, "\"co2\": %i, ", co2);
	if(pressure > PRESSURE_MIN && pressure < PRESSURE_MAX)
		len += sprintf(payload + len, "\"p\": %f, ", pressure);
	len += sprintf(payload + len, "\"ID\": \"%s\", \"v\": \"%s\"}",  WiFi.macAddress().c_str(), GIT_TAG);
	ESP_LOGI(logtag, "MQTT: publish message: %s", payload);
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
	ESP_LOGI(logtag, "Disconnect MQTT");
	mqttClient.disconnect();
	ESP_LOGI(logtag, "Disconnect WiFi");
	WiFi.disconnect();
	ESP_LOGI(logtag, "Waiting %i seconds before going to sleep", TIME_BEFORE_SLEEP_IN_MS/1000);
	vTaskDelay(TIME_BEFORE_SLEEP_IN_MS / portTICK_PERIOD_MS);
	ESP_LOGI(logtag, "Going to sleep");
	ESP.deepSleep(CYCLE_TIME_IN_S * 1000000);
}

// This is never reached since ESP.deepSleep is called before.
void loop()
{
	vTaskDelay(portMAX_DELAY); /*wait as much as possible ... */
}
