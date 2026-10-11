#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    vector<int> leftRightDifference(vector<int>& nums) {
        PrefixSum1D p;
        p.build(nums);
        vector<int> answer(nums.size());
        int n = (int) nums.size();
        for (int i = 0; i < n; i++) answer[i] = abs(p.sum(0, i) - p.sum(i + 1, n));
        return answer;
    }
};

void solve() {
    vector<int> nums = {1};
    auto res = Solution().leftRightDifference(nums);
    for (auto elem : res )cout << elem << ' ';
}

signed main() {
    solve();
}