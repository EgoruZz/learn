#include <iostream>
using namespace std;

void solve() {
    double x;
    cin >> x;

    double res = 0, k = 2;
    while (res < x) res += 1/k++;

    cout << k - 2 << " card(s)";
}

signed main() {
    solve();
}