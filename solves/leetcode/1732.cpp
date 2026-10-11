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
    int largestAltitude(vector<int>& gain) {
        PrefixSum1D p;
        p.build(gain);
        return *max_element(p.begin(), p.end());
    }
};

void solve() {
    vector<int> arr = {-4,-3,-2,-1,4,3,2};
    auto res = Solution().largestAltitude(arr);
    cout << res;
}

signed main() {
    solve();
}
