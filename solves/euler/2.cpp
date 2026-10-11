#include <iostream>
using namespace std;

void solve() {
    int f3 = 2;
    vector<int> a;
    a.push_back(f3), a.push_back(4 * f3);
    int s = a[0] + a[1];
    for (int i = 2;; i++) {
        int res = 4 * a[i - 1] + a[i - 2];
        if (res > 4e6) break;
        a.push_back(res), s += res;
    }
    cout << s;
    cout << endl;
}

signed main() {
    solve();
}
