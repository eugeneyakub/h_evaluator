
#import <stdio.h>
#import "./holdem/holdem.h"
#import <time.h>

void  someCallbackHoldem(resultAccumulated ra2)
{
    printf("accumulated version: ");
    for (int i = 0; i < 10 ; i++)
        printf("probabilty %f of %i \n", ra2._resultGetHand.getOdds[i], i);
    printf("winCount monte %i \n", ra2._resultGame.winCount);
    printf("tieCount monte %i \n",ra2._resultGame.tieCount);
    printf("win odds monte %f \n", ra2._resultGame.winOdds);
    printf("tie odds monte %f \n",ra2._resultGame.tieOdds);
    printf("lose odds monte %f \n", (1.0 - ra2._resultGame.tieOdds - ra2._resultGame.winOdds));
    printf("cards generated monte %i \n", ra2._resultGame.globalCount);
};

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
                                {0 , 4, 8, 12, 16, 24, 28},
                                {0 , 4, 8, 12, 16, 28, 36},
                                {25 , 29, 33, 37, 41, 45, 49},     //Одна из рук для сравнения
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
            use_cards);                                                //количество карт


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
    //int h_e = handEval(cards, l);
    //printf("%d\n",h_e);


    /*
      0-7 карт.
      При:
      5 - флоп
      2 - префлоп
      6 - тёрн
      7  - ривер
     */

    int cards3[7] = {48, 44, 40, 36, 28, 11, 2};
    resultAccumulated ra2 = monteCarloSimulation_enchanced2(cards3, 7,                                  //7 штукмаксимум

                                                          2,                                  //общее число игроков ( я + число оппонентов)
                                                          40000,                            //число итераций в монтекарло
                                                           5,   someCallbackHoldem);                               //карт известно 0-7

                                                         
    printf("accumulated version: ");
    for (int i = 0; i < 10 ; i++)
        printf("probabilty %f of %i \n", ra2._resultGetHand.getOdds[i], i);
    printf("winCount monte %i \n", ra2._resultGame.winCount);
    printf("tieCount monte %i \n",ra2._resultGame.tieCount);
    printf("win odds monte %f \n", ra2._resultGame.winOdds);
    printf("tie odds monte %f \n",ra2._resultGame.tieOdds);
    printf("lose odds monte %f \n", (1.0 - ra2._resultGame.tieOdds - ra2._resultGame.winOdds));
    printf("cards generated monte %i \n", ra2._resultGame.globalCount);

}

