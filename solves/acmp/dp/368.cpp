#include <iostream>
#include <vector>
#include <string>
using namespace std;

void solve() {
    int n;
    cin >> n;

    vector<vector<int>> t(n, vector<int>(n));
    string val;
    for (int i = 0; i < n * n; i++) {
        if (i % n == 0) cin >> val;
        t[i / n][n - 1 - i % n] = stoi(to_string(val[n - 1 - i % n]));
    }

    vector<vector<int>> dp(n, vector<int>(n, 1e9));
    vector<vector<int>> path(n, vector<int>(n, 0));
    dp[0][0] = t[0][0];
    for (int i = 1; i < n; i++) {
        dp[i][0] = t[i][0] + dp[i - 1][0], dp[0][i] = t[0][i] + dp[0][i - 1];
        path[i][0] = 1, path[0][i] = 2;
    }
    for (int i = 1; i < n * n; i++) {
        if (i % n == 0 || i / n == 0) continue;
        dp[i / n][i % n] = t[i / n][i % n] + min(dp[i / n - 1][i % n], dp[i / n][i % n - 1]);
        path[i / n][i % n] = (dp[i / n - 1][i % n] <= dp[i / n][i % n - 1]) ? 1 : 2;
    }

    vector<vector<char>> picture(n, vector<char>(n, '.'));
    int i = n * n - 1;
    while (i >= 0) {
        picture[i / n][i % n] = '#';
        i = (path[i / n][i % n] == 1) ? i - n : i - 1;
    }

    for (auto row : picture) {
        for (auto elem : row) cout << elem;
        cout << '\n';
    }
}

signed main() {
    solve();
}
