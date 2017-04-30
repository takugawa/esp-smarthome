#include <Ticker.h>
#include <Wire.h>
#include "oled1306.h"
#include "SSD1306.h" 
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
#include "fonts.h"
#include "icons.h"
#include "i2cCfg.h"
#include "mhz19.h"
#include <Adafruit_GFX.h>

extern "C" {
#include "user_interface.h"
}

// Wifi Credentials
// Leaving * will try to connect with SDK saved credentials
//#define MY_SSID     "IT"
//#define MY_PASSWORD "q1w2e3r4--"

#define MY_SSID     ""
#define MY_PASSWORD ""


char ssid[33] ;
char password[65];


//for ESP.getVCC//
ADC_MODE(ADC_VCC);
int volt = ESP.getVcc()/1024.00f;

Ticker ticker;
bool readyForUpdate = false;  // flag to launch update (I2CScan) basic setup

bool has_display          = false;  // if I2C display detected
uint8_t NumberOfI2CDevice = 0;      // number of I2C device detected
int8_t NumberOfNetwork    = 0;      // number of wifi networks detected




/* ======================================================================
  Function: updateData
  Purpose : update by rescanning I2C bus
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
void updateData(OLEDDisplay *display) {
  // connected
  if ( WiFi.status() == WL_CONNECTED  ) {
 //   LedRGBON(COLOR_GREEN);
  } else {
 //   LedRGBON(COLOR_ORANGE);
  }

  drawProgress(display, 0, "Scanning I2C...");
  NumberOfI2CDevice = i2c_scan();
  // Simulate slow scan to be able to see on display
  for (uint8_t i = 1; i < 100; i++) {
    drawProgress(display, i, "Scanning I2C...");
    delay(2);
  }
  drawProgress(display, 100, "Done...");
 readyForUpdate = false;
}

/* ======================================================================
  Function: drawProgress
  Purpose : prograss indication
  Input   : OLED display pointer
          percent of progress (0..100)
          String above progress bar
          String below progress bar
  Output  : -
  Comments: -
  ====================================================================== */
void drawProgress(OLEDDisplay *display, int percentage, String labeltop, String labelbot) {
  if (has_display) {
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(Roboto_Condensed_Bold_Bold_16);
    display->drawString(64, 8, labeltop);
    display->drawProgressBar(10, 28, 108, 12, percentage);
    display->drawString(64, 48, labelbot);
    display->display();
  }
}

/* ======================================================================
  Function: drawProgress
  Purpose : prograss indication
  Input   : OLED display pointer
          percent of progress (0..100)
          String above progress bar
  Output  : -
  Comments: -
  ====================================================================== */
void drawProgress(OLEDDisplay *display, int percentage, String labeltop ) {
  drawProgress(display, percentage, labeltop, String(""));
}

/* ======================================================================
  Function: drawFrameHWInfo
  Purpose : HW & SW version
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
//void drawFrameHWInfo(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
void drawFrameHWInfo(void)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Roboto_Medium_Plain_12);
  display.drawString(0, 0, "HWInfoID:");
  display.drawString(60, 0, String(system_get_chip_id()));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Roboto_10);
  display.drawString(0, 12, "sdk:");
  display.drawString(45, 12, system_get_sdk_version());
  display.drawString(0, 22, "boot:");
  display.drawString(45, 22, String(system_get_boot_version()));
  display.drawString(0, 32, "userbin:");
  display.drawString(45, 32, String(system_get_userbin_addr()));
  display.drawString(0, 42, "memFree/Totl:");
  display.drawString(80, 42, String(ESP.getFreeHeap()/1024));
  //display->drawString(80, 42, String(ESP.getSketchSize()/1024));
  display.drawString(98, 42, String(ESP.getFlashChipSize()/1024));
  display.drawString(0, 52, "vcc:");
  display.drawString(22, 52, String(ESP.getVcc()/1024.00f));
  ui.disableIndicator();
  display.display();
}

/* ======================================================================
  Function: drawWiFiSignal
  Purpose : HW & SW version
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
  /*
void drawWiFiSignal(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->clear();
  display->drawXbm(x + (128 - wifi_width) / 2, y, wifi_width, wifi_height, wifi_width_bits);
  ui.disableIndicator();
}
*/



/* ======================================================================
  Function: drawFrameWifi
  Purpose : Mac & IP address 
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
void drawFrameWifi(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Roboto_Condensed_Bold_Bold_16);
  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
  // on how to create xbm files
 // display->drawXbm( x + (128 - WiFi_width) / 2, 0, WiFi_width, WiFi_height, WiFi_bits);
  display->drawString(x + 64, 0, "Net info");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Roboto_10);
  display->drawString(x + 0, 16, WiFi.localIP().toString());
  display->drawString(x + 0, 26, WiFi.macAddress());
  display->drawString(x + 0, 36, "connect to:");
  display->drawString(x + 57, 36, WiFi.SSID());
  display->drawString(x + 0, 46, "signal:");
  display->drawString(x + 57, 46, String(WiFi.RSSI()));
  ui.disableIndicator();
}

/* ======================================================================
  Function: drawFrameI2C
  Purpose : I2C info screen (called by OLED ui)
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
void drawFrameI2C(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  char buff[16];
  sprintf(buff, "%d I2C Device%c", NumberOfI2CDevice, NumberOfI2CDevice > 1 ? 's' : ' ');

  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Roboto_Condensed_Bold_Bold_16);
  display->drawString(x + 64, y +  0, buff);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  //display->setFont(Roboto_Condensed_Plain_16);
  display->setFont(Roboto_Condensed_12);

  for (uint8_t i = 0; i < NumberOfI2CDevice; i++) {
    if (i < I2C_DISPLAY_DEVICE)
      display->drawString(x + 0, y + 16 + 12 * i, i2c_dev[i]);
  }
  ui.disableIndicator();
}

/* ======================================================================
  Function: drawFrameNet
  Purpose : WiFi network info screen (called by OLED ui)
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
void drawFrameNet(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  char buff[64];
  sprintf(buff, "%d Wifi Network", NumberOfNetwork);
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Roboto_Condensed_Bold_Bold_16);
  display->drawString(x + 64, y + 0 , buff);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Roboto_Condensed_12);

  for (int i = 0; i < NumberOfNetwork; i++) {
    // Print SSID and RSSI for each network found
    if (i < WIFI_DISPLAY_NET) {
      sprintf(buff, "%s %c", WiFi.SSID(i).c_str(), WiFi.encryptionType(i) == ENC_TYPE_NONE ? ' ' : '*' );
      display->drawString(x + 0, y + 16 + 12 * i, buff);
    }
  }

  ui.disableIndicator();
}

/* ======================================================================
  Function: drawFrameLogo
  Purpose : Company logo info screen (called by OLED ui)
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
 /*
void drawFrameLogo(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->clear();
  display->drawXbm(x + (128 - ch2i_width) / 2, y, ch2i_width, ch2i_height, ch2i_bits);
  ui.disableIndicator();
}
*/

/* ======================================================================
  Function: drawFrameSensors
  Purpose : Sensors info screen (called by OLED ui)
  Input   : OLED display pointer
  Output  : -
  Comments: -
  ====================================================================== */
void drawFrameSensors(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
 display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Roboto_Condensed_Bold_Bold_16);
  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
  // on how to create xbm files
 // display->drawXbm((128 - WiFi_width) / 2, 0, WiFi_width, WiFi_height, WiFi_bits);
   //display->drawXbm((128 - wifi_width_s), 0, wifi_width_s, wifi_height_s, wifi_bits_s);


//display signal strength
      if (WiFi.RSSI() <= -90 && WiFi.RSSI() > -100) display->drawXbm((128 - wifi_width), 2, wifi_width, wifi_height, wifi_bits00);
else if (WiFi.RSSI() >= 0 ) display->drawXbm((128 - wifi_width), 2, wifi_width, wifi_height, wifi_bits00);      
else if (WiFi.RSSI() <= -80 && WiFi.RSSI() > -90 ) display->drawXbm((128 - wifi_width), 2, wifi_width, wifi_height, wifi_bits20);
else if (WiFi.RSSI() <= -70 && WiFi.RSSI() > -80 ) display->drawXbm((128 - wifi_width), 2, wifi_width, wifi_height, wifi_bits40);
else if (WiFi.RSSI() <= -60 && WiFi.RSSI() > -70) display->drawXbm((128 - wifi_width), 2, wifi_width, wifi_height, wifi_bits60);
else if (WiFi.RSSI() <= -50 && WiFi.RSSI() > -60) display->drawXbm((128 - wifi_width), 2, wifi_width, wifi_height, wifi_bits80);
else if (WiFi.RSSI() <= -40 && WiFi.RSSI() > -50) display->drawXbm((128 - wifi_width), 2, wifi_width, wifi_height, wifi_bits100);

 //display battery charge
 if (ESP.getVcc()/1024.00f > 1 && ESP.getVcc()/1024.00f < 2) {display->drawXbm(0, 5, bat_width, bat_height, bat_bmp0);}
 else if (ESP.getVcc()/1024.00f > 2 && ESP.getVcc()/1024.00f < 2.1 ) {display->drawXbm(0, 5, bat_width, bat_height, bat_bmp10);}
 else if (ESP.getVcc()/1024.00f > 2.1 && ESP.getVcc()/1024.00f < 2.3) {display->drawXbm(0, 5, bat_width, bat_height, bat_bmp25);}
 else if (ESP.getVcc()/1024.00f > 2.3 && ESP.getVcc()/1024.00f < 2.5) {display->drawXbm(0, 5, bat_width, bat_height, bat_bmp50);}
 else if (ESP.getVcc()/1024.00f > 2.5 && ESP.getVcc()/1024.00f < 2.8) {display->drawXbm(0, 5, bat_width, bat_height, bat_bmp75);}
 else if (ESP.getVcc()/1024.00f > 2.8 && ESP.getVcc()/1024.00f < 3) {display->drawXbm(0, 5, bat_width, bat_height, bat_bmp90);}
 else if (ESP.getVcc()/1024.00f > 3 && ESP.getVcc()/1024.00f < 4) {display->drawXbm(0, 5, bat_width, bat_height, bat_bmp100);}

//  display->drawString(x + 64, 0, "Sensors info");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Roboto_10);


//display CO2
  display->drawString(5, 17, "CO2");
    if (readCO2() < 200)  { display->drawString(80, 16, "not work"); }
    else if (readCO2() < 600 )  { display->drawString(80, 16, "good"); }
    else if (readCO2() > 600 && readCO2() < 1000)  { display->drawString(80, 16, "normal"); }
    else if (readCO2() > 1000 && readCO2() < 2000)  { display->drawString(80, 16, "bad"); }
    else if (readCO2() > 2000) { display->drawString(80, 16, "alarm"); }

  display->drawString(54, 1, "ppm");
  display->drawString(80, 1, "status");
  display->drawString((128/3)+11, 40, "°C");
  display->drawString((128*0,67)+35, 40, "%");
  
  display->drawLine(00, 36, 128, 36);//gorizontal line
  display->drawLine((128/2+3), 36, (128/2+3), 64);//vertical line
    
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Roboto_Mono_Bold_20);
  display->drawString((128-24)/2, 11, String(readCO2()));
  display->drawString((128/3)-12, 40, "24.3");
  display->drawString((128*0,67)+22, 40, "36");

  ui.disableIndicator();
}


// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
FrameCallback frames[] = { drawFrameSensors };
//REM OLD//FrameCallback frames[] = {drawWiFiSignal, drawFrameHWInfo, drawFrameWifi, drawFrameI2C, drawFrameSensors, drawFrameNet};
//REM OLD//int numberOfFrames = 4;
int numberOfFrames = 1;

/*
FrameCallback startframes[] = { drawFrameHWInfo };
int numberOfstartFrames = 1;
*/



/* ======================================================================
  Function: setReadyForUpdate
  Purpose : Called by ticker to tell main loop we need to update data
  Input   : -
  Output  : -
  Comments: -
  ====================================================================== */
void setReadyForUpdate() {
  Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true; // basic setup
}

/* ======================================================================
  Function: setup
  Purpose : you should know ;-)
  ====================================================================== */
void setup()
{
  uint16_t led_color ;
  char thishost[33];
  uint8_t pbar = 0;
  
  Serial.begin(115200);
  Serial.print(F("\r\nBooting on "));
  Serial.println(ARDUINO_BOARD);



  
  /*
   * Settings for display
   */
  Wire.begin(SDA_PIN, SDC_PIN);
  Wire.setClock(100000);

  if (i2c_scan(I2C_DISPLAY_ADDRESS)) {
    has_display = true;
  } else {
    if (i2c_scan(I2C_DISPLAY_ADDRESS + 1)) {
      has_display = true;
    }
  }

  if (has_display) {
    Serial.println(F("Display found"));
    // initialize dispaly
    display.init();
    display.flipScreenVertically();
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setContrast(255);
    delay(500);
  }

  //Started HWInfo screen
    drawFrameHWInfo();
    delay(10000);
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  delay(100);
  strcpy(ssid, MY_SSID);
  strcpy(password, MY_PASSWORD);

  // empty sketch SSID, try with SDK ones
  if ( *ssid == '*' && *password == '*' ) {
    // empty sketch SSID, try autoconnect with SDK saved credentials
    Serial.println(F("No SSID/PSK defined in sketch\r\nConnecting with SDK ones if any"));
    struct station_config conf;
    wifi_station_get_config(&conf);
    strcpy(ssid, reinterpret_cast<char*>(conf.ssid));
    strcpy(password, reinterpret_cast<char*>(conf.password));
  }

  Serial.println(F("WiFi scan start"));
  drawProgress(&display, pbar, F("Scanning WiFi"));
 
  //drawFrameBatInfo(&display, pbar, F("battery %"));

  // WiFi.scanNetworks will return the number of networks found
  led_color = 0;
  NumberOfNetwork = 0;
  WiFi.scanNetworks(true);
  // Async, wait start
  while (WiFi.scanNetworks() != WIFI_SCAN_RUNNING );

  do {
     delay(5);

    // Rainbow loop
    if (++led_color > 360)
      led_color = 360;

    // WiFi scan max 50% of progress bar
    pbar = led_color * 100 / 360 / 2;
    drawProgress(&display, pbar, F("Scanning WiFi"));
    //drawFrameBatInfo(&display, pbar, F("battery %"));

    NumberOfNetwork = WiFi.scanComplete();
    //Serial.printf("NumberOfNetwork=%d\n", NumberOfNetwork);
  } while (NumberOfNetwork == WIFI_SCAN_RUNNING || NumberOfNetwork == WIFI_SCAN_FAILED);

  Serial.println(F("scan done"));

  Serial.println(F("I2C scan start"));
  pbar = 50;
  drawProgress(&display, pbar, F("Scanning I2C"));
  delay(200);
  NumberOfI2CDevice = i2c_scan();
  Serial.println(F("scan done"));

  // Set Hostname for OTA and network (add only 2 last bytes of last MAC Address)
  sprintf_P(thishost, PSTR("ScanI2CWiFi-%04X"), ESP.getChipId() & 0xFFFF);

  Serial.printf("connecting to %s with psk %s\r\n", ssid, password );
  WiFi.begin(ssid, password);

  // Loop until connected or 20 sec time out
#define WIFI_TIME_OUT 20
  unsigned long this_start = millis();
 // led_color = 360;
  while ( WiFi.status() != WL_CONNECTED && millis() - this_start < (WIFI_TIME_OUT * 1000) ) {
    // 125 ms wait
    for (uint8_t j = 0; j < 125; j++) {
 
    }
    if (pbar++ > 99) {
      pbar = 99;
    }
    drawProgress(&display, pbar, F("Connecting WiFi"), ssid);
  }

  if (  WiFi.status() == WL_CONNECTED  ) {
    Serial.printf("OK from %s@", thishost);
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println(F("Error unable to connect to WiFi"));
  }

  Serial.println(F("Setup done"));
  drawProgress(&display, 100, F("Setup Done"));
  
  //Frames_of_HW_INFO
 /* if (has_display) {
    ui.setTargetFPS(30);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setstartFrames(startframes, numberOfstartFrames);
    ui.init();
    display.flipScreenVertically();
  }
*/

  
  if (has_display) {
    ui.setTargetFPS(30);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, numberOfFrames);
    ui.init();
    display.flipScreenVertically();
  }

  //updateData(&display);

  // Rescan I2C every 600 seconds
  ticker.attach(600, setReadyForUpdate);
  

}

/* ======================================================================
  Function: loop
  Purpose : you should know ;-)
  ====================================================================== */
void loop()
{

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }


/*
 * CO2 SETTINGS
 
 unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < INTERVAL)
    return;
  previousMillis = currentMillis;
 // Serial.println("loop started");

  if (errorCount > MAX_DATA_ERRORS)
  {
    Serial.println("Too many errors, resetting");
    delay(2000);
    resetFunc();
  }


  //Serial.println("reading data:");
  int ppm = readCO2();
  bool dataError = false;
  Serial.println("  PPM = " + String(ppm));
  delay(20000);
*/

 // reboot ESP if sensort not alive
 /*if (readCO2() <10) {
   
  ESP.reset();
}
*/
 // int mem = ESP.getFreeHeap();
 // Serial.println("  Free RAM: " + String(mem));

}
