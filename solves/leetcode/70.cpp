#include <iostream>
using namespace std;

class Solution {
public:
    int climbStairs(int n) {
        vector<int> dp(n + 2, 0);
        dp[1] = 1;
        for (int i = 2; i <= n + 1; i++) dp[i] = dp[i - 2] + dp[i - 1];
        return dp[n + 1];
    }
};

signed main() {
    int k = 7;
    auto res = Solution().climbStairs(k);
    cout << res << endl;
}
