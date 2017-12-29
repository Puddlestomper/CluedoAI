#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <time.h>
#include <iterator>

#include "Cluedo.h"
#include "Clue.h"
#include "Player.h"

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

bool moved = false, running = true;

vector<Node*> pathTo(const short& pos);

static const string cardNames[21] = {"Miss Scarlet", "Colonel Mustard", "Mrs White", "Reverend Green", "Mrs Peacock", "Professor Plum", "Daggar", "Candlestick", 
									 "Revolver", "Rope", "Lead Pipe", "Spanner", "Hall", "Lounge", "Dining Room", "Kitchen", "Ball Room", "Conservatory", "Billiard Room",
									 "Library", "Study"}; //Reference with enum

//This operator allows the printing of the cards with their names instead of their enum value
ostream& operator<< (ostream& stream, Card c) { return stream << cardNames[c]; };

int numPossible(const vector<bool>& stuff)
{
	int counter = 0;
	for (bool b : stuff) if (b) counter++;
	return counter;
}

//The Node struct is used to represent the squares on the board as nodes in a graph
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

int findRoom(const int& position)
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

//The MoveAction struct handles moving the player
struct MoveAction : public Action
{
	//Node to move to and path to take
	vector<Node*> path;

	//Move as far on the path as possible
	bool perform() override
	{
		if (DEBUG) cout << "\n[DEBUG] MoveAction.perform() Called!\n";
		
		if (path.size() == 0)
		{
			cout << "Staying!";
			return true;
		}
		
		Node* temp = nullptr;
		for (int i = 0; i < roll; ++i)
		{
			temp = path.back();
			if (DEBUG) cout << "[DEBUG] Moving to square " << temp->index << "\n";
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
		return true;
	}

	MoveAction() {};

	MoveAction(vector<Node*> path)
		: path(path) {};
};

//The GameEndAction struct handles moving to the final square and making the accusation
struct GameEndAction : public MoveAction
{
	MoveAction endMA;

	bool perform() override
	{
		if (DEBUG) cout << "\n[DEBUG] GameEndAction.perform() Called!\n";

		if (!endMA.perform()) return false;
		vector<Card> hand = answer->getHand();

		cout << hand[0] << " did it with a " << hand[1] << " in the " << hand[2] << "\n";

		return true;
	}

	GameEndAction()
		: endMA(pathTo(END)) 
	{
		if (DEBUG) cout << "[DEBUG] GameEndAction created!\n";
	}
};

//The QueryAction struct handles making queries
struct QueryAction : public Action
{
	Card room = (Card)21, weapon = (Card)21, suspect = (Card)21;
	
	bool perform() override
	{
		if (DEBUG) cout << "\n[DEBUG] QueryAction.perform() Called!\n";
		
		if (room == -1) return true;
		if (suspect == 21 || weapon == 21 || room == 21) return false;

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
				answering.impossible(suspect, false);
				answering.impossible(weapon, false);
				answering.impossible(room, false);
			}
		}
		return true;
	}
};

//The CreateBoard method creates the board vector
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
	if (DEBUG) cout << "\n[DEBUG] getMove(const short& roll) Called!\n";
	
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
	if (DEBUG) cout << "\n[DEBUG] getQuery() Called!\n";
	
	QueryAction qa;
	vector<bool> weapons = answer->posWeapons();
	vector<bool> suspects = answer->posSuspects();

	qa.room = (Card)findRoom(pos);

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
	if (qa.suspect == 21) qa.suspect = (Card)(rand() % 6);

	return qa;
}

bool answerQuery(const short& index)
{
	if (DEBUG) cout << "\n[DEBUG] answerQuery(const short& index) Called!\n";
	
	Player* p = &(*players)[index];

	char input;
	cout << "\nIs " << p->name << " going to query?(Y/N) ";
	cin >> input;
	if (input == 'Y')
	{
		short s1, s2, s3;

		cout << "What is the first card he is asking for?(Suspect) ";
		cin >> s1;
		cout << "What is the second card he is asking for?(Weapon) ";
		cin >> s2;
		cout << "What is the third card he is asking for?(Room) ";
		cin >> s3;

		Card c1 = (Card)s1;
		Card c2 = (Card)s2;
		Card c3 = (Card)s3;

		vector<Card> query = { c1, c2, c3 };
		sort(query.begin(), query.end());

		if (query[0] == mee->character)
		{
			short prevpos = pos;
			pos = rooms[query[2] - 12];
			if(pos != prevpos) moved = true;
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
			cin >> input;
			while (input != 'Y' && input != 'N')
			{
				cout << "\nPlease enter a Y or a N. ";
				cin >> input;
			}

			if (input == 'Y')
			{
				answering->addClue({c1, c2}, vector<bool>{true, true}, true, c3);
				answering->addClue({c1, c3}, vector<bool>{true, true}, true, c2);
				answering->addClue({c2, c3}, vector<bool>{true, true}, true, c1);
				break;
			}
			else
			{
				for (Card c : query) answering->impossible(c, false);
			}
		}
		if (answer->handFull()) while (actionQueue->size() > 0) actionQueue->pop();
	}
	else
	{

		cout << "Is " << p->name << " going to make an accusation?(Y/N) ";
		cin >> input;
		while (input != 'Y' && input != 'N')
		{
			cout << "\nPlease enter a Y or a N. ";
			cin >> input;
		}
		
		if (input == 'Y')
		{
			short s1, s2, s3;

			cout << "Who is he accusing?(Suspect) ";
			cin >> s1;
			cout << "How does he say it was done?(Weapon) ";
			cin >> s2;
			cout << "Where does he claim it happened?(Room) ";
			cin >> s3;

			cout << "Is he right?(Y/N) ";
			cin >> input;
			while (input != 'Y' && input != 'N')
			{
				cout << "\nPlease enter a Y or a N. ";
				cin >> input;
			}

			if (input == 'Y')
			{
				running = false;
				cout << "Congratz to " << p->name << " for winning!\n";
				return false;
			}

			Card c1 = (Card)s1;
			Card c2 = (Card)s2;
			Card c3 = (Card)s3;

			vector<Card> accusation = { c1, c2, c3 };
			sort(accusation.begin(), accusation.end());

			answer->addClue({c1, c2}, vector<bool>{false, false}, false, c3);
			answer->addClue({c1, c3}, vector<bool>{false, false}, false, c2);
			answer->addClue({c2, c3}, vector<bool>{false, false}, false, c1);

			cout << "Better luck next time!\n";
		}
	}
	return true;
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
			if (p.name == "AdriBot")
			{
				meIndex = j;
				continue;
			}
			//If the loop is past me make their character values larger than mine
			if (meIndex != -1) p.character = lplayers[(i + meIndex) % lplayers.size()].character + j - meIndex;
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
		cout << i.name;
		if (DEBUG) cout << ": " << i.character;
		cout << "\n";
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
				if (actionQueue->front().perform())
				{
					if (!getQuery().perform()) cout << "[ERROR] Performing query failed!\n";
				}
				else cout << "[ERROR] Performing move failed!\n";
				actionQueue->pop();
			}
			else if (!answerQuery(i)) break;
		}
	}

	system("PAUSE");
	return 0;
}