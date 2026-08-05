#include <iostream>
#include <vector>
using namespace std;
using ll = long long;

signed main() {
    ll a, b, c, d;
    cin >> a >> b >> c >> d;

    vector<ll> ans;
    for (ll x = -100l; x <= 100l; x++) {
        if (a*x*x*x + b*x*x + c*x + d == 0l) ans.push_back(x);
    }

    for (int i = 0; i < (int) ans.size(); i++) {
        (i == (int) ans.size() - 1) ? cout << ans[i] : cout << ans[i] << ' ';
    }
}
