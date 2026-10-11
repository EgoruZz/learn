#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    int s = 0;
    for (int i = 0; i < n; i++) {
        int val;
        cin >> val;
        s += val;
    }
    cout << s - (n - 1);
}

signed main() {
    solve();
}
