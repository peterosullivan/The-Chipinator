#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"
#include "secret.h"


//#define DETECT 13  // pin D7 for  sensor
//#define DETECT 15  // pin D8 for  sensor
#define DETECT 2  // pin D4 for  sensor
int tonePin = 12;

int count = 0;
int lazer_previous_state = HIGH;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
//LiquidCrystal_I2C lcd(0x27,20,4);

void setup() {
  Serial.begin(115200);
  pinMode(DETECT, INPUT);//define detect input pin

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  lcd.init();                  
  lcd.backlight();
  lcd.print("Connect WIFI");
  
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    lcd.print("WiFi Failed!");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();


  lcd.print("Counter: ");
  lcd.setCursor(0, 1);
  lcd.print(count);
  //lcd.setCursor(0,2);
  //lcd.print("Arduino LCM IIC 2004");
  //lcd.setCursor(2,3);
  //lcd.print("Power By Ec-yuan!");
  //lcd.print(WiFi.localIP());
}

void loop() {
  int detected = digitalRead(DETECT);// read Laser sensor
 
  if(detected == LOW && lazer_previous_state == HIGH){
    //Serial.println("Beam broken");
     // Laser Sensor code for Robojax.com
    //Serial.print("Count Value: ");
    //Serial.println(count);
    count++;

    //mario_sound
    tone(tonePin, NOTE_E6,125);
    delay(130);
    tone(tonePin, NOTE_G6,125);
    delay(130);
    tone(tonePin, NOTE_E7,125);
    delay(130);
    tone(tonePin, NOTE_C7,125);
    delay(130);
    tone(tonePin, NOTE_D7,125);
    delay(130);
    tone(tonePin, NOTE_G7,125);
    delay(125);
    noTone(tonePin);

    lazer_previous_state = LOW;
  }
  else if (detected == HIGH && lazer_previous_state == LOW) {
    lazer_previous_state = HIGH;
  }
  else{
    Serial.print("Current: ");
    Serial.print(detected);
    Serial.print(" Pre: ");
    Serial.print(lazer_previous_state);
    Serial.print(" Counter: ");
    Serial.println(count);
  }

  //delay(200);
  lcd.setCursor(0, 1);
  lcd.print(count);
}
 