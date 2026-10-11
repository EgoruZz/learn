#include <iostream>
#include <string>
using namespace std;

void solve() {
    string a, b, c;
    cin >> a >> b >> c;

    string res = "";

    if (a != "0") res += a;
    if (b != "0") res += (b == "-1" ? "-x" : (b == "1") ? "+x" : (((b[0] == '-') ? "" : "+") + b + "x"));
    if (c != "0") res += (c == "-1" ? "-y" : (c == "1") ? "+y" : (((c[0] == '-') ? "" : "+") + c + "y"));

    if (res.length() == 0) res = "0";
    if (res[0] == '+') res = res.substr(1);

    cout << res;
}

signed main() {
    solve();
}
