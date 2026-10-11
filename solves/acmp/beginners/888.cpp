#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    long long prev = 0, points = 0;
    for (int i = 0, val; i < n; i++) {
        cin >> val;
        if (val) points += 3 + prev++;
        else prev = max(prev - 3, 0ll);
    }
    cout << points;
}

signed main() {
    solve();
}