#include <iostream>
using namespace std;

class Solution {
public:
    int deleteAndEarn(vector<int>& nums) {
        unordered_map<int, int> cnt;
        for (auto elem : nums) cnt[elem]++;

        vector<int> keys;
        for (auto [key, val] : cnt) keys.push_back(key);
        sort(keys.begin(), keys.end());

        vector<int> dp(keys.size() + 1, 0);
        dp[1] = keys[0] * cnt[keys[0]];
        for (int i = 2; i <= keys.size(); i++) {
            dp[i] = max(dp[i - 1] + ((abs(keys[i - 2] - keys[i - 1]) > 1) ? keys[i - 1] * cnt[keys[i - 1]] : 0),
            dp[i - 2] + keys[i - 1] * cnt[keys[i - 1]]);
        }

        return dp[keys.size()];
    }
};

void func() {
    vector<int> nums = {2,2,3,3,3,4};
    auto res = Solution().deleteAndEarn(nums);
    cout << res << ' ';
}

signed main() {
    func();
}