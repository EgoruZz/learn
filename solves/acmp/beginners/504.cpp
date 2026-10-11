#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int k;
    cin >> k;
    int shift = k % 3;
    vector<char> ans(3);
    ans[(0 + shift) % 3] = 'G';
    ans[(1 + shift) % 3] = 'C';
    ans[(2 + shift) % 3] = 'V';
    cout << ans[0] << ans[1] << ans[2];
}

signed main() {
    solve();
}
