#include <iostream>
using namespace std;

class Solution {
public:
    bool divisorGame(int n) {
        vector<int> dp(n + 1, 0);
        for (int i = 2; i <= n; i++) {
            for (int x = 1; x * x <= i; x++) {
                if (i % x == 0) {
                    if (dp[i - x] == 0 || dp[i - i / x] == 0 && x != 1) { dp[i] = 1; break;}
                }
            }
        }
        return dp[n];
    }
};

void func() {
    int n = 3;
    auto res = Solution().divisorGame(n);
    cout << res;
}

signed main() {
    func();
}