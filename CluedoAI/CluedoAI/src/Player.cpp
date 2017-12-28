#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <string>

#include "Clue.h"
#include "Player.h"

bool Player::operator== (const Player& p) const { return name == p.name; }

bool Player::cardPossible(const Card& c) { return possible[c]; }

bool Player::handContains(const Card& c) const
{
	for (Card x : hand) if (x == c) return true;
	return false;
}

bool Player::handFull() const
{
	if (hand.size() >= handsize) return true;
	return false;
}

void Player::updateClues()
{
	if (DEBUG) cout << "[DEBUG] Player::updateClues() Called! Player: " << name << "\n";
	for (vector<Clue>::iterator i = clues.begin(); i != clues.end(); ++i)
	{
		if (i->check())
		{
			clues.erase(i);
			i = clues.begin() - 1;
		}
	}
	if (DEBUG) cout << "[DEBUG] Player::updateClues() Ended! Player: " << name << "\n";
}

void Player::setHandSize(const short& size)
{
	handsize = size;
	hand.reserve(size);
}

void Player::addClue(const vector<Card>& variables, const vector<bool>& nots, const bool& addHand, const Card& card)
{
	clues.emplace_back(variables, nots, addHand, card, this);
	if (clues.back().check())
	{
		clues.erase(clues.end() - 1);
		updateClues();
	}
}

bool Player::addHand(const Card& c, const bool& clue)
{
	if (!handContains((Card)c) && !handFull() && possible[c])
	{
		hand.emplace_back((Card)c);
		sort(hand.begin(), hand.end());
		if (DEBUG) cout << "[DEBUG] Added " << c << " to " << name << "'s hand!\n";

		if (answer != this) answer->impossible(c, false);
		for (Player& p : *players) if (&p != this) p.impossible(c, false);

		if (handFull()) for (int i = 0; i < possible.size(); ++i) if (!handContains((Card)i)) possible[i] = false;

		if (!clue) updateClues();

		return true;
	}
	else return false;
}

void Player::setHand()
{
	if (!handFull())
	{
		for (int i = 0; i < 21; ++i) addHand((Card)i, false);
		clues.clear();
	}
}

vector<Card> Player::getHand() const { return hand; }

void Player::impossible(const Card& n, const bool& clue)
{
	if (!handFull() && possible[n])
	{
		if (DEBUG) cout << "[DEBUG] " << n << " is impossible for " << name << "\n";

		possible[n] = false;
		short size = numPossible(possible);
		if (size == handsize) setHand();

		//If card is impossible for all players add it to the answer
		bool answerCard = true;
		for (Player& p : *players) if (p.cardPossible(n))
		{
			answerCard = false;
			break;
		}

		if (answerCard)
		{
			answer->addHand((Card)n, false);
			if (n < DAGGER) for (short i = 0; i < DAGGER; ++i) if (i != n) answer->impossible((Card)n, false);
			else if (n < HALL) for (short i = DAGGER; i < HALL; ++i) if (i != n) answer->impossible((Card)n, false);
			else for (short i = HALL; i < END; ++i) if (i != n) answer->impossible((Card)n, false);
		}

		if(!clue) updateClues();
	}
}

vector<bool> Player::posRooms() const
{
	vector<bool> rooms;
	rooms.reserve(END - HALL);
	for (int i = HALL; i < END; ++i) rooms.emplace_back(possible[i]);
	return rooms;
}

vector<bool> Player::posSuspects() const
{
	vector<bool> susps;
	susps.reserve(6);
	for (int i = MISSSCARLET; i < DAGGER; ++i) susps.emplace_back(possible[i]);
	return susps;
}

vector<bool> Player::posWeapons() const
{
	vector<bool> weps;
	weps.reserve(6);
	for (int i = DAGGER; i < HALL; ++i) weps.emplace_back(possible[i]);
	return weps;
}

vector<Card> Player::handSuspects() const
{
	vector<Card> susps;
	for (Card c : hand) if (MISSSCARLET <= c && c <= PROFESSORPLUM) susps.emplace_back(c);
	return susps;
}

vector<Card> Player::handWeapons() const
{
	vector<Card> weps;
	for (Card c : hand) if (DAGGER <= c && c <= SPANNER) weps.emplace_back(c);
	return weps;
}

vector<Card> Player::handRooms() const
{
	vector<Card> rooms;
	for (Card c : hand) if (HALL <= c && c <= SPANNER) rooms.emplace_back(c);
	return rooms;
}

Player::Player(string name, short handsize)
	: name(name), handsize(handsize) {}

Player::Player(string name)
	: name(name) {}

Player::Player() {}

