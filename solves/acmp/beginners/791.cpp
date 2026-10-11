#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    int up = n - 8, left = n - 1, right = n + 1, down = n + 8;
    string skip = "";
    cout << ((up >= 1) ? to_string(up) + " " : skip)
         << ((n % 8 != 1) ? to_string(left) + " " : skip)
         << ((n % 8 != 0) ? to_string(right) + " " : skip)
         << ((down <= 64) ? to_string(down) + " " : skip);
}

signed main() {
    solve();
}
