#include <iostream>
#include <vector>
using namespace std;

void solve() {
    vector<vector<char>> t(3, vector<char>(3, 0));
    for (int i = 0; i < 3; i++) {
        string s;
        cin >> s;
        for (int j = 0; j < 3; j++) t[i][j] = s[j];
    }
    vector<int> res(4);
    for (int i = 0; i < 3; i++) {
        res[0] = 0, res[1] = 0;
        for (int j = 0; j < 3; j++)
            res[0] += (t[i][j] == 'X') ? 1 : ((t[i][j] == 'O') ? -1 : 0);
        for (int j = 0; j < 3; j++)
            res[1] += (t[j][i] == 'X') ? 1 : ((t[j][i] == 'O') ? -1 : 0);
        if ((res[0] + 3) % 3 == 0 && res[0] != 0 || (res[1] + 3) % 3 == 0 && res[1] != 0) break;
    }
    for (int i = 0; i < 3; i++) {
        res[2] += (t[i][i] == 'X') ? 1 : ((t[i][i] == 'O') ? -1 : 0);
        res[3] += (t[2-i][i] == 'X') ? 1 : ((t[2-i][i] == 'O') ? -1 : 0);
    }
    for (int elem : res) if (elem == -3) { cout << "Lose"; return; }
    for (int elem : res) if (elem == 3) {cout << "Win"; return; }
    cout << "Draw";
}

signed main() {
    solve();
}
