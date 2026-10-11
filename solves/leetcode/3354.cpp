#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build (const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    int countValidSelections(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        int selections = 0;
        for (size_t i = 0; i < nums.size(); i++) {
            int left_move = p.sum(0, i) - p.sum(i, (int) nums.size());
            int right_move = p.sum(0, i + 1) - p.sum(i + 1, (int) nums.size());
            if ((0 <= left_move && left_move <= 1) && nums[i] == 0) selections++;
            if ((-1 <= right_move && right_move <= 0) && nums[i] == 0) selections++;
        }
        return selections;
    }
};

void solve() {
    vector<int> nums = {16,13,10,0,0,0,10,6,7,8,7};
    auto res = Solution().countValidSelections(nums);
    cout << res;
}

signed main() {
    solve();
}