#include <iostream>
using namespace std;

void solve() {
    int w, h, r;
    cin >> w >> h >> r;

    if (2 * r <= w && 2 * r <= h) cout << "YES";
    else cout << "NO";
}

signed main() {
    solve();
}
