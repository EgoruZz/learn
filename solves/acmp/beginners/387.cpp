#include <iostream>
#include <string>
using namespace std;

void solve() {
    int n;
    cin >> n;
    string s;
    int cnt = 0;
    for (int i = 0 ; i < n; i++) {
        cin >> s;
        if (s[0] == s[3]) cnt++;
    }
    cout << cnt;
}

signed main() {
    solve();
}