#include <iostream>
using namespace std;

class Solution {
public:
    bool isSubsequence(string s, string t) {
        int pos = 0;
        for (auto c1 : t) {
            if (s[pos] == c1) pos++;
            if (pos == s.length()) break;
        }
        return (pos == s.length());
    }
};

void solve() {
    string s = "abc", t = "ahbgdc";
    auto res = Solution().isSubsequence(s, t);
    cout << res;
}

signed main() {
    solve();
}