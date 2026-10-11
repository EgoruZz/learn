#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    vector<string> getLongestSubsequence(vector<string>& words, vector<int>& groups) {
        vector<int> dp(groups.size(), 0), path;
        dp[0] = 1, path.push_back(0);
        for (int i = 1; i < groups.size(); i++) {
            dp[i] = dp[i - 1];
            if (groups[i - 1] != groups[i]) {
                dp[i]++; path.push_back(i);
            }
        }
        vector<string> sub;
        for (auto elem : path) sub.push_back(words[elem]);
        return sub;
    }
};

void func() {
    vector<string> arg1 = {"a"};
    vector<int> arg2 = { 1};
    auto res = Solution().getLongestSubsequence(arg1, arg2);
    for (auto elem : res) cout << elem << ' ';
}

signed main() {
    func();
}