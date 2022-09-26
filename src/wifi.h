#ifndef wifi_h
#define wifi_h

#include <WiFi.h>

int wifi_init();
int wifi_connect(const char* ssid, const char* pw);

#endif //wifi_h

