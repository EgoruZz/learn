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
    vector<int> runningSum(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        return vector<int> (p.begin() + 1, p.end());
    }
};

void solve() {
    vector<int> nums = {1,2,3,4};
    auto res = Solution().runningSum(nums);
    for (auto elem : res) cout << elem << ' ';
}

signed main() {
    solve();
}