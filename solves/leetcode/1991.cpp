#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    int findMiddleIndex(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        int mid = -1, n = nums.size();
        for (int i = 0; mid == -1 && i < n; i++) if (p.sum(0, i) == p.sum(i + 1, n)) mid = i;
        return mid;
    }
};

void solve() {
    vector<int> a = {2,3,-1,8,4};
    auto res = Solution().findMiddleIndex(a);
    cout << res;
}

signed main() {
    solve();
}