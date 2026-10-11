#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    int mn = 1e9, mx = -1e9;
    for (int i = 0; i < n; i++) {
        int val;
        cin >> val;
        if (val < mn) mn = val;
        if (mx < val) mx = val;
    }
    cout << mn << ' ' << mx;
}

signed main() {
    solve();
}
