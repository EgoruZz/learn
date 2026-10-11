#include <iostream>
#include <string>
using namespace std;

void solve() {
    string s;
    cin >> s;
    int j = s[0] - 'a', i = stoi(string (s, 1)) - 1;
    
    for (auto di : {-1, -2, 1, 2}) {
        for (auto dj : {-1, -2, 1, 2}) {
            if (abs(di) + abs(dj) != 3) continue;
            int ni = i + di, nj = j + dj;
            if (0 <= ni && ni <= 7 && 0 <= nj && nj <= 7)
                cout << (char) ('a' + nj) << ni + 1 << '\n';
        }
    }

}

signed main() {
    solve();
}