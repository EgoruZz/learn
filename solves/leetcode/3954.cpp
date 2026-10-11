#include <iostream>
using namespace std;

class Solution {
public:
    int sumOfGoodIntegers(int n, int k) {
        vector<int> dp(n + k + 1, 0);
        for (int x = max(0, n - k); x <= n + k; x++) {
            dp[x] = dp[x - 1] + ((n & x) == 0 ? x : 0);
        }
        return dp[n + k];
    }
};

void func() {
    int n = 2, k = 3;
    auto res = Solution().sumOfGoodIntegers(n, k);
    cout << res << ' ';
}

signed main() {
    func();
}