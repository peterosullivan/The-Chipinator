#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"

#include "secret.h"
#include "game.h"
#include "html_template.h"

#include <vector>
#include <numeric>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

volatile boolean interrupt_occurred = false;
const byte interruptPin = D4; // pin 2 =  D4 for  sensor
const byte tonePin      = D6; // pin 12;

Game game;
float average_score = 0.0;
std::vector<int> past_scores;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
//LiquidCrystal_I2C lcd(0x27,20,4);

ESP8266WebServer server(80);

float calculateAverage(){
  float average = accumulate( past_scores.begin(), past_scores.end(), 0.0)/past_scores.size(); 
  return average;
}

void updateDisplay(){
  lcd.clear();
  lcd.print("Score ");
  lcd.print(game.getScore());
  lcd.print("/");
  lcd.print(game.target_score);
  lcd.print(" ");
  lcd.print(game.getPercentScore());
  lcd.print("%");
  lcd.setCursor(0, 1);

  lcd.print("Game ");
  lcd.print(past_scores.size());
  lcd.print(" AVG ");
  lcd.print(average_score, 1); 
}

void newGame(){
  //record scores
  past_scores.push_back(game.getPercentScore());
  average_score = calculateAverage();

  //rest scores
  game.resetScore();
  updateDisplay();
}

void handleRoot() {
 String htmlPage =
    html_header +
    "<table class='table'><tr>" +
    "<th>Current Score</th>" +
    "<td>" + String(game.getScore()) + "/" +
    String(game.target_score) +
    " (" + String(game.getPercentScore()) + "%)" +
    "</td></tr>" +
    "<tr><th>Games Played</th>" +
    "<td class='games_played'>" + String(past_scores.size()) +
    "</td></tr>" +
    "<tr><th>Average Score</th>" +
    "<td><span class='average_score'>" + String(average_score, 1) + "</span>%" +
    "&nbsp;<a href='/reset'><i class='fas fa-minus-circle text-danger'></i></a>" +
    "</td></tr></table>" +
    "<button type='button' class='btn btn-lg btn-success' onclick='newGame()'><i class='fas fa-crosshairs'></i> New Game</button>" +
    html_footer;

  server.send(200, "text/html", htmlPage);
}

void newResetRoute(){
  
  //clear current and past scores
  past_scores.clear();
  game.resetScore();
  average_score = 0;
  updateDisplay();

  String message = "Reset!\n\n";
  message += "\n\nCurrent Score: ";
  message += game.getPercentScore();
  message += "\n\nNumber Games: ";
  message += past_scores.size();
  message += "\nAverage Score: ";
  message += average_score;
  server.send(200, "text/plain", message);
}

void newGameRoute(){
  newGame(); 

  String message = "\"average_score\": ";
  message += average_score;
  message += ", \"games_played\": ";
  message += past_scores.size();

  server.send(200, "text/json", "{" + message + "}");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void ICACHE_RAM_ATTR ISR(){
  game.incrementScore();
  interrupt_occurred = true;   // Record that an interrupt occurred
}

void setup() {
  Serial.begin(115200);

  pinMode(interruptPin, INPUT);                                       //define interruptPin input pin
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

  server.on("/", handleRoot);
  server.on("/new", newGameRoute);
  server.on("/reset", newResetRoute);
  server.onNotFound(handleNotFound);

  if (MDNS.begin("chip")) {
    Serial.println("MDNS responder started");
  }
  server.begin();
  Serial.println("HTTP server started");
}

void playSound(){
  if (game.getScore() == 50){
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
  server.handleClient();
  MDNS.update();

  if (interrupt_occurred){
    interrupt_occurred = false;
    playSound();
    updateDisplay();
  }
 
  /*
  Serial.print(" score: ");
  Serial.print(score);
  Serial.print(" target: ");
  Serial.println(game.target_score);
  */
}
 