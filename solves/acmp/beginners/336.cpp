#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;

void solve() {
    string s;
    cin >> s;

    unordered_set<int> visited;
    int pos = 0;
    visited.insert(pos);

    for (int i = 0; i < s.length(); i++) {
        pos += (s[i] == '1') ? 1 : -1;
        if (visited.find(pos) == visited.end()) visited.insert(pos);
    }
    cout << visited.size();
}

signed main() {
    solve();
}