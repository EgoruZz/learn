#include <iostream>
#include <cmath>
using namespace std;

void solve() {
    int k;
    cin >> k;
    cout << ((int) sqrt(1 + 8 * k) - 1) / 2;
}

signed main() {
    solve();
}
