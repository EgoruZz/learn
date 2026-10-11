#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); ++i) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    int sumOddLengthSubarrays(vector<int>& arr) {
        PrefixSum1D p;
        p.build(arr);
        int n = (int) arr.size();
        vector<int> dp(n + 1, 0);
        for (int i = 1; i <= n; i += 2) {
            for (int j = 0; i + j <= n; j += 1) dp[i + j] += p.sum(j, i + j);
            dp[i] += dp[i - 1] + ((i - 2 > 0) ? dp[i - 2] : 0);
        }
        if (n % 2 == 0) dp[n] += dp[n - 1];
        return dp[n];
    }
};

void solve() {
    vector<int> arr = {1, 2};
    auto res = Solution().sumOddLengthSubarrays(arr);
    cout << res;
}

signed main() {
    solve();
}
