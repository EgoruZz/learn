#include <iostream>
#include <string>
using namespace std;

struct bestPerson {
    string name = "";
    int val = 1e9;
};

void solve() {
    int n, m;
    cin >> n >> m;
    bestPerson best_person;
    for (int i = 0; i < n; i++) {
        string name;
        cin >> name;
        int val = 0;
        for (int j = 0, number; j < m; j++) {
            cin >> number;
            val += number;
        }
        if (best_person.val > val) best_person.val = val, best_person.name = name;
    }
    cout << best_person.name;
}

signed main() {
    solve();
}
