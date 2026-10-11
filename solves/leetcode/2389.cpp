#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
    auto begin() const { return p.begin(); }
    auto end() const { return p.end(); }
};

class Solution {
public:
    vector<int> answerQueries(vector<int>& nums, vector<int>& queries) {
        sort(nums.begin(), nums.end());

        PrefixSum1D p;
        p.build(nums);

        vector<int> answer(queries.size(), 0);
        for (int i = 0; i < queries.size(); i++)
            answer[i] = upper_bound(p.begin() + 1, p.end(), queries[i]) - p.begin() - 1;

        return answer;
    }
};

void solve() {
    vector<int> nums = {2,3,4,5};
    vector<int> queries = {1};
    auto res = Solution().answerQueries(nums, queries);
    for (auto elem : res) cout << elem << ' ';
}

signed main() {
    solve();
}
