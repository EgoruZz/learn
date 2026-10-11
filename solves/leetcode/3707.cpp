#include <iostream>
using namespace std;

struct PrefixSum1D {
    vector<int> p;

    void build(const string& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); ++i) p[i + 1] = p[i] + (a[i] - 'a' + 1);
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

class Solution {
public:
    int scoreBalance(string s) {
        PrefixSum1D p;
        p.build(s);
        int n = s.length();
        bool is = false;
        for (int i = 0; !is && i < n - 1; i++)
            if (p.sum(0, i + 1) == p.sum(i + 1, n)) is = !is;
        return is;
    }
};

void func() {
    string s = "abdcd";
    auto res = Solution().scoreBalance(s);
    cout << res;
}

signed main() {
    func();
}
