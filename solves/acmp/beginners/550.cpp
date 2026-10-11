#include <iostream>
#include <iomanip>
using namespace std;

void solve() {
    int n;
    cin >> n;
    if ((n % 400 == 0) || (n % 4 == 0 && n % 100 != 0))
        cout << "12/09/" << setfill('0') << setw(4) << n;
    else
        cout << "13/09/" << setfill('0') << setw(4) << n;
}

signed main() {
    solve();
}
