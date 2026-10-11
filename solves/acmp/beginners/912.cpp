#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;

    vector<int> c(100 + 1, 0);
    for (int i = 0, color; i < n; i++) cin >> color, c[color]++;

    pair<int, int> ans = {0, -1};
    bool uno = true;
    for (int i = 0; i < c.size(); i++) {
        if (ans.first < c[i]) ans.first = c[i], ans.second = i, uno = true;
        else if (ans.first == c[i]) uno = false;
    }

    if (uno) cout << ans.second;
    else cout << 0;
}

signed main() {
    solve();
}
