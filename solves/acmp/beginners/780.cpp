#include <iostream>
#include <cmath>
using namespace std;

void solve() {
    int k;
    cin >> k;
    int n = (-1 + sqrt(1 + 8 * k)) / 2;
    cout << n;
}

signed main() {
    solve();
}
