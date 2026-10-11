#include <iostream>
#include <iomanip>
using namespace std;

void solve() {
    int h_start, m_start;
    char c;
    int h, m;
    cin >> h_start >> c >> m_start >> h >> m;
    cout << setfill('0') << setw(2) << (h_start + h + (m_start + m) / 60) % 24 << ':' << setfill('0') << setw(2) << (m + m_start) % 60;
}

signed main() {
    solve();
}
