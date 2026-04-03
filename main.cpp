#include <iostream>
#include <string>
#include <fstream>

using namespace std;

// 1. Simple struct to group a word and its frequency
struct WordFreq {
    string word = "";
    long long freq = 0;
};

// 2. Shortened Node struct ({} automatically sets pointers to null)
struct Node {
    Node* child[26] = {}; 
    WordFreq top3[3]; 
};

// 3. Insert and update Top 3 instantly
void insert(Node* root, string word, long long freq) {
    Node* curr = root;
    for (char c : word) {
        int i = tolower(c) - 'a';
        if (i < 0 || i > 25) continue; 
        
        if (!curr->child[i]) curr->child[i] = new Node();
        curr = curr->child[i];
        
        // Simple manual swap to keep top3 sorted
        WordFreq temp = {word, freq};
        for (int j = 0; j < 3; j++) {
            if (temp.freq > curr->top3[j].freq) {
                WordFreq displaced = curr->top3[j];
                curr->top3[j] = temp;
                temp = displaced;
            }
        }
    }
}

// 4. Quick search that returns the node
Node* search(Node* root, string prefix) {
    Node* curr = root;
    for (char c : prefix) {
        int i = tolower(c) - 'a';
        if (i < 0 || i > 25 || !curr->child[i]) return nullptr; 
        curr = curr->child[i];
    }
    return curr;
}

// 5. Shortened TXT loader for word-only files
void loadTxt(Node* root, string filename) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        cout << "Error: Could not open " << filename << endl;
        return;
    }

    string word;
    long long simulatedFreq = 20000; // Start with a high fake frequency
    
    // Only extract the word, since there are no numbers in the file!
    while (file >> word) {
        insert(root, word, simulatedFreq);
        simulatedFreq--; // Decrease it so earlier words stay the most "frequent"
    }
}

// 6. Minimal Main Loop
int main() {
    Node* root = new Node();
    
    // Loading your specific text file
    loadTxt(root, "20k.txt");
    
    
    string input;
    
    while (true) {
        cout << "Enter the word (or 'exit'): ";
        cin >> input;
        if (input == "exit") break;
        
        Node* res = search(root, input);
        
        if (!res || res->top3[0].freq == 0) {
            cout << "-> No suggestions.\n\n";
            continue;
        }
        
        // Only print the words, not the fake frequencies
        for (int i = 0; i < 3 && res->top3[i].freq > 0; i++) {
            cout << "-> " << res->top3[i].word << "\n";
        }
        cout << endl;
    }
    
    return 0;
}