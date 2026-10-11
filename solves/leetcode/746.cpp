#include <iostream>
using namespace std;

class Solution {
public:
    int minCostClimbingStairs(vector<int>& cost) {
        cost.push_back(0);

        vector<int> min_to(cost.size(), 0);
        min_to[0] = cost[0], min_to[1] = min(min_to[0] + cost[1], cost[1]);

        for (size_t i = 2; i < cost.size(); ++i)
            min_to[i] = min(min_to[i - 2], min_to[i - 1]) + cost[i];

        return min_to[cost.size() - 1];
    }
};

void solve() {
    vector<int> cost = {1};
    auto res = Solution().minCostClimbingStairs(cost);
    cout << res;
}

signed main() {
    solve();
}