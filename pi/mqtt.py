# library for sht21: https://github.com/kif/sht21_python.git

import sht21, paho.mqtt.client as mqtt
from gpiozero import CPUTemperature
from wetterdienst.provider.dwd.observation import DwdObservationRequest, DwdObservationDataset, DwdObservationPeriod, DwdObservationResolution

# sensor on raspi
sht21 = sht21.SHT21(1)
temp = sht21.read_temperature()
hum = sht21.read_humidity()
cpu = CPUTemperature()

# DWD
station = DwdObservationRequest(parameter=DwdObservationDataset.TEMPERATURE_AIR, resolution=DwdObservationResolution.MINUTE_10, period=DwdObservationPeriod.NOW).filter_by_station_id(station_id=(433))
results = station.values.query()
for i in results:
	pressure = i.df[i.df['parameter']=='pressure_air_site'].iloc[-1]['value']
	humidity = i.df[i.df['parameter']=='humidity'].iloc[-1]['value']
	temperature_in_K = i.df[i.df['parameter']=='temperature_air_mean_005'].iloc[-1]['value']

temperature_in_C = temperature_in_K - 273.15

# MQTT
client = mqtt.Client()
client.username_pw_set("esp32", "password")
client.connect("localhost")
p = "{\"T\": %f, \"h\": %f, \"ID\": \"DWD\", \"p\": %f}" % (temperature_in_C, humidity, pressure)
client.publish("home/", payload=p)
p = "{\"T\": %f, \"h\": %f, \"ID\": \"raspi\", \"cpu\": %f}" % (temp, hum, cpu.temperature)
client.publish("home/", payload=p)
client.disconnect()