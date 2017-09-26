#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <time.h>
#include <iterator>

#define DEBUG false

using namespace std;

struct Player;
struct MoveAction;
struct Node;

static const vector<int> starts = { 115, 0, 1, 98, 103, 109 };
static const vector<int> rooms = { 57, 65, 68, 14, 18, 31, 37, 46, 52, 121 }; //Subtract 12 when referencing the rooms using the enum

Player* answer, *mee, *openn;
vector<Player>* players;
queue<MoveAction>* actionQueue;
vector<Node> board;

short pos, roll, myIndex;

bool moved = false, running = true, suspFound = false, wepFound = false, roomFound = false;

vector<Node*> pathTo(const short& pos);

static const string cardNames[21] = {"Miss Scarlet", "Colonel Mustard", "Mrs White", "Reverend Green", "Mrs Peacock", "Professor Plum", "Daggar", "Candlestick", 
									 "Revolver", "Rope", "Lead Pipe", "Spanner", "Hall", "Lounge", "Dining Room", "Kitchen", "Ball Room", "Conservatory", "Billiard Room",
									 "Library", "Study"}; //Reference with enum

enum Card
{
	MISSSCARLET, COLONELMUSTARD, MRSWHITE, REVERENDGREEN, MRSPEACOCK, PROFESSORPLUM, DAGGER, CANDLESTICK, REVOLVER, ROPE, LEADPIPE, SPANNER,
	HALL, LOUNGE, DININGROOM, KITCHEN, BALLROOM, CONSERVATORY, BILLIARDROOM, LIBRARY, STUDY, END
};

ostream& operator<< (ostream& stream, Card c) { return stream << cardNames[c]; };

int numPossible(const vector<bool>& stuff)
{
	int counter = 0;
	for (bool b : stuff) if (b) counter++;
	return counter;
}

struct Combination
{
	vector<Card> combination;

	short size() const { return combination.size(); }

	short contains(const Card& c) const
	{
		for (int i = 0; i < combination.size(); ++i) if (combination[i] == c) return i;
		return -1;
	}

	void impossible(const Card& c)
	{
		short n = this->contains(c);
		if (n != -1) combination.erase(combination.begin() + n);
	}

	Combination(vector<Card>& combination)
		: combination(combination) {}
};

struct Player
{
private:
	vector<Card> hand;
	vector<bool> possible = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};
	vector<Combination> combos;
	short handsize;
public:
	string name;
	short character = -1;

	bool operator== (const Player& p) const { return name == p.name; }
	
	//Is it still possible for this player to have this card?
	bool cardPossible(const Card& c) { return possible[c]; }
	
	//Does the player's hand contain a certain card?
	bool handContains(const Card& c) const
	{
		for (Card x : hand) if (x == c) return true;
		return false;
	}

	//Is the player's hand full?
	bool handFull() const
	{
		if (hand.size() >= handsize) return true;
		return false;
	}

	//Check if any combos have been fulfilled and update them
	void updateCombos()
	{
		for (vector<Combination>::iterator i = combos.begin(); i != combos.end(); ++i)
		{
			bool shouldCont = false;
			
			if (i->size() == 1)
			{
				addHand(i->combination[0], true);
				i = combos.erase(i) - 1;
				continue;
			}

			for (int j = 0; j < i->combination.size(); ++j)
			{
				if (handContains(i->combination[j]))
				{
					i = combos.erase(i) - 1;
					shouldCont = true;
					break;
				}
				if (!possible[i->combination[j]])
				{
					i->impossible(i->combination[j]);
					j--;
				}
			}
			if (shouldCont) continue;

			if (i->size() < 1)
			{
				i = combos.erase(i) - 1;
				continue;
			}
		}
	}

	//Set the hand size
	void setHandSize(const short& size)
	{
		handsize = size;
		hand.reserve(size);
	}

	//Add a combination to a player
	void addCombo(Combination& c)
	{
		for (int i = 0; i < players->size(); ++i)
		{
			Player* p = &players->at(i);
			if (p == this) continue;
			for (Card& card : p->getHand()) if (c.contains(card)) c.impossible(card);
		}
		
		combos.emplace_back(c);
		updateCombos();
	}

	//Add a card to the player's hand and makes it impossible for any other player to have it in his hand
	bool addHand(const Card& c, const bool& combo)
	{
		if (!handContains((Card) c) && !handFull() && possible[c])
		{
			hand.emplace_back((Card) c);
			if ( DEBUG ) cout << "[DEBUG] Added " << c << " to " << name << "'s hand!\n";
			if(answer != this) answer->impossible(c);
			for (Player& p : *players) if(&p != this) p.impossible(c);
			if (handFull()) for (int i = 0; i < possible.size(); ++i) if (!handContains((Card)i)) possible[i] = false;
			if(!combo) for(Player& p : *players) p.updateCombos();
			return true;
		}
		else return false;
	}
	
	//Set the player's hand
	void setHand()
	{
		if (!handFull())
		{
			for (int i = 0; i < 21; ++i) addHand((Card)i, false);
			combos.clear();
		}
	}

	//Sort and get the player's hand
	vector<Card> getHand() 
	{ 
		sort(hand.begin(), hand.end());
		return hand; 
	}
	
	//Set that it is impossible for the player to have a certain card
	void impossible(const Card& n)
	{
		if (!handFull())
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
			
			if (answerCard) answer->addHand((Card)n, false);

			for(Combination c : combos) c.impossible((Card)n);
			updateCombos();
		}
	}

	//The possible rooms
	vector<bool> posRooms() const
	{
		vector<bool> rooms;
		rooms.reserve(END - HALL);
		for (int i = HALL; i < END; ++i) rooms.emplace_back(possible[i]);
		return rooms;
	}

	//The possible suspects
	vector<bool> posSuspects() const
	{
		vector<bool> susps;
		susps.reserve(6);
		for (int i = MISSSCARLET; i < DAGGER; ++i) susps.emplace_back(possible[i]);
		return susps;
	}

	//The possible weapons
	vector<bool> posWeapons() const
	{
		vector<bool> weps;
		weps.reserve(6);
		for (int i = DAGGER; i < HALL; ++i) weps.emplace_back(possible[i]);
		return weps;
	}

	//THe suspect cards in this  person's hand
	vector<Card> handSuspects()
	{
		sort(hand.begin(), hand.end());

		vector<Card> susps;
		for (Card c : hand) if (MISSSCARLET <= c && c <= PROFESSORPLUM) susps.emplace_back(c);
		return susps;
	}
	
	//The weapon cards in this person's hand
	vector<Card> handWeapons()
	{
		sort(hand.begin(), hand.end());

		vector<Card> weps;
		for (Card c : hand) if (DAGGER <= c && c <= SPANNER) weps.emplace_back(c);
		return weps;
	}

	//The room cards in this player's hand
	vector<Card> handRooms()
	{
		sort(hand.begin(), hand.end());

		vector<Card> rooms;
		for (Card c : hand) if (HALL <= c && c <= SPANNER) rooms.emplace_back(c);
		return rooms;
	}

	Player(string name, short handsize) 
		: name(name), handsize(handsize) {}

	Player(string name) 
		: name(name) {}

	Player() {}
};

struct Node
{
	vector<Node*> neighbours;
	Node* entry;
	short distance = -1, index = -1;
	bool isRoom = false;

	void print() const
	{
		cout << "Neighbours: " << neighbours.size() << "\n";
	}
	
	static void Connect(Node& a, Node& b)
	{
		a.neighbours.emplace_back(&b);
		b.neighbours.emplace_back(&a);
	}
};

int findPos(const int& position)
{
	for (int i = 0; i < rooms.size(); ++i) if (rooms[i] == position) return i + 12;
	return -1;
}

int cardScore(const Card& c) 
{
	if (!answer->cardPossible(c)) return 0;
	int score = 0;
	for (Player p : *players) if (p.cardPossible(c) && !p.handFull()) score++;
	return score;
}

void updateAnswer()
{
	short current;

	//Suspect
	if (!suspFound && numPossible(answer->posSuspects()) == 1)
	{
		for (int i = MISSSCARLET; i < DAGGER; ++i) if (answer->cardPossible((Card)i))
		{
			current = i;
			answer->addHand((Card)current, false);
			suspFound = true;
			cout << "Found Suspect!\n";
		}
	}

	//Weapon
	if (!wepFound && numPossible(answer->posWeapons()) == 1)
	{
		for (int i = DAGGER; i < HALL; ++i) if (answer->cardPossible((Card)i))
		{
			current = i;
			answer->addHand((Card)current, false);
			wepFound = true;
			cout << "Found Weapon!\n";
		}
	}

	//Room
	if (!roomFound && numPossible(answer->posRooms()) == 1)
	{
		for (int i = HALL; i < END; ++i) if (answer->cardPossible((Card)i))
		{
			current = i;
			answer->addHand((Card)current, false);
			roomFound = true;
			cout << "Found Room!\n";
		}
	}
}

struct Action
{
	virtual bool perform() = 0;
};

struct MoveAction : public Action
{
	//Node to move to and path to take
	vector<Node*> path;

	//Move as far on the path as possible
	bool perform() override
	{
		if (DEBUG) cout << "[DEBUG] MoveAction.perform() Called!\n";
		
		if (path.size() == 0)
		{
			cout << "Staying!";
			return true;
		}
		
		Node* temp = nullptr;
		for (int i = 0; i < roll; ++i)
		{
			temp = path.back();
			if (DEBUG) cout << "Moving to square " << temp->index << "\n";
			path.pop_back();
			if (path.size() == 0)
			{
				cout << "Moved to square " << temp->index << "\n";
				pos = temp->index;
				return true;
			}
		}
		cout << "Moved to square " << temp->index << "\n";
		pos = temp->index;
		return false;
	}

	MoveAction() {};

	MoveAction(vector<Node*> path)
		: path(path) {};
};

struct GameEndAction : public MoveAction
{
	MoveAction endMA;

	bool perform() override
	{
		if (!endMA.perform()) return false;
		vector<Card> hand = answer->getHand();

		cout << hand[0] << " did it with a " << hand[1] << " in the " << hand[2] << "\n";

		return true;
	}

	GameEndAction()
		: endMA(pathTo(END)) {}
};

struct QueryAction : public Action
{
	Card room = (Card)21, weapon = (Card)21, suspect = (Card)21;
	
	bool perform() override
	{
		if (suspect == 21) return false;
		else if (weapon == 21) return false;
		else if (room == 21) return false;

		cout << "I think it was " << suspect << " in the " << room << " using a " << weapon << "\n";

		//Check who shows what
		for (int i = 1; i < players->size(); ++i)
		{
			Player& answering = (*players)[(myIndex + i) % players->size()];
			
			char input;
			cout << "Is " << answering.name << " showing a card?(Y/N) ";
			cin >> input;
			if (input == 'Y')
			{
				short scard;
				cout << "What card is he showing? ";
				cin >> scard;
				answering.addHand((Card)scard, false);
				break;
			}
			else
			{
				answering.impossible(suspect);
				answering.impossible(weapon);
				answering.impossible(room);
			}
		}
		return true;
	}
};

vector<Node> CreateBoard()
{
	cout << "Creating Board!\n";

	vector<Node> board(122);

	for (int i = 0; i < board.size(); ++i) board[i].index = i;

	//Normal Board Blocks
	for(int i = 1; i < 14; ++i) Node::Connect(board[i], board[i + 1]);

	Node::Connect(board[10], board[15]);

	for (int i = 15; i < 18; ++i) Node::Connect(board[i], board[i + 1]);

	Node::Connect(board[17], board[19]);
	Node::Connect(board[18], board[28]);

	for (int i = 19; i < 31; ++i) Node::Connect(board[i], board[i + 1]);

	Node::Connect(board[25], board[38]);
	for (int i = 0; i < 3; ++i) Node::Connect(board[25 + i], board[35 - i]);
	Node::Connect(board[29], board[33]);
	Node::Connect(board[30], board[32]);

	for (int i = 32; i < 37; ++i) Node::Connect(board[i], board[i + 1]);
	Node::Connect(board[36], board[38]);
	for (int i = 38; i < 46; ++i) Node::Connect(board[i], board[i + 1]);

	Node::Connect(board[45], board[47]);

	for (int i = 47; i < 52; ++i) Node::Connect(board[i], board[i + 1]);

	Node::Connect(board[47], board[53]);

	for (int i = 53; i < 57; ++i) Node::Connect(board[i], board[i + 1]);

	Node::Connect(board[56], board[58]);

	Node::Connect(board[55], board[121]);
	Node::Connect(board[56], board[121]);
	Node::Connect(board[58], board[121]);

	for (int i = 58; i < 65; ++i) Node::Connect(board[i], board[i + 1]);

	for (int i = 0; i < 3; ++i) Node::Connect(board[i + 60], board[71 - i]);
	Node::Connect(board[62], board[66]);
	Node::Connect(board[64], board[66]);

	for (int i = 66; i < 68; ++i) Node::Connect(board[i], board[i + 1]);
	for (int i = 69; i < 75; ++i) Node::Connect(board[i], board[i + 1]);
	Node::Connect(board[72], board[75]);
	Node::Connect(board[70], board[73]);

	Node::Connect(board[74], board[76]);
	Node::Connect(board[75], board[77]);

	for (int i = 76; i < 91; ++i) Node::Connect(board[i], board[i + 1]);
	for (int i = 76; i < 83; i += 2) Node::Connect(board[i], board[i + 3]);

	Node::Connect(board[16], board[84]);
	Node::Connect(board[17], board[85]);
	for (int i = 19; i < 25; ++i) Node::Connect(board[i], board[i + 67]);
	Node::Connect(board[91], board[38]);

	for (int i = 92; i < 98; ++i) Node::Connect(board[i], board[i + 1]);
	Node::Connect(board[28], board[92]);

	for (int i = 99; i < 103; ++i) Node::Connect(board[i], board[i + 1]);
	Node::Connect(board[32], board[99]);

	for (int i = 104; i < 109; ++i) Node::Connect(board[i], board[i + 1]);
	Node::Connect(board[50], board[104]);

	for (int i = 110; i < 115; ++i) Node::Connect(board[i], board[i + 1]);
	Node::Connect(board[63], board[110]);

	for (int i = 116; i < 120; ++i) Node::Connect(board[i], board[i + 1]);
	Node::Connect(board[66], board[116]);
	Node::Connect(board[0], board[120]);

	//Secret Passageways
	Node::Connect(board[14], board[52]);
	Node::Connect(board[31], board[65]);

	//Set the Rooms
	board[14].isRoom = true;
	board[18].isRoom = true;
	board[31].isRoom = true;
	board[37].isRoom = true;
	board[46].isRoom = true;
	board[52].isRoom = true;
	board[57].isRoom = true;
	board[65].isRoom = true;
	board[68].isRoom = true;
	board[121].isRoom = true;

	cout << "Board Made!\n";
	return board;
}

vector<Node*> pathTo(const short& position)
{
	Node* cur = &board[position];
	vector<Node*> path;
	path.reserve(cur->distance);
	while (cur->distance > 0)
	{
		path.emplace_back(cur);
		cur = cur->entry;
	}
	return path;
}

MoveAction getMove(const short& roll)
{
	updateAnswer();
	
	//BFS
	queue<Node*> q;
	q.push(&board[pos]);
	q.front()->entry = nullptr;
	q.front()->distance = 0;
	while (!q.empty())
	{
		Node* front = q.front();
		if (front->isRoom && front->entry != nullptr)
		{
			for (Node* n : front->neighbours)
			{
				if (n->distance == -1)
				{
					n->entry = front;
					n->distance = n->entry->distance + 12;
					q.push(n);
				}
				else if (n->distance > front->distance + 12)
				{
					n->entry = front;
					n->distance = front->distance + 12;
					q.push(n);
				}
			}
			
			q.pop();
			continue;
		}
		for (Node* n : front->neighbours)
		{
			if (n->distance == -1)
			{
				n->entry = front;
				n->distance = n->entry->distance + 1;
				q.push(n);
			}
			else if (n->distance > front->distance + 1)
			{
				n->entry = front;
				n->distance = front->distance + 1;
				q.push(n);
			}
		}
		q.pop();
	}

	//Decide where to move

	//If answer is known end the game
	if (answer->handFull()) return GameEndAction();

	//Else Find a move

	vector<int> distances;
	int max = -1, maxadr, min = 256, minadr;
	bool roomFound = false;

	if (numPossible(answer->posRooms()) == 1) roomFound = true;
	for (int i = 0; i < 9; ++i)
	{
		if (!roomFound && !answer->posRooms()[i]) continue;
		if (!moved && rooms[i] == pos) continue;

		Node* room = &board[rooms[i]];

		if (room->distance < min)
		{
			min = room->distance;
			minadr = rooms[i];
		}

		if (room->distance <= roll && room->distance > max)
		{	
			max = room->distance;
			maxadr = rooms[i];
		}
	}

	//If a room is in range, move to the one furthest away, else move towards the closest one
	MoveAction ma(max != -1 ? pathTo(maxadr) : pathTo(minadr));

	//Clear for next BFS
	for (Node &n : board) n.distance = -1;

	moved = false;

	return ma;
}

QueryAction getQuery()
{
	QueryAction qa;
	vector<bool> weapons = answer->posWeapons();
	vector<bool> suspects = answer->posSuspects();

	qa.room = (Card)findPos(pos);

	if (numPossible(weapons) > 1)
	{
		vector<int> weaponscore(weapons.size());
		short max = 0;
		for (int i = 0; i < weaponscore.size(); ++i)
		{
			weaponscore[i] = cardScore((Card)(i + 6));
			if (weaponscore[i] > weaponscore[max]) max = i;
		}
		qa.weapon = (Card)(max + 6);
	}
	else if  (openn->handWeapons().size() > 0) qa.weapon = openn->handWeapons()[rand() % openn->handWeapons().size()];
	else
	{
		for (int i = 0; i < players->size(); ++i)
		{
			Player* cur = &players->at((myIndex - i) % players->size());
			if (cur->handWeapons().size() > 0)
			{
				qa.weapon = cur->handWeapons()[rand() % cur->handWeapons().size()];
				break;
			}
		}
	}
	if(qa.weapon == 21) qa.weapon = (Card)(rand() % 6 + 6);

	if (numPossible(suspects) > 1)
	{
		vector<int> suspectscore(suspects.size());
		short max = 0;
		for (int i = 0; i < suspectscore.size(); ++i)
		{
			suspectscore[i] = cardScore((Card)i);
			if (suspectscore[i] > suspectscore[max]) max = i;
		}
		qa.suspect = (Card)max;
	}
	else if (openn->handSuspects().size() > 0) qa.suspect = openn->handSuspects()[rand() % openn->handSuspects().size()];
	else
	{
		for (int i = 0; i < players->size(); ++i)
		{
			Player* cur = &players->at((myIndex - i) % players->size());
			if (cur->handSuspects().size() > 0)
			{
				qa.weapon = cur->handSuspects()[rand() % cur->handSuspects().size()];
				break;
			}
		}
	}
	if (qa.suspect == 21) qa.weapon = (Card)(rand() % 6);

	return qa;
}

void answerQuery(const short& index)
{
	Player* p = &(*players)[index];

	char input;
	cout << "Is " << p->name << " going to query?(Y/N) ";
	cin >> input;
	if (input == 'Y')
	{
		short s1, s2, s3;

		cout << "What is the first card he is asking for?(Suspect)";
		cin >> s1;
		cout << "What is the second card he is asking for?(Weapon)";
		cin >> s2;
		cout << "What is the third card he is asking for?(Room)";
		cin >> s3;

		Card c1 = (Card)s1;
		Card c2 = (Card)s2;
		Card c3 = (Card)s3;

		vector<Card> query = { c1, c2, c3 };
		sort(query.begin(), query.end());

		if (query[0] == mee->character)
		{
			pos = rooms[query[2] - 12];
			moved = true;
			while (actionQueue->size() > 0) actionQueue->pop();
		}

		for (int i = 1; i < players->size(); ++i)
		{
			Player* answering = &((*players)[(index + i) % players->size()]);

			if (answering == mee)
			{
				bool showed = false;
				for (Card c : query) if (answering->handContains(c))
				{
					cout << "Show " << c << "\n";
					showed = true;
					break;
				}
				if (showed) break;
				cout << "No cards to show.\n";
				continue;
			}

			cout << "Did " << answering->name << " show a card?(Y/N)";
			char showed;
			cin >> showed;
			while (showed != 'Y' && showed != 'N')
			{
				cout << "\nPlease enter a Y or a N. ";
				cin >> showed;
			}
			if (showed == 'Y')
			{
				Combination c(query);
				answering->addCombo(c);
				break;
			}
			else
			{
				for (Card c : query) answering->impossible(c);
			}
		}
		if (answer->handFull()) while (actionQueue->size() > 0) actionQueue->pop();
	}
}

bool sortPlayers(const Player& a, const Player& b) { return a.character < b.character; };

int main()
{
	board = CreateBoard();

	Player me("AdriBot");
	mee = &me;

	vector<Player> lplayers;
	players = &lplayers;

	Player solution("Solution", 3);
	answer = &solution;

	queue<MoveAction> lactionQueue;
	actionQueue = &lactionQueue;

	srand(time(nullptr));
	
	cout << "Hello!\n";

	//Input required information

	//Picking char
	cout << "Pick your Characer(Enter the number):\n1. Miss Scarlet\n2. Colonel Mustard\n3. Mrs White\n4. Reverend Green\n5. Mrs Peacock\n6. Professor Plum\n";
	short s;
	cin >> s;
	while (s > 6 || s < 1)
	{
		cout << "Please pick a number that is 6 or lower! ";
		cin >> s;
	}
	switch (s)
	{
	case 1:
		pos = starts[MISSSCARLET];
		cout << "You picked Miss Scarlet.\n";
		break;
	case 2:
		pos = starts[COLONELMUSTARD];
		cout << "You picked Colonel Mustard.\n";
		break;
	case 3:
		pos = starts[MRSWHITE];
		cout << "You picked Mrs White.\n";
		break;
	case 4:
		pos = starts[REVERENDGREEN];
		cout << "You picked Reverend Green.\n";
		break;
	case 5:
		pos = starts[MRSPEACOCK];
		cout << "You picked Mrs Peacock.\n";
		break;
	case 6:
		pos = starts[PROFESSORPLUM];
		cout << "You picked Professor Plum.\n";
		break;
	}

	me.character = s - 1;

	//Amount of players
	cout << "\nHow many other players are playing?\n";
	short pnum;
	cin >> pnum;
	while (pnum > 5 || pnum < 1)
	{
		cout << "A maximum of 6 players is supported and a minimum amount of 2 is required! Please enter a positive amount 5 or lower! ";
		cin >> pnum;
	}

	lplayers.reserve(pnum + 1);

	//Player Names and handsize and showing players
	cout << "Name the players clockwise from you.\n";
	for (int i = 0; i < pnum; i++)
	{
		string name;
		cin >> name;
		Player p (name);
		//p.character = (s + i) % 6;
		lplayers.emplace_back(p);
	}
	lplayers.emplace_back(me);

	//Set the order of play
	cout << "Who is going first? ";
	string name;
	cin >> name;

	for (int i = 0; i < lplayers.size(); ++i) if (name == lplayers[i].name)
	{
		short meIndex = -1;
		for (int j = 0; j < lplayers.size(); ++j)
		{
			Player& p = lplayers[(i + j) % lplayers.size()];
			if (meIndex == -1 && p.name == "AdriBot")
			{
				meIndex = j;
				continue;
			}
			//If the loop is past me make their character values larger than mine
			if (j > meIndex) p.character = lplayers[(i + meIndex) % lplayers.size()].character + j - meIndex;
			else p.character = j;
		}
		break;
	}

	sort(lplayers.begin(), lplayers.end(), sortPlayers);
	for (int i = 0; i < lplayers.size(); ++i) if (lplayers[i] == me)
	{
		myIndex = i;
		mee = &lplayers[i];
		if (DEBUG) cout << "[DEBUG] myIndex set!\n[DEBUG] My character: " << mee->character << "\n";
		break;
	}
	//Handsize and showing players
	short hand = 18 / lplayers.size();
	short opencards = 18 - hand * lplayers.size();
	if (opencards == 1) cout << "\nEach player has " << hand << " cards in their hand and there is 1 public card.\n";
	else cout << "\nEach player has " << hand << " cards in their hand and there are " << opencards << " public cards.\n";
	cout << "\nThe players, in order, are:\n";
	for (Player &i : lplayers)
	{
		i.setHandSize(hand);
		cout << i.name << "\n";
	}
	cout << "\n";
	Player open("OpenCards", opencards);

	//Input Opencard(s)
	if (opencards > 0)
	{
		if (opencards == 1) cout << "Public card found!\n";
		else cout << "Public cards found!\n";
		for (int i = 0; i < opencards; i++)
		{
			cout << "Enter the number of the public card:\n";
			short scard;
			cin >> scard;
			while (!open.addHand((Card)scard, false))
			{
				cout << "This is not a valid card! Enter a different card:\n";
				cin >> scard;
			}
		}
	}

	openn = &open;

	//Input hand
	cout << "You have " << hand << " cards in your hand.\nEnter the numbers of your cards:\n";
	for (int i = 0; i < hand; ++i)
	{
		short scard;
		cin >> scard;
		while (!mee->addHand((Card)scard, false))
		{
			cout << "This is not a valid card! Enter a different card:\n";
			cin >> scard;
		}
	}

	//Start Game Loop

	while (running)
	{
		for (int i = 0; i < lplayers.size(); ++i)
		{
			if (&lplayers[i] == mee)
			{
				if (DEBUG)
				{
					cout << "[DEBUG] Enter the roll: ";
					cin >> roll;
				}
				else roll = 2 + rand() % 6 + rand() % 6;
				cout << "Rolled " << roll << "\n";
				actionQueue->push(getMove(roll));
				if (actionQueue->front().perform()) getQuery().perform();
				actionQueue->pop();
			}
			else answerQuery(i);
		}
	}

	system("PAUSE");
	return 0;
}