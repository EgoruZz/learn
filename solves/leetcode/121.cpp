#include <iostream>
using namespace std;

class Solution {
public:
    int maxProfit(vector<int>& prices) {
        vector<int> profit(prices.size(), 0);
        int mx = 0;
        for (size_t i = 1; i < prices.size(); i++) {
            profit[i] = max(prices[i] - prices[i - 1] + profit[i - 1], profit[i]);
            mx = max(mx, profit[i]);
        }
        return mx;
    }
};

signed main() {
    vector<int> prices = {1};
    auto res = Solution().maxProfit(prices);
    cout << res << endl;
}
