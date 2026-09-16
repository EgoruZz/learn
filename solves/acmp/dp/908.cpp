#include <iostream>
#include <unordered_map>
using namespace std;

unordered_map<int, int> memo;

void solve() {
    int n;
    cin >> n;
    memo[n] = 0;

    for (int i = n; i >= 1; i--) {
        memo[i - 1] = (memo.find(i - 1) != memo.end()) ? min(memo[i - 1], memo[i] + 1) : memo[i] + 1;
        if (i % 3 == 0) memo[i / 3] = (memo.find(i / 3) != memo.end()) ? min(memo[i / 3], memo[i] + 1) : memo[i] + 1;
        if (i % 2 == 0) memo[i / 2] = (memo.find(i / 2) != memo.end()) ? min(memo[i / 2], memo[i] + 1) : memo[i] + 1;
    }
    
    cout << memo[1];
}

signed main() {
    solve();
}
