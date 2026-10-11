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
    int maximumPopulation(vector<vector<int>>& logs) {
        PrefixSum1D p;
        vector<int> a(100 + 1, 0);
        for (auto vec : logs) a[vec[0] - 1950]++, a[vec[1] - 1950]--;
        p.build(a);
        return 1950 + (max_element(p.begin() + 1, p.end()) - (p.begin() + 1));
    }
};

void solve() {
    vector<vector<int>> logs = {{1993, 2001}, {1970, 2008}, {1980, 1997}, {1994, 2018}};
    auto res = Solution().maximumPopulation(logs);
    cout << res;
}

signed main() {
    solve();
}
