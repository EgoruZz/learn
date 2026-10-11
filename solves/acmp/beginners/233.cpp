#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;
    bool crash = false;
    int i = 0, height;
    for (; i < n && !crash; i++) {
        cin >> height;
        if (437 >= height) crash = true;
    }
    (crash) ? (cout << "Crash " << i) : (cout << "No crash");
}

signed main() {
    solve();
}
