#include <iostream>
#include <string>
using namespace std;

void solve() {
    int n, m;
    cin >> n >> m;
    string row;
    int mn = 1e9;
    for (int i = 0; i < n; i++) {
        cin >> row;
        auto left = row.find('.', 0);
        auto right = row.rfind('.', row.length());
        mn = min(mn, (int) (right - left) + (left != string::npos ? 1 : 0));
    }
    cout << mn;
}

signed main() {
    solve();
}