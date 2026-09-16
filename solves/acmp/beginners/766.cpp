#include <iostream>
using namespace std;

void solve() {
    int n, m, k;
    cin >> n >> m >> k;

    if (n * m >= k) cout << "YES";
    else cout << "NO";
}

signed main() {
    solve();
}
