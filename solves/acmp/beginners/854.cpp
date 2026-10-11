#include <iostream>
#include <string>
using namespace std;

void solve() {
    int t_room, t_cond;
    cin >> t_room >> t_cond;

    string power;
    cin >> power;
    
    int delta = t_cond - t_room;
    if (delta > 0) cout << ((power == "heat" || power == "auto") ? t_cond : t_room);
    else if (delta < 0) cout << ((power == "freeze" || power == "auto") ? t_cond : t_room);
    else cout << t_room;
}

signed main() {
    solve();
}
