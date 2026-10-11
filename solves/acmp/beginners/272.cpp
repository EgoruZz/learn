#include <iostream>
#include <sstream>
#include <string>
using namespace std;

void solve() {
    string s;
    getline(cin, s);

    stringstream ss(s);
    int pos = 1, mx = -1e5, mn = 1e5;

    while (ss >> s) {
        (pos % 2) ? mn = min(mn, stoi(s)) : mx = max(mx, stoi(s));
        pos = !pos;
    }
    
    cout << mx + mn;
}

signed main() {
    solve();
}