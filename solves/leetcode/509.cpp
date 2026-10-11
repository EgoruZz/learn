#include <iostream>
using namespace std;

class Solution {
public:
    int fib(int n) {
        int fi = 0, fj = 1;
        for (int i = 2; i <= n; i++) swap(fi, fj), fj = fi + fj;
        return (n == 0 ? fi : fj);
    }
};

void solve() {
    int n = 0;
    auto res = Solution().fib(n);
    cout << res;
}

signed main() {
    solve();
}