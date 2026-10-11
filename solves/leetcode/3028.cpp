#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    }
    auto begin() const { return p.begin(); }
    auto end() const { return p.end(); }
};

class Solution {
public:
    int returnToBoundaryCount(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        return count(p.begin() + 1, p.end(), 0);
    }
};

void solve() {
    vector<int> nums = {2,3,-5};
    auto res = Solution().returnToBoundaryCount(nums);
    cout << res;
}

signed main() {
    solve();
}