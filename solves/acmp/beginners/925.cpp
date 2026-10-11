#include <iostream>
using namespace std;

void solve() {
    int k;
    cin >> k;

    int n, a, b, c;
    cin >> n >> a >> b >> c;

    int mx = min(a, min(b, c));
    int mn = (n - a - b - c < 0) ? abs(n - a - b - c) % n : 0;

    cout << (k == 1 ? mn : mx);
}

signed main() {
    solve();
}
