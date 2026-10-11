#include <iostream>
#include <cmath>
using namespace std;

int digit(int pos, int n) {
    return (n / (int) pow(10, pos)) % 10;
}

void solve() {
    int n;
    cin >> n;
    int half1 = 0, half2 = 0;
    for (int i = 0; i < 3; i++) {
        half1 += digit(i, n);
        half2 += digit(6 - 1 - i, n);
    }

    cout << ((half1 == half2) ? "YES" : "NO");
}

signed main() {
    solve();
}
