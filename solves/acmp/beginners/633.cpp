#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

void solve() {
    vector<string> team(4);
    for (int i = 0; i < 4; i++) getline(cin, team[i]);
    sort(team.begin() + 1, team.end());

    stringstream ss;
    ss << team[0] << ": " << team[1] << ", " << team[2] << ", " << team[3];
    string full_name = ss.str();

    cout << full_name;
}

signed main() {
    solve();
}