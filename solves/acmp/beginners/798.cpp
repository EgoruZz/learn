#include <iostream>
using namespace std;

void solve() {
    int m, n, i, j, c;
    cin >> m >> n >> i >> j >> c;
    if (m % 2 == 1 && n % 2 == 1) {
        cout << (((i + j) % 2 == 0) ? ((c == 0) ? "black" : "white")
        : ((c == 0) ? "white" : "black"));
        return;
    }
    cout << "equal";
}

signed main() {
    solve();
}
