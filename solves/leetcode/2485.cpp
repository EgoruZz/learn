#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + (i + 1);
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    int pivotInteger(int n) {
        PrefixSum1D p;
        p.build(vector<int>(n, 0));

        int pivot = -1;
        for (int i = 1; pivot == -1 && i <= n; i++) if (p.sum(0, i) == p.sum(i - 1, n)) pivot = i;
        return pivot;
    }
};

void solve() {
    int n;
    cin >> n;
    auto res = Solution().pivotInteger(n);
    cout << res;
}

signed main() {
    solve();
}