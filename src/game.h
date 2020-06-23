#include <iostream>
#include <ArduinoJson.h>
using namespace std;

class Game{
    private:
      volatile int score;
    public:
      int target_score = 10;
      int getScore(void);
      void resetScore(void);
      void incrementScore(void);
      int getPercentScore(void);
      char* toJson(void);
      Game();
};

Game::Game(void){
  cout << "Object is being created" << endl;
}

char* Game::toJson(void){
  DynamicJsonDocument doc(1024);
  char json_string[512];

  doc["target_score"] = target_score;
  doc["score"] = score;
  doc["percent_score"] = getPercentScore();

  serializeJson(doc, json_string);
  return json_string;
}

void Game::resetScore(void){
    score = 0;
}

int Game::getPercentScore(void){
  return (int)(score * 100) / target_score;
}

int Game::getScore(void){
  return score;
}

void Game::incrementScore(void){
  score++;
}