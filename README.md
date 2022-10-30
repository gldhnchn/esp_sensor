# ESP sensor
This is a IOT project to measure and plot CO2, temperature, humidity and pressure.
![screenshot grafana](https://github.com/gldhnchn/esp_sensor/blob/main/doc/screenshot_grafana.png)
### Hardware
* ESP32, microcontroller, https://funduinoshop.com/elektronische-module/wireless-iot/esp-wifi/funduino-esp32-wroom32-wlan-wifi-entwicklungsboard-mit-cp2102
* SHT21, temperature, humidity sensor, https://funduinoshop.com/elektronische-module/sensoren/temperatur/gy-213v-sht21-sensor-fuer-temperatur-und-luftfeuchtigkeit-i2c
* MHZ19-B, CO2 sensor, https://funduinoshop.com/elektronische-module/sensoren/gase/co2-sensor-vergleichbar-mit-mh-z19b-mit-pinleiste
* BME280, temperature, humidity, pressure sensor, https://funduinoshop.com/elektronische-module/sensoren/druck-gewicht/bme280-luftdruck-feuchtigkeits-temperatursensor-i2c/spi
* Raspberry Pi 3b+, mini computer
### Software
* ESP32 firmware from this repo
* [mosquitto](https://github.com/eclipse/mosquitto) as mqtt broker
* [python script](https://github.com/gldhnchn/esp_sensor/blob/main/pi/mqtt.py) for reading a SHT21 sensor on a raspberry pi GPIO and Deutscher Wetterdienst data
* [node-red](https://github.com/node-red/node-red) to subscribe to the mqtt broker, process the data and write it to influxdb
* [influxdb](https://github.com/influxdata/influxdb) to store data
* [grafana](https://github.com/grafana/grafana) to plot data
* [nginx](https://github.com/nginx/nginx) as reverse proxy
### System strucutre
![system structure](https://github.com/gldhnchn/esp_sensor/blob/main/doc/esp_sensor.svg)
### Build
* Install [platformio](https://platformio.org/install/)
* `git clone https://github.com/gldhnchn/esp_sensor.git`
* `cd esp_sensor`
* `make`
### Used libraries
* [esp32FOTA](https://github.com/chrisjoyce911/esp32FOTA) by [chrisjoyce911](https://github.com/chrisjoyce911/esp32FOTA)
* [MQTT](https://github.com/256dpi/arduino-mqtt) by [256dpi](https://github.com/256dpi/arduino-mqtt)
* [SHT21](https://github.com/gldhnchn/SHT21) by [markbeee](https://github.com/markbeee) forked by [gldhnchn](https://github.com/gldhnchn/)
* [MHZ19](https://github.com/gldhnchn/MHZ19) by [crisap94](https://github.com/crisap94/) forked by [gldhnchn](https://github.com/gldhnchn/)
* [Adafruit_BME280_Library](https://github.com/adafruit/Adafruit_BME280_Library) by [adafruit](https://github.com/adafruit/)
