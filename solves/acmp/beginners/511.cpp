#include <iostream>
using namespace std;

void solve() {
    int k;
    cin >> k;
    if (max(0, k - 1) * 5 > 12 * 60) cout << "NO";
    else cout << max(0, k - 1) * 5 / 60 << ' ' << (max(0, k - 1) * 5) % 60;
}

signed main() {
    solve();
}
