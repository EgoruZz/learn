#include <iostream>

using namespace std;

void solve() {
    int a1, a2, n;
    cin >> a1 >> a2 >> n;

    int an = a1 + (n - 1) * (a2 - a1);
    cout << an;
}

signed main() {
    solve();
}
