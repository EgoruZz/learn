#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    int minOperations(vector<int>& nums) {
        int n = (int) nums.size();
        vector<int> pref(n + 1), suf(n + 1);
        pref[0] = 0, suf[n] = 0;

        for (int i = 1; i <= n; i++) {
            pref[i] = gcd(pref[i - 1], nums[i]);
            suf[i] = gcd(nums[n - 1 - i], suf[n - i]);
        }

        int l = 1, r = 1, g = nums[0];
        while (r < n + 1) {
            while (r < n + 1 && g == 1) {
                r++;
                g = gcd(g, nums[r - 1]);
            }
            while (g != 1) {
                g = gcd(pref[l - 1], suf[r]);
            }
        }
        return -1;
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