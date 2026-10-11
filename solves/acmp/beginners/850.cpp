#include <iostream>
using namespace std;

void solve() {
    int a, b;
    cin >> a >> b;
    cout << max(a, b) / 2 + max(a, b) % 2 << ' ' << min(a, b);
}

signed main() {
    solve();
}
