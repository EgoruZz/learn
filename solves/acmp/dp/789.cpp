#include <iostream>
#include <vector>
using namespace std;
using ull = unsigned long long;

void solve() {
    int n;
    cin >> n;

    vector<ull> dp(n + 1, 0);
    dp[1] = 1;
    int p1 = 1, p2 = 1, p3 = 1;

    for (int i = 2; i <= n; i++) {
        ull res1 = dp[p1] * 2;
        ull res2 = dp[p2] * 3;
        ull res3 = dp[p3] * 5;
        ull mn = min({res1, res2, res3});
        if (res1 == mn) dp[i] = res1, p1++;
        if (res2 == mn) dp[i] = res2, p2++;
        if (res3 == mn) dp[i] = res3, p3++;
    }
    
    cout << dp[n];
}

signed main() {
    solve();
}
