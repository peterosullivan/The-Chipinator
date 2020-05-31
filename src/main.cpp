#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <vector>
#include <numeric>

#include "pitches.h"
#include "secret.h"
#include "game.h"
#include "html_template.h"

//https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char *OTAName = "ESP8266";           // A name and a password for the OTA service
const char *OTAPassword = "esp8266";
const char* mdnsName = "chip"; //http://chip.local

volatile boolean interrupt_occurred = false;
const byte interruptPin = D4; // pin 2 =  D4 for  sensor
const byte tonePin      = D6; // pin 12;


Game game;
float average_score = 0.0;
std::vector<int> past_scores;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
//LiquidCrystal_I2C lcd(0x27,20,4);

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
    "<div class='card mb-4 shadow-sm'>" +
      "<div class='card-body'>" +
        "<h1 class='card-title'>" +
         "<span class='score'>" + String(game.getScore()) + "</span>"
        "<small class='text-muted'> / " + String(game.target_score) + "</small>" +
        "</h1>" +
        "<button type='button' id='new_game' class='btn btn-block btn-lg btn-success' onclick='newGame()'><i class='fas fa-crosshairs'></i> New Game</button>" +
      "</div>" +
    "</div>" +

    "<table class='table'><tr>" +
    "<th>Current Score</th>" +
    "<td><span class='score_percent'>" + String(game.getPercentScore()) + "</span>%" +
    "</td></tr>" +
    "<tr><th>Games Played</th>" +
    "<td class='games_played'>" + String(past_scores.size()) +
    "</td></tr>" +
    "<tr><th>Average Score</th>" +
    "<td><span class='average_score'>" + String(average_score, 1) + "</span>%" +
    "&nbsp;<a href='/reset'><i class='fas fa-minus-circle text-danger'></i></a>" +
    "</td></tr></table>" +
    html_footer;

  server.send(200, "text/html", htmlPage);
}

void resetRoute(){
  
  //clear current and past scores
  past_scores.clear();
  game.resetScore();
  average_score = 0;
  updateDisplay();

  String html = "<!DOCTYPE HTML><html lang='en'><head><meta http-equiv='refresh' content='2;url=/' /></head><body><h1>Reset!</h1></body></html>";
  server.send(200, "text/html", html);
}

void newGameRoute(){
  newGame(); 

  String message = "\"average_score\": ";
  message += average_score;
  message += ", \"games_played\": ";
  message += past_scores.size();

  server.send(200, "application/json", "{" + message + "}");
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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == 'n') {                      // the browser sends an n then new game
       Serial.printf("New Game Socket button!!!");
       newGame();
      }
      break;
    case WStype_ERROR:
      Serial.printf("Error");
      break;
    // For everything else: do nothing
    case WStype_BIN:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void startWebSocket() { 
  webSocket.begin();                          
  webSocket.onEvent(webSocketEvent);    // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started");
}

void startMDNS() { // Start the mDNS responder
   if (MDNS.begin(mdnsName)) {
    Serial.print("mDNS responder started: http://");
    Serial.print(mdnsName);
    Serial.println(".local");
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

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
  delay(1000);
  lcd.clear();

  updateDisplay();

  server.on("/", handleRoot);
  server.on("/new", newGameRoute);
  server.on("/reset", resetRoute);
  server.onNotFound(handleNotFound);

  startWebSocket();
  startMDNS();

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
  webSocket.loop();                           // constantly check for websocket events
  server.handleClient();

  if (interrupt_occurred){
    interrupt_occurred = false;
    playSound();
    updateDisplay();
    String msg = String(game.getScore());
    webSocket.broadcastTXT(msg);
  }
 
  /*
  Serial.print(" score: ");
  Serial.print(game.score);
  Serial.print(" target: ");
  Serial.println(game.target_score);
  */
}
 