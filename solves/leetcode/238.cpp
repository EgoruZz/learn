#include <iostream>
using namespace std;

int rec(const vector<int>& nums, int i, int suff, vector<int>& answer) {
    if (i < 0) return 1;
    int pref = rec(nums, i - 1, nums[i] * suff, answer);
    answer[i] = pref * suff;
    return pref * nums[i];
}


class Solution {
public:
    vector<int> productExceptSelf(vector<int>& nums) {
        vector<int> answer(nums.size());
        rec(nums, (int) nums.size() - 1, 1, answer);
        return answer;
    }
};

void func() {
    vector<int> nums = {1,2,3,4};
    auto res = Solution().productExceptSelf(nums);
    for (auto elem : res) cout << elem << ' ';
}

signed main() {
    func();
}
