#include "wifi.h"

const char *wifi_logtag = "wifi";

/**
 * @brief init wifi
 * call once
 * @return
 */
int wifi_init()
{
	ESP_LOGD(wifi_logtag, "Set WiFi mode");
	WiFi.mode(WIFI_STA);
	ESP_LOGD(wifi_logtag, "Set WiFi auto reconnect true");
	WiFi.setAutoReconnect(true);
	return 0;
}

int wifi_connect(const char* ssid, const char* pw)
{
	ESP_LOGD(wifi_logtag, "Begin WiFi");
	WiFi.begin(ssid, pw);
	unsigned long old_time = millis();
	unsigned long new_time = old_time;
	while (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		ESP_LOGD(wifi_logtag, "Waiting for connection...");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		new_time = millis();
		if(new_time - old_time > 5000)
		{
			ESP_LOGI(wifi_logtag, "Trying reconnect.");
			WiFi.reconnect();
			old_time = millis();
		}
	}
	ESP_LOGD(wifi_logtag, "Connection established!");
	ESP_LOGI(wifi_logtag, "IP address: %s", WiFi.localIP().toString().c_str());
	return 0;
}