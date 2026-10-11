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
    int subarraySum(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        int summ = 0;
        for (int i = 0; i < nums.size(); i++) {
            summ += p.sum(max(0, i - nums[i]), i + 1);
        }
        return summ;
    }
};

void func() {
    vector<int> nums = {2,3,1};
    auto res = Solution().subarraySum(nums);
    cout << res;
}

signed main() {
    func();
}