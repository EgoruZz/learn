#include <iostream>
using namespace std;

void solve() {
    int n;
    cin >> n;

    if (n == 12) n = 0;
    switch (n / 3 * 3) {
        case 0: cout << "Winter"; break;
        case 3: cout << "Spring"; break;
        case 6: cout << "Summer"; break;
        case 9: cout << "Autumn"; break;
        default: cout << "Error";
    }
}

signed main() {
    solve();
}
