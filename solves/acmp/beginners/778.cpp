#include <iostream>
using namespace std;

void solve() {
    int visits = 0;
    for (int i = 0, val; i < 31; i++) {
        cin >> val;
        visits += val;
    }
    cout << visits / 27;
}

signed main() {
    solve();
}
