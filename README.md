# ESP8266WeatherStation

This demo is using ESP8266 controller with DHT11 and oled 1306 128*64 to build a weather station at home.
 * develop in platformio
 * using the packages as below:
 *    OLED Driver:  https://thingpulse.com              I2C mode  connected to IO4 and IO5 with Esp8266
 *    DHT11 Driver: DHT sensor library by Adafruit      One wire mode connected to IO14
 *    RTC Driver:                                       Three wire mode connected to IO12, IO13, IO15
 * ![avatar](https://github.com/bluejazzCHN/ESP8266WeatherStation/blob/master/schematic.png)

## RTC usage

+ 1.RTC hardware init
``` c++
ThreeWire myWire(13, 15, 12); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
Rtc.Begin();
```
+ 2.Get RTC inital time
``` c++
RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__)
```
+ 3.Just only when get time < compiled time , set compiled time to rtc hareware
``` c++
RtcDateTime now = Rtc.GetDateTime();
if (now < compiled)
{
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);    
}
```
+ 4.Get current time
``` c++
RtcDateTime now = Rtc.GetDateTime();
```

## SSD1306 usage


## Contact Info

[BluejazzCHN:](https://github.com/bluejazzCHN) songjiangzhang@hotmail.com
