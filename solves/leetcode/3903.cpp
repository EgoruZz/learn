#include <iostream>
using namespace std;

class Solution {
public:
    int firstStableIndex(vector<int>& nums, int k) {
        int n = (int) nums.size(), smallest = -1;

        vector<int> prefix_mx(n), suffix_mn(n);
        prefix_mx[0] = nums[0], suffix_mn[n - 1] = nums[n - 1];
        for (int i = 1; i < n; i++) {
            prefix_mx[i] = max(prefix_mx[i - 1], nums[i]);
            suffix_mn[n - 1 - i] = min(suffix_mn[n - i], nums[n - 1 - i]);
        }

        for (int i = 0; smallest == -1 && i < n; i++) {
            if (prefix_mx[i] - suffix_mn[i] <= k) smallest = i;
        }
        
        return smallest;
    }
};

void func() {
    vector<int> nums = {5,0,1,4};
    int k = 3;
    auto res = Solution().firstStableIndex(nums, k);
    cout << res;
}

signed main() {
    func();
}
