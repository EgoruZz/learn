#include <iostream>
using namespace std;

class Solution {
public:
    int numberOfPoints(vector<vector<int>>& nums) {
        unordered_map<int, int> mp;
        for (auto vec : nums) {
            if (mp.find(vec[0]) == mp.end()) mp[vec[0]] = vec[1];
            else mp[vec[0]] = max(mp[vec[0]], vec[1]);
        }
        
        vector<int> keys;
        for (auto& [key, val] : mp) keys.push_back(key);
        sort(keys.begin(), keys.end());

        int points = 1, last = keys[0];
        for (auto key : keys) {
            if (last < mp[key]) {
                points += mp[key] - ((key > last) ? key - 1 : last);
                last = mp[key];
            }
        }
        return points;
    }
};

void solve() {
    vector<vector<int>> nums = {{3, 9}, {4, 6}, {5, 70}, {73, 78}};
    auto res = Solution().numberOfPoints(nums);
    cout << res;
}

signed main() {
    solve();
}
