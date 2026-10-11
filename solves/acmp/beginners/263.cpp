#include <iostream>
using namespace std;

void solve() {
    int n, i, j;
    cin >> n >> i >> j;
    cout << min(abs(j - i) - 1, n - (abs(j - i) + 1));
}

signed main() {
    solve();
}
