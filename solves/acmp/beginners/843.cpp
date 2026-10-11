#include <iostream>
using namespace std;

void func() {
    int n, m, f, l;
    cin >> n >> m >> f >> l;
    int a = n - l - (n - l - f) - (n - l - m);
    cout << a << ' ' << n - l - f << ' ' << n - l - m;
}

signed main() {
    func();
}