#include <iostream>
#include <vector>
using namespace std;
using ll = long long;

ll solve(int n) {
    vector<ll> fence(n + 1, 0);
    fence[1] = 3;
    for (int i = 2; i <= n; i++) {
        fence[i] = 2 * fence[i - 1];
    }
    return fence[n];
}

signed main() {
    int n;
    cin >> n;
    auto res = solve(n);
    cout << res << endl;
}
