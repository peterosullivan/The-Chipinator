#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <vector>
#include <numeric>
#include <FS.h>
#include <ArduinoJson.h>

#include "pitches.h"
#include "secret.h"
#include "game.h"
#include "helper.h"

//https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// hold uploaded file
File fsUploadFile;
String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)


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

bool handleFileRead(String path){  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
    server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if(upload.status == UPLOAD_FILE_START){
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.endsWith(".gz")) {                          // The file server always prefers a compressed version of a file 
      String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
      if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
         SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void handleFileList()
{
  String path = "/";
  // Assuming there are no subdirectories
  Dir dir = SPIFFS.openDir(path);
  String output;
  Serial.println("Start list");
  while(dir.next())
  {
    File entry = dir.openFile("r");
    size_t fileSize = dir.fileSize();
    Serial.printf("\tFS File: %s, size: %s\r\n", entry.name(), formatBytes(fileSize).c_str());
    output += String(entry.name()).substring(1) + " - " + formatBytes(fileSize).c_str() + "<br/>"; 
    entry.close();
  }
  server.send(200, "text/html", output);
}


void handleRoot() {
  Serial.print("handleRoot method");
  File file = SPIFFS.open("/index.html.gz","r");
  server.streamFile(file, "text/html");
  file.close();
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
        webSocket.broadcastTXT( game.toJson() );
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
  SPIFFS.begin();
  //SPIFFS.remove("/index.html"); //TODO: create file delete route
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

  server.on("/new", newGameRoute);
  server.on("/reset", resetRoute);
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/upload", HTTP_POST, [](){
    server.send(200, "text/plain", "{\"success\":1}");
  }, handleFileUpload);

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

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
    webSocket.broadcastTXT(game.toJson());
  }
 
  /*
  Serial.print(" score: ");
  Serial.print(game.score);
  Serial.print(" target: ");
  Serial.println(game.target_score);
  */
}
 