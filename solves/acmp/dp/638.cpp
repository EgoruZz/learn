#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n, k;
    cin >> n >> k;

    int w, d_w, s;
    cin >> w >> d_w >> s;
    vector<int> month(n, 1);
    for (int i = 0, val; i < d_w; i++) {
        cin >> val;
        for (int i = (val < s) ? w - s + val : val - s; i < n; i += w) month[i] = 0;
    }

    int d_m;
    cin >> d_m;
    for (int i = 0, val; i < d_m; i++) {
        cin >> val;
        month[--val] = 0;
    }

    vector<int> dp(n, 0);
    
    int add = 0;
    for (int i = 0; i < k; i++) add += month[i];
    if (add == k) dp[k - 1] = 1;

    for (int i = k; i < n; i++) {
        if (month[i - k]) add -= 1;
        if (month[i]) add += 1;
        dp[i] = (add == k) ? dp[i - 1] + 1 : dp[i - 1];
    }

    cout << dp[n - 1];
}

signed main() {
    solve();
}
