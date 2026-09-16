#include <iostream>
#include <vector>
using namespace std;

int solve(int n, int m) {
    vector<vector<int>> t(n, vector<int>(m, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) cin >> t[i][j];
    }

    vector<vector<int>> dp(n, vector<int>(m, (int) 1e9));
    dp[0][0] = t[0][0];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (0 <= i - 1) dp[i][j] = min(dp[i][j], dp[i - 1][j] + t[i][j]);
            if (0 <= j - 1) dp[i][j] = min(dp[i][j], dp[i][j - 1] + t[i][j]);
        }
    }
    return dp[n - 1][m - 1];
}

signed main() {
    int n, m;
    cin >> n >> m;
    auto res = solve(n, m);
    cout << res;
}
