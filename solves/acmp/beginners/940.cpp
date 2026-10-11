#include <iostream>
#include <string>
using namespace std;

void solve() {
    int k;
    cin >> k;
    string s;
    cin >> s;
    cout << s.erase(k - 1, 1);
}

signed main() {
    solve();
}
