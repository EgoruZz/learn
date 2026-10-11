#include <iostream>
using namespace std;

void solve() {
    int a, b, c, t;
    cin >> a >> b >> c >> t;
    cout << min(t, a) * b + max(t - a, 0) * c;
}

signed main() {
    solve();
}
