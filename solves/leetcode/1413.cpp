#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
    auto begin() const {return p.begin(); }
    auto end() const { return p.end(); }
};

class Solution {
public:
    int minStartValue(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        auto mn = *min_element(p.begin() + 1, p.end());
        return (mn < 1 ? abs(mn) + 1 : 1);
    }
};

void solve() {
    vector<int> nums = {2,3,5,-5,-1};
    auto res = Solution().minStartValue(nums);
    cout << res;
}

signed main() {
    solve();
}