#include <iostream>
#include <vector>
using namespace std;

int solve(int n) {
    vector<int> diagonal(n, 0);
    for (int i = 0; i < n; i++) cin >> diagonal[i];
    if (n == 1) return diagonal[0];

    vector<vector<int>> dp(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) dp[i][i] = diagonal[i];

    for (int step = 1; step < n; step++) {
        for (int i = step, j = 0; i < n && j < n - step; i++, j++) {
            if ((step % 2) == !(n % 2)) dp[i][j] = max(dp[i - 1][j], dp[i][j + 1]);
            else dp[i][j] = min(dp[i - 1][j], dp[i][j + 1]);
        }
    }

    return dp[n - 1][0];
}

signed main() {
    int n;
    cin >> n;
    auto res = solve(n);
    cout << res;
}
