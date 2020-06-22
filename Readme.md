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

TODO; 
  Add SSL - https://stackoverflow.com/questions/42766670/arduino-library-for-esp8266-wificlientsecure-which-ssl-certificates-are-need, https://nofurtherquestions.wordpress.com/2016/03/14/making-an-esp8266-web-accessible/

  Add delete a file route - `SPIFFS.remove("/index.html");`
