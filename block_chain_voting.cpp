/*

The blockchain used in the code below stores each vote
as a block in the blockchain, containing a hash of the previous block.
Once a vote (block) is added, it cannot be altered without altering all the
subsequent blocks, making it difficult to tamper with individual votes.

*/

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
    Voter() : voterID(""), name("") {}

    // Parameterized constructor for registering new voters
    Voter(const string& id, const string& name) : voterID(id), name(name) {}
};

// VoterRegistry class to manage voter registration and verification
class VoterRegistry {
private:
    unordered_map<string, Voter> voterMap; // Stores voter ID and Voter object
    unordered_map<string, bool> hasVoted;  // Tracks if a voter has already voted

public:
    // Load voter registry from a specified CSV file
    void loadVoterRegistry(const string& filePath) {
        ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "Error: Could not open voter registry file: " << filePath << endl;
            exit(1);  // Exit the program if the file cannot be opened
        }

        string line, header;
        getline(file, header); // Skip the header line
        while (getline(file, line)) {
            stringstream ss(line);
            string voterID, firstName, lastName;
            getline(ss, voterID, ',');
            getline(ss, firstName, ',');
            getline(ss, lastName, ',');

            string fullName = firstName + " " + lastName;
            voterMap[voterID] = Voter(voterID, fullName);
            hasVoted[voterID] = false; // Initialize as not voted
        }
        file.close();
        cout << "Voter registry loaded successfully from " << filePath << endl;
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

// Block class represents each node in the blockchain
class Block {
public:
    string data;
    string prevHash;
    string hash;
    Block* next;

    // Constructor to initialize each block with data and hash of the previous block
    Block(const string& data, const string& prevHash) : data(data), prevHash(prevHash), next(nullptr) {
        calculateHash();
    }

    // Calculate the hash for the block to ensure immutability
    void calculateHash() {
        stringstream concat;
        concat << prevHash << data;
        hash = hash256_hex_string(concat.str());
    }
};

class Blockchain {
private:
    Block* head; // Head of the blockchain

public:
    Blockchain() : head(nullptr) {}

    // Adds the genesis block to the blockchain
    void addGenesisBlock() {
        if (head == nullptr) {
            head = new Block("0", "0");  // Genesis block with arbitrary data
            saveToFile(head->hash);
        }
    }

    // Adds a new block (vote) to the blockchain
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

    // Retrieves the hash of the last block
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

    // Verifies blockchain integrity by comparing the last block's hash 
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

    // Prints the blockchain to demonstrate reading all blocks 
    void print() const {
        Block* current = head;
        while (current != nullptr) {
            cout << current->data << "->";
            current = current->next;
        }
        cout << "END" << endl;
    }

    // Tallies votes and displays the winner
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

    // Save the hash of the latest block to a file for verification 
    void saveToFile(const string& hash) const {
        ofstream hashFile("lasthash.txt");
        if (hashFile.is_open()) {
            hashFile << hash;
            hashFile.close();
        } else {
            cout << "Unable to save hash to file." << endl;
        }
    }

    // Destructor to clean up dynamically allocated blocks
    ~Blockchain() {
        Block* current = head;
        while (current != nullptr) {
            Block* temp = current;
            current = current->next;
            delete temp;
        }
    }
};

// Main function
int main() {
    Blockchain blockchain;
    VoterRegistry voterRegistry;

    // Load the voter registry from the CSV file
    voterRegistry.loadVoterRegistry("voter_registry.csv");

    blockchain.addGenesisBlock();

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

    // Display the order of votes for demonstration 
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
