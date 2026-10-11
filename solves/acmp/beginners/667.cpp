#include <iostream>
#include <cmath>
using namespace std;

void solve() {
    int n, m, k;
    cin >> n >> m >> k;

    if (k <= 2) {
        cout << 0;
        return;
    }

    int buses = n / (k - 2) + (n % (k - 2) ? 1 : 0);
    if (m >= 2 * buses) cout << buses + ceil((1.0 * (m - 2 * buses - (n % (k - 2) ? k - n % (k - 2) - 2 : 0)) / k));
    else cout << 0;
}

signed main() {
    solve();
}
