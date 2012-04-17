 
//#include <stdio.h>

/* 
 * The next #include line is generally present in all Objective-C
 * source files that use GNUstep.  The Foundation.h header file
 * includes all the other standard header files you need.
 */
//#include <Foundation/Foundation.h>  

/* 
 * File:   main.cpp
 * Author: eugene
 *
 * Created on January 30, 2012, 2:59 PM
 */




//#include <stdio.h>
//#include <time.h>
#import "holdem.h"

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

int is_containing_ace(int arr[], int l){
    //int l = sizeof(arr) / sizeof(int);
    int contain = 0;
    for (int i = 0; i < l; i++)
        if (arr[i] == 12){
            contain = 1;
            break;
        };
    return contain;
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

int completeHandValue(int  handValue, int cards[], int l){  
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


/*
    handEvalResult  --  Хранит результат работы handEval, которая вычисляет качество руки одного игрока.

    handValue       --  значение руки .
    handType        --  тип руки
    win_cards       --  содержит ранги карт, привёдших к победе
    int suite;		-- масть (важна лишь для флашей)
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
        isPresent[i] = 0;
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
        isPresent[card] = 1;
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
        if (is_containing_ace(run, l) == 1)
            handType = 9;// royal flush
        else
            handType = 8;// straight flush
        for (int j = 0; j < 5; j++){
            _handEvalResult.win_cards[j] = run[j];
        };
        handValue = completeHandValue(handValue, run, l);
      } else {
        handValue = 5; // flush
        handType = 5;
        for (int j = 0; j < good_length_of_array(ranksInFlushSuit, 5); j++){
            _handEvalResult.win_cards[j] = ranksInFlushSuit[j];
        };
        handValue = completeHandValue(handValue, ranksInFlushSuit, l);
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
        handValue = completeHandValue(handValue, pattern[4], 7);


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
        handValue = completeHandValue(handValue, pattern[3], 7);
        handValue = completeHandValue(handValue, pattern[2], 7);

      }
    else if (good_length_of_array(pattern[0], 7) >= 5) { // straight
        handValue = 4;
        handType = 4;
        for (int j = 0; j < 7; j++){
            _handEvalResult.win_cards[j] = pattern[0][j];
        };
        handValue = completeHandValue(handValue, pattern[0], 7);

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
        handValue = completeHandValue(handValue, pattern[3], 7);
        handValue = completeHandValue(handValue, pattern[1], 7);

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
        handValue = completeHandValue(handValue, pattern[2], 7);
        handValue = completeHandValue(handValue, pattern[1], 7);

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

        handValue = completeHandValue(handValue, pattern[2], 7);
        handValue = completeHandValue(handValue, pattern[1], 7);

      }
    else { // high card
        handValue = 0;
        handType = 0;
        for (int j = 0; j < 7; j++){
            _handEvalResult.win_cards[j] = pattern[1][j];
        };

        handValue = completeHandValue(handValue, pattern[1], 7);
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
        return min + (random() % (int)(max - min + 1));
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
    int exist = 0;
    
    for (int i = 0; i < cardCount; i++){
        int generatedNumber = -1;
        
        for(;;){
            generatedNumber = getRandomInt(0, 51);  

            exist = 0;         
            //банним карты уже на руках игроков
            if (l >0)
                    if (bannedArray[generatedNumber] != -1){
                        exist = 1;
                    }
            //банним, созданные ранее в цикле, карты
            if (i > 0)
                for (int j = 0; j < i; j++)
                    if (generatedHand.cards[j] == generatedNumber)
                        exist = 1;         
            if (exist == 0)
                break;
        };
        generatedHand.cards[i] = generatedNumber;       
    };

    return generatedHand;
}



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


        int _cards[7] = {-1, -1, -1, -1, -1, -1, -1};
        int _count = 0;

        //получим ранги без -1 и повторений
        quick_sort( hand_eval.win_cards, 0, 7);
        int n = unique( hand_eval.win_cards, 7);
        n = remove_minusOne(hand_eval.win_cards, n);
        //найдем карты для рангов
        for(int k = 0; k < 7; k++){
            int card_rank = cards[winners_arr[i].number][k] >> 2;
            for(int j = 0; j < n; j++)
                if (card_rank ==  hand_eval.win_cards[j]){
                    _cards[_count] = cards[winners_arr[i].number][k];
                    _count++;
                };
        };

        /* для флашей */
        if (hand_eval.suite != -1){
            int r = remove_redundant(_cards, _count, hand_eval.suite);
	    while (r != -1 ){
            //printf("r! %i \n", r);

	      for(int k =r; k < _count - 1; k++){
		  _cards[k] = _cards[k+1];
	      };
	      _count--;
	      r = remove_redundant(_cards, _count, hand_eval.suite);
	    }
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
        if ((cards[i] == 48) || (cards[i] == 49) || (cards[i] == 50) || (cards[i] == 51))
            return 1;
    return 0;
}




//----------------------------------------------------------------------------------------------------------
resultAccumulated monteCarloSimulation_enchanced2(int cards[], int l, int playerCount, int monteCarloMaxIteration, int r, void (*callbackHoldem)(resultAccumulated)){
    resultGetHand _resultGetHand;
    for (int i = 0; i < 10 ; i++)
        _resultGetHand.getOdds[i] = 0;

    int oppCount = playerCount - 1;//число оппонентов

    int p1[2] = {-1, -1};
    //рука игрока
    if (r>=2){
        p1[0] = cards[0];
        p1[1] = cards[1];

    }
    else if (r==1){
        p1[0] = cards[0];

    }

    int alreadyBoardCount = 0;
    int boardCount = 0;
    //нет карт на столе (префлоп)
    int alreadyboard[5] = {-1, -1, -1, -1, -1};

    if (r == 2){//префлоп
      alreadyBoardCount = 0; //число карт на столе (известны)
      boardCount = 5;        //число карт, которые будем генерировать случайным образом
    }
    else if (r == 3){//недофлоп
      alreadyBoardCount = 1;
      alreadyboard[0] = cards[2];
      boardCount = 4;
    }
    else if (r == 4){//недофлоп
      alreadyBoardCount = 2;
      alreadyboard[0] = cards[2];
      alreadyboard[1] = cards[3];
      boardCount = 3;
    }
    else if (r == 5){//флоп
      alreadyBoardCount = 3;
      alreadyboard[0] = cards[2];
      alreadyboard[1] = cards[3];
      alreadyboard[2] = cards[4];
      boardCount = 2;
    }
    else if (r == 6){//тёрн
      alreadyBoardCount = 4;
      alreadyboard[0] = cards[2];
      alreadyboard[1] = cards[3];
      alreadyboard[2] = cards[4];
      alreadyboard[3] = cards[5];
      boardCount = 1;
    }
    else if (r == 7){//ривер
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


        //генерим рандомно карты противников без забанненых
        int v = 0; //число побед против оппонентов
        int e = 0; //число нечейных

        partOfRandomHand p2_evals[oppCount];



        for (int  j = 0; j < oppCount; j++){
              partOfRandomHand p2 = makeRandomHand(bannedArray, 52, 2); //две карты противника из префлопа
              //банним их
              if (p2.count > 0)
              for (int k = 0; k < p2.count; k++){
                  bannedArray[p2.cards[k]]++;

                }
              p2_evals[j] = p2;


        }

        if (r <= 2){ //префлоп
            board = makeRandomHand(bannedArray, 52,  5); //5 карт на столе неизвестны
        }
        else if (r == 3){ //недофлоп
            board = makeRandomHand(bannedArray, 52,  4);
        }
        else if (r == 4){ //недофлоп
            board = makeRandomHand(bannedArray, 52,  3);
        }
        else if (r == 5){ //флоп
            board = makeRandomHand(bannedArray, 52,  2);
        }
        else if (r == 6){ //тёрн
            board = makeRandomHand(bannedArray, 52,  1);
        }

        //банним карты, сгенерированные рандомно на столе
        if (board.count > 0)
            for (int j = 0; j < board.count; j++)
                  bannedArray[board.cards[j]]++;

        int p2_evals_values[oppCount];
        for (int  j = 0; j < oppCount; j++){
            handEvalResult p2_eval;

            if (r <= 2){ //префлоп
                int arr[7] = {p2_evals[j].cards[0], p2_evals[j].cards[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4] };
                p2_eval = handEval(arr, 7);
            }
            if (r == 3){ //недофлоп
                int arr[7] = {p2_evals[j].cards[0], p2_evals[j].cards[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], alreadyboard[0] };
                p2_eval = handEval(arr, 7);
            }
            if (r == 4){ //недофлоп
                int arr[7] = {p2_evals[j].cards[0], p2_evals[j].cards[1], board.cards[0], board.cards[1], board.cards[2], alreadyboard[0], alreadyboard[1] };
                p2_eval = handEval(arr, 7);
            }
            else if (r == 5){ //флоп
                int arr[7] = {p2_evals[j].cards[0], p2_evals[j].cards[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board.cards[0], board.cards[1] };
                p2_eval = handEval(arr, 7);
            }
            else if (r == 6){ //тёрн
                int arr[7] = {p2_evals[j].cards[0], p2_evals[j].cards[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], board.cards[0]};
                p2_eval = handEval(arr, 7);
            }
            else if (r == 7){ //ривер
                int arr[7] = {p2_evals[j].cards[0], p2_evals[j].cards[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] };
                p2_eval = handEval(arr, 7);
            }
            p2_evals_values[j] = p2_eval.handValue;
        }



        partOfRandomHand p1_nead;

        if (r == 0){ //недопрефлоп
            p1_nead = makeRandomHand(bannedArray, 52,  2);
            int arr[7] = {p1_nead.cards[0], p1_nead.cards[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4] };
            p1_eval = handEval(arr, 7);

        }
        if (r == 1){ //недопрефлоп
            p1_nead = makeRandomHand(bannedArray, 52,  1);
            int arr[7] = {p1[0], p1_nead.cards[0], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4] };
            p1_eval = handEval(arr, 7);

        }
        if (r == 2){ //префлоп

            int arr[7] = {p1[0], p1[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4] };
            p1_eval = handEval(arr, 7);

        }
        else if (r == 3){ //недофлоп

            int arr[7] = {p1[0], p1[1], alreadyboard[0], board.cards[2], board.cards[3], board.cards[0], board.cards[1] };
            p1_eval = handEval(arr, 7);

        }
        else if (r == 4){ //недофлоп

            int arr[7] = {p1[0], p1[1], alreadyboard[0], alreadyboard[1], board.cards[2], board.cards[0], board.cards[1] };
            p1_eval = handEval(arr, 7);

        }
        else if (r == 5){ //флоп

            int arr[7] = {p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board.cards[0], board.cards[1] };
            p1_eval = handEval(arr, 7);

        }
        else if (r == 6){ //тёрн

            int arr[7] = {p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], board.cards[0]};
            p1_eval = handEval(arr, 7);

        }
        else if (r == 7){ //ривер
            int arr[7] = {p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] };
            p1_eval = handEval(arr, 7);

        }

        _resultGetHand.getOdds[p1_eval.handType]++;



        for (int  j = 0; j < oppCount; j++){

            if (p1_eval.handValue > p2_evals_values[j]){
                  v++; }
              else if (p1_eval.handValue ==  p2_evals_values[j]){
                  e++; }
        }
        if (v == oppCount)
          winCount++;
        if (e == oppCount)
          tieCount++;
        globalCount++;

	if (globalCount % 10000 == 0){
	  float winOdds = winCount * 1.0 / globalCount ;
	  float tieOdds = tieCount * 1.0 / globalCount;
	  resultGame p_resultGame;
	  p_resultGame.winOdds = winOdds;
	  p_resultGame.tieOdds = tieOdds;
	  p_resultGame.winCount = winCount;
	  p_resultGame.tieCount = tieCount;
	  p_resultGame.globalCount = globalCount;
	  
	  resultGetHand p_resultGetHand;
	  for (int i=0; i < 10 ; i++)
	      p_resultGetHand.getOdds[i] = _resultGetHand.getOdds[i];

	  for (int i = 0; i < 10; i++)
	      p_resultGetHand.getOdds[i] /= (1.0 * globalCount);

	  resultAccumulated partialAccumulated;
	  partialAccumulated._resultGame = p_resultGame;
	  partialAccumulated._resultGetHand = p_resultGetHand;
	  callbackHoldem(partialAccumulated);
	};
    };

    float winOdds = winCount * 1.0 / globalCount ;
    float tieOdds = tieCount * 1.0 / globalCount;
    resultGame _resultGame;
    _resultGame.winOdds = winOdds;
    _resultGame.tieOdds = tieOdds;
    _resultGame.winCount = winCount;
    _resultGame.tieCount = tieCount;
    _resultGame.globalCount = globalCount;




    //printf("global %i \n", globalCount);

    for (int i = 0; i < 10; i++)
        _resultGetHand.getOdds[i] /= (1.0 * globalCount);

    resultAccumulated _resultAccumulated;
    _resultAccumulated._resultGame = _resultGame;
    _resultAccumulated._resultGetHand = _resultGetHand;
    return _resultAccumulated;
}
