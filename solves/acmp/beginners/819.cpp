#include <iostream>
using namespace std;

void solve() {
    long long a, b, c;
    cin >> a >> b >> c;

    cout << 2 * (a*b + b*c + a*c) << ' ' << a * b * c;
}

signed main() {
    solve();
}
