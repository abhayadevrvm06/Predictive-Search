#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

using namespace std;

// just a simple struct to group the word and its score together so we can rank them
struct WordFreq {
    string word = "";
    long long freq = 0;
};

// making the trie node. instead of doing a full search later i am just caching the 
// top 3 words right here in the node. saves so much time during the actual search
struct Node {
    Node* child[26] = {}; 
    WordFreq top3[3]; 
};

// this function walks down the tree and creates nodes if they dont exist
// the cool part is it updates the top 3 array at every single step of the way
void insert(Node* root, string word, long long freq) {
    Node* curr = root;
    for (char c : word) {
        // convert the char to an index from 0 to 25
        int i = tolower(c) - 'a';
        
        // just ignore anything that isnt a normal alphabet letter
        if (i < 0 || i > 25) continue; 
        
        if (curr->child[i] == nullptr) {
            curr->child[i] = new Node();
        }
        curr = curr->child[i];
        
        // this is basically a manual mini insertion sort
        // we check the new word against the top 3 array and if its score is higher
        // we swap them and push the smaller word down the list
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

// standard trie search. just loops through the prefix letters
// if we hit a dead end we return null otherwise return the node we stopped at
Node* search(Node* root, string prefix) {
    Node* curr = root;
    for (char c : prefix) {
        int i = tolower(c) - 'a';
        if (i < 0 || i > 25 || curr->child[i] == nullptr) {
            return nullptr; 
        }
        curr = curr->child[i];
    }
    return curr;
}

// reads the words from our text file
// since the file is already sorted i just simulate the frequencies starting from 20000 
// and going down so the first words read stay at the top of the rankings
void loadTxt(Node* root, string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "error: could not open " << filename << endl;
        return;
    }

    string word;
    long long simulatedFreq = 20000; 
    while (file >> word) {
        insert(root, word, simulatedFreq);
        simulatedFreq--; 
    }
    file.close();
}

// i figured out this hack to get real time typing without pressing enter
// it uses system() to quickly turn off canonical mode and echo in the mac terminal
// grabs exactly one character and then turns them back on immediately
char getKeystroke() {
    char c;
    system("stty -icanon -echo"); 
    c = getchar();                
    system("stty icanon echo");   
    return c;
}

int main() {
    Node* root = new Node();
    
    cout << "loading dataset..." << endl;
    loadTxt(root, "20k.txt");
    
    string prefix = "";
    
    // infinite loop for the real time search bar
    while (true) {
        // clear the mac terminal screen every time so it looks like a clean ui
        system("clear");
        
        cout << "---------------------------------" << endl;
        cout << "       OptiType Engine           " << endl;
        cout << "---------------------------------" << endl;
        cout << "(type to search | 1/2/3 to autocomplete | '-' backspace | '?' quit)\n" << endl;
        
        cout << "Search: " << prefix << "\n\n";
        
        // move result up here so we can read the words later when the user presses 1, 2, or 3
        Node* result = nullptr;
        
        // we only want to bother searching if the user actually typed something
        if (prefix.length() > 0) {
            result = search(root, prefix);
            
            // if we got null or the first top3 slot is empty then we have nothing
            if (result == nullptr || result->top3[0].freq == 0) {
                cout << "Suggestions:\n  no suggestions found" << endl;
            } else {
                cout << "Suggestions:" << endl;
                // loop through our cached array and print the words
                for (int i = 0; i < 3; i++) {
                    if (result->top3[i].freq > 0) {
                        cout << "  " << i + 1 << ". " << result->top3[i].word << endl;
                    }
                }
            }
        }
        
        // wait for the user to hit a key
        char c = getKeystroke();
        
        if (c == '?') {
            break; 
        } else if (c == '-' || c == 127) { 
            // 127 is the ascii code for the mac backspace key
            if (prefix.length() > 0) {
                prefix.pop_back(); 
            }
        } else if (c >= '1' && c <= '3') {
            // NEW FEATURE: autocomplete if they press 1, 2, or 3!
            // subtract the character '1' to convert it to an array index (0, 1, or 2)
            int index = c - '1'; 
            
            // make sure we actually have a suggestion at that number before replacing
            if (result != nullptr && result->top3[index].freq > 0) {
                prefix = result->top3[index].word; 
            }
        } else if (isalpha(c)) {
            // append the new letter to the prefix
            prefix += c; 
        }
    }
    
    // clear screen one last time before exiting so the terminal is clean
    system("clear");
    cout << "exiting program..." << endl;
    return 0;
}