// This is a simple *viewmodel* - JavaScript that defines the data and behavior of your UI
var viewModel = {
	card0: ko.observable(0),
    card1: ko.observable(20),
    card2: ko.observable(1),
    card3: ko.observable(4),
    card4: ko.observable(5),
    card5: ko.observable(9),
    card6: ko.observable(25)

    //capitalizeLastName: function() {
    //    var currentVal = this.lastName();        // Read the current value
    //    this.lastName(currentVal.toUpperCase()); // Write back a modified value
    //}
};




viewModel.naiveHandEval = function(card0, card1, card2, card3, card4, card5, card6) {
 
/*
  Input :
 
    Exactly 7 distinct numbers between 0 and 51 included.
 
    Each number represents a card. Its quotient by 4 represents its rank,
    from 0 (Deuce) to 12 - 0xc - (Ace). The remainder represents the suit.
 
  Output :
 
    undefined if the input is incorrect.
 
    Otherwise, a number describing the best poker hand that can be made
    with the 7 cards, such that higher numbers represent better hands.
 
    It has the following structure :
 
    - 4 leading bits representing the category of the best 5 card poker
      hand that can be made out of the 7 cards :
        0 (0x0) => high card,
        1 (0x1) => one pair,
        2 (0x2) => two pairs,
        3 (0x3) => three of a kind a.k.a. trip,
        4 (0x4) => straight,
        5 (0x5) => flush,
        6 (0x6) => full house,
        7 (0x7) => four of a kind a.k.a. quad,
        8 (0x8) => straight flush.
      ("Royal flush" is just a fancy name for a straight flush whose
      high card is an ace.)
 
     - 20 bits representing the ranks of the 5 most significant cards,
        4 bits per card, from 0 (0x0) => deuce to 12 (0xc) => ace.
 
     E.g, 0x27744a means 2 pairs (0x2), nines (0x77) and sixes (0x44),
     fifth card a queen (0xa). 0x6999cc means Full house, jacks full
     of aces.
 
*/
var cardArr = Array();
for (var i = 0; i < arguments.length; i++){
  //if (arguments[i] != '')
  cardArr.push(arguments[i]);
};

var l = cardArr.length;
viewModel.cardCount = l;

//console.log(' l ' + cardArr.length);
 
  var handValue; // the desired result
 
  function completeHandValue(cards) {
  /* A local function to add the top cards of a given array
  to a partially calculated handValue. */
 
    if (handValue < 0x10) { // put in a sentinel: when that bit
                            // reaches the 21st position, we're done
      handValue |= 0x10;
    }
 
    while (handValue < 0x200000 && cards.length > 0) {
      handValue <<= 4;
      handValue += cards.shift();
    }
 
    if (handValue & 0x1000000) { // done: remove the sentinel
      handValue &= 0xffffff;
    }
  } // end of local function completeHandValue
 
 //argumkents
 //if (arguments.length != 7){
 //  return;
 //}

  if (cardArr.length != l) {
    return;
  }
 
  // Set up the three main data structures we shall use
  var suitCounts = [0, 0, 0, 0];
  var rankCounts = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
  var isPresent = []; // isPresent[card] is true iff card is one
                      // of the 7 in the hand
  var card;
  var rank;
  var suit;
 
 //i < 7
  //for (var i = 0; i < 7; i++) {
    //card = arguments[i];
  for (var i = 0; i < l; i++) {
    card = cardArr[i];
    if (isPresent[card] || card < 0 || card > 51) {
      return;
    }
 
    isPresent[card] = true;
 
    rank = card >> 2;
    rankCounts[rank]++;
 
    suit = card & 3;
    suitCounts[suit]++;
  }
 
  // Determine if we have a flush, and if so, of what suit
  var flushSuit = -1;
  for (suit = 0; suit < 4; suit++) {
    if (suitCounts[suit] >= 5) {
      flushSuit = suit;
      break;
    }
  }
 
  if (flushSuit >= 0) { // there is a flush
    var run = []; // if this array ever reaches a length of 5,
                  // we have a Straight Flush!
    var ranksInFlushSuit = []; // this array collects the 5 highest
                               // ranks in the suit, in case we have not
    for (rank = 12; rank >= 0; rank--) {
      if (!isPresent[rank * 4 + flushSuit]) {
        continue;
      }
      if (rank === run[run.length - 1] - 1) {
        run.push(rank);
      } else if (run.length < 5) {
        run = [rank];
      }
      ranksInFlushSuit.push(rank);
    }
 
    // check for "wheel": straight 5432A
    if (run[3] === 0 && isPresent[48 + flushSuit]) {
      run.push(0xc); // push an ace
    }
 
    if (run.length >= 5) {
      handValue = 8; // straight flush
      completeHandValue(run);
    } else {
      handValue = 5; // flush
      completeHandValue(ranksInFlushSuit);
    }
 
    return handValue;
  }
 
  // there is no flush
  var pattern = [ // an array of 5 arrays
    [], // one for runs that could make straights, like above
    [], // one for high cards - it is pattern[1] by no coincidence
    [], // pattern[2] for pairs
    [], // pattern[3] for trips
    []  // pattern[4] for quads
  ];
 
  for (rank = 12; rank >= 0; rank--) {
    var rankCount = rankCounts[rank];
 
    if (!rankCount) {
      continue;
    }
 
    if (rank === pattern[0][pattern[0].length - 1] - 1) {
      pattern[0].push(rank);
    } else if (pattern[0].length < 5) {
      pattern[0] = [rank];
    }
 
    for (i = 1; i <= rankCount; i++) {
      pattern[rankCount].push(rank);
    }
  }
 
  // check for "wheel": straight 5432A
  if (pattern[0][3] === 0 && rankCounts[0xc]) {
    pattern[0].push(0xc); // push an ace
  }
 
  if (pattern[4].length) { // quad
    handValue = 7;
    // the fith card is tricky - it can be in trips, pairs or high cards
    var fifthCard = pattern[1][0]; // could be undefined
    if (fifthCard === undefined || fifthCard < pattern[2][0]) {
      fifthCard = pattern[2][0];
    }
    if (fifthCard === undefined || fifthCard < pattern[3][0]) {
      fifthCard = pattern[3][0];
    }
    pattern[4].push(fifthCard);
    completeHandValue(pattern[4]);
  } else if (pattern[3].length > 3 ||
      pattern[3].length && pattern[2].length) { // full house
    handValue = 6;
    completeHandValue(pattern[3]);
    completeHandValue(pattern[2]);
  } else if (pattern[0].length >= 5) { // straight
    handValue = 4;
    completeHandValue(pattern[0]);
  } else if (pattern[3].length) { // trip
    handValue = 3;
    completeHandValue(pattern[3]);
    completeHandValue(pattern[1]);
  } else if (pattern[2].length > 2) { // two pairs
    handValue = 2;
    if (pattern[2].length > 4 && pattern[2][4] < pattern[1][0]) {
      pattern[2].length = 4;
    }
    completeHandValue(pattern[2]);
    completeHandValue(pattern[1]);
  } else if (pattern[2].length) { // pair
    handValue = 1;
    completeHandValue(pattern[2]);
    completeHandValue(pattern[1]);
  } else { // high card
    handValue = 0;
    completeHandValue(pattern[1]);
  }
 
  return handValue;
} // end of function naiveHandEval



viewModel.getHandValue = ko.dependentObservable(function() {
    return this.naiveHandEval(this.card0(), this.card1(), this.card2(), this.card3(), this.card4(), this.card5(), this.card6());
}, viewModel);




viewModel.makeRandomHand = function(bannedArray, cardCount){
  var generatedHand = Array();
  var exist = false;

  for(var i = 0; i < cardCount; i++){
    
    var generatedNumber;
    
    for (;;){
      generatedNumber = getRandomInt(0, 51);
      //if (generatedNumber == 52)
      //console.log('fu');

      exist = false;
      //если есть уже руки, то перебираем в них все карты и сравниваем с созданной
      if (bannedArray.length){
        for (var j = 0; j < bannedArray.length; j++ )
            if (bannedArray[j] == generatedNumber)
              exist = true;
      };
      //проверяем и созаданные предыдущие карты:
      if (generatedHand.length){
        for (var j = 0; j < generatedHand.length; j++ )
          if (generatedHand[j] == generatedNumber)
              exist = true;
      };

      if (exist === false)    break;
      
    };

    generatedHand.push(generatedNumber);

  };

  return generatedHand;
};

viewModel.Exhaustive = function(){
  var playersNumber = this.playersNumber();  

  //var p1 = [parseInt(this.card0()), parseInt(this.card1())];
  p1 = [parseInt(this.card0()), parseInt(this.card1())];
  var globalCount = 0;
  var winCount = 0;
  var tieCount = 0;

  var alreadyboard = [parseInt(this.card2()), parseInt(this.card3()),parseInt(this.card4()), parseInt(this.card5()),parseInt(this.card6())]
  /*
  for(var i = 0; i < this.monteCarloMaxIteration(); i++ ){
    //var p1 = this.makeRandomHand();
      var bannedArray = Array();
      bannedArray = bannedArray.concat(p1);
      bannedArray = bannedArray.concat(alreadyboard);
      var p2 = this.makeRandomHand(bannedArray, 2);
      bannedArray = bannedArray.concat(p2)
      //var board = this.makeRandomHand(bannedArray, 5);
      var board = this.makeRandomHand(bannedArray, 2);

     

      //var p1_eval = this.naiveHandEval(p1[0], p1[1], board[0], board[1], board[2], board[3], board[4] );
      //var p2_eval = this.naiveHandEval(p2[0], p2[1], board[0], board[1], board[2], board[3], board[4] );
      
      //var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board[0], board[1] );
      //var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board[0], board[1] );
      var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );
      var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );

      if (p1_eval > p2_eval){
        winCount++ }
      else if (p1_eval === p2_eval){
        //console.log(p2[0]+ ' '+ p2[1]);
        tieCount++;
      };

      globalCount++;
      

  }
*/

  var hand = Array();
  for (hand[0] = 0; hand[0] < 51; hand[0]++) {
    for (hand[1] = hand[0]+1; hand[1] < 52; hand[1]++) {

      if (hand[0] != p1[0] && hand[0] != p1[1] && hand[0] != alreadyboard[0] && hand[0] != alreadyboard[1] && hand[0] != alreadyboard[2] && hand[0] != alreadyboard[3] && hand[0] != alreadyboard[4]  &&
      hand[1] != p1[0] && hand[1] != p1[1] && hand[1] != alreadyboard[0] && hand[1] != alreadyboard[1] && hand[1] != alreadyboard[2] && hand[1] != alreadyboard[3] && hand[1] != alreadyboard[4] ){
       
        var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );
        var p2_eval = this.naiveHandEval(hand[0], hand[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );

        if (p1_eval > p2_eval){
          console.log(hand[0]+ ' '+ hand[1]);
          console.log(p2_eval);
          winCount++; }
        else if (p1_eval === p2_eval){
          //console.log(p2[0]+ ' '+ p2[1]);
          //console.log(hand[0]+ ' '+ hand[1]);
          tieCount++;
        };

        globalCount++;
      }
    } 
  }

  var winOdds = (winCount )/ globalCount;
  var tieOdds = tieCount / globalCount;
  console.log('winCount ' + winCount);
  console.log('tieCount ' + tieCount);
  console.log('win odds ' + winOdds);
  console.log('tie odds ' + tieOdds);
  console.log('slave odds ' + (1.0 - tieOdds - winOdds));
  console.log('all possible cards ' + globalCount);

};

viewModel.getOddsOfWin = ko.dependentObservable(function() {
    //return this.OddsOfWin();
    //return this.monteCarloSimulation();
    var _eval = this.naiveHandEval(this.card0(), this.card1(), this.card2(), this.card3(), this.card4(), this.card5(), this.card6() );
    console.log('bad: '+ _eval);

}, viewModel);

viewModel.monteCarloMaxIteration = ko.observable(20000);
viewModel.playersNumber = ko.observable(2);


viewModel.phase = ko.observable(0);





viewModel.monteCarloSimulation = function(){
  //p1 = [parseInt(this.card0()), parseInt(this.card1())];

  var oppCount = parseInt(this.playersNumber()) - 1;

  var ph = parseInt(this.phase());
  if (ph == 0)
    phase = 'pre-flop'
  else if (ph == 1)
   phase  = 'flop'
  else if (ph == 2) 
   phase = 'turn'
  else if (ph == 3) 
    phase =  'river'

  var p1 = [parseInt(this.card0()), parseInt(this.card1())];

  if (phase == 'pre-flop'){
    alreadyBoardCount = 0;
    boardCount = 5;
  }
  else if (phase  == 'flop'){
    alreadyBoardCount = 3;
    var alreadyboard = [parseInt(this.card2()), parseInt(this.card3()), parseInt(this.card4())];
    boardCount = 2;
  }
  else if (phase  == 'turn'){
    alreadyBoardCount = 4;
    var alreadyboard = [parseInt(this.card2()), parseInt(this.card3()),parseInt(this.card4()), parseInt(this.card5())];
  
    boardCount = 1;
  }
  else if (phase  == 'river'){
    alreadyBoardCount = 5;
    var alreadyboard = [parseInt(this.card2()), parseInt(this.card3()),parseInt(this.card4()), parseInt(this.card5()),parseInt(this.card6())]
    boardCount = 0;
  } 


  var globalCount = 0;
  var winCount = 0;
  var tieCount = 0;
  var loseCount = 0;


  //var alreadyboard = [parseInt(this.card2()), parseInt(this.card3()),parseInt(this.card4()), parseInt(this.card5()),parseInt(this.card6())]
  
  for(var i = 0; i < parseInt(this.monteCarloMaxIteration()); i++ ){
    //var p1 = this.makeRandomHand();
      var bannedArray = Array();
      bannedArray = bannedArray.concat(p1);

      if (alreadyBoardCount != 0)
        bannedArray = bannedArray.concat(alreadyboard);

      var lose = false;

      if (phase == 'pre-flop'){
        var board = this.makeRandomHand(bannedArray, 5);
        var p1_eval = this.naiveHandEval(p1[0], p1[1], board[0], board[1], board[2], board[3], board[4] );
      }
      else if (phase  == 'flop'){
        var board = this.makeRandomHand(bannedArray, 2);
        var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board[0], board[1] );
      }
      else if (phase  == 'turn'){
        var board = this.makeRandomHand(bannedArray, 1);
        var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], board[0] );
     
      }
      else if (phase  == 'river'){
        var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );
      } 
      //var board = this.makeRandomHand(bannedArray, 5);
      //var board = this.makeRandomHand(bannedArray, 2);
      if (boardCount != 0)
        bannedArray = bannedArray.concat(board);

      
      //var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );


       

      //var p1_eval = this.naiveHandEval(p1[0], p1[1], board[0], board[1], board[2], board[3], board[4] );
      //var p2_eval = this.naiveHandEval(p2[0], p2[1], board[0], board[1], board[2], board[3], board[4] );
        
      //var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2]);//, board[0]), board[1] );
      //var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board[0], board[1] );
        



      var v = 0;
      var e = 0;
      for(var j = 0; j < oppCount; j++ ){

        var p2 = this.makeRandomHand(bannedArray, 2);
        bannedArray = bannedArray.concat(p2)


        if (phase == 'pre-flop'){
          var p2_eval = this.naiveHandEval(p2[0], p2[1], board[0], board[1], board[2], board[3], board[4] );
        }
        else if (phase  == 'flop'){
          var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board[0], board[1] );
        }
        else if (phase  == 'turn'){
          var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], board[0] );      
        }
        else if (phase  == 'river'){
          var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );
        }

        //var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2]), alreadyboard[3]), alreadyboard[4] );
      //var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2]);//, board[0]);//, board[1] );
       // var p2_eval = this.naiveHandEval(p2[0], p2[1], board[0], board[1], board[2], board[3], board[4] );

        if (p1_eval > p2_eval){
          v++ }
        else if (p1_eval === p2_eval){
          //console.log(p2[0]+ ' '+ p2[1]);
          e++;
        };
        
      };

      if (v == oppCount)
        winCount++;
      if (e == oppCount)
        tieCount++;
          

      globalCount++;

  };

   var winOdds = (winCount )/ globalCount;
  var tieOdds = tieCount / globalCount;
  console.log('winCount monte' + winCount);
  console.log('tieCount monte' + tieCount);
  console.log('win odds monte' + winOdds);
  console.log('tie odds monte' + tieOdds);
  console.log('slave odds monte' + (1.0 - tieOdds - winOdds));
  console.log('all possible cards monte' + globalCount);
 
};



function getRandomInt (min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
};













//Не нужно:
viewModel.monteCarloSimulation2 = function(){
  p1 = [parseInt(this.card0()), parseInt(this.card1())];
  var globalCount = 0;
  var winCount = 0;
  var tieCount = 0;

  var alreadyboard = [parseInt(this.card2()), parseInt(this.card3()),parseInt(this.card4()), parseInt(this.card5()),parseInt(this.card6())]
  
  for(var i = 0; i < parseInt(this.monteCarloMaxIteration()); i++ ){
    //var p1 = this.makeRandomHand();
      var bannedArray = Array();
      bannedArray = bannedArray.concat(p1);
      bannedArray = bannedArray.concat(alreadyboard);

      var greaterthan = true;
      var greaterthanequal = true;

     // for(var j = 0; j < parseInt(this.playersNumber()) - 1; j++ ){


        var p2 = this.makeRandomHand(bannedArray, 2);
        bannedArray = bannedArray.concat(p2)
        //var board = this.makeRandomHand(bannedArray, 5);
        var board = this.makeRandomHand(bannedArray, 3);

       

        var p1_eval = this.naiveHandEval(p1[0], p1[1], board[0], board[1], board[2]);//, board[3], board[4] );
        var p2_eval = this.naiveHandEval(p2[0], p2[1], board[0], board[1], board[2]);//, board[3], board[4] );
        
        //var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board[0], board[1] );
        //var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], board[0], board[1] );
        //var p1_eval = this.naiveHandEval(p1[0], p1[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );
        //var p2_eval = this.naiveHandEval(p2[0], p2[1], alreadyboard[0], alreadyboard[1], alreadyboard[2], alreadyboard[3], alreadyboard[4] );
/*
        if (p1_eval < p2_eval){
          greaterthan = false;
          greaterthanequal = false;
          break;
        }
        else if (p1_eval <= p2_eval){
          greaterthan = false;
        };
        */
        if (p1_eval > p2_eval){
          winCount++ }
        else if (p1_eval === p2_eval){
          //console.log(p2[0]+ ' '+ p2[1]);
          tieCount++;
        };
        globalCount++;
        

      //};
/*
      if (greaterthan == true)
        winCount++;
      if (greaterthanequal == true)
        tieCount++;
        
      globalCount++*/

  };
   var winOdds = (winCount )/ globalCount;
  var tieOdds = tieCount / globalCount;
  console.log('winCount monte' + winCount);
  console.log('tieCount monte' + tieCount);
  console.log('win odds monte' + winOdds);
  console.log('tie odds monte' + tieOdds);
  console.log('slave odds monte' + (1.0 - tieOdds - winOdds));
  console.log('all possible cards monte' + globalCount);
}




// Activates knockout.js
ko.applyBindings(viewModel);