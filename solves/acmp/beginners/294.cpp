#include <iostream>
using namespace std;

void solve() {
    int k1, l1, m1;
    cin >> k1 >> l1 >> m1;
    int k2, l2, m2;
    cin >> k2 >> l2 >> m2;
    int ans = k1 * l1 / 100 * m1
            + k2 * l2 / 100 * m2;
    int r1 = k1 * (100 - l1) / 100, r2 = k2 * (100 - l2) / 100;
    ans += (r1 > r2) ? (r1 - r2) * m1 : (r2 - r1) * m2;
    cout << ans;
}

signed main() {
    solve();
}
