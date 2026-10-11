#include <iostream>
using namespace std;

int n;

void rec(int step, int& mx, int& pos) {
    if (step == 0) return;
    int v, s;
    cin >> v >> s;
    if (s) ((mx < v) ? mx = v, pos = n - (step - 1) : mx);
    rec(step - 1, mx, pos);
}

void solve() {
    cin >> n;
    int mx = -1e9, pos = -1;
    rec(n, mx, pos);
    cout << pos;
}

signed main() {
    solve();
}
