#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;
    vector<int> a(n);
    for (int i = 0; i < n; i++) cin >> a[i];
    vector<int> dp(n, 0);
    for (int i = 0; i < n; i++) {
        dp[i] = max(a[i] + a[(i - 1 + n) % n] + a[(i - 2 + n) % n], dp[(i - 1 + n) % n]);
    }
    cout << dp[n - 1];
}

signed main() {
    solve();
}
