#include <iostream>
#include <vector>
using namespace std;

int solve(int n) {
    vector<int> dp(n + 1, (int) 1e9);
    dp[0] = 0;
    for (int i = 1; i < n + 1; i++) {
        for (int j = 0; j < n + 1 - i; j++) {
            int val;
            cin >> val;
            dp[i + j] = min(dp[i + j], dp[i - 1] + val);
        }
    }
    return dp[n];
}

signed main() {
    int n;
    cin >> n;
    auto res = solve(n);
    cout << res;
}
