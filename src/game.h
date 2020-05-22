#include <iostream>
using namespace std;

class Game{
    private:
      volatile int score;
    public:
      int getScore(void);
      void resetScore(void);
      void incrementScore(void);
      int getPercentScore(void);
      Game();
};

Game::Game(void){
  cout << "Object is being created" << endl;
}

void Game::resetScore(void){
    score = 0;
}

int Game::getPercentScore(void){
  return (int)(score * 100) / 10;
}

int Game::getScore(void){
  return score;
}

void Game::incrementScore(void){
  score++;
}