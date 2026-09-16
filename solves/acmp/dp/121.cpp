#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

void solve() {
    int n;
    cin >> n;

    vector<int> a(n, 0);
    for (int i = 0; i < n; i++) cin >> a[i];
    sort(a.begin(), a.end());

    vector<int> dp(n, 0);
    dp[0] = (int) 1e9, dp[1] = a[1] - a[0];
    for (int i = 2; i < n; i++) 
        dp[i] = min(dp[i - 2], dp[i - 1]) + (a[i] - a[i - 1]);

    cout << dp[n - 1];
}

signed main() {
    solve();
}
