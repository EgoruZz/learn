#include <iostream>
using namespace std;
using ll = long long;

void solve() {
    ll n = 600'851'475'143, mx = -1;
    for (ll x = 2; n > 1; x++) {
        if (n % x == 0) mx = x;
        while (n % x == 0) n /= x;
    }
    cout << mx;
    cout << endl;
}

signed main() {
    solve();
}
