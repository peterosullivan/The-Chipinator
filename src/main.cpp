#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"
#include "secret.h"
#include <math.h>
#include <vector>

const byte interruptPin = D4; // pin 2 =  D4 for  sensor
const byte tonePin = D6;      //pin 12;
volatile boolean interrupt_occurred = false;
volatile int score = 0;
int target_score = 10;
int percent_score = 0;
float average_score = 0.0;

int past_scores [3];
std::vector<int> ps;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
//LiquidCrystal_I2C lcd(0x27,20,4);

void ICACHE_RAM_ATTR ISR(){
  score++;
  interrupt_occurred = true;   // Record that an interrupt occurred
}

float calculateAverage(){
  float total = 0.0;
  for(int i = 0; i < ps.size(); i++){
    total += ps[i];
  }
  return total / ps.size();
}

void newGame(){
  score = 0;
  average_score = calculateAverage();
}

void updateDisplay(){
  percent_score = (score * 100) / 10;
  Serial.println(percent_score);

  if(score > target_score){
    score--;
    percent_score = (score * 100) / 10;
    ps.push_back(percent_score);
    newGame();
    percent_score = 0;
  }

  lcd.clear();
  lcd.print("Score ");
  lcd.print(score);
  lcd.print("/");
  lcd.print(target_score);
  lcd.print(" ");
  lcd.print(percent_score);
  lcd.print("%");
  lcd.setCursor(0, 1);

  lcd.print("Game ");
  lcd.print(ps.size());
  lcd.print(" AVG ");
  lcd.print(average_score, 1); 
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

  updateDisplay();
}

void playSound(){
  if (score == 50){
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
    interrupt_occurred = false;
    playSound();
    updateDisplay();
  }
 
  /*
  Serial.print(" score: ");
  Serial.print(score);
  Serial.print(" target: ");
  Serial.println(target_score);
  */

  
}
 