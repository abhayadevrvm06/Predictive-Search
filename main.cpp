#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

// For portable raw terminal input:
// Windows has _getch() built into conio.h which does exactly what we need.
// On Unix/Mac we use termios.h to manually configure the terminal.
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;

struct WordFreq {
    string word = "";
    long long freq = 0;
};

struct Node {
    Node* child[26] = {}; 
    WordFreq top3[3]; 

    // Recursively deletes all child nodes when this node is destroyed.
    // This means a single `delete root` frees the entire trie. Memory: managed.
    ~Node() {
        for (Node* c : child)
            delete c; // delete on nullptr is a no-op, so no null check needed
    }
};

void insert(Node* root, string word, long long freq) {
    Node* curr = root;
    for (char c : word) {
        int i = tolower(c) - 'a';
        if (i < 0 || i > 25) continue; 
        if (curr->child[i] == nullptr) {
            curr->child[i] = new Node();
        }
        curr = curr->child[i];
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

char getKeystroke() {
#ifdef _WIN32
    return (char)_getch();
#else
    // Switch terminal to raw mode so each keypress arrives immediately,
    // without waiting for Enter. Restore original settings after reading.
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    // ANSI escape sequence — clears screen directly, no shell process spawned
    cout << "\033[2J\033[H";
#endif
}

int main() {
    Node* root = new Node();
    
    cout << "loading dataset..." << endl;
    loadTxt(root, "20k.txt");
    
    string prefix = "";
    
    while (true) {
        clearScreen();
        
        cout << "---------------------------------" << endl;
        cout << "       OptiType Engine           " << endl;
        cout << "---------------------------------" << endl;
        cout << "(type to search | 1/2/3 to autocomplete | '-' backspace | '?' quit)\n" << endl;
        
        cout << "Search: " << prefix << "\n\n";
        
        Node* result = nullptr;
        
        if (prefix.length() > 0) {
            result = search(root, prefix);
            
            if (result == nullptr || result->top3[0].freq == 0) {
                cout << "Suggestions:\n  no suggestions found" << endl;
            } else {
                cout << "Suggestions:" << endl;
                for (int i = 0; i < 3; i++) {
                    if (result->top3[i].freq > 0) {
                        cout << "  " << i + 1 << ". " << result->top3[i].word << endl;
                    }
                }
            }
        }
        
        char c = getKeystroke();
        
        if (c == '?') {
            break; 
        } else if (c == '-' || c == 127) { 
            if (prefix.length() > 0) {
                prefix.pop_back(); 
            }
        } else if (c >= '1' && c <= '3') {
            int index = c - '1'; 
            if (result != nullptr && result->top3[index].freq > 0) {
                prefix = result->top3[index].word; 
            }
        } else if (isalpha(c)) {
            prefix += c; 
        }
    }
    
    clearScreen();
    cout << "exiting program..." << endl;

    delete root; // destructor recursively frees all nodes. Memory: managed.
    return 0;
}
