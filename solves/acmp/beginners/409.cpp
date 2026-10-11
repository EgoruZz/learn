#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;

    vector<int> h(n);
    for (int i = 0; i < n; i++) cin >> h[i];

    double area = 0;
    for (int i = 1; i < n; i++) {
        area += abs(max(h[i - 1], h[i]) - 1.0 * abs(h[i] - h[i - 1]) / 2);
    }

    cout << fixed << setprecision(7) << area / (n - 1);
}

signed main() {
    solve();
}