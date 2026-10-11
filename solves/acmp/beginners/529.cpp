#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

void solve() {
    long double x1, y1, x2, y2;
    cin >> x1 >> y1 >> x2 >> y2;
    cout << fixed << setprecision(5) << sqrtl((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

signed main() {
    solve();
}
