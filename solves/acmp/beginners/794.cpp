#include <iostream>
#include <cmath>
using namespace std;

void solve() {
    int n, m, k;
    cin >> n >> m >> k;
    int b = ceil(1.0 * (m - (k - 1)) / k);
    cout << n * (min(k - 1, m) + max(0, b));
}

signed main() {
    solve();
}
