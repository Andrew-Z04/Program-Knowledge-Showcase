#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace this_thread;     // sleep_for, sleep_until
using namespace chrono_literals; // ns, us, ms, s, h, etc.
using chrono::system_clock;

const string fileName = "DataBase.txt";
const int firstMoney = 1000;
string systemToClearScreen;

void clearingScreenInfo() {
  cout << "NOTE: While answering questions, please use only numbers!" << endl;
  int systemAnswer = 0;
  while (systemAnswer != 1 && systemAnswer != 2) {
    cout << "What system or program are you using to run this app?" << endl;
    cout << "1. Windows (MSV)" << endl;
    cout << "2. Replit.org" << endl;
    cin >> systemAnswer;
    if (systemAnswer == 1) {
      systemToClearScreen = "cls";
      break;
    }
    if (systemAnswer == 2) {
      systemToClearScreen = "clear";
      break;
    }
    cout << "Invalid input! Please try again." << endl;
    systemAnswer = 0;
  }
}

void clearScreen() {
  if (system(systemToClearScreen.c_str()) != 0) {
    // Handle the error
    cerr << "Failed to clear the screen." << endl;
    exit(0);
    return;
  }
}

vector<pair<char, string>> CardDeck;
vector<pair<char, string>> communityCards;
map<char, int> rankValues = {{'2', 2},  {'3', 3},  {'4', 4},  {'5', 5},
                             {'6', 6},  {'7', 7},  {'8', 8},  {'9', 9},
                             {'T', 10}, {'J', 11}, {'Q', 12}, {'K', 13},
                             {'A', 14}}; // Mapping of card ranks to values

/*
RF - Royal Flush: The highest ace through ten of the same suit.
SF - Straight Flush: Five consecutive cards of the same suit.
FK - Four of a Kind: Four cards of the same rank.
FH - Full House: Three cards of one rank and two cards of another rank.
F - Flush: Five cards of the same suit, not consecutive.
S - Straight: Five consecutive cards, not of the same suit.
TK - Three of a Kind: Three cards of the same rank.
TP - Two Pair: Two cards of one rank, two cards of another rank.
OP - One Pair: Two cards of the same rank.
HC - High Card: The highest card in the hand if none of the above are achieved.
*/

struct HandValue {
  string handType; // "RF", "SF", etc.
  int strength;    // Numerical strength of the hand
};
map<string, HandValue> playerHands; // Maps player names to their hand value
vector<string> combinations;

bool isRoyalFlush(const vector<pair<char, string>> cards, int &valueToAssign) {
  map<string, vector<int>> suitToRanks;

  // Organize cards by suit
  for (auto card : cards) {
    suitToRanks[card.second].push_back(rankValues[card.first]);
  }

  // Check each suit for a Royal Flush
  for (auto pair : suitToRanks) {
    string suit = pair.first;
    vector<int> ranks = pair.second;
    if (ranks.size() < 5)
      continue; // Can't be a flush if fewer than 5 cards of the same suit
    // Check for 10, J, Q, K, A
    vector<int> royalFlushRanks = {10, 11, 12, 13,
                                   14}; // Ranks for a Royal Flush
    bool hasRoyalFlush = true;
    for (int rank : royalFlushRanks) {
      if (find(ranks.begin(), ranks.end(), rank) == ranks.end()) {
        hasRoyalFlush = false;
        break;
      }
    }
    if (hasRoyalFlush) {
      valueToAssign = 1000;
      return true;
    }
  }

  return false;
}
bool isStraightFlush(const vector<pair<char, string>> cards,
                     int &valueToAssign) {
  map<string, vector<int>> suitToRanks;

  // Organize cards by suit
  for (auto card : cards) {
    suitToRanks[card.second].push_back(rankValues[card.first]);
  }

  // Check each suit for a Straight Flush
  for (auto pair : suitToRanks) {
    vector<int> ranks = pair.second;

    // Sort the ranks to check for consecutive values
    sort(ranks.begin(), ranks.end());

    // Check for at least five consecutive cards
    int consecutiveCount = 1; // Start with the first card being considered

    for (size_t i = 1; i < ranks.size(); ++i) {
      if (ranks[i] ==
          ranks[i - 1] +
              1) { // Check if current card is consecutive to the last
        ++consecutiveCount;
        if (consecutiveCount >= 5) {
          valueToAssign = 900 + consecutiveCount;
          return true; // Found a Straight Flush
        }
      } else if (ranks[i] != ranks[i - 1]) {
        // Reset the count if the sequence breaks
        consecutiveCount = 1;
      }
    }
  }

  return false; // No Straight Flush found
}
bool isFourOfKind(const vector<pair<char, string>> cards, int &valueToAssign) {
  map<char, int> rankCount;
  char fourKindRank = 0; // To store the rank that has four of a kind

  // Count each rank in the cards
  for (auto card : cards) {
    rankCount[card.first]++;
  }

  // Check if any rank count is exactly four and identify the rank
  for (auto count : rankCount) {
    if (count.second == 4) {
      fourKindRank = count.first;
      break;
    }
  }

  if (fourKindRank != 0) {
    // Assign value based on the rank of the four cards
    valueToAssign =
        800 +
        rankValues[fourKindRank]; // 800 can be a base value for Four of a Kind
    return true;
  }

  return false;
}
bool isFullHouse(const vector<pair<char, string>> cards, int &valueToAssign) {
  map<char, int> rankCount;

  // Count each rank in the cards
  for (auto card : cards) {
    rankCount[card.first]++;
  }

  char threeKindRank = 0;
  char twoKindRank = 0;

  // Find ranks that have exactly three cards and two cards
  for (auto count : rankCount) {
    if (count.second == 3) {
      if (!threeKindRank || rankValues[count.first] > rankValues[threeKindRank])
        threeKindRank = count.first;
    } else if (count.second == 2) {
      if (!twoKindRank || rankValues[count.first] > rankValues[twoKindRank])
        twoKindRank = count.first;
    }
  }

  // Check if there is both a three of a kind and a pair
  if (threeKindRank && twoKindRank) {
    // The value could be based on the ranks of the cards involved
    valueToAssign =
        700 + (rankValues[threeKindRank] * 3) + (rankValues[twoKindRank] * 2);
    return true;
  }

  return false;
}
bool isFlush(const vector<pair<char, string>> cards, int &valueToAssign) {
  map<string, vector<int>> suitToRanks;

  // Organize cards by suit
  for (auto card : cards) {
    suitToRanks[card.second].push_back(rankValues[card.first]);
  }

  // Check each suit for at least five cards
  for (auto suit : suitToRanks) {
    if (suit.second.size() >= 5) {
      // Sort to find the highest rank card in the flush
      sort(suit.second.begin(), suit.second.end(), greater<int>());
      // Assign value: base of 600 for flush, plus the value of the highest card
      valueToAssign = 600 + suit.second.front();
      return true;
    }
  }

  return false;
}
bool isStraight(const vector<pair<char, string>> cards, int &valueToAssign) {
  vector<int> ranks;

  // Collect all ranks in a single vector, accounting for Aces as both high and
  // low
  for (auto card : cards) {
    ranks.push_back(rankValues[card.first]);
    if (card.first ==
        'A') { // Ace can also be treated as '1' for an A-2-3-4-5 straight
      ranks.push_back(1);
    }
  }

  // Sort and remove duplicates
  sort(ranks.begin(), ranks.end());
  ranks.erase(unique(ranks.begin(), ranks.end()), ranks.end());

  // Find five consecutive cards
  int consecutiveCount = 1;
  for (size_t i = 1; i < ranks.size(); ++i) {
    if (ranks[i] == ranks[i - 1] + 1) {
      ++consecutiveCount;
      if (consecutiveCount == 5) {
        valueToAssign =
            500 + ranks[i]; // 500 base for straight + highest card in straight
        return true;
      }
    } else {
      consecutiveCount = 1;
    }
  }

  return false;
}
bool isThreeOfKind(const vector<pair<char, string>> cards, int &valueToAssign) {
  map<char, int> rankCount;

  // Count the occurrences of each rank
  for (auto card : cards) {
    rankCount[card.first]++;
  }

  // Check for any count of three
  for (auto count : rankCount) {
    if (count.second == 3) {
      valueToAssign = 400 + rankValues[count.first]; // 400 can be a base value
      // for Three of a Kind
      return true;
    }
  }

  return false;
}
bool isTwoPair(const vector<pair<char, string>> cards, int &valueToAssign) {
  map<char, int> rankCount;

  // Count the occurrences of each rank
  for (auto card : cards) {
    rankCount[card.first]++;
  }

  int pairCount = 0;
  char firstPairRank = 0, secondPairRank = 0;

  // Identify ranks that have exactly two cards
  for (auto count : rankCount) {
    if (count.second == 2) {
      if (pairCount == 0) {
        firstPairRank = count.first; // First pair found
      } else if (pairCount == 1) {
        secondPairRank = count.first; // Second pair found
      }
      pairCount++;
    }
  }

  // Check if exactly two different pairs are found
  if (pairCount == 2) {
    // The value could be based on the ranks of the cards in the pairs
    valueToAssign =
        300 + rankValues[firstPairRank] * 2 + rankValues[secondPairRank] * 2;
    return true;
  }

  return false;
}
bool isOnePair(const vector<pair<char, string>> cards, int &valueToAssign) {
  map<char, int> rankCount;

  // Count each rank in the cards
  for (auto card : cards) {
    rankCount[card.first]++;
  }

  char pairRank = 0;

  // Find ranks that have exactly two cards
  for (auto count : rankCount) {
    if (count.second == 2) {
      if (pairRank) {
        return false; // More than one pair found
      }
      pairRank = count.first; // Store the rank of the pair
    }
  }

  if (pairRank) {
    // Assign a value based on the rank of the cards in the pair
    valueToAssign = 200 + rankValues[pairRank] * 2;
    return true;
  }

  return false;
}
bool highCard(const vector<pair<char, string>> cards, int &valueToAssign) {
  char highestRank = 0;

  // Find the highest rank in the cards
  for (auto card : cards) {
    if (!highestRank || rankValues[card.first] > rankValues[highestRank]) {
      highestRank = card.first;
    }
  }

  if (highestRank) {
    // Assign value based on the highest card's rank
    valueToAssign = 100 + rankValues[highestRank];
    return true;
  }

  return false;
}

class Player {
public:
  Player() : Money(0), MoneyPool(0), folded(false) {
    Card1 = {'?', "?"};
    Card2 = {'?', "?"};
  }

  string Name;
  int Money;
  int MoneyPool;
  bool folded;
  pair<char, string> Card1;
  pair<char, string> Card2;

  virtual void setCard(pair<char, string> &card1,
                       pair<char, string> &card2) = 0;
  virtual void setMoneyPool(int moneyP) = 0;
  virtual int get_MoneyPool() const = 0;
  virtual void set_fold(bool info) = 0;
  virtual bool get_fold() const = 0;

  virtual ~Player() {}
};

class User : public Player {
private:
  int moneyPool;
  int playableMoney;

public:
  int Wins, Losses;
  User() : Player(), playableMoney(0), Wins(0), Losses(0) {}

  User(string username, int money, int wins, int losses, int moneyPoolP) {
    Name = username;
    Money = money;
    Wins = wins;
    Losses = losses;
    moneyPool = moneyPoolP;
    playableMoney = 0;
  }

  ~User() {}

  void setCard(pair<char, string> &card1, pair<char, string> &card2) override {
    Card1 = card1;
    Card2 = card2;
  }

  void setPlayableMoney(int moneyP) { playableMoney = moneyP; }
  int getPlayableMoney() { return playableMoney; }

  void setMoneyPool(int moneyP) override { moneyPool = moneyP; }
  int get_MoneyPool() const override { return moneyPool; }

  void set_fold(bool info) override { folded = info; }
  bool get_fold() const override { return folded; }
};

class NPC : public Player {
private:
  double riskTolerant; // If the chances of winning is less than this, the NPC
  // will fold
  double callTolerant; // If the percentage of money that needs to make a call
  // is higher that this the NPC will fold
  double raisePercent; // The percent of the NPC's money that the NPC will raise
public:
  NPC() : Player(), riskTolerant(0.0), callTolerant(0.0), raisePercent(0.0) {}

  NPC(string name, int money, double riskTolerant, double callTolerant,
      double raisePercent, int moneyPool) {
    Name = name;
    Money = money;
    MoneyPool = moneyPool;
    this->riskTolerant = riskTolerant;
    this->callTolerant = callTolerant;
    this->raisePercent = raisePercent;
  }

  ~NPC() {}

  void setCard(pair<char, string> &card1, pair<char, string> &card2) override {
    Card1 = card1;
    Card2 = card2;
  }

  void setMoneyPool(int moneyP) override { MoneyPool = moneyP; }
  int get_MoneyPool() const override { return MoneyPool; }

  void set_fold(bool info) override { folded = info; }
  bool get_fold() const override { return folded; }

  // NPC Methods for Actions
  bool playOrFold() {
    // Define hand strength
    int handStrength = 0;
    // Assign strength values: 2-10 for numerical cards, 10 for T, 11 for J, 12
    // for Q, 13 for K, 14 for A)
    auto getCardStrength = [](char rank) -> int {
      if (rank >= '2' && rank <= '9')
        return rank - '0';
      else if (rank == 'T')
        return 10;
      else if (rank == 'J')
        return 11;
      else if (rank == 'Q')
        return 12;
      else if (rank == 'K')
        return 13;
      else if (rank == 'A')
        return 14;
      return 0;
    };
    // Calculate hand strength
    handStrength += getCardStrength(Card1.first);
    handStrength += getCardStrength(Card2.first);
    // Determine if the NPC should play or fold based on risk tolerance and card
    // strength
    if (handStrength >=
        static_cast<int>(riskTolerant *
                         26)) { // 28 is the maximum combined score (A + A), but
      // I lowered it to 26 to make game more dynamic
      return true; // continue playing
    } else {
      return false; // fold
    }
  }
  bool call(int callAmount) {
    int valueToAssign;
    vector<pair<char, string>> currentHand = {Card1, Card2};
    currentHand.insert(currentHand.end(), communityCards.begin(),
                       communityCards.end());

    // Evaluate hand strength
    if (isRoyalFlush(currentHand, valueToAssign) ||
        isStraightFlush(currentHand, valueToAssign) ||
        isFourOfKind(currentHand, valueToAssign) ||
        isFullHouse(currentHand, valueToAssign) ||
        isFlush(currentHand, valueToAssign) ||
        isStraight(currentHand, valueToAssign) ||
        isThreeOfKind(currentHand, valueToAssign) ||
        isTwoPair(currentHand, valueToAssign) ||
        isOnePair(currentHand, valueToAssign) ||
        highCard(currentHand, valueToAssign)) {

      // Evaluate if calling is worthwhile
      if (valueToAssign >= callTolerant * 800) { // 800 is a treshold
        if (MoneyPool >= callAmount * callTolerant) {
          return true;
        }
      }
    }
    return false;
  }
  bool raise(int callAmount, int &raiseAmount) {
    int valueToAssign;
    vector<pair<char, string>> currentHand = {Card1, Card2};
    currentHand.insert(currentHand.end(), communityCards.begin(),
                       communityCards.end());

    // Evaluate hand strength
    if (isRoyalFlush(currentHand, valueToAssign) ||
        isStraightFlush(currentHand, valueToAssign) ||
        isFourOfKind(currentHand, valueToAssign) ||
        isFullHouse(currentHand, valueToAssign) ||
        isFlush(currentHand, valueToAssign) ||
        isStraight(currentHand, valueToAssign) ||
        isThreeOfKind(currentHand, valueToAssign) ||
        isTwoPair(currentHand, valueToAssign) ||
        isOnePair(currentHand, valueToAssign) ||
        highCard(currentHand, valueToAssign)) {

      // Evaluate if calling is worthwhile
      // Determine raise amount based on hand strength
      double riskFactor = static_cast<double>(valueToAssign) / 1000.0;
      raiseAmount = static_cast<int>(Money * raisePercent * riskFactor *
                                     1.5); // 1.5 is for increase raise amount
      if (raiseAmount > callAmount && Money > raiseAmount) {
        return true;
      }
    }
    return false;
  }
};

void logIn(User &user) {
  ofstream out(fileName);
  ifstream in(fileName);
  if (!in) {
    cerr << "No matching file. Creating a new one..." << endl;
    ofstream out(fileName);
    ifstream in(fileName);
  }
  string username, password, readingName, readingPass;
  int money, win, loss;

  cout << "Enter your username: ";
  cin >> username;
  cout << "Enter your password: ";
  cin >> password;

  bool found = false;
  while (in >> readingName >> readingPass >> money >> win >> loss) {
    if (readingName == username && readingPass == password) {
      user = User(username, money, win, loss, 0);
      found = true;
      break;
    }
  }

  if (!found) {
    cout << "Username or password is incorrect! Please try again" << endl;
    logIn(user);
  } else {
    out.close();
  }
}

void signIn(User& user) {
    ifstream in(fileName);
    if (!in) {
        cerr << "No matching file. Creating a new one..." << endl;
        ofstream out(fileName); // Creating the file if it doesn't exist
    } else {
        in.close(); // Close if it exists to reopen later for writing
    }

    cout << endl;
    cout << "NOTE: Username and password can't be longer than 10 characters and can't be the same." << endl;
    string username, password;
    bool isValid = false;

    while (!isValid) {
        cout << "Create your username: ";
        cin >> username;
        if (username.length() > 10) {
            cout << "Username is too long! Please try again." << endl;
            continue;
        }

        // Check if username already exists
        in.open(fileName);
        bool exists = false;
        string existingUser;
        while (in >> existingUser) {
            if (existingUser == username) {
                cout << "Username already exists! Please try again." << endl;
                exists = true;
                break;
            }
        }
        in.close();

        if (!exists) {
            isValid = true;
        }
    }

    isValid = false;
    while (!isValid) {
        cout << "Create your password: ";
        cin >> password;
        if (password.length() > 10 || password == username) {
            cout << "Password is too long or matches username! Please try again." << endl;
            continue;
        }

        cout << "Repeat password: ";
        string repeatPassword;
        cin >> repeatPassword;
        if (repeatPassword == password) {
            isValid = true;
        } else {
            cout << "Passwords do not match! Please try again." << endl;
        }
    }

    // All checks passed, write to file
    ofstream out(fileName, ios::app);
    if (!out) {
        cerr << "Failed to open database file for writing." << endl;
        return;
    }
    out << username << endl;
    out << password << endl;
    out << firstMoney << endl;
    out << 0 << endl;
    out << 0 << endl;
    out.close();
    cout << "Account created successfully!" << endl;
    cout << endl;

    // Initialize user object
    user = User(username, firstMoney, 0, 0, 0);
}

void start(User &user) {
  cout << "Welcome to the Poker game!" << endl;
  cout << endl;
  cout << "NOTE: While answering questions, please use only numbers!" << endl;
  cout << endl;
  bool pass = false;
  while (pass == false) {
    cout << "Are you existing player?" << endl << endl;
    cout << "1. Yes" << endl;
    cout << "2. No" << endl << endl;
    char answer = 0;
    cin >> answer;
    cout << endl;
    switch (answer) {
    case '1':
      logIn(user);
      pass = true;
      break;
    case '2':
      signIn(user);
      pass = true;
      break;
    default:
      cout << endl << "Invalid answer!" << endl;
      cout << "Please try again!" << endl << endl;
      break;
    }
  }
}

void save(User& user) {
    ifstream in(fileName);
    if (!in) {
        cerr << "Failed to open database file for reading." << endl;
        return;
    }

    vector<string> lines;
    string line;
    bool userFound = false;
    string username = user.Name;
    string password;

    while (getline(in, line)) {
        if (line == username) {
            userFound = true;
            lines.push_back(line); // Add username
            if (getline(in, password)) {
                lines.push_back(password); // Add password
            }
            lines.push_back(to_string(user.Money)); // Add updated money
            lines.push_back(to_string(user.Wins));  // Add updated wins
            lines.push_back(to_string(user.Losses));// Add updated losses
            // Skip the original money, wins, and losses lines
            getline(in, line);
            getline(in, line);
            getline(in, line);
        } else {
            lines.push_back(line);
        }
    }
    in.close();

    if (!userFound) {
        cerr << "User not found in the database file." << endl;
        return;
    }

    ofstream out(fileName);
    if (!out) {
        cerr << "Failed to open database file for writing." << endl;
        return;
    }
    for (const auto& l : lines) {
        out << l << endl;
    }
    out.close();
}

void levels(User &user, int &level) {
  char checked = 0b00;
  char answer;
  int cost = 0;

  while (checked == 0b00) {
    cout << "Your have: " << user.Money << " , Your have won, " << user.Wins
         << " rounds"
         << " , You have lost: " << user.Losses << " rounds." << endl
         << endl;

    if (user.Money < 100) {
      cout << "You don't have enough money to play! You are given 100$" << endl
           << endl;
      user.Money += 100;
    }

    cout << "What level do you want to play?" << endl << endl;
    cout << "Choose your level:" << endl;
    cout << "1. Beginner (1 opponent) | Cost: 100$" << endl;
    cout << "2. Intermediate (2 opponents) | Cost: 300$" << endl;
    cout << "3. Expert (3 opponents) | Cost: 500$" << endl;
    cout << "4. Save and Exit" << endl << endl;
    cin >> answer; // Ensure input is taken within the loop
    cout << endl;
    switch (answer) {
    case '1':
      level = 1;
      cost = 100;
      checked = 0b01;
      break;
    case '2':
      level = 2;
      cost = 300;
      checked = 0b01;
      break;
    case '3':
      level = 3;
      cost = 500;
      checked = 0b01;
      break;
    case '4':
      level = 4;
      checked = 0b01;
      break;
    default:
      cout << endl << "Invalid answer!" << endl;
      cout << "Please try again!" << endl << endl;
      break;
    }
    if (level != 4) {
      if (user.Money < cost) {
        cout << "You don't have enough money!" << endl;
        cout << "Choose another level!" << endl;
        checked = 0b00;
      }
    }
  }

  while (checked == 0b01) {
    cout << "Do you want to choose option " << level << "?" << endl;
    cout << "1. Yes" << endl;
    cout << "2. No" << endl << endl;
    cin >> answer;
    cout << endl;
    switch (answer) {
    case '1':
      checked = 0b11;
      break;
    case '2':
      levels(user, level);
      break;
    default:
      cout << endl << "Invalid answer!" << endl;
      cout << "Please try again!" << endl << endl;
      break;
    }
  }

  if (level == 4) {
    save(user);
    exit(0);
  }

  user.Money -= cost;
  user.setPlayableMoney(cost);
}

void shuffle() {
  // Define all cards
  const char ranks[] = {'2', '3', '4', '5', '6', '7', '8',
                        '9', 'T', 'J', 'Q', 'K', 'A'};
  const string suits[] = {"H", "D", "C",
                          "S"}; // Hearts, Diamonds, Clubs, Spades
  CardDeck.clear();

  // Fill the deck with all cards
  for (string suit : suits) {
    for (char rank : ranks) {
      CardDeck.push_back(std::make_pair(rank, suit));
    }
  }

  // Shuffle the deck
  srand(static_cast<unsigned int>(time(NULL))); // Randomize proccess each time
  for (int i = 0; i < 52; i++) {
    int randNumber = rand() % 52;            // Generate a random position
    swap(CardDeck[i], CardDeck[randNumber]); // Swap the current card with the
                                             // card at the random position
  }
}

void dealCommunityCards(int count) {
  for (int i = 0; i < count; ++i) {
    if (!CardDeck.empty()) {
      communityCards.push_back(CardDeck.back());
      CardDeck.pop_back();
    }
  }
}

void initializeOpponents(NPC *opponents, int level) {
  // Define ranges for riskTolerance, callTolerance, raisePercent based on level
  auto getRandomValue = [](double min, double max) {
    return min + (static_cast<double>(rand()) / RAND_MAX) * (max - min);
  };

  switch (level) {
  case 1: // Beginner
    opponents[0] =
        NPC("NPC1", 100, getRandomValue(0.30, 0.35), getRandomValue(0.20, 0.25),
            getRandomValue(0.20, 0.25), 0);
    break;
  case 2: // Intermediate
    opponents[0] =
        NPC("NPC1", 300, getRandomValue(0.20, 0.25), getRandomValue(0.15, 0.20),
            getRandomValue(0.15, 0.20), 0);
    opponents[1] =
        NPC("NPC2", 300, getRandomValue(0.20, 0.25), getRandomValue(0.15, 0.20),
            getRandomValue(0.15, 0.20), 0);
    break;
  case 3: // Expert
    opponents[0] =
        NPC("NPC1", 500, getRandomValue(0.10, 0.15), getRandomValue(0.10, 0.15),
            getRandomValue(0.10, 0.15), 0);
    opponents[1] =
        NPC("NPC2", 500, getRandomValue(0.10, 0.15), getRandomValue(0.10, 0.15),
            getRandomValue(0.10, 0.15), 0);
    opponents[2] =
        NPC("NPC3", 500, getRandomValue(0.10, 0.15), getRandomValue(0.10, 0.15),
            getRandomValue(0.10, 0.15), 0);
    break;
  default:
    cout << "Error occurred !!! Restart the game !!!" << endl;
    delete[] opponents;
    return; // early return on error
  }
}

void dealInitialCardsAndSetPools(NPC *opponents, int level, User &user,
                                 int &MoneyPool, int &currentCall) {
  // Deal cards to each NPC
  for (int i = 0; i < level; i++) {
    pair<char, string> card1 = CardDeck.back();
    CardDeck.pop_back();
    pair<char, string> card2 = CardDeck.back();
    CardDeck.pop_back();
    opponents[i].setCard(card1, card2);
    opponents[i].setMoneyPool(level * 5);
    opponents[i].Money -= level * 5;
    MoneyPool += level * 5;
  }

  // Deal cards to the user
  pair<char, string> userCard1 = CardDeck.back();
  CardDeck.pop_back();
  pair<char, string> userCard2 = CardDeck.back();
  CardDeck.pop_back();
  user.setCard(userCard1, userCard2);
  user.setMoneyPool(level * 5);
  user.setPlayableMoney(user.getPlayableMoney() - level * 5);
  MoneyPool += 5 * level;
}

void displayInfo(NPC *opponents, int level, User &user) {
  cout << "Players Info: " << endl;
  // Display players info
  for (int i = 0; i < level; i++) {
    if (opponents[i].get_fold()) {
      if (opponents[i].Money > (level * 5)) {
        cout << opponents[i].Name << " - [??] [??] "
             << "| Folded | Bid: " << opponents[i].get_MoneyPool()
             << " Money Left: " << opponents[i].Money << endl;
      } else {
        cout << opponents[i].Name << " - Doesn't have enough money to play!"
             << endl;
      }
    } else {
      cout << opponents[i].Name << " - [??] [??] "
           << "| Bid: " << opponents[i].get_MoneyPool()
           << " Money Left: " << opponents[i].Money << endl;
    }
  }
  if (!user.get_fold()) {
    cout << "You - [" << user.Card1.first << user.Card1.second << "] ["
         << user.Card2.first << user.Card2.second
         << "] | Bid: " << user.get_MoneyPool()
         << " Money Left: " << user.getPlayableMoney() << endl
         << endl;
  } else {
    cout << "You - [" << user.Card1.first << user.Card1.second << "] ["
         << user.Card2.first << user.Card2.second
         << "] | Folded | Bid: " << user.get_MoneyPool()
         << " Money Left: " << user.getPlayableMoney() << endl
         << endl;
  }

  cout << "Table: ";
  for (size_t i = 0; i < 5; ++i) {
    if (i < communityCards.size()) {
      cout << "[" << communityCards[i].first << communityCards[i].second
           << "] ";
    } else {
      cout << "[??] ";
    }
  }
  cout << endl << endl;
}

void processNPCActions(NPC &npc, int &currentCall, int &MoneyPool) {
  if (npc.get_fold())
    return; // Skip if already folded

  int callAmount =
      currentCall - npc.get_MoneyPool(); // Amount needed to make a call
  int raiseAmount = 0;

  // Raise
  if (npc.raise(callAmount, raiseAmount)) {
    npc.Money -= raiseAmount;
    npc.setMoneyPool(npc.get_MoneyPool() + raiseAmount);
    MoneyPool += raiseAmount;
    cout << npc.Name << " raises by " << raiseAmount << " to "
         << npc.get_MoneyPool() << "." << endl;

    // Call
  } else if (npc.call(callAmount)) {
    npc.Money -= callAmount;
    npc.setMoneyPool(npc.get_MoneyPool() + callAmount);
    MoneyPool += callAmount;
    if (callAmount != 0) {
      cout << npc.Name << " calls " << callAmount << "." << endl;
    } else {
      cout << npc.Name << " checks." << endl;
    }
  } else {
    npc.set_fold(true);
    cout << npc.Name << " folds." << endl;
  }
}

int calculateCall(NPC *opponents, User user, int level) {
  int highestBid = -1;
  for (int j = 0; j < level; j++) {
    if (opponents[j].get_MoneyPool() > highestBid) {
      highestBid = opponents[j].get_MoneyPool();
    }
  }
  if (user.get_MoneyPool() > highestBid) {
    highestBid = user.get_MoneyPool();
  }
  return highestBid;
}

void performBettingRound(NPC *opponents, int level, User &user, int &MoneyPool,
                         int &currentCall, bool &NPCsDecided) {
  if (!NPCsDecided) {
    for (int i = 0; i < level; i++) {
      if (!opponents[i].playOrFold()) {
        cout << opponents[i].Name << " folds." << endl;
        opponents[i].set_fold(true);
      }
    }
    NPCsDecided = true; // Ensure this check is only done once
  }

  // Calculate currentCall
  currentCall = calculateCall(opponents, user, level);

  // Process NPC actions
  for (int i = 0; i < level; i++) {
    if (!opponents[i].get_fold()) {
      if (opponents[i].Money > 0) {
        processNPCActions(opponents[i], currentCall, MoneyPool);
        currentCall = calculateCall(
            opponents, user, level); // Update currentCall after each NPC action
      }
    }
  }

  // Check if all NPCs have folded
  bool allFolded = true;
  for (int i = 0; i < level; i++) {
    if (!opponents[i].get_fold()) {
      allFolded = false;
      break;
    }
  }
  if (allFolded) {
    cout << "All opponents have folded. You win the pot!" << endl;
    user.setPlayableMoney(user.getPlayableMoney() + MoneyPool);
    MoneyPool = 0; // Reset the pot
    for (int i = 0; i < level; ++i) {
      opponents[i].setMoneyPool(0);
    }                     // Reset all NPC pools
    user.setMoneyPool(0); // Reset user pool
    return;               // Exit the function early as the round is over
  }

  sleep_for(10s);
  clearScreen();
  displayInfo(opponents, level, user);

  // User's move
  if (!user.get_fold()) {
    if (user.Money > 0) {
      bool madeDecision = false;
      while (!madeDecision) {
        int choice;
        cout << "Make your move:" << endl;
        cout << "1. Fold" << endl;
        cout << "2. Call (or Check if no raise)" << endl;
        cout << "3. Raise" << endl << endl;
        cin >> choice;
        cout << endl;
        switch (choice) {
        case 1:
          user.set_fold(true);
          cout << "You fold." << endl;
          madeDecision = true;
          break;

        case 2: {
          int amountToCall = (currentCall - user.get_MoneyPool());
          if (amountToCall == 0) {
            cout << "You check." << endl;
            madeDecision = true;
          } else if (user.getPlayableMoney() >= amountToCall) {
            user.setMoneyPool(user.get_MoneyPool() + amountToCall);
            user.setPlayableMoney(user.getPlayableMoney() - amountToCall);
            MoneyPool += amountToCall;
            cout << "You call the bet of " << amountToCall << endl;
            madeDecision = true;
          } else {
            cout << "Not enough money to call!" << endl;
          }
          break;
        }
        case 3: {
          cout << "Enter raise amount (greater than current call of "
               << (currentCall - user.get_MoneyPool()) << "): ";
          int raise;
          cin >> raise;
          if (raise > (currentCall - user.get_MoneyPool()) &&
              user.getPlayableMoney() >= raise) {
            user.setMoneyPool(user.get_MoneyPool() + raise);
            user.setPlayableMoney(user.getPlayableMoney() - raise);
            MoneyPool += raise;
            currentCall = user.get_MoneyPool(); // Update currentCall to reflect
            // the new raise
            cout << "You raise to " << currentCall << endl;
            madeDecision = true;
          } else {
            cout << "Invalid raise amount." << endl;
          }
          break;
        }
        default:
          cout << "Invalid option selected." << endl;
          break;
        }
      }
    }
  }
}

void evaluateAllHands(NPC *opponents, User &user, int level) {
  // Cards for checking
  vector<pair<char, string>> userCards = {user.Card1, user.Card2};
  userCards.insert(userCards.end(), communityCards.begin(),
                   communityCards.end());
  int valueToAssign = 0; // Value of the card combination
  // Checking User's combinations
  if (isRoyalFlush(userCards, valueToAssign)) {
    playerHands[user.Name] = {"RF", valueToAssign};
    cout << "User has a Royal Flush." << endl;
  } else if (isStraightFlush(userCards, valueToAssign)) {
    playerHands[user.Name] = {"SF", valueToAssign};
    cout << "User has a Straight Flush." << endl;
  } else if (isFourOfKind(userCards, valueToAssign)) {
    playerHands[user.Name] = {"FK", valueToAssign};
    cout << "User has a Four of a Kind." << endl;
  } else if (isFullHouse(userCards, valueToAssign)) {
    playerHands[user.Name] = {"FH", valueToAssign};
    cout << "User has a Full House." << endl;
  } else if (isFlush(userCards, valueToAssign)) {
    playerHands[user.Name] = {"F", valueToAssign};
    cout << "User has a Flush." << endl;
  } else if (isStraight(userCards, valueToAssign)) {
    playerHands[user.Name] = {"S", valueToAssign};
    cout << "User has a Straight." << endl;
  } else if (isThreeOfKind(userCards, valueToAssign)) {
    playerHands[user.Name] = {"TK", valueToAssign};
    cout << "User has a Three of a Kind." << endl;
  } else if (isTwoPair(userCards, valueToAssign)) {
    playerHands[user.Name] = {"TP", valueToAssign};
    cout << "User has a Two Pair." << endl;
  } else if (isOnePair(userCards, valueToAssign)) {
    playerHands[user.Name] = {"OP", valueToAssign};
    cout << "User has a One Pair." << endl;
  } else if (highCard(userCards, valueToAssign)) {
    playerHands[user.Name] = {"HC", valueToAssign};
    cout << "User has a High Card." << endl;
  }
  // Checking opponents' combinations
  for (int i = 0; i < level; i++) {
    if (!opponents[i].get_fold()) {
      vector<pair<char, string>> npcCards = {opponents[i].Card1,
                                             opponents[i].Card2};
      npcCards.insert(npcCards.end(), communityCards.begin(),
                      communityCards.end());

      if (isRoyalFlush(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"RF", valueToAssign};
        cout << opponents[i].Name << " has a Royal Flush." << endl;
      } else if (isStraightFlush(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"SF", valueToAssign};
        cout << opponents[i].Name << " has a Straight Flush." << endl;
      } else if (isFourOfKind(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"FK", valueToAssign};
        cout << opponents[i].Name << " has a Four of a Kind." << endl;
      } else if (isFullHouse(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"FH", valueToAssign};
        cout << opponents[i].Name << " has a Full House." << endl;
      } else if (isFlush(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"F", valueToAssign};
        cout << opponents[i].Name << " has a Flush." << endl;
      } else if (isStraight(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"S", valueToAssign};
        cout << opponents[i].Name << " has a Straight." << endl;
      } else if (isThreeOfKind(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"TK", valueToAssign};
        cout << opponents[i].Name << " has a Three of a Kind." << endl;
      } else if (isTwoPair(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"TP", valueToAssign};
        cout << opponents[i].Name << " has a Two Pair." << endl;
      } else if (isOnePair(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"OP", valueToAssign};
        cout << opponents[i].Name << " has a One Pair." << endl;
      } else if (highCard(npcCards, valueToAssign)) {
        playerHands[opponents[i].Name] = {"HC", valueToAssign};
        cout << opponents[i].Name << " has a High Card." << endl;
      }
    }
  }
}

void gameFinall(NPC *opponents, int level, User &user, int &MoneyPool,
                int &currentCall) {
  // Show cards
  for (int i = 0; i < level; i++) {
    if (!opponents[i].get_fold()) {
      cout << opponents[i].Name << "'s Cards: [" << opponents[i].Card1.first
           << opponents[i].Card1.second << "] [" << opponents[i].Card2.first
           << opponents[i].Card2.second << "]" << endl;
    }
  }
  if (!user.get_fold()) {
    cout << "Your Cards: [" << user.Card1.first << user.Card1.second << "] ["
         << user.Card2.first << user.Card2.second << "]" << endl;
  }
  // Determine the winner
  evaluateAllHands(opponents, user, level);

  string winner;
  int highestStrength = -1;

  // Check user's hand strength
  if (playerHands.find(user.Name) != playerHands.end() && !user.get_fold()) {
    if (playerHands[user.Name].strength > highestStrength) {
      highestStrength = playerHands[user.Name].strength;
      winner = user.Name;
    }
  }

  // Check opponents' hand strength
  for (int i = 0; i < level; ++i) {
    if (!opponents[i].get_fold() &&
        playerHands.find(opponents[i].Name) != playerHands.end()) {
      if (playerHands[opponents[i].Name].strength > highestStrength) {
        highestStrength = playerHands[opponents[i].Name].strength;
        winner = opponents[i].Name;
      }
    }
  }

  // Announce the winner and allocate the prize
  if (!winner.empty()) {
    cout << winner << " wins the round! " << endl;
    // Allocate the prize
    if (winner == user.Name && !user.get_fold()) {
      user.setPlayableMoney(user.getPlayableMoney() + MoneyPool);
      user.Money += user.getPlayableMoney();
      user.Wins++;
    } else {
      for (int i = 0; i < level; ++i) {
        if (opponents[i].Name == winner) {
          opponents[i].Money += MoneyPool;
          user.Losses++;
          break;
        }
      }
    }
  } else {
    cout << "No winner this round -> ERROR" << endl;
    return;
  }

  // Reset the pot and player money pools
  MoneyPool = 0;
  for (int i = 0; i < level; ++i) {
    opponents[i].setMoneyPool(0);
  }
  user.setMoneyPool(0);

  // Reset cards (take cards from players and table, back to deck)
  // Return community cards to the deck
  while (!communityCards.empty()) {
    CardDeck.push_back(communityCards.back());
    communityCards.pop_back();
  }

  // Return user's cards to the deck
  if (user.Card1.first != '?' && user.Card1.second != "?") {
    CardDeck.push_back(user.Card1);
    user.Card1 = {'?', "?"};
  }
  if (user.Card2.first != '?' && user.Card2.second != "?") {
    CardDeck.push_back(user.Card2);
    user.Card2 = {'?', "?"};
  }

  // Return each opponent's cards to the deck
  for (int i = 0; i < level; ++i) {
    if (opponents[i].Card1.first != '?' && opponents[i].Card1.second != "?") {
      CardDeck.push_back(opponents[i].Card1);
      opponents[i].Card1 = {'?', "?"};
    }
    if (opponents[i].Card2.first != '?' && opponents[i].Card2.second != "?") {
      CardDeck.push_back(opponents[i].Card2);
      opponents[i].Card2 = {'?', "?"};
    }
  }

  // Unfold everybody
  for (int i = 0; i < level; ++i) {
    if (opponents[i].Money == 0) {
      opponents[i].set_fold(true); // Ensure they are marked as folded
    } else {
      opponents[i].set_fold(false);
    }
  }
  user.set_fold(false);
}

void game(int level, User &user) {
  int currentCall = 0;
  bool NPCsDecided = false;
  int MoneyPool = 0;
  NPC *opponents = new NPC[level];

  // Initialize opponents based on the level
  initializeOpponents(opponents, level);

  // Checking to continue round until money pools are equal
  bool equalMoney = true;
  int moneyToCheck = 0;

  // Checking if opponents have enough money to play
  bool NPCcanPlay = true;

  while (user.getPlayableMoney() > (level * 5) && NPCcanPlay == true) {

    // Checking if opponents have enough money to play
    NPCcanPlay = false;
    for (int i = 0; i < level; i++) {
      if (opponents[i].Money > (level * 5)) {
        NPCcanPlay = true;
      } else {
        opponents[i].set_fold(true);
        opponents[i].Money = 0;
      }
    }

    if (!NPCcanPlay) {
      cout << "No NPCs can play. Ending game." << endl;
      break; // Exit if no NPCs can play
    }

    // Shuffle the deck
    shuffle();

    // Deal initial cards and set initial money pool contributions
    dealInitialCardsAndSetPools(opponents, level, user, MoneyPool, currentCall);

    // First Round
    sleep_for(2s);
    clearScreen();
    displayInfo(opponents, level, user);
    performBettingRound(opponents, level, user, MoneyPool, currentCall,
                        NPCsDecided);
    while (!equalMoney) {
      moneyToCheck = user.get_MoneyPool();
      equalMoney = true;
      for (int i = 0; i < level; i++) {
        if (!opponents[i].get_fold() && opponents[i].Money != 0) {
          if (moneyToCheck != opponents[i].get_MoneyPool()) {
            equalMoney = false;
          }
        }
      }
      if (!equalMoney) {
        performBettingRound(opponents, level, user, MoneyPool, currentCall,
                            NPCsDecided);
      }
    }
    dealCommunityCards(3); // Flop

    // Second Round
    sleep_for(2s);
    clearScreen();
    displayInfo(opponents, level, user);
    performBettingRound(opponents, level, user, MoneyPool, currentCall,
                        NPCsDecided);
    while (!equalMoney) {
      moneyToCheck = user.get_MoneyPool();
      equalMoney = true;
      for (int i = 0; i < level; i++) {
        if (!opponents[i].get_fold() && opponents[i].Money != 0) {
          if (moneyToCheck != opponents[i].get_MoneyPool()) {
            equalMoney = false;
          }
        }
      }
      if (!equalMoney) {
        performBettingRound(opponents, level, user, MoneyPool, currentCall,
                            NPCsDecided);
      }
    }
    dealCommunityCards(1); // Turn

    // Third Round
    sleep_for(2s);
    clearScreen();
    displayInfo(opponents, level, user);
    performBettingRound(opponents, level, user, MoneyPool, currentCall,
                        NPCsDecided);
    while (!equalMoney) {
      moneyToCheck = user.get_MoneyPool();
      equalMoney = true;
      for (int i = 0; i < level; i++) {
        if (!opponents[i].get_fold() && opponents[i].Money != 0) {
          if (moneyToCheck != opponents[i].get_MoneyPool()) {
            equalMoney = false;
          }
        }
      }
      if (!equalMoney) {
        performBettingRound(opponents, level, user, MoneyPool, currentCall,
                            NPCsDecided);
      }
    }
    dealCommunityCards(1); // River

    // Finall Round
    sleep_for(2s);
    clearScreen();
    displayInfo(opponents, level, user);
    performBettingRound(opponents, level, user, MoneyPool, currentCall,
                        NPCsDecided);
    while (!equalMoney) {
      moneyToCheck = user.get_MoneyPool();
      equalMoney = true;
      for (int i = 0; i < level; i++) {
        if (!opponents[i].get_fold() && opponents[i].Money != 0) {
          if (moneyToCheck != opponents[i].get_MoneyPool()) {
            equalMoney = false;
          }
        }
      }
      if (!equalMoney) {
        performBettingRound(opponents, level, user, MoneyPool, currentCall,
                            NPCsDecided);
      }
    }

    // Game Finall
    sleep_for(2s);
    clearScreen();
    displayInfo(opponents, level, user);
    gameFinall(opponents, level, user, MoneyPool, currentCall);
    sleep_for(10s);
    clearScreen();
  }
  delete[] opponents;
}

int main() {
  int level = 0;
  clearingScreenInfo();
  clearScreen();
  User user;
  start(user);
  cout << endl << "Starting a Game..." << endl;
  while (level != 4) {
    sleep_for(2s);
    clearScreen();
    levels(user, level);
    game(level, user);
    sleep_for(2s);
    clearScreen();
  }
}