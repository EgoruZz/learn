#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    if (n <= 0) {
        cout << "NO";
        return;
    }
    if (((n - 1) & n) == 0) cout << "YES";
    else cout << "NO";
}

signed main() {
    solve();
}
