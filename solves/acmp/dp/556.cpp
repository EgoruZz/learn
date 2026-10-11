#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

void solve() {
    int n;
    cin >> n;

    vector<double> p(n), q(n);
    for (int i = 0; i < n; i++) {
        cin >> p[i];
        q[i] = 1 - p[i];
    }

    vector<double> t(n), f(n);
    t[0] = p[0], f[0] = q[0];
    for (int i = 1; i < n; i++) {
        f[i] = f[i - 1] * p[i] + t[i - 1] * q[i];
        t[i] = t[i - 1] * p[i] + f[i - 1] * q[i];
    }
    cout << setprecision(6) << t[n - 1];
}

signed main() {
    solve();
}
