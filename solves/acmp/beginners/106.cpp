#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    const int N = n;
    for (int i = 0, val; i < N && cin >> val; i++, n -= val);
    cout << min(N - n, n);
}

signed main() {
    solve();
}
