#include <iostream>
using namespace std;

class Solution {
public:
    string longestPalindrome(string s) {
        int mx = 1, idx = 0, n = (int) s.length();

        vector<vector<int>> dp(n + 1, vector<int> (n + 1, 0));
        for (int i = 0; i < n; i++) dp[i][i] = 1;

        for (int len = 2; len <= n; len++) {
            for (int i = 0; i <= n - len; i++) {
                int j = i + len - 1;
                if (s[i] == s[j] && (dp[i + 1][j - 1] || len==2)) {
                    dp[i][j] = 1;
                    if (mx < j - i + 1) mx = j - i + 1, idx = i;
                }
            }
        }
        return s.substr(idx, mx);
    }
};

void func() {
    string s = "cbbd";
    auto res = Solution().longestPalindrome(s);
    cout << res << ' ';
}

signed main() {
    func();
}