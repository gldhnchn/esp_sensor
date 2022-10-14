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
	mhz19_pwm.setPwmData(CALC_5000_PPM);
	digitalRead(MH_Z19B_PWM_PIN);
	int co2 = mhz19_pwm.getPpmPwm();
	ESP_LOGI(logtag, "co2: %i", co2);

	ESP_LOGD(logtag, "EEPROM begin");
	EEPROM.begin(EEPROM_SIZE);
	float temperature_last = EEPROM.readFloat(EEPROM_ADDR_TEMPERATURE);
	int co2_last = EEPROM.readInt(EEPROM_ADDR_CO2);
	float humidity_last = EEPROM.readFloat(EEPROM_ADDR_HUMIDITY);
	float pressure_last = EEPROM.readFloat(EEPROM_ADDR_PRESSURE);
	ESP_LOGD(logtag, "temperature last: %f, co2 last: %i, humidity last %f, pressure last: %f", temperature_last, co2_last, humidity_last, pressure_last);
	EEPROM.writeFloat(EEPROM_ADDR_TEMPERATURE, temperature);
	EEPROM.writeInt(EEPROM_ADDR_CO2, co2);
	EEPROM.writeFloat(EEPROM_ADDR_HUMIDITY, humidity);
	EEPROM.writeFloat(EEPROM_ADDR_PRESSURE, pressure);
	ESP_LOGD(logtag, "EEPROM commit");
	EEPROM.commit();

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
	if(temperature > TEMPERATURE_MIN && temperature < TEMPERATURE_MAX && abs(temperature - temperature_last) < TEMPERATURE_MAX_DIFF)
		len += sprintf(payload + len, "\"T\": %f, ", temperature);
	if(humidity > HUMIDITY_MIN && humidity < HUMIDITY_MAX && abs(humidity - humidity_last) < HUMIDITY_MAX_DIFF)
		len += sprintf(payload + len, "\"h\": %f, ", humidity);
	if(co2 > CO2_MIN && co2 < CO2_MAX && abs(co2 - co2_last) < CO2_MAX_DIFF)
		len += sprintf(payload + len, "\"co2\": %i, ", co2);
	if(pressure > PRESSURE_MIN && pressure < PRESSURE_MAX && abs(pressure - pressure_last) < PRESSURE_MAX_DIFF)
		len += sprintf(payload + len, "\"p\": %f, ", pressure);
	len += sprintf(payload + len, "\"ID\": \"%s\", \"v\": \"%s\"}",  WiFi.macAddress().c_str(), GIT_TAG);
	ESP_LOGI(logtag, "MQTT: publish message: %s", payload);
	if(mqttClient.publish(MQTT_TOPIC, payload))
		ESP_LOGI(logtag, "MQTT: publishing succeeded");
	else
		ESP_LOGE(logtag, "MQTT: publishing failed");
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
