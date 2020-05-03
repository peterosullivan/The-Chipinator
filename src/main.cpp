#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"
#include "secret.h"

const byte interruptPin = D4; // pin 2 =  D4 for  sensor
const byte tonePin = D6;      //pin 12;
volatile boolean interrupt_occurred = false;
volatile int count = 0;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
//LiquidCrystal_I2C lcd(0x27,20,4);

void ICACHE_RAM_ATTR ISR(){
  count++;
  interrupt_occurred = true;   // Record that an interrupt occurred
}

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT);                                //define interruptPin input pin
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISR, FALLING); //Respond to falling edges on the pin

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
}

void playSound(){
  if (count == 100){
    //mario_sound
    tone(tonePin, NOTE_E6, 125);
    delay(130);
    tone(tonePin, NOTE_G6, 125);
    delay(130);
    tone(tonePin, NOTE_E7, 125);
    delay(130);
    tone(tonePin, NOTE_C7, 125);
    delay(130);
    tone(tonePin, NOTE_D7, 125);
    delay(130);
    tone(tonePin, NOTE_G7, 125);
    delay(125);
    noTone(tonePin);
  }
  else{
    //simple beep tone
    tone(tonePin, 170);
    delay(50);
    noTone(tonePin);
  }
}

void loop() {
  
  if (interrupt_occurred){
    playSound();
    interrupt_occurred = false;
  }
 
  Serial.print("Current: ");
  Serial.print(" Pre: ");
  Serial.print(" Counter: ");
  Serial.println(count);
  
  //delay(200);
  lcd.setCursor(0, 1);
  lcd.print(count);
}
 