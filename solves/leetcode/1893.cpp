#include <iostream>
#include <unordered_map>
using namespace std;

class Solution {
public:
    bool isCovered(vector<vector<int>>& ranges, int left, int right) {
        unordered_map<int, int> a;
        for (auto vec : ranges) {
            if (a.find(vec[1]) == a.end()) a[vec[1]] = vec[1] - vec[0] + 1;
            else a[vec[1]] = max(a[vec[1]], vec[1] - vec[0]);
        }
        vector<int> keys;
        for (auto& [ai ,ni] : a) keys.push_back(ai);
        sort(keys.rbegin(), keys.rend());
        for (auto key : keys) {
            int ai = key, ni = a[key];
            if (0 <= ai - right && ai - right < ni) right -= ni - (ai - right);
            if (right < left) break;
        }
        return (right < left);
    }
};

void solve() {
    int left = 1, right = 30'332'948;
    vector<vector<int>> ranges = {{1, 30000}, {30, 2342341}};
    auto res = Solution().isCovered(ranges, left, right);
    cout << res;
}

signed main() {
    solve();
}
