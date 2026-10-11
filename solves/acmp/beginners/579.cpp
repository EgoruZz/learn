#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;

    vector<int> a(n);
    for (int i = 0; i < n; i++) cin >> a[i];

    vector<pair<int, int>> minus, plus;
    int m = 0, p = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < 0) minus.push_back({a[i], i}), m += abs(a[i]);
        else plus.push_back({a[i], i}), p += abs(a[i]);
    }

    if (m <= p) {
        cout << plus.size() << '\n';
        for (auto&& [val, idx] : plus) cout << idx + 1 << ' ';
    } else {
        cout << minus.size() << '\n';
        for (auto&& [val, idx] : minus) cout << idx + 1 << ' ';
    }

}

signed main() {
    solve();
}
