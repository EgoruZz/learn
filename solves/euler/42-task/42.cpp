#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;
using ull = unsigned long long;

void solve() {
    ifstream fin("0042_words.txt");
    string line;
    getline(fin, line);

    vector<string> words;
    string word;
    stringstream ss(line);

    while (getline(ss, word, ',')) {
        word.erase(remove(word.begin(), word.end(), '"'), word.end());
        words.push_back(word);
    }

    int count = 0;
    for (auto w : words) {
        int val = 0;
        for (char c : w) val += c - 'A' + 1;
        int d = 1 + 8 * val;
        int n = (-1 + sqrt(d)) / 2;
        if (n * (n + 1) == 2 * val) count++;
    }
    cout << count;
    cout << endl;
}

signed main() {
    solve();
}
