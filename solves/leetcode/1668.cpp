#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    int maxRepeating(string sequence, string word) {
        vector<int> dp(sequence.length() + 1, 0);
        int w = word.length();
        bool sub;
        for (int i = 0; i <= (int) sequence.length() - w; i++) {
            sub = true;
            for (int j = 0; j < w && sub; j++) {
                if (sequence[i + j] != word[j]) sub = false;
            }
            if (sub) dp[i + w] = dp[i] + 1;
        }
        return *max_element(dp.begin(), dp.end());
    }
};

void func() {
    string s = "a", w = "aa";
    auto res = Solution().maxRepeating(s, w);
    cout << res;
}

signed main() {
    func();
}