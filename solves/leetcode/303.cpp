#include <iostream>
#include <vector>
using namespace std;

class NumArray {
public:
    vector<int> pref;
    NumArray(vector<int>& nums) {
        pref.assign(nums.size() + 1, 0);
        for (size_t i = 0; i < nums.size(); ++i)
            pref[i + 1] = pref[i] + nums[i];
    }
    
    int sumRange(int left, int right) {
        return pref[right + 1] - pref[left];
    }
};

/**
* Your NumArray object will be instantiated and called as such:
* NumArray* obj = new NumArray(nums);
* int param_1 = obj->sumRange(left,right);
*/

void solve() {
    vector<int> nums = {-2,0,3,-5,2,-1};
    NumArray* obj = new NumArray(nums);
    int param_1 = obj -> sumRange(0, 2 + 1);
    int param_2 = obj -> sumRange(2, 5 + 1);
    int param_3 = obj -> sumRange(0, 5 + 1);
    cout << param_1 << ' ' << param_2 << ' ' << param_3;
}

signed main() {
    solve();
}
