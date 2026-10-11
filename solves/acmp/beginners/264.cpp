#include <iostream>
using namespace std;

void solve() {
    int n, mx = 0, ans = 0;
    cin >> n;
    for (int i = 0, val; i < n; i++) {
        cin >> val;
        if (val > 0) ans++;
        else mx = max(mx, ans), ans = 0;
    }
    mx = max(mx, ans);
    cout << mx;
}

signed main() {
    solve();
}
