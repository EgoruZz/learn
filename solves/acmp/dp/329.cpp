#include <iostream>
#include <vector>

using namespace std;

void solve() {
    int n;
    cin >> n;

    vector<int> a(n, 0);
    for (int i = 0; i < n; i++)
        cin >> a[i];

    vector<int> dp(n + 1, -1e9);
    dp[0] = 0, dp[1] = a[0];

    vector<int> parent(n + 1, -1);

    for (int i = 2; i <= n; i++) {
        if (dp[i - 2] > dp[i - 1]) parent[i] = i - 2;
        else parent[i] = i - 1;
        dp[i] = max(dp[i - 2], dp[i - 1]) + a[i - 1];
    }

    cout << dp[n] << endl;
    vector<int> path;
    path.push_back(n);
    for (int p = parent[n]; p > 0; p = parent[p])
        path.push_back(p);
    for (int i = path.size() - 1; i >= 0; i--) {
        cout << path[i] << ' ';
    }
}

signed main() { solve(); }
