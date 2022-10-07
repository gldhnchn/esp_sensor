#ifndef wifi_h
#define wifi_h

#include <WiFi.h>

#define TIMEOUT_FOR_RECONNECT_IN_MS 5000

int wifi_init();
int wifi_connect(const char* ssid, const char* pw);

#endif //wifi_h

