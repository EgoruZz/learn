#include <iostream>
using namespace std;

void solve() {
    int x1, y1, x2, y2;
    cin >> x1 >> y1 >> x2 >> y2;
    cout << (((x1 + y1 - x2 - y2) % 2 == 0) ? "YES" : "NO");
}

signed main() {
    solve();
}
