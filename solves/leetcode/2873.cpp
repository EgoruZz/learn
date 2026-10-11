#include <iostream>
using namespace std;

class Solution {
public:
    long long maximumTripletValue(vector<int>& nums) {
        vector<int> mx(nums.size(), 0);
        mx[0] = nums[0];
        for (size_t i = 1; i < nums.size(); i++) mx[i] = max(mx[i - 1], nums[i]);

        vector<int> mx_diff(nums.size(), 0);
        for (size_t j = 1; j < nums.size(); j++)
            mx_diff[j] = max(mx_diff[j - 1], mx[j - 1] - nums[j]);

        long long answer = 0;
        for (size_t k = 2; k < nums.size(); k++)
            answer = max(answer, 1ll * mx_diff[k - 1] * nums[k]);
        return answer;
    }
};

void solve() {
    vector<int> nums = {12,6,1,2,7};
    auto res = Solution().maximumTripletValue(nums);
    cout << res;
}

signed main() {
    solve();
}
