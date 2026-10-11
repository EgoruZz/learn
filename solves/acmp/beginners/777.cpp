#include <iostream>
using namespace std;

void solve() {
    int s, t;
    cin >> s >> t;
    cout << (t - s + 12) % 12;
}

signed main() {
    solve();
}
