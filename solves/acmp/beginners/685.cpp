#include <iostream>
#include <vector>
using namespace std;

void solve() {
    vector<int> a(3), b(3);
    cin >> a[0] >> a[1] >> a[2] >> b[0] >> b[1] >> b[2];
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    for (int i = 0, k = 0; i <= 3; k += a[i]*b[i], i++) if (i == 3) cout << k;
}

signed main() {
    solve();
}
