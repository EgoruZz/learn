#include <iostream>
#include <cmath>
using namespace std;

void solve() {
    int k, n;
    cin >> k >> n;
    cout << ceil(1.0 * n / k) << ' ' << ((n % k == 0) ? k : n % k);
}

signed main() {
    solve();
}
