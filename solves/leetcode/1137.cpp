#include <iostream>
using namespace std;

class Solution {
public:
    int tribonacci(int n) {
        vector<int> t = {0, 1, 1};
        int tmp;
        for (int i = 3; i <= n; i++) {
            tmp = t[0] + t[1] + t[2];
            t[0] = t[1], t[1] = t[2], t[2] = tmp;
        }
        return (n == 0 ? t[0] : t[2]);
    }
};

void func() {
    int n = 34;
    auto res = Solution().tribonacci(n);
    cout << res;
}

signed main() {
    func();
}