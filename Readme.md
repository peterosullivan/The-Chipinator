# The-Chipinator
## Indoor golf chipping gamified

Components:
- Wemos D1 Mini
- KY-008 650nm Laser sensor Module
- TZT Laser Receiver Sensor Module
- PCBWayer - Solar Powered WiFi Weather Station V2.0  https://www.pcbway.com/project/gifts_detail/Solar_Powered_WiFi_Weather_Station_V2_0.html
- Piezo
- LCD Module Display Monitor 1602 5V

Compress files `gzip -c index.html > index.html.gz`
Upload a files `curl -F "file=@$PWD/index.html.gz" 192.168.0.250/upload`

One liner `gzip -c index.html > index.html.gz && curl -F "file=@$PWD/index.html.gz" 192.168.0.250/upload`

Delete a file `SPIFFS.remove("/index.html");`