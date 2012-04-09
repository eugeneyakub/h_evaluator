//
// holdem.h
// ViewsPlayground
//
// Created by Андрей Лазарев on 03.03.12.
// Copyright (c) 2012 MyCompanyName. All rights reserved.
//

#ifndef ViewsPlayground_holdem_h
#define ViewsPlayground_holdem_h



//для определения вероятности победы и пр.---------------------------------------------------------------------------------------------
/*
    результат работы:
    winOdds -- вероятность выйгрыша против игроков
    tieOdds -- вероятность нечьих против игроков
    winCount -- количество побед
    tieCount -- количнство нечьих
    globalCount -- сколько всего итераций

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
resultGame monteCarloSimulation(int cards[],			//карты 7 штук (если префлоп, то реальном нужны лишь первые 2, флоп : 5, тёрн : 6, ривер : 7)
				int l,				//7 штук
				int ph,				//фаза. 0-префлоп, 1 - флоп, 2 - тёрн, 3 -ривер
				int playerCount,		//общее число игроков ( я + число оппонентов)
				int monteCarloMaxIteration);	//число итераций в монтекарло


//для определения победителя среди множества игроков с известными картами----------------------------------------------------------------
/*
    winnerData и findWinner
    для опеределения победителя среди множества игроков
*/

typedef struct winnerData{
    int number;                 //номер победителя
    int handValue;              //значение его руки
    int handType;		//тип руки победителя
    int _cards[7];              //карты, приведшие к победе
    int _count;                 //их число (остальные: -1)
} winnerData;

typedef struct winners{		//набор победителей (при сравнении рук)
    winnerData _winners[23];	//данные победителя (максимум 23 игрока)
    int count;			//их количество
} winners;

//определяет победителя среди множества рук
/*	
 *     	int cards[][7] = {
                                {1 , 6, 19, 32, 52, 51, 22},     //Одна из рук для сравнения
                                {2 , 7, 19, 32, 52, 51, 22},
                                {3 , 8, 19, 32, 52, 51, 22},
                                {4 , 9, 19, 32, 52, 51, 22},
                                {5 , 10, 19, 32, 52, 51, 22},
                          };
	int playerCount = 5 (5 cтрок)
	int cardCount   = 7 (сколько первых карт учитывать в строке; зависит от фазы (от 5 -префлоп до 7 - ривер)
    
 
 */
winners findWinner(int cards[][7],
		      int playerCount,
		      int cardCount);


int is_royal_flush(int cards[], int l);   //определяет является ли straight flush royal flush

//для определения вероятностей собрать определенную колоду(флаш и пр)--------------------------------------------------------------------
/*
    результат работы:
    getOdds               -- вероятность получения конкретной комбинации (флаш, кольцо)
    handType              -- сама комбинация
*/
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
typedef struct resultGetHand{
    float   getOdds[10];		//каждые элемент содержит вероятность собрать определённый тип руки (тип руки - индекс элемента массива)
} resultGetHand;
//вх параметры имеют тот же смысл, что и  для 'monteCarloSimulation'
resultGetHand monteCarloSimulation_getHand(int cards[], int l, int ph, int playerCount, int monteCarloMaxIteration);

typedef struct resultAccumulated{
    resultGetHand   _resultGetHand;
    resultGame _resultGame;
} resultAccumulated;

resultAccumulated monteCarloSimulation_enchanced2(int cards[], int l, int playerCount, int monteCarloMaxIteration, int r, void (*callbackHoldem)(resultAccumulated, int));

void  someCallbackHoldem(resultAccumulated ra2, int iter);

#endif