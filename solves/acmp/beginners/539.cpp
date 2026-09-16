#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    if (n == 1) {
        cout << 0;
        return;
    }
    if (n % 2) cout << n;
    else cout << n / 2;
}

signed main() {
    solve();
}
