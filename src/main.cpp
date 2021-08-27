/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "BluetoothSerial.h"
#include "Button2.h"

#define BUTTON_1 35
#define BUTTON_2 0
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
const int backlight[5] = {16,32,64,128,255};
byte tft_brightness=4;

BluetoothSerial SerialBT;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

TFT_eSPI tft = TFT_eSPI();


void tft_init() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(1);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 255);
}

void displayConnectionState(uint8_t state) {
  char buf[32];
  switch (state)
  {
  case 1:
    sprintf(buf, "Waiting");
    break;
  case 2:
    sprintf(buf, "Connected");
    break;
  
  default:
    break;
  }
  
  tft.setTextColor(TFT_SKYBLUE);
  tft.fillRect(0,25,tft.width(),tft.height(), TFT_BLACK);
  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.setCursor(0,tft.height()-10);
  tft.print(buf);
}
void setup() {
  tft_init();

  Serial.begin(115200);
  SerialBT.begin("Btserial"); //Bluetooth device name
  SerialBT.enableSSP();
  tft.setTextColor(TFT_WHITE);
  tft.fillRect(0,0,tft.width(),tft.height(), TFT_BLACK);
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setCursor(0,20);
  tft.print("Classic UART Bridge");
  displayConnectionState(1);
}


uint8_t transferbuffer[4096];
uint32_t bytesAvailable = 0;
void loop() {
  bytesAvailable = Serial.available();
  if (bytesAvailable) {
    Serial.readBytes(transferbuffer,bytesAvailable);
    SerialBT.write(transferbuffer,bytesAvailable);
  }
  bytesAvailable = SerialBT.available();
  if (bytesAvailable) {
    SerialBT.readBytes(transferbuffer,bytesAvailable);
    Serial.write(transferbuffer,bytesAvailable);
  }
  delay(10);

  // disconnecting
  if (deviceConnected && !SerialBT.hasClient()) {
      displayConnectionState(1);
      delay(500); // give the bluetooth stack the chance to get things ready
      deviceConnected = false;
  }
  // connecting
  if (!deviceConnected && SerialBT.hasClient()) {
      displayConnectionState(2);
      deviceConnected=true;
  }
}