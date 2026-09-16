#include <iostream>
#include <vector>
using namespace std;

int solve() {
   int n, k;
   cin >> n >> k;

   if (n > k || (k - n) % 2 != 0) return 0;

   vector<int> dp((k - n) / 2 + 1, 0);
   dp[0] = 1;

   for (int i = 1; i <= k; i++) {
        for (int j = (k - n) / 2; j >= 0; j--) {
            if ((i - j < 0) || (i - 2 * j >= n && i < k)) {
                dp[j] = 0;
                continue;
            }
            if (j > 0) dp[j] += dp[j - 1];
        }
   }

   return dp[dp.size() - 1];
}

signed main() {
    cout << solve();
}
