#pragma once
#include <vector>

#include "Cluedo.h"
#include "Clue.h"

//The Player struct handles tracking the player's hand and which cards are still possible for the player to have in their hand
struct Player
{
private:
	vector<Card> hand;
	vector<bool> possible = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };
	vector<Clue> clues;
	short handsize;
public:
	string name;
	short character = -1;

	bool operator== (const Player& p) const;

	//Is it still possible for this player to have this card?
	bool cardPossible(const Card& c);

	//Does the player's hand contain a certain card?
	bool handContains(const Card& c) const;

	//Is the player's hand full?
	bool handFull() const;

	//Check if any combos have been fulfilled and update them
	void updateClues();

	//Set the hand size
	void setHandSize(const short& size);

	//Add a combination to a player
	void addClue(const vector<Card>& variables, const vector<bool>& nots, const bool& addHand, const Card& card);

	//Add a card to the player's hand and makes it impossible for any other player to have it in his hand
	bool addHand(const Card& c, const bool& clue);

	//Set the player's hand to be all the cards he could possibly have
	void setHand();

	//Sort and get the player's hand
	vector<Card> getHand() const;

	//Set that it is impossible for the player to have a certain card
	void impossible(const Card& n, const bool& clue);

	//The rooms' possibilities
	vector<bool> posRooms() const;

	//The suspects' possibilities
	vector<bool> posSuspects() const;

	//The weapons' possibilities
	vector<bool> posWeapons() const;

	//THe suspect cards in this  person's hand
	vector<Card> handSuspects() const;

	//The weapon cards in this person's hand
	vector<Card> handWeapons() const;

	//The room cards in this player's hand
	vector<Card> handRooms() const;

	Player(string name, short handsize);

	Player(string name);

	Player();
};