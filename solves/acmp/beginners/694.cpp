#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;
    vector<int> day(31 + 1, 0);
    int mx = 0;
    for (int i = 0; i < n; i++) {
        int l, r;
        cin >> l >> r;
        for (int j = l; j <= r; j++) day[j]++, mx = max(mx, day[j]);
    }
    cout << (mx == n ? "YES" : "NO");
}

signed main() {
    solve();
}