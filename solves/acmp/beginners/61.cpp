#include <iostream>
#include <vector>
using namespace std;

void solve() {
    vector<int> points(8);
    for (int i = 0; i < 8; i++) cin >> points[i];

    int p1 = 0, p2 = 0;
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) p1 += points[i];
        else p2 += points[i];
    }

    if (p1 < p2) cout << 2;
    if (p1 == p2) cout << "DRAW";
    if (p1 > p2) cout << 1;
}

signed main() {
    solve();
}
