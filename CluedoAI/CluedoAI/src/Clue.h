#pragma once
#include <vector>

#include "Cluedo.h"

struct Player;

//The Clue struct checks if certain booleans are in a certain state and then acts if they are
struct Clue
{
	vector<Card> vars; //Variables to check
	vector<bool> nots; //Whether they should be inverted
	bool addHand; //Whether the card should be added to the host's hand or made impossible
	Card card; //The card
	Player* host; //The host

	//Go through all variables, check if they are true and respond appropriately if they are
	bool check() const;

	Clue(const vector<Card>& variables, const vector<bool>& nots, const bool& addHand, const Card& card, Player* host);
};