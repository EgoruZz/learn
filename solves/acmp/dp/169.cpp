#include <iostream>
#include <vector>
using namespace std;

void solve() {
   int n, k;
   cin >> n >> k;

   vector<vector<int>> dp(k + 1, vector<int>(k + n + 1, 0));
   dp[0][0] = 1;
   for (int step = 1; step <= k; step++) {
        for (int dist = 1; dist <= k + n; dist++) {
            dp[step][dist] += dp[step - 1][dist - 1];
            dp[step][dist] += dp[step - 1][dist + 1];
        }
   }
   cout << dp[k][n];
}

signed main() {
    solve();
}
