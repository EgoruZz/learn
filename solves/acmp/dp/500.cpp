#include <iostream>
#include <vector>
using namespace std;

bool comp(const pair<int, int>& pair1, const pair<int, int>& pair2) {
    return pair1.first <= pair2.first;
}

void solve() {
    int n;
    cin >> n;

    vector<pair<int, int>> a(n);
    for (int i = 0; i < n; i++) cin >> a[i].first >> a[i].second;
    sort(a.begin(), a.end(), comp);

    vector<int> dp(n, 0);
    dp[0] = a[1].second, dp[1] = a[1].second;
    if (n > 2) dp[2] = dp[0] + a[2].second;

    for (int i = 3; i < n; i++) {
        int three = dp[i - 3] + a[i - 1].second + a[i].second;
        int two = dp[i - 2] + a[i].second;
        dp[i] = min(two, three);
    }

    cout << dp[n - 1];
}

signed main() {
    solve();
}
