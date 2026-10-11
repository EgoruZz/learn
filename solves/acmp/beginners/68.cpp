#include <iostream>
#include <string>
using namespace std;

void func() {
    string s;
    cin >> s;
    int x;
    cin >> x;
    cout << (s == "School" && x % 2 == 0 ? "No" : "Yes");
}

signed main() {
    func();
}