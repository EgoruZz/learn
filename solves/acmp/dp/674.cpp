#include <iostream>
#include <unordered_map>

using namespace std;

unordered_map<int, int> memo;

int func(int n) {
    if (n < 3) return 0;
    if (n == 3) return 1;
    if (memo.find(n) != memo.end()) return memo[n];

    int val = func(n / 2) + func(n / 2 + n % 2);

    return memo[n] = val;
}

void solve() {
   int n;
   cin >> n;
   
   int res = func(n);
   cout << res << endl;
}

signed main() {
    solve();
}
