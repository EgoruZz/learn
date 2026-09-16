#include <iostream>
using namespace std;

void func(int n, bool& up) {
    if (n == 0) {
        up = true;
        return;
    }
    int val;
    if (!up) cin >> val;
    func(n - 1, up);
    if (up) cout << val << ' ';
}

void solve() {
    int n;
    cin >> n;

    bool up = false;
    func(n, up);
}

signed main() {
    solve();
}
