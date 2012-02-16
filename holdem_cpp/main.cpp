/* 
 * File:   main.cpp
 * Author: eugene
 *
 * Created on January 30, 2012, 2:59 PM
 */

#include <cstdlib>

using namespace std;

#include <stdio.h>
#include <time.h>

//-----Для C
//проверки на undefined в js коде заменятся на -1 в С коде
//длина массива без учета элементов с -1
int good_length_of_array(int arr[], int l){
    //int l = sizeof(arr) / sizeof(int);
    int count = 0;
    for (int i = 0; i < l; i++)
        if (arr[i] != -1)
            count++;
    return count;
}
//положение в массиве после которого начинаются -1
int find_position_for_push(int arr[], int l){
    //int l = sizeof(arr) / sizeof(int);
    int position = 0;
    for (int i = 0; i < l - 1; i++)
        if (arr[i] != -1 && arr[i+1] == -1){
            position = i + 1;
            break;
        };
    return position;
}
//длина без -1
int normalize_length(int arr[], int l){
    int count = 0;
    for (int i = 0; i < l; i++){
        if (arr[i] == -1)
            return i;
        if (arr[i] != -1 && i == l-1)
            return l;
    };
    return -1;
}
//-----Для С

int completeHandValue(int& handValue, int cards[], int l){  
    //убираем все -1
    l = normalize_length(cards, l);
    if (l == -1)
        return -1;    
    if (handValue < 0x10) { // put in a sentinel: when that bit
                            // reaches the 21st position, we're done
      handValue |= 0x10;
    }
    /*
    while (handValue < 0x200000 && cards.length > 0) {
      handValue <<= 4;
      handValue += cards.shift();
    }*/
    int k = 0;
    while (handValue < 0x200000 && good_length_of_array(cards, l) > 0){
      handValue <<= 4;
      handValue += cards[k];
      cards[k] = -1;
      k++;
    }
    if (handValue & 0x1000000) { // done: remove the sentinel
      handValue &= 0xffffff;
    }
    return handValue;
}

int handEval(int cardArr[], int l){
    int handValue = -1; //качество руки

    int suitCounts[4] = {0, 0, 0, 0};
    int rankCounts[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int isPresent[52] = {}; // isPresent[card] is true iff card is one
                         // of the 7 in the hand
    for (int i = 0; i < 52; i++){
        isPresent[i] = false;
    }
    int card;
    int rank;
    int suit;

    for (int i = 0; i < l; i++) {
        card = cardArr[i];
        if (isPresent[card] || card < 0 || card > 51) {
          return -1;
        }
        isPresent[card] = true;
        rank = card >> 2;
        rankCounts[rank]++;
        suit = card & 3;
        suitCounts[suit]++;
    }
    int flushSuit = -1;
    for (suit = 0; suit < 4; suit++) {
      if (suitCounts[suit] >= 5) {
        flushSuit = suit;
        break;
      }
    }
    if (flushSuit >= 0) { // there is a flush
        //var run = [];
       // if this array ever reaches a length of 5,
                    // we have a Straight Flush!
      int run[5] = {-1,-1,-1,-1,-1};
      //если исчезнут все -1, то Straight Falsh
      int ranksInFlushSuit[5] = {}; // this array collects the 5 highest
                                 // ranks in the suit, in case we have not
      int k = 0;
      for (rank = 12; rank >= 0; rank--) {
        if (!isPresent[rank * 4 + flushSuit]) {
          continue;
        }
        if (rank == run[good_length_of_array(run, l) - 1] - 1) {
            run[find_position_for_push(run, l)] = rank;
        } else if (good_length_of_array(run, l) < 5) {
            for (int j =0 ; j <l; j++)
                run[j] = -1;
          //run = [rank];
            run[0] = rank;
        }
        ranksInFlushSuit[k] = rank;
        k++;
      }
      // check for "wheel": straight 5432A
      if (run[3] == 0 && isPresent[48 + flushSuit]) {
        run[4] = 0xc; // push an ace
      }
      if (good_length_of_array(run, l)>= 5) {
        handValue = 8; // straight flush
        completeHandValue(handValue, run, l);
      } else {
        handValue = 5; // flush
        completeHandValue(handValue, ranksInFlushSuit, l);
      }

      return handValue;
    }
    // there is no flush
    int pattern[5][7] = { // an array of 5 arrays
      {-1,-1,-1,-1,-1,-1,-1}, // one for runs that could make straights, like above
      {-1,-1,-1,-1,-1,-1,-1}, // one for high cards - it is pattern[1] by no coincidence
      {-1,-1,-1,-1,-1,-1,-1}, // pattern[2] for pairs
      {-1,-1,-1,-1,-1,-1,-1}, // pattern[3] for trips
      {-1,-1,-1,-1,-1,-1,-1}  // pattern[4] for quads
    };
    //проверки на undefined в js коде заменятся на -1 в С коде
    for (rank = 12; rank >= 0; rank--) {
      int rankCount = rankCounts[rank];

      if (!rankCount) {
        continue;
      }

      if (rank == pattern[0][good_length_of_array(pattern[0], l) - 1] - 1) {
          pattern[0][find_position_for_push(pattern[0], l)] = rank;
      } else if (good_length_of_array(pattern[0], l) < 5) {
          for (int j =0; j < l; j++)
              pattern[0][j] = -1;
          pattern[0][0] = rank;
      }

      for (int i = 1; i <= rankCount; i++) {
          pattern[rankCount][find_position_for_push(pattern[rankCount], l)] = rank;
      }
    }
    // check for "wheel": straight 5432A
    if (pattern[0][3] == 0 && rankCounts[0xc]) {
        pattern[0][4] = 0xc; // push an ace
    }
    if (good_length_of_array(pattern[4], l) != 0) { // quad
        handValue = 7;
        // the fith card is tricky - it can be in trips, pairs or high cards
        int fifthCard = pattern[1][0]; // could be undefined
        if (fifthCard == -1 || fifthCard < pattern[2][0]) {
          fifthCard = pattern[2][0];
        }
        if (fifthCard == -1 || fifthCard < pattern[3][0]) {
          fifthCard = pattern[3][0];
        }
        pattern[4][find_position_for_push(pattern[4], 7)] = fifthCard;
        completeHandValue(handValue, pattern[4], 7);
      }
    else if (good_length_of_array(pattern[3], 7) > 3 ||
          good_length_of_array(pattern[3], 7) && good_length_of_array(pattern[2], 7)) { // full house
        handValue = 6;
        completeHandValue(handValue, pattern[3], 7);
        completeHandValue(handValue, pattern[2], 7);
      }
    else if (good_length_of_array(pattern[0], 7) >= 5) { // straight
        handValue = 4;
        completeHandValue(handValue, pattern[0], 7);
      }
    else if (good_length_of_array(pattern[3], 7) != 0) { // trip
        handValue = 3;
        completeHandValue(handValue, pattern[3], 7);
        completeHandValue(handValue, pattern[1], 7);
      }
    else if (good_length_of_array(pattern[2], 7) > 2) { // two pairs
        handValue = 2;
        if (good_length_of_array(pattern[2], 7) > 4 && pattern[2][4] < pattern[1][0]) {
          //pattern[2].length = 4;
            pattern[2][4] = -1;
        }
        completeHandValue(handValue, pattern[2], 7);
        completeHandValue(handValue, pattern[1], 7);
      }
    else if (good_length_of_array(pattern[2], 7) != 0) { // pair
        handValue = 1;
        completeHandValue(handValue, pattern[2], 7);
        completeHandValue(handValue, pattern[1], 7);
      }
    else { // high card
        handValue = 0;
        completeHandValue(handValue, pattern[1], 7);
      }
      return handValue;
}

typedef struct partOfRandomHand{
    int cards[5];       //по умолчанию все -1
    int count;          //количество не -1
} partOfRandomHand;

//должно генерировать от 0 до 51 (концы включительно)
int getRandomInt(int min, int max){
        return min + (rand() % (int)(max - min + 1));
}

partOfRandomHand makeRandomHand(
        int bannedArray[],      //карты, уже сгенерированные ранее (у игроков/на столе)
        int l,                  //их количество
        int cardCount           //сколько карт нужно сгенерировать
){
    partOfRandomHand generatedHand; //результат работы: сгенерированные карты и их число
    generatedHand.count = cardCount;
    for (int j = 0; j < 5; j++)
        generatedHand.cards[j] = -1; 
    bool exist = false;
    
    for (int i = 0; i < cardCount; i++){
        int generatedNumber = -1;
        
        for(;;){
            generatedNumber = getRandomInt(0, 51);  
            if (generatedNumber == -1 )
                printf("dd");
            exist = false;         
            //банним карты уже на руках игроков
            if (l >0)
                    if (bannedArray[generatedNumber] != -1){
                        exist = true;
                    }
            //банним, созданные ранее в цикле, карты
            if (i > 0)
                for (int j = 0; j < i; j++)
                    if (generatedHand.cards[j] == generatedNumber)
                        exist = true;         
            if (exist == false)
                break;
        };
        generatedHand.cards[i] = generatedNumber;       
    };

    return generatedHand;
}

typedef struct resultGame{
    float winOdds;
    float tieOdds;
    int winCount;
    int tieCount;
    int globalCount;
} resultGame;

resultGame monteCarloSimulation(int cards[], int l, int ph, int playerCount, int monteCarloMaxIteration){
  int oppCount = playerCount - 1;//число оппонентов
  //рука игрока
  int p1[2] = {cards[0], cards[1]};  
  int alreadyBoardCount = 0;
  int boardCount = 0;
  //нет карт на столе (префлоп)
  int alreadyboard[5] = {-1, -1, -1, -1, -1};
 
  if (ph == 0){//префлоп
    alreadyBoardCount = 0; //число карт на столе (известны)
    boardCount = 5;        //число карт, которые будем генерировать случайным образом
  }
  else if (ph == 1){//флоп
    alreadyBoardCount = 3;
    alreadyboard[0] = cards[2];
    alreadyboard[1] = cards[3];
    alreadyboard[2] = cards[4];
    boardCount = 2;
  }
  else if (ph == 2){//тёрн
    alreadyBoardCount = 4; 
    alreadyboard[0] = cards[2];
    alreadyboard[1] = cards[3];
    alreadyboard[2] = cards[4];
    alreadyboard[3] = cards[5]; 
    boardCount = 1;
  }
  else if (ph == 3){//ривер
    alreadyBoardCount = 5;
    alreadyboard[0] = cards[2];
    alreadyboard[1] = cards[3];
    alreadyboard[2] = cards[4];
    alreadyboard[3] = cards[5];
    alreadyboard[4] = cards[6];
    boardCount = 0;
  }   
  int globalCount = 0;
  int winCount = 0;
  int tieCount = 0;
  int loseCount = 0;
  
  for(int i = 0; i < monteCarloMaxIteration; i++ ){    
      //если изменим на отличное от -1, то карта уже сгенерирована, значит банним её
      int bannedArray[52] = {};
      for (int j = 0; j < 52; j++)
          bannedArray[j] = -1;     
      //банним руку игрока
      bannedArray[cards[0]]++;
      bannedArray[cards[1]]++;
      //банним карты уже на столе (флоп/тёрн/ривер)
      if (alreadyBoardCount != 0)
          for (int j = 0; j < alreadyBoardCount; j++)
              bannedArray[alreadyboard[j]]++;
      int p1_eval = -1; //качество руки игрока (7 карт: 2 его + 3-5 стол + 2-0 сгенерированных)
      partOfRandomHand board;
      board.count = -1;
     
      if (ph == 0){ //префлоп
          board = makeRandomHand(bannedArray, 52,  5); //5 карт на столе неизвестны 
          int arr[7] = {p1[0], p1[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4] };
          p1_eval = handEval(arr, 7);

      }
      else if (ph == 1){ //флоп
          board = makeRandomHand(bannedArray, 52,  2);
          int arr[7] = {p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board.cards[0], board.cards[1] };
          p1_eval = handEval(arr, 7);
      }
      else if (ph == 2){ //тёрн
          board = makeRandomHand(bannedArray, 52,  1);
          int arr[7] = {p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], board.cards[0]};
          p1_eval = handEval(arr, 7);
      }
      else if (ph == 3){ //ривер
          int arr[7] = {p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] };
          p1_eval = handEval(arr, 7);
      }     
      //банним карты, сгенерированные рандомно на столе
      if (board.count > 0)
          for (int j = 0; j < board.count; j++)
                bannedArray[board.cards[j]]++;       
      //генерим рандомно карты противников без забанненых
      int v = 0; //число побед против оппонентов
      int e = 0; //число нечейных
      for (int  j = 0; j < oppCount; j++){
            partOfRandomHand p2 = makeRandomHand(bannedArray, 52, 2); //две карты противника из префлопа 
            int p2_eval = -1;
            //банним их
            if (p2.count > 0)
            for (int j = 0; j < p2.count; j++)
                bannedArray[p2.cards[j]]++;    
            if (ph == 0){ //префлоп
                int arr[7] = {p2.cards[0], p2.cards[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4] };
                p2_eval = handEval(arr, 7);
            }
            else if (ph == 1){ //флоп
                int arr[7] = {p2.cards[0], p2.cards[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board.cards[0], board.cards[1] };
                p2_eval = handEval(arr, 7);
            }
            else if (ph == 2){ //тёрн
                int arr[7] = {p2.cards[0], p2.cards[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], board.cards[0]};
                p2_eval = handEval(arr, 7);
            }
            else if (ph == 3){ //ривер
                int arr[7] = {p2.cards[0], p2.cards[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] };
                p2_eval = handEval(arr, 7);
            }                 
            if (p1_eval > p2_eval){
                v++; }
            else if (p1_eval == p2_eval){
                e++; }    
      }
      if (v == oppCount)
        winCount++;
      if (e == oppCount)
        tieCount++;
      globalCount++;
  };
  
  float winOdds = winCount * 1.0 / globalCount ;
  float tieOdds = tieCount * 1.0 / globalCount;
  resultGame _resultGame;
  _resultGame.winOdds = winOdds;
  _resultGame.tieOdds = tieOdds;
  _resultGame.winCount = winCount;
  _resultGame.tieCount = tieCount;
  _resultGame.globalCount = globalCount;
  return _resultGame;
}

typedef struct winnerData{
    int number;                 //номер победителя
    int handValue;              //значение его руки
} winnerData;

//определяет победителя среди множества рук
winnerData findWinner(int cards[][7], int playerCount, int cardCount){  
    int winnerNumber = -1;
    int winnerHandValue = -1;
    for (int i = 0; i < playerCount; i++){
        int hand_eval = handEval(cards[i], cardCount);
        printf("handValue %i \n", hand_eval);
        if (hand_eval > winnerHandValue){
            winnerHandValue = hand_eval;
            winnerNumber = i;
        }
    }
    winnerData _winnerData;
    _winnerData.handValue = winnerHandValue;
    _winnerData.number = winnerNumber;

    return _winnerData;
}

int main(int argc, char *argv[])
{  
    //int cards[7] = {16, 17, 18, 1, 2, 15, 14} ;
    int cards[7] = {1, 4, 24, 32, 5, 51, 2} ; //номера есть карты в картинке рядом

    srand(time(NULL));  
    //monteCarloSimulation(int cards[], int l, int ph, int playerCount, int monteCarloMaxIteration)
    resultGame rg = monteCarloSimulation(cards, //карты 7 штук (если префлоп, то реальном нужны лишь первые 2, флоп : 5, тёрн : 6, ривер : 7)
            7,                                  //7 штук
            2,                                  //фаза. 0-префлоп, 1 - флоп, 2 - тёрн, 3 -ривер
            2,                                  //общее число игроков ( я + число оппонентов)
            40000);                             //число итераций в монтекарло
    
    printf("winCount monte %i \n", rg.winCount);
    printf("tieCount monte %i \n",rg.tieCount);
    printf("win odds monte %f \n", rg.winOdds);
    printf("tie odds monte %f \n",rg.tieOdds);
    printf("lose odds monte %f \n", (1.0 - rg.tieOdds - rg.winOdds));
    printf("cards generated monte %i \n", rg.globalCount);  
    
    
    
    
    //выбор победителя 
    int cards_judgement[][7] = {
                                {1 , 4, 19, 32, 24, 51, 2},     //Одна из рук для сравнения
                                {1 , 4, 24, 32, 5, 51, 2},
                                {10 , 4, 20, 32, 5, 50, 2},

                          };
    
    winnerData _winnerData = findWinner(cards_judgement,
            3,                                                  //количество игроков (строки)
            7);                                                 //количество карт
    printf("\n\njudgement: \n");
    printf("number of winner %i \n", _winnerData.number);
    printf("handValue of winner %i \n", _winnerData.handValue);    
    
    //int l = sizeof(cards) / sizeof(int); //размер массива
    //int h_e = handEval(cards, l);
    //printf("%d\n",h_e);
}
