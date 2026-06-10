#include <iostream>
using namespace std;
using ull = unsigned long long;

signed main() {
    ull n, m, d, k;
    cin >> n >> m >> d >> k;
    ull s = k * k - (k - d * m) * (k - d * n);
    cout << s;
}
