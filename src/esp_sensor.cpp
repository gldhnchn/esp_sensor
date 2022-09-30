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
	HTU2xD_SHT2x_SI70xx sht21(SHT2x_SENSOR, HUMD_12BIT_TEMP_14BIT); //sensor type, resolution
	ESP_LOGD(logtag, "Start sensor SHT21");
	if (sht21.begin() != true) //reset sensor, set heater off, set resolution, check power (sensor doesn't operate correctly if VDD < +2.25v)
	{
		ESP_LOGE(logtag, "SHT21 not connected, fail or VDD < +2.25v"); //(F()) save string to flash & keeps dynamic memory free
		delay(5000);
	}
	else
	{
		ESP_LOGI(logtag, "SHT21 OK");
		temperature = sht21.readTemperature(); //accuracy +-0.3C in range 0C..60C at 14-bit
		if (temperature != HTU2XD_SHT2X_SI70XX_ERROR) //HTU2XD_SHT2X_SI70XX_ERROR = 255, library returns 255 if error occurs
			ESP_LOGI(logtag, "Temperatur is %f", temperature);
		else
			ESP_LOGE(logtag, "Error reading temperature");
		delay(500); //measurement with high frequency leads to heating of the sensor, see NOTE
		if (temperature != HTU2XD_SHT2X_SI70XX_ERROR)         //if temperature OK, measure RH & calculate compensated humidity
			humidity = sht21.getCompensatedHumidity(temperature); //accuracy +-2% in range 0%..100%/0C..80C at 12-bit, to compensates influence of T on RH
		else
			humidity = sht21.readHumidity(); ////accuracy +-2% in range 20%..80%/25C at 12-bit
		if (humidity != HTU2XD_SHT2X_SI70XX_ERROR)
			ESP_LOGI(logtag, "Humidity is %f", humidity);
		else
			ESP_LOGE(logtag, "Error reading humidity");
	}

	int co2 = INT_MAX;
	int co2_temp = INT_MAX;
	HardwareSerial uart2(2);
	MHZ19 mhz19;
	ESP_LOGD(logtag, "Begin uart2 for MHZ19");
	uart2.begin(MHZ19_BAUDRATE);
	ESP_LOGD(logtag, "Begin MHZ19");
	mhz19.begin(uart2);
	ESP_LOGD(logtag, "Set auto calibration on");
	mhz19.autoCalibration();
	ESP_LOGD(logtag, "Get CO2");
	co2 = mhz19.getCO2();
	ESP_LOGI(logtag, "CO2 is %i", co2);
	ESP_LOGD(logtag, "Get CO2 sensor temperature");
	co2_temp = mhz19.getTempAdjustment();
	ESP_LOGI(logtag, "CO2 sensor temperature is %i", co2_temp);

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
	sprintf(payload, "{\"T\": %f, \"h\": %f, \"co2\": %i, \"co2t\": %i, \"ID\": \"%s\"}", temperature, humidity, co2, co2_temp, WiFi.macAddress().c_str());
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
