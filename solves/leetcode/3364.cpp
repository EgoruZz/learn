#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build (const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    int minimumSumSubarray(vector<int>& nums, int l, int r) {
        PrefixSum1D p;
        p.build(nums);
        int n = (int) nums.size();
        int ans = 1e9;

        for (int i = 0; i <= n - l; i++) {
            for (int j = i + l; j <= i + r && j <= n; j++) {
                int s = p.sum(i, j);
                if (s > 0) ans = min(ans, s);
            }
        }

        return ((ans == 1e9) ? - 1 : ans);
    }
};

void solve() {
    int l = 1, r = 1;
    vector<int> nums = {4,-10};
    auto res = Solution().minimumSumSubarray(nums, l, r);
    cout << res;
}

signed main() {
    solve();
}
