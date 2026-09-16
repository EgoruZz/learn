#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

unordered_map<char, string> command;
vector<vector<int>> memo(26, vector<int>(101, -1));

int func(char dir, int param) {
    if (param == 1) return 1;
    if (memo[dir - 'A'][param] != -1) return memo[dir - 'A'][param];
    int val = 1;
    for (auto symbol : command[dir]) {
        val += func(symbol, param - 1);
    }
    return memo[dir - 'A'][param] = val;
}

void solve() {
    for (char s : string("NSWEUD")) getline(cin, command[s]);
    char dir;
    int param;
    cin >> dir >> param;

    int num = func(dir, param);
    cout << num;
}

signed main() {
    solve();
}
