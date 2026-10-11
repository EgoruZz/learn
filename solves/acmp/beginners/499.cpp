#include <iostream>
#include <vector>
using namespace std;

void solve() {
    int k, w;
    cin >> k >> w;
    vector<int> a(3), b(3);
    for (int i = 0; i < 3; i++) cin >> a[i] >> b[i];
    bool pulse = true;
    for (int i1 = 0; i1 <= 1 && pulse; i1++) {
        for (int i2 = 0; i2 <= 1 && pulse; i2++) {
            for (int i3 = 0; i3 <= 1 && pulse; i3++) {
                int wi = ((i1) ? a[0] : 0) + ((i2) ? a[1] : 0) + ((i3) ? a[2] : 0);
                int ki = ((i1) ? b[0] : 0) + ((i2) ? b[1] : 0) + ((i3) ? b[2] : 0);
                if (ki >= k && wi <= w) pulse = false;
            }
        }
    }
    cout << ((!pulse) ? "YES" : "NO");
}

signed main() {
    solve();
}
