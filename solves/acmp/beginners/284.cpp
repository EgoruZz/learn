#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int n;
    cin >> n;
    vector<int> a(n);
    for (int i = 0; i < n; i++) cin >> a[i];

    int m;
    cin >> m;
    for (int mi = 0; mi < m; mi++) {
        int i, j;
        cin >> i >> j;
        i--, j--;
        for (int pos = i; pos <= j; pos++) cout << a[pos] << ' ';
        cout << '\n';
    }
}

signed main() {
    solve();
}
