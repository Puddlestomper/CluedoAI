#pragma once
#include <vector>

#define DEBUG 1

using namespace std;

struct Player;

extern Player* answer;
extern vector<Player>* players;

extern bool suspFound, wepFound, roomFound;

//The Card enum contains all the cards that are in the game as well as a representaion for the final room which doesn't have a card
enum Card
{
	MISSSCARLET, COLONELMUSTARD, MRSWHITE, REVERENDGREEN, MRSPEACOCK, PROFESSORPLUM, DAGGER, CANDLESTICK, REVOLVER, ROPE, LEADPIPE, SPANNER,
	HALL, LOUNGE, DININGROOM, KITCHEN, BALLROOM, CONSERVATORY, BILLIARDROOM, LIBRARY, STUDY, END
};

ostream& operator<< (ostream& stream, Card c);

int numPossible(const vector<bool>& stuff);