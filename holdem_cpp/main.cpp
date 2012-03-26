/* 
 * File:   main.cpp
 * Author: eugene
 *
 * Created on January 30, 2012, 2:59 PM
 */
#include <QtCore/QCoreApplication>

#include <cstdlib>

using namespace std;

#include <stdio.h>
#include <time.h>

//-----Для C. Вспомогательный служебные функции.--------------------------------------------------------------

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
//-----Для С----------------------------------------------------------------------------------------------------


/*
Вспомогательная функция для handEval.
Заканчивает получение 'качества' руки игрока.
(каждой руке ставится в соответствие ее качество: целое положительное число;
чем больше число, тем лучше рука)
*/

int completeHandValue(int& handValue, int cards[], int l){  
    //убираем все -1
    l = normalize_length(cards, l);
    if (l == -1)
        return -1;    
    if (handValue < 0x10) { // put in a sentinel: when that bit
                            // reaches the 21st position, we're done
      handValue |= 0x10;
    }
    //printf("l %i \n", good_length_of_array(cards, l));
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


/*
    handEvalResult  --  Хранит результат работы handEval, которая вычисляет качество руки одного игрока.

    handValue       --  значение руки .
    handType        --  тип руки
    win_cards       --  содержит ранги карт, привёдших к победе
  */
typedef struct handEvalResult{
    int handValue;
    int handType;
    int win_cards[7];
    int suite;
} handEvalResult;

/*
    handEval        --  вычисляет качество одной руки (handValue) и какая комбинация карт (handType)
*/
handEvalResult handEval(int cardArr[], int l){
    int handValue   = -1; //качество руки
    int handType    = -1; //тип руки
    handEvalResult _handEvalResult;
    _handEvalResult.handValue = -1;
    _handEvalResult.handType = -1;
    _handEvalResult.suite = -1;
    for(int j = 0; j < 7; j++)
        _handEvalResult.win_cards[j] = -1;

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
          //return -1;
            return _handEvalResult;
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
        _handEvalResult.suite = flushSuit;
        //var run = [];
       // if this array ever reaches a length of 5,
                    // we have a Straight Flush!
      int run[7] = {-1,-1,-1,-1,-1, -1, -1};
      //если исчезнут все -1, то Straight Falsh
      int ranksInFlushSuit[7] = {-1, -1, -1, -1, -1, -1, -1}; // this array collects the 5 highest
                                 // ranks in the suit, in case we have not
      int k = 0;
      for (rank = 12; rank >= 0; rank--) {
        if (!isPresent[rank * 4 + flushSuit]) {
          continue;
        } else {
            //printf("flush card %i \n", rank * 4 + flushSuit);
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
        handType = 8;
        for (int j = 0; j < 5; j++){
            _handEvalResult.win_cards[j] = run[j];
            //printf("flush card %i \n", run[j]);
        };
        completeHandValue(handValue, run, l);
      } else {
        handValue = 5; // flush
        handType = 5;
        for (int j = 0; j < good_length_of_array(ranksInFlushSuit, 5); j++){
            _handEvalResult.win_cards[j] = ranksInFlushSuit[j];
            //printf("flush card %i \n", ranksInFlushSuit[j]);
        };
        completeHandValue(handValue, ranksInFlushSuit, l);
      }


      _handEvalResult.handValue = handValue;
      _handEvalResult.handType = handType;
      return _handEvalResult;
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
        handType = 7;
        // the fith card is tricky - it can be in trips, pairs or high cards
        int fifthCard = pattern[1][0]; // could be undefined
        if (fifthCard == -1 || fifthCard < pattern[2][0]) {
          fifthCard = pattern[2][0];
        }
        if (fifthCard == -1 || fifthCard < pattern[3][0]) {
          fifthCard = pattern[3][0];
        }
        pattern[4][find_position_for_push(pattern[4], 7)] = fifthCard;
        for (int j = 0; j < 7; j++){
            _handEvalResult.win_cards[j] = pattern[4][j];
        };
        completeHandValue(handValue, pattern[4], 7);


      }
    else if (good_length_of_array(pattern[3], 7) > 3 ||
          good_length_of_array(pattern[3], 7) && good_length_of_array(pattern[2], 7)) { // full house
        handValue = 6;
        handType = 6;
        int j = 0;
        while (pattern[3][j] != -1){
            _handEvalResult.win_cards[j] = pattern[3][j];
            j++;
        };
        int k = 0;
        while (pattern[2][k] != -1){
            _handEvalResult.win_cards[k + j] = pattern[2][k];
            k++;
        };
        completeHandValue(handValue, pattern[3], 7);
        completeHandValue(handValue, pattern[2], 7);

      }
    else if (good_length_of_array(pattern[0], 7) >= 5) { // straight
        handValue = 4;
        handType = 4;
        for (int j = 0; j < 7; j++){
            _handEvalResult.win_cards[j] = pattern[0][j];
        };
        completeHandValue(handValue, pattern[0], 7);

      }
    else if (good_length_of_array(pattern[3], 7) != 0) { // trip
        handValue = 3;
        handType = 3;
        int j = 0;
        while (pattern[3][j] != -1){
            _handEvalResult.win_cards[j] = pattern[3][j];
            j++;
        };
        /*
        int k = 0;
        while (pattern[1][k] != -1){
            _handEvalResult.win_cards[k + j] = pattern[1][k];
            k++;
        };
        */
        completeHandValue(handValue, pattern[3], 7);
        completeHandValue(handValue, pattern[1], 7);

      }
    else if (good_length_of_array(pattern[2], 7) > 2) { // two pairs
        handValue = 2;
        handType = 2;
        if (good_length_of_array(pattern[2], 7) > 4 && pattern[2][4] < pattern[1][0]) {
          //pattern[2].length = 4;
            pattern[2][4] = -1;
        }
        int j = 0;
        while (pattern[2][j] != -1){
            _handEvalResult.win_cards[j] = pattern[2][j];
            j++;
        };
        /*
        int k = 0;
        while (pattern[1][k] != -1){
            _handEvalResult.win_cards[k + j] = pattern[1][k];
            k++;
        };
        */
        completeHandValue(handValue, pattern[2], 7);
        completeHandValue(handValue, pattern[1], 7);

      }
    else if (good_length_of_array(pattern[2], 7) != 0) { // pair
        handValue = 1;
        handType = 1;

        int j = 0;
        while (pattern[2][j] != -1){
            _handEvalResult.win_cards[j] = pattern[2][j];
             //printf("w c %i \n", pattern[2][j]);
            j++;
        };
        /*
        int k = 0;
        while (pattern[1][k] != -1){
            _handEvalResult.win_cards[k + j] = pattern[1][k];
            //             printf("w c %i \n", pattern[1][k]);
            k++;
        };
        */

        completeHandValue(handValue, pattern[2], 7);
        completeHandValue(handValue, pattern[1], 7);

      }
    else { // high card
        handValue = 0;
        handType = 0;
        for (int j = 0; j < 7; j++){
            _handEvalResult.win_cards[j] = pattern[1][j];
        };

        completeHandValue(handValue, pattern[1], 7);
      }

    _handEvalResult.handType = handType;
    _handEvalResult.handValue = handValue;
      return _handEvalResult;
}

/*
    сдесь будут хранится сгенерированные части руки игрока (board, случаные карты др. игроков)
*/
typedef struct partOfRandomHand{
    int cards[5];       //по умолчанию все '-1'
    int count;          //количество не '-1', т.е. сколько нужно сгенерировать случайно из 5ти
} partOfRandomHand;

//должно генерировать от 0 до 51 (концы включительно)
int getRandomInt(int min, int max){
        return min + (rand() % (int)(max - min + 1));
}

/*
    Создание случайных рук, карт на столе.
*/
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

/*
    результат работы:
    winOdds               -- вероятность выйгрыша против игроков
    tieOdds               -- вероятность нечьих против игроков
    winCount              -- количество побед
    tieCount              -- количнство нечьих
    globalCount           -- сколько всего итераций

*/
typedef struct resultGame{
    float winOdds;
    float tieOdds;
    int winCount;
    int tieCount;
    int globalCount;
} resultGame;

/*
    симуляция методом Монте-Карло.
    1. Случайно создаем карты на столе (3-5 в зависимости от фазы) и начальные карты противникам (2)
    2. Вычисляем качество руки игрока и противниковс помощью handEval
    3. Если у игрока рука лучше, то winCount++; если все равны, то tieCount++
    4. повторяем шаг 1 globalCount число раз

*/
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
      handEvalResult p1_eval;
      //int p1_eval = -1; //качество руки игрока (7 карт: 2 его + 3-5 стол + 2-0 сгенерированных)
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
            handEvalResult p2_eval;
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
            if (p1_eval.handValue > p2_eval.handValue){
                v++; }
            else if (p1_eval.handValue == p2_eval.handValue){
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

//-------------------------------------------------------------------------------------------------------
/*
    результат работы:
    getOdds               -- вероятность получения конкретной комбинации (флаш, кольцо)
    handType              -- сама комбинация
*/
typedef struct resultGetHand{
    float   getOdds[9];
} resultGetHand;

resultGetHand monteCarloSimulation_getHand(int cards[], int l, int ph, int playerCount, int monteCarloMaxIteration){
    resultGetHand _resultGetHand;
    for (int i = 0; i < 9 ; i++)
        _resultGetHand.getOdds[i] = 0;

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
        handEvalResult p1_eval;
        //int p1_eval = -1; //качество руки игрока (7 карт: 2 его + 3-5 стол + 2-0 сгенерированных)
        partOfRandomHand board;
        board.count = -1;




        //банним карты, сгенерированные рандомно на столе
        if (board.count > 0)
            for (int j = 0; j < board.count; j++)
                  bannedArray[board.cards[j]]++;
        //генерим рандомно карты противников без забанненых
        int v = 0; //число побед против оппонентов
        int e = 0; //число нечейных
        for (int  j = 0; j < oppCount; j++){
              partOfRandomHand p2 = makeRandomHand(bannedArray, 52, 2); //две карты противника из префлопа
              //банним их
              if (p2.count > 0)
              for (int j = 0; j < p2.count; j++)
                  bannedArray[p2.cards[j]]++;
        }

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

        _resultGetHand.getOdds[p1_eval.handType]++;

        globalCount++;
    };

    //printf("global %i \n", globalCount);

    for (int i = 0; i < 9; i++)
        _resultGetHand.getOdds[i] /= (1.0 * globalCount);


    return _resultGetHand;
}



//-------------------------------------------------------------------------------------------------------
/*
    winnerData и findWinner
    для опеределения победителя среди множества игроков
*/

typedef struct winnerData{
    int number;                 //номер победителя
    int handValue;              //значение его руки
    int handType;
    int _cards[7];              //карты, приведшие к победе
    int _count;                 //их число (остальные: -1)
} winnerData;

typedef struct winners{
    winnerData _winners[23];
    int count;
} winners;


void swap(int *a, int *b)
{
  int t=*a; *a=*b; *b=t;
}

void quick_sort(int arr[], int beg, int end)
{
  if (end > beg + 1)
  {
    int piv = arr[beg], l = beg + 1, r = end;
    while (l < r)
    {
      if (arr[l] <= piv)
        l++;
      else
        swap(&arr[l], &arr[--r]);
    }
    swap(&arr[--l], &arr[beg]);
    quick_sort(arr, beg, l);
    quick_sort(arr, r, end);
  }
}

void quick_sort_by_rank(int arr[], int beg, int end)
{
  if (end > beg + 1)
  {
    int piv = arr[beg], l = beg + 1, r = end;
    while (l < r)
    {
      if (arr[l] >> 2 >= piv >> 2)
        l++;
      else
        swap(&arr[l], &arr[--r]);
    }
    swap(&arr[--l], &arr[beg]);
    quick_sort_by_rank(arr, beg, l);
    quick_sort_by_rank(arr, r, end);
  }
}


void special_swap(winnerData *a, winnerData *b)
{
  winnerData t=*a; *a=*b; *b=t;
}


void special_quick_sort(winnerData arr[], int beg, int end)
{
  if (end > beg + 1)
  {
    int piv = arr[beg].handValue, l = beg + 1, r = end;
    while (l < r)
    {
      if (arr[l].handValue >= piv)
        l++;
      else
        special_swap(&arr[l], &arr[--r]);
    }
    special_swap(&arr[--l], &arr[beg]);
    special_quick_sort(arr, beg, l);
    special_quick_sort(arr, r, end);
  }
}


int unique(int *a, int n)
{
    int i, k;

    k = 0;
    for (i = 1; i < n; i++) {
        if (a[k] != a[i]) {
            a[k+1] = a[i];
            k++;
        }
    }
    return (k+1);
}

int remove_minusOne (int arr[], int n){
    for(int k = 0; k < n; k++)
        if (arr[k] == -1){
            for(int j = k; j < n ; j++)
                arr[j] = arr[j+1];
            n--;
        }
    return n;
}

//вернёт номер элемента с ненужной мастью (для флашей)
int remove_redundant(int arr[], int n, int suite){
    //printf("suite!! %i \n",  suite);
    for(int k = 0; k < n; k++)
        if ((arr[k] & 3) != suite){
            //printf("&!k: %i val: %i\n", k, arr[k] & 3);
            return k;
        }
    return -1;
}




//определяет победителя среди множества рук
winners findWinner(int cards[][7], int playerCount, int cardCount){
    winners _w;
    _w._winners;
    _w.count = playerCount;

    winnerData winners_arr[23];
    for(int i = 0;  i < 23; i++){
        winners_arr[i].handValue  =   -1;
        winners_arr[i].number     =   i;
        winners_arr[i].handType   =   -1;
        for(int j = 0; j < 7; j++)
            winners_arr[i]._cards[j] = -1;

    };

    for(int i = 0; i < playerCount; i++){
        handEvalResult hand_eval = handEval(cards[i], cardCount);
        winners_arr[i].handValue = hand_eval.handValue;
        winners_arr[i].handType = hand_eval.handType;

        //printf("suite!! %i \n",  hand_eval.suite);
        int _cards[7] = {-1, -1, -1, -1, -1, -1, -1};
        int _count = 0;

        //получим ранги без -1 и повторений
        quick_sort( hand_eval.win_cards, 0, 7);
        int n = unique( hand_eval.win_cards, 7);
        //printf("n!! %i \n", n);
        n = remove_minusOne(hand_eval.win_cards, n);
        //printf("n!! %i \n", n);
        //for(int p = 0; p < n; p++)
        //    printf("pt!! %i \n", hand_eval.win_cards[p]);
        //найдем карты для рангов
        for(int k = 0; k < 7; k++){
            int card_rank = cards[winners_arr[i].number][k] >> 2;
            for(int j = 0; j < n; j++)
                if (card_rank ==  hand_eval.win_cards[j]){
                    _cards[_count] = cards[winners_arr[i].number][k];
                    _count++;
                };
        };
        //printf("_count!! %i \n", _count);

        /* для флашей */
        if (hand_eval.suite != -1){
            int r = remove_redundant(_cards, _count, hand_eval.suite);
            printf("r! %i \n", r);
            if (r != -1 ){
            for(int k =r; k < _count - 1; k++){
                _cards[k] = _cards[k+1];
            };
             _count--;
            };
        };

        /* для флашей */


        for(int k =0; k < _count; k++){
            winners_arr[i]._cards[k] = _cards[k];
            //printf("wincards!! %i \n", _cards[k]);
        };
        winners_arr[i]._count = _count;
    };

    special_quick_sort(winners_arr, 0, playerCount);

    //printf("handValue 0 %i\n", winners_arr[0].handValue);
    //printf("handValue 1 %i\n", winners_arr[1].handValue);

    int stop = 1;
    for(int i = 0; i < playerCount - 1 ; i++){
        if (winners_arr[i].handValue != winners_arr[i+1].handValue){
            break;
        };
        stop++;
        //printf("handValue %i \n", winners_arr[i].handValue);
    };




    //printf("win count %i \n", stop);
    for(int i = 0; i < stop; i++){
        _w._winners[i] = winners_arr[i];
    }
    _w.count = stop;



    return _w;
}


int is_royal_flush(int cards[], int l){
    for(int i = 0; i < l; i++)
        if (cards[i] == 48 || cards[i] == 49 || cards[i] == 50 || cards[i] == 51)
            return 1;
    return 0;
}

//----------------------------------------------------------------------------------------------------------


int main(int argc, char *argv[])
{  
    //int cards[7] = {16, 17, 18, 1, 2, 15, 14} ;

    int cards[7] = {1, 4, 24, 32, 5, 51, 2} ; //номера есть карты в картинке из корня проекта

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
                                //{0 , 4, 8, 12, 16, 20, 24},
                                //{0 , 4, 8, 12, 16, 28, 36},
                                //{25 , 29, 33, 37, 41, 45, 49},     //Одна из рук для сравнения
                                //{1 , 13, 25, 33, 5, 51, 2},

                                {1 , 4, 24, 32, 11, 51, 2},
                                //{10 , 4, 20, 32, 5, 50, 2},

                                //{1 , 4, 24, 32, 5, 51, 2},
                                //{1 , 4, 23, 32, 51, 50, 2},

                                {1 , 4, 24, 32, 15, 51, 2},
                                //{1 , 2, 3, 4, 5, 51, 25},
                          };


    int use_cards = 7;
    winners _winners = findWinner(cards_judgement,

            2,                                                  //количество игроков (строки)
            use_cards);                                                 //количество карт



    for (int i = 0; i < _winners.count; i++){

        if (_winners._winners[i].handType == 8){
            if (is_royal_flush(_winners._winners[i]._cards, use_cards) == 1)
                printf("\n\n handType of winner: royal flash \n"); else
            printf("handType of winner %i \n", _winners._winners[i].handType);
        }
        else printf("handType of winner %i \n", _winners._winners[i].handType);
        printf("\n\n judgement: \n");
        printf("number of winner %i \n", _winners._winners[i].number);
        printf("handValue of winner %i \n", _winners._winners[i].handValue);
        //printf("handType of winner %i \n", _winners._winners[i].handType);

        if (_winners._winners[i]._count >= 5)
            for(int k =0; k < _winners._winners[i]._count; k++){
                printf("win card!!: %i \n",_winners._winners[i]._cards[k]);
            }
        else {
                    int _nead = 5 - _winners._winners[i]._count;
                    //добавляем карты из руки наибольшие по рангу, чтобы было 5 штук

                    int arr_nead[7] = {-1, -1, -1, -1, -1, -1, -1};
                    int iter = 0;

                    for(int h = 0; h < use_cards; h++){
                        int not_in_winner_cards = 1;
                        for(int k =0; k < _winners._winners[i]._count; k++){
                            if (_winners._winners[i]._cards[k] == cards_judgement[_winners._winners[i].number][h])
                                not_in_winner_cards = 0;
                        };

                        if (not_in_winner_cards == 1){
                            arr_nead[iter] = cards_judgement[_winners._winners[i].number][h];
                            iter++;
                        }
                    }

                    //for(int l = 0; l < 7; l++){
                    //    printf("before __: %i \n",arr_nead[l]);
                    //}


                    quick_sort_by_rank(arr_nead, 0, 7);
                    //for(int l = 0; l < 7; l++){
                    //    printf("after __: %i \n",arr_nead[l]);
                    //}

                    for(int k =0; k < _winners._winners[i]._count; k++){
                        printf("win card: %i \n",_winners._winners[i]._cards[k]);
                    }


                    for(int l = 0; l < _nead; l++){
                        printf("win card __: %i \n",arr_nead[l]);
                    }

            }


    };

    /*
      тип руки:  handType
      straight  flush           8
      quad                      7
      full house                6
      flush                     5
      straight (по возрастанию) 4
      trip                      3
      2 pairs                   2
      pair                      1
      high cards                0
      */

    int cards2[7] = {2, 3, 10, 14, 22, 51, 32} ;
    resultGetHand rgh = monteCarloSimulation_getHand(cards2, //карты 7 штук (если префлоп, то реальном нужны лишь первые 2, флоп : 5, тёрн : 6, ривер : 7)
                                                     7,                                  //7 штук
                                                     1,                                  //фаза. 0-префлоп, 1 - флоп, 2 - тёрн, 3 -ривер
                                                     10,                                  //общее число игроков ( я + число оппонентов)
                                                     40000                             //число итераций в монтекарло
                                                     );
    printf("\n\n chance to collect specific hand; can be not victorious: \n");
    for (int i = 0; i < 9 ; i++)
        printf("probabilty %f of %i \n", rgh.getOdds[i], i);





    //int l = sizeof(cards) / sizeof(int); //размер массива
    //h_e = handEval(cards, l);
    //printf("%d\n",h_e);


}
