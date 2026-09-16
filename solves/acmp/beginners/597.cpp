#include <iostream>
using namespace std;

void solve() {
    int r1, r2, r3;
    cin >> r1 >> r2 >> r3;

    if (2 * (r2 + r3) <= 2 * r1) cout << "YES";
    else cout << "NO";
}

signed main() {
    solve();
}
