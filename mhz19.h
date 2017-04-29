#include <SoftwareSerial.h>
//#include <ESP8266WiFi.h>
//#include <Wire.h>

#define INTERVAL 5000
#define MH_Z19_RX 12
#define MH_Z19_TX 13
#define MAX_DATA_ERRORS 15 //max of errors, reset after them

long previousMillis = 0;
int errorCount = 0;

SoftwareSerial co2Serial(MH_Z19_RX, MH_Z19_TX); // define MH-Z19

void(* resetFunc) (void) = 0; //declare reset function @ address 0


int readCO2()
{

  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  // command to ask for data
  char response[9]; // for answer

  co2Serial.write(cmd, 9); //request PPM CO2
  co2Serial.readBytes(response, 9);
  if (response[0] != 0xFF)
  {
    Serial.println("Wrong starting byte from co2 sensor!");
    return -1;
  }

  if (response[1] != 0x86)
  {
    Serial.println("Wrong command from co2 sensor!");
    return -1;
   }

  int responseHigh = (int) response[2];
  int responseLow = (int) response[3];
  int ppm = (256 * responseHigh) + responseLow;
  return ppm;
}

