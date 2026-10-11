#include <iostream>
#include <string>
using namespace std;

void solve() {
    string s;
    cin >> s;
    cout << ((string(s).find('0') == s.npos) ? "YES" : "NO");
}

signed main() {
    solve();
}
