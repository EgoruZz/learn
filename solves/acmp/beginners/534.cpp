#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;
    vector<int> c(n);
    for (int i = 0; i < n; i++) cin >> c[i];

    int k;
    cin >> k;
    for (int i = 0, val; i < k; i++) cin >> val, c[val - 1]--;

    for (int i = 0; i < n; i++) cout << (c[i] < 0 ? "yes\n" : "no\n");
}

signed main() {
    solve();
}
