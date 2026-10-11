#include <iostream>
using namespace std;

bool f(int a, int b, int c) {
    return a + b > c;
}

void solve() {
    int x, y, z;
    cin >> x >> y >> z;
    if (f(x, y, z) && f(x, z, y) && f(y, z, x)) cout << "YES";
    else cout << "NO";
}

signed main() {
    solve();
}
