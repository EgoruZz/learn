#include <iostream>
#include <cmath>
using namespace std;

void solve() {
    double l, w, h;
    cin >> l >> w >> h;
    cout << ceil(2.0 * (w * h + l * h) / 16);
}

signed main() {
    solve();
}
