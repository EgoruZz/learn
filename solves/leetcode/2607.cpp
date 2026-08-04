#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

class Solution {
public:
    typedef long long ll;
    ll makeSubKSumEqual(vector<int>& arr, int k) {
        int n = (int) arr.size();
        int g = gcd(n, k);
        ll ans = 0;

        for (int i = 0; i < g; i++) {
            vector<int> sub(n / g);
            for (int j = 0, z = i; j < n / g; z = (z + k) % n, j++) sub[j] = arr[z];

            nth_element(sub.begin(), sub.begin() + sub.size() / 2, sub.end());
            int median = sub[sub.size() / 2];
            for (auto elem : sub) ans += abs(static_cast<ll>(median - elem));
        }

        return ans;
    }

    int gcd(int a, int b) {
        while (b) swap(a %= b, b);
        return a;
    }
};

signed main() {
    vector<int> v = {2, 10, 9};
    int k = 1;
    auto res = Solution().makeSubKSumEqual(v, k);
    cout << res << '\n';
}