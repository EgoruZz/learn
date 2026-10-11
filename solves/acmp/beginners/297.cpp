#include <iostream>
using namespace std;

void solve() {
    string n;
    cin >> n;
    int ans = 0;
    for (size_t i = 0; i < n.length(); i++) {
        if (string("069").find(n[i]) != string::npos) ans++;
        if (n[i] == '8') ans += 2;
    }
    cout << ans;
}

signed main() {
    solve();
}
