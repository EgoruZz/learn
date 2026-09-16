#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    int minOperations(vector<int>& nums) {
        int n = (int) nums.size();

        int count = 0;
        for (auto elem : nums) if (elem == 1) count++;
        if (count != 0) return n - count;

        int ans = 1e9;

        for (int i = 0; i < (int) nums.size(); i++) {
            int g = nums[i];
            for (int j = i + 1; j < (int) nums.size(); j++) {
                g = gcd(g, nums[j]);
                if (g == 1) ans = min(ans, (j - i) + n - 1);
            }
        }

        return (ans == 1e9) ? -1 : ans;
    }

    int gcd(int a, int b) {
        while (b) swap(a %= b, b);
        return a;
    }
};

signed main() {
    vector<int> v = {2,6,3,4};//{6,10,15}; //{2, 10, 9};
    auto res = Solution().minOperations(v);
    cout << res << '\n';
}
