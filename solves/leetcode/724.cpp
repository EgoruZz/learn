#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++)
            p[i + 1] = p[i] + a[i];
    }

    int sum(int l, int r) const {
        return p[r] - p[l];
    }
};

class Solution {
public:
    int pivotIndex(vector<int>& nums) {
        PrefixSum1D pref;
        pref.build(nums);

        int n = (int) nums.size(), pivot = -1;
        for (int i = 0; pivot == -1 && i < n; i++) {
            if (pref.sum(0, i) == pref.sum(i + 1, n)) pivot = i;
        }

        return pivot;
    }
};

void solve() {
    vector<int> nums = {2,1,-1};
    auto result = Solution().pivotIndex(nums);
    cout << result;
}

signed main() {
    solve();
}
