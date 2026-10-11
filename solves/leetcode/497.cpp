#include <vector>
#include <iostream>
#include <chrono>
#include <random>
using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

long long getRandom(long long l, long long r) {
    uniform_int_distribution<long long> dist(l, r);
    return dist(rng);
}

class Solution {
public:
    vector<vector<int>> r;
    vector<int> p;

    Solution(vector<vector<int>>& rects) {
        r = rects;
        p.assign(r.size() + 1, 0);
        for (size_t i = 0; i < r.size(); ++i)
            p[i + 1] = p[i] + (abs(rects[i][0] - rects[i][2]) + 1) * (abs(rects[i][1] - rects[i][3]) + 1);
    }
    
    vector<int> pick() {
        int n = p[r.size()];
        int rand = getRandom(1, n);
        int idx = lower_bound(p.begin(), p.end(), rand) - p.begin() - 1;
        int i = r[idx][3] - (rand - p[idx] - 1) / (abs(r[idx][0] - r[idx][2]) + 1);
        int j = r[idx][0] + (rand - p[idx] - 1) % (abs(r[idx][0] - r[idx][2]) + 1);
        return {j, i};
    }
};

void func() {
    vector<vector<int>> rects = {{-2, -2, 1, 1}, {2, 2, 4, 6}};
    Solution* obj = new Solution(rects);
    auto res = {obj -> pick(), obj -> pick(), obj -> pick()};
    for (auto&& elem : res) cout << elem[0] << ' ' << elem[1] << ' ';
}

signed main() {
    func();
}

/**
* Your Solution object will be instantiated and called as such:
* Solution* obj = new Solution(rects);
* vector<int> param_1 = obj->pick();
*/