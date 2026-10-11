#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

void solve() {
    int n;
    cin >> n;

    string s;
    long double val, last, l = 30, r = 4000;
    cin >> val;
    last = val;

    for (int i = 1; i < n; i++) {
        cin >> val >> s;
        if (abs(val - last) < 1e-10) {last = val; continue;}
        long double t = (val + last) / 2.0;
        if (s == "closer") (val <= last) ? r = min(r, t) : l = max(l, t);
        else (val <= last) ? l = max(l, t) : r = min(r, t);
        last = val;
    }

    cout << fixed << setprecision(10) << l << ' ' << r;
}

signed main() {
    solve();
}
