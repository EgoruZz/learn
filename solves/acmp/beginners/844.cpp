#include <iostream>
#include <cmath>
using namespace std;

void solve() {
    long long a, b;
    cin >> a >> b;
    long long root = sqrt(a * b);
    cout << ((root * root == a * b) ? root : 0);
}

signed main() {
    solve();
}
