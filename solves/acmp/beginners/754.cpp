#include <iostream>
using namespace std;

bool f(int mi) {
    return (94 <= mi && mi <= 727);
}

void solve() {
    int m1, m2, m3;
    cin >> m1 >> m2 >> m3;
    if (f(m1) && f(m2) && f(m3)) cout << max(max(m1, m2), m3);
    else cout << "Error";
}

signed main() {
    solve();
}
