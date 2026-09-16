#include <iostream>
#include <vector>
#include <string>
#include "algorithm"
using namespace std;

bool comp(string& s1, string& s2) {
    return s1.length() < s2.length();
}

void solve() {
    int m;
    cin >> m;

    vector<string> s(m);
    for (int i = 0; i < m; i++) cin >> s[i];
    sort(s.begin(), s.end(), comp);

    vector<int> dp(m, 1);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < i; j++) {
            if (s[i].rfind(s[j], 0) == 0 && s[j].length() < s[i].length())
                dp[i] = max(dp[i], dp[j] + 1);
        }
    }

    cout << *max_element(dp.begin(), dp.end());
}

signed main() {
    solve();
}
