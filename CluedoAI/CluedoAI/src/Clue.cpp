#include <iostream>
#include <vector>

#include "Clue.h"
#include "Player.h"

bool Clue::check() const
{
	if (DEBUG) cout << "\n[DEBUG] Clue.check() Called!\n";

	//Check if all variables are true
	for (short i = 0; i < vars.size(); ++i)
	{
		bool var = host->cardPossible(vars[i]);
		if (nots[i]) var = !var;

		if (!var) return false;
	}

	//Either add the card in the players hand or make it impossible for the player to have the card
	if (addHand) host->addHand(card, true);
	else host->impossible(card, true);

	return true;
}

	Clue::Clue(const vector<Card>& variables, const vector<bool>& nots, const bool& addHand, const Card& card, Player* host)
	: vars(variables), nots(nots), addHand(addHand), card(card), host(host) {}
