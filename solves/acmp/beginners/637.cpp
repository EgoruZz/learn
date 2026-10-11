#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;
    vector<int> a(n);
    for (int i = 0; i < n; i++) cin >> a[i];
    int k;
    cin >> k;
    int sit_down = 0;
    for (int i = 0; i < n; i++) sit_down += a[i] - max(0, a[i] - k);
    cout << sit_down;
}

signed main() {
    solve();
}
