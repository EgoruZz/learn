#include <iostream>
using namespace std;

signed main() {
    int n, m, y, x;
    cin >> n >> m >> y >> x;

    if (y % 2 == 1) cout << (y - 1) * m - 1 + x;
    else cout << (y - 1) * m + (m - x);
}
