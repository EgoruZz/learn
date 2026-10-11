#include <iostream>
using namespace std;

void solve() {
    int n, a, b;
    cin >> n >> a >> b;
    for (int ni = n; ni > 1; ni--) {
        swap(a, b = b - a);
    }
    cout << a << ' ' << b;
}

signed main() {
    solve();
}
