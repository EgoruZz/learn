#include <iostream>
using namespace std;

void solve() {
    long long c, h, o;
    cin >> c >> h >> o;
    cout << min (min(c / 2, h / 6), o);
}

signed main() {
    solve();
}
