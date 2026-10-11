#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;
    vector<int> res(n);
    for (int i = 0; i < n; i++) cin >> res[i];
    for (int i = 0, p; i < n; i++) cin >> p, res[i] *= p;
    cout << max_element(res.begin(), res.end()) - res.begin() + 1;
}

signed main() {
    solve();
}
