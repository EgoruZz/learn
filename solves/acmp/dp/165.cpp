#include <iostream>
#include <vector>
using namespace std;

int solve() {
    int n, m;
    cin >> n >> m;
    vector<vector<int>> t(n, vector<int>(m, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) cin >> t[i][j];
    }

    vector<vector<int>> dp(n, vector<int>(m, 0));
    dp[0][0] = 1;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m && i + j != m + n - 2; j++) {
            if (i + t[i][j] < n) dp[i + t[i][j]][j] += dp[i][j];
            if (j + t[i][j] < m) dp[i][j + t[i][j]] += dp[i][j];
        }
    }

    return dp[n - 1][m - 1];
}

signed main() {
    auto res = solve();
    cout << res;
}
