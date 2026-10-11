#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n, k;
    cin >> n >> k;

    vector<vector<int>> t(n, vector<int>(n));
    string s;
    for (int i = 0; i < n; i++) {
        cin >> s;
        for (int j = 0; j < s.length(); j++) t[i][j] = s[j] - '0';
    }

    vector<vector<vector<int>>> dp(k + 1, vector<vector<int>>(n, vector<int>(n, 0)));
    dp[0][0][0] = 1;
    vector<vector<int>> d = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};
    for (int step = 1; step <= k; step++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (t[i][j] == 1) continue;
                for (auto& delta : d) {
                    int di = delta[0], dj = delta[1];
                    int ni = i + di, nj = j + dj;
                    if (0 <= ni && ni < n && 0 <= nj && nj < n && t[ni][nj] == 0)
                        dp[step][i][j] += dp[step - 1][ni][nj];
                }
            }
        }
    }

    cout << dp[k][n - 1][n - 1];
}

signed main() {
    solve();
}