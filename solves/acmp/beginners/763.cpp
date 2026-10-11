#include <iostream>
using namespace std;

void solve() {
    unsigned long long x, y;
    cin >> x >> y;
    cout << ((x * y == 1) ? 0 : (x == y) ? 2 : 1);
}

signed main() {
    solve();
}
