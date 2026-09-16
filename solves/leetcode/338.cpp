#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    vector<int> countBits(int n) {
        vector<int> count(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            if ((i & (i - 1)) == 0) {
                count[i] = 1;
                continue;
            }
            if (i % 2 == 0) count[i] = count[i / 2];
            else count[i] = count[i - 1] + 1;
        }
        
        return count;
    }

};

signed main() {
    int n = 5;
    auto res = Solution().countBits(n);
    for (auto elem : res) cout << elem << ' ';
}