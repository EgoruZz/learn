#include <iostream>
using namespace std;

signed main() {
    string pos;
    cin >> pos;

    char hor = pos[0] - 'A' + 1, ver = pos[1];
    (hor % 2 == ver % 2) ? cout << "BLACK" : cout << "WHITE";
}
