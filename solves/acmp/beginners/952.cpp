#include <iostream>
using namespace std;

void solve() {
    int n, m;
    cin >> n >> m;
    if (n + m == 0) {
        cout << 0 << ' ' << 0;
        return;
    }
    cout << ((n == 0) ? "Impossible" : to_string(n + max(m - n, 0)) + " " + to_string(n + max(m - 1, 0)));
}

signed main() {
    solve();
}
