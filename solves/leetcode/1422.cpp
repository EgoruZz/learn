#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const string& s) {
        p.assign(s.length() + 1, 0);
        for (size_t i = 0; i < s.length(); i++)
            p[i + 1] = p[i] + (s[i] - '0');
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    int maxScore(string s) {
        PrefixSum1D p;
        p.build(s);
        int n = s.length(), mx = 0;
        for (size_t i = 1; i <= n - 1; i++) {
            int zeroes = i - p.sum(0, i);
            int ones = p.sum(i, n);
            mx = max(mx, zeroes + ones);
        }
        return mx;
    }
};

void solve() {
    string s = "011101";
    auto res = Solution().maxScore(s);
    cout << res;
}

signed main() {
    solve();
}
