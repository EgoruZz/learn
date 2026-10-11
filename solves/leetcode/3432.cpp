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
    int countPartitions(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        int n = (int) nums.size(), count = 0;
        for (int i = 0; i < n - 1; i++) {
            if (abs(p.sum(0, i + 1) - p.sum(i + 1, n)) % 2 == 0) count++;
        }
        return count;
    }
};

void func() {
    vector<int> nums = {10,10,3,7,6};
    auto res = Solution().countPartitions(nums);
    cout << res;
}

signed main() {
    func();
}