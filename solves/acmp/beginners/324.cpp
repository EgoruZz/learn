#include <iostream>
using namespace std;

int my_pow(int a, int exp) {
    int res = 1;
    while (exp > 0) {
        if (exp & 1) res = res * a;
        a = a * a;
        exp >>= 1;
    }
    return res;
}

int get_digit(int pos, int n) {
    return n / my_pow(10, pos) % 10;
}

void solve() {
    int n;
    cin >> n;

    bool yes = true;
    for (int i = 0; i < 2 && yes; i++) {
        if (get_digit(4 - 1 - i, n) != get_digit(i, n)) yes = !yes;
    }

    cout << (yes ? "YES" : "NO");
}

signed main() {
    solve();
}
