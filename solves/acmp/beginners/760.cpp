#include <iostream>
#include <iomanip>
using namespace std;

void solve() {
    double n, v, l;
    cin >> n >> v >> l;

    double time = l / v * 60;
    for (int i = 0, xi, ti; i < n; i++) cin >> xi >> ti, time += ti;
    
    cout << fixed << setprecision(2) << time;
}

signed main() {
    solve();
}
