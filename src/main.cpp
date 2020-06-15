/**
 * This demo is using ESP8266 controller with DHT11 and oled 1306 128*64 to build a weather station at home.
 * develop in platformio using arduino
 * in this demo using the packages as below:
 *    OLED Driver:  https://thingpulse.com     I2C mode  connected to IO4 and IO5 with Esp8266
 *    DHT11 Driver: DHT sensor library by Adafruit one wire mode connected to IO14
 *    RTC Driver:                              three wire mode connected to IO12, IO13, IO15
 */
#include <Arduino.h>
#include <TimeLib.h>
#include "DHT.h"

#include <ThreeWire.h>
#include <RtcDS1302.h>

// Include the correct display library
// For a connection via I2C using Wire include

#include <Wire.h> // Only needed for Arduino 1.6.5 and earlier

#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
// or #include "SH1106Wire.h", legacy include: `#include "SH1106.h"`
// For a connection via I2C using brzo_i2c (must be installed) include
// #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Brzo.h"
// #include "SH1106Brzo.h"
// For a connection via SPI include
// #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Spi.h"
// #include "SH1106SPi.h"

// Include the UI lib
#include "OLEDDisplayUi.h"

void printDateTime(const RtcDateTime &dt);

// Include custom images
// #include "images.h"
DHT dht;

// ======RTC functions=======
// RTC init
ThreeWire myWire(13, 15, 12); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// RTC adjust: compare RTC time and compile time
void RTCAdjust()
{
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();   //1. init RTC

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__); //2. Get compile time that will be stored in RTC harware.
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);          
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);    //3.  Just only when gettime < compiled time , set compiled time to rtc hareware
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
    printDateTime(now);
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
}

//RTC get time now
RtcDateTime timeNow()
{
  RtcDateTime now = Rtc.GetDateTime();
  // printDateTime(now);
  return now;
}
//RTC time show
void printDateTime(const RtcDateTime &dt)
{
  char datestring[20];

  snprintf_P(datestring,
             sizeof(datestring) / sizeof(datestring[0]),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
}

//====End=====

// Use the corresponding display class:

// Initialize the OLED display using SPI
// D5 -> CLK
// D7 -> MOSI (DOUT)
// D0 -> RES
// D2 -> DC
// D8 -> CS
// SSD1306Spi        display(D0, D2, D8);
// or
// SH1106Spi         display(D0, D2);

// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, 4, 5);
// SH1106 display(0x3c, D3, D5);

OLEDDisplayUi ui(&display);

int screenW = 128;
int screenH = 64;
int clockCenterX = screenW / 2;
int clockCenterY = ((screenH - 16) / 2) + 16; // top yellow part is 16 px height
int clockRadius = 23;

// utility function for digital clock display: prints leading 0
String twoDigits(int digits)
{
  if (digits < 10)
  {
    String i = '0' + String(digits);
    return i;
  }
  else
  {
    return String(digits);
  }
}

void weatherStationOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0, 0, "Weather Station");
}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  //  ui.disableIndicator();
  // Draw the clock face
  //  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  for (int z = 0; z < 360; z = z + 30)
  {
    //Begin at 0° and stop at 360°
    float angle = z;
    angle = (angle / 57.29577951); //Convert degrees to radians
    int x2 = (clockCenterX + (sin(angle) * clockRadius));
    int y2 = (clockCenterY - (cos(angle) * clockRadius));
    int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 8))));
    int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 8))));
    display->drawLine(x2 + x, y2 + y, x3 + x, y3 + y);
  }

  // display second hand
  float angle = second() * 6;
  angle = (angle / 57.29577951); //Convert degrees to radians
  int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 5))));
  int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 5))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
  //
  // display minute hand
  angle = minute() * 6;
  angle = (angle / 57.29577951); //Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 4))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 4))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
  //
  // display hour hand
  angle = hour() * 30 + int((minute() / 12) * 6);
  angle = (angle / 57.29577951); //Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 2))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 2))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  RtcDateTime compiled = Rtc.GetDateTime(); //timeNow();
  // String timenow = String(hour()) + ":" + twoDigits(minute()) + ":" + twoDigits(second());

  String timenow = String(compiled.Hour()) + ":" + twoDigits(compiled.Minute()) + ":" + twoDigits(compiled.Second());
  String daynow = String(compiled.Year()) + "-" + String(compiled.Month()) + "-" + String(compiled.Day());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x, clockCenterY + y, timenow);
  display->drawString(clockCenterX + x, clockCenterY + y - 24, daynow);
}

void tempHumiFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  String temp = "Temp:" + String(dht.getTemperature());
  String humi = "Humi:" + String(dht.getHumidity());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x, clockCenterY + y, temp);
  display->drawString(clockCenterX + x, clockCenterY + y - 24, humi);

  delay(dht.getMinimumSamplingPeriod());
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
// FrameCallback frames[] = {analogClockFrame, digitalClockFrame, tempHumiFrame};
FrameCallback frames[] = {digitalClockFrame, tempHumiFrame};

// how many frames are there?
int frameCount = 2;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = {weatherStationOverlay};
int overlaysCount = 1;

void setup()
{
  Serial.begin(9600);
  Serial.println();
  dht.setup(14);
  RTCAdjust();
  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  // Customize the active and inactive symbol
  // ui.setActiveSymbol(activeSymbol);
  // ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(TOP);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();

  unsigned long secsSinceStart = millis();
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = secsSinceStart - seventyYears * SECS_PER_HOUR;
  setTime(epoch);
}

void loop()
{
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0)
  {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}