#include <iostream>
#include <vector>
using namespace std;
using ll = long long;

ll solve(int n) {
    vector<ll> dp = {1, 2, 4, 7};
    dp.resize(n);
    for (int i = 4; i < n; i++) {
        for (int j = 1; j <= 3; j++) {
            dp[i] += dp[i - j];
        }
    }
    return dp[n - 1];
}

signed main() {
    int n;
    cin >> n;
    auto res = solve(n);
    cout << res;
}
