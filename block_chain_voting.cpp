#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "picosha2.h"

using namespace std;
using namespace picosha2;

// Voter class to handle registration and verification
class Voter {
public:
    string voterID;
    string name;

    // Default constructor
    // We need to make a constructor for voter class because unordered map library expects constructor
    Voter() : voterID(""), name("") {}

    // Parameterized constructor
    Voter(const string& id, const string& name) : voterID(id), name(name) {}
};

// VoterRegistry class to manage voter registration and verification
class VoterRegistry {
private:
    unordered_map<string, Voter> voterMap; // Stores voter ID and Voter object
    unordered_map<string, bool> hasVoted;  // Tracks if a voter has already voted

public:
    // Register a new voter
    bool registerVoter(const string& id, const string& name) {
        if (voterMap.find(id) != voterMap.end()) {
            cout << "Voter ID already registered." << endl;
            return false;
        }
        voterMap[id] = Voter(id, name);
        hasVoted[id] = false;
        cout << "Voter registered: " << name << " (ID: " << id << ")" << endl;
        return true;
    }

    // Verify if a voter is registered and hasn't voted yet
    bool verifyVoter(const string& id) {
        if (voterMap.find(id) == voterMap.end()) {
            cout << "Voter ID not found." << endl;
            return false;
        }
        if (hasVoted[id]) {
            cout << "Voter has already voted." << endl;
            return false;
        }
        return true;
    }

    // Mark voter as having voted
    void markAsVoted(const string& id) {
        if (voterMap.find(id) != voterMap.end()) {
            hasVoted[id] = true;
        }
    }
};

// This is the node class in our node based data-structure
class Block {
public:
    string data;
    string prevHash;
    string hash;
    Block* next;

    Block(const string& data, const string& prevHash) : data(data), prevHash(prevHash), next(nullptr) {
        calculateHash();
    }

    void calculateHash() {
        stringstream concat;
        concat << prevHash << data;
        hash = hash256_hex_string(concat.str());
    }
};

class Blockchain {
private:
    Block* head;

public:
    Blockchain() : head(nullptr) {}

    void addGenesisBlock() {
        if (head == nullptr) {
            head = new Block("0", "0");
            saveToFile(head->hash);
        }
    }

    void addBlock(const string& data) {
        if (head != nullptr) {
            Block* last = head;
            while (last->next != nullptr) {
                last = last->next;
            }
            Block* newBlock = new Block(data, last->hash);
            last->next = newBlock;
            saveToFile(newBlock->hash);
        }
    }

    string getLastHash() const {
        if (head == nullptr) {
            return "f1534392279bddbf9d43dde8701cb5be14b82f76ec6607bf8d6ad557f60f304e";
        } else {
            Block* last = head;
            while (last->next != nullptr) {
                last = last->next;
            }
            return last->hash;
        }
    }

    bool verify() const {
        string lastHash = getLastHash();
        string fileHash;
        ifstream hashFile("lasthash.txt");
        if (hashFile.is_open()) {
            getline(hashFile, fileHash);
            hashFile.close();
            return (fileHash == lastHash);
        }
        return false;
    }

    void print() const {
        Block* current = head;
        while (current != nullptr) {
            cout << current->data << "->";
            current = current->next;
        }
        cout << "END" << endl;
    }

    void checkWinner() const {
        int count1 = 0, count2 = 0, count3 = 0;
        Block* current = head;
        while (current != nullptr) {
            if (current->data == "1") {
                count1++;
            } else if (current->data == "2") {
                count2++;
            } else if (current->data == "3") {
                count3++;
            }
            current = current->next;
        }

        vector<pair<int, string>> candidates = {
            {count1, "RAHUL SINGH"},
            {count2, "KOMAL GUPTA"},
            {count3, "ABHISHEK TOMAR"}
        };

        sort(candidates.rbegin(), candidates.rend());

        if (candidates.empty() || candidates[0].first == 0) {
            cout << "No one is the winner" << endl;
        } else {
            cout << "Winner is: " << candidates[0].second << endl;
        }
    }

    void saveToFile(const string& hash) const {
        ofstream hashFile("lasthash.txt");
        if (hashFile.is_open()) {
            hashFile << hash;
            hashFile.close();
        } else {
            cout << "Unable to save hash to file." << endl;
        }
    }

    ~Blockchain() {
        Block* current = head;
        while (current != nullptr) {
            Block* temp = current;
            current = current->next;
            delete temp;
        }
    }
};

int main() {
    Blockchain blockchain;
    VoterRegistry voterRegistry;
    blockchain.addGenesisBlock();

    // Register voters
    voterRegistry.registerVoter("1001", "Alice");
    voterRegistry.registerVoter("1002", "Bob");
    voterRegistry.registerVoter("1003", "Charlie");

    int exit = 5;
    while (exit != 0) {
        if (!blockchain.verify()) {
            cout << "Blockchain is compromised" << endl;
            return 0;
        }

        string voterID;
        cout << "\nEnter your Voter ID to cast a vote: ";
        cin >> voterID;
        cin.ignore();

        // Verify voter before allowing them to vote
        if (!voterRegistry.verifyVoter(voterID)) {
            continue;  // Skip voting if verification fails
        }

        cout << "\nChoose the candidate to vote:\n";
        cout << "  1. RAHUL SINGH\n";
        cout << "  2. KOMAL GUPTA\n";
        cout << "  3. ABHISHEK TOMAR\n";
        cout << "  4. Any other number to choose NOTA\n";
        cout << "-> ";
        
        string input;
        getline(cin, input);
        blockchain.addBlock(input);

        // Mark the voter as having voted
        voterRegistry.markAsVoted(voterID);

        cout << "\nTO CONTINUE PRESS ANY NUMBER\n\nTO EXIT PRESS '0'\n";
        cin >> exit;
        cin.ignore();
    }

    cout << "THE ORDER OF THE VOTES IS: ";
    blockchain.print();

    int temp;
    cout << "\nPRESS 1 TO CHECK THE WINNER OR ANY NUMBER TO EXIT: ";
    cin >> temp;
    if (temp == 1) {
        blockchain.checkWinner();
    }

    return 0;
}
