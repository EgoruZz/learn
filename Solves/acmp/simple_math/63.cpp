#include <iostream>
using namespace std;

signed main() {
    int s, p;
    cin >> s >> p;

    for (int y = 1; y <= 1000; y++) {
        if (y * y - s * y + p == 0) {
            int x = s - y;
            (x <= y) ? cout << x << ' ' << y : cout << y << ' ' << x;
            break;
        }
    }
}
