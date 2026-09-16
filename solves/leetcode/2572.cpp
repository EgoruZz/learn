#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    int squareFreeSubsets(vector<int>& nums) {
        vector<int> count(30 + 1, 0);
        for (auto elem : nums) count[elem]++;

        vector<int> dp(1024, 0);
        dp[0] = 1;
        vector<int> primes = {2,3,5,7,11,13,17,19,23,29};
        const int MOD = 1e9 + 7;

        for (int elem = 2; elem <= 30; elem++) {
            if (count[elem] == 0) continue;
            int num_mask = 0;
            bool is_square_free = true;
            
            for (int i = 9; i >= 0; i--) {
                if (elem % (primes[i] * primes[i]) == 0) {
                    is_square_free = false;
                    break;
                }
                if (elem % primes[i] == 0) num_mask |= (1 << i);
            }
            if (!is_square_free) continue;

            for (int mask = 1023; mask >= 0; mask--) {
                if ((mask & num_mask) == 0) {
                    dp[mask | num_mask] = (1LL * dp[mask | num_mask] + (1LL * dp[mask] * count[elem])) % MOD;
                }
            }
        }

        int ans = 0;
        for (int i = 0; i < 1024; i++) ans = (ans + dp[i]) % MOD;
        for (int i = 0; i < count[1]; i++) ans = (ans * 2) % MOD;
        ans = (ans - 1 + MOD) % MOD;

        return ans;
    }
};

signed main() {
    vector<int> v = {3, 4, 4, 5}; //{3, 7, 12, 14};
    auto res = Solution().squareFreeSubsets(v);
    cout << res << '\n';
}
