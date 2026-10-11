#include <iostream>
#include <vector>
using namespace std;

void func() {
    int m, n;
    cin >> m >> n;

    vector<long long> dp(n + 1, 0);
    for (int i = 0; i < m; i++) dp[i] = 1;

    for (int i = m; i <= n; i++) {
        dp[i] = dp[i - m] + dp[i - 1];
    }
    cout << dp[n];
}

signed main() {
    func();
}