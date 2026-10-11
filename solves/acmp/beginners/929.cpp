#include <iostream>
using namespace std;

void solve() {
    long long n;
    cin >> n;
    cout << n / 6 + 7 - ((n % 6 == 0) ? 7 : n % 6) << ' ' << n * 6;
}

signed main() {
    solve();
}
