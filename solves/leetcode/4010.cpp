#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    long long maxPairStrength(vector<int>& nums) {
        int n = (int) nums.size();
        long long ans = 1;

        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                auto g = (gcd(nums[i], nums[j]));
                ans = max(ans,  ((long long) nums[i] * nums[j]) / (g * g));
            }
        }

        return ans;
    }

    long long gcd(int a, int b) {
        while (b) swap(a %= b, b);
        return a;
    }
};

signed main() {
    vector<int> nums = {2, 3, 5};
    auto res = Solution().maxPairStrength(nums);
    cout << res << endl;
}
