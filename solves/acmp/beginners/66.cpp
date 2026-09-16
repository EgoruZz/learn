#include <iostream>
#include <string>
using namespace std;

void solve() {
    char c;
    cin >> c;
    string keyboard = "qwertyuiopasdfghjklzxcvbnm";
    cout << keyboard[(keyboard.find(c) + 1) % keyboard.length()];
}

signed main() {
    solve();
}
