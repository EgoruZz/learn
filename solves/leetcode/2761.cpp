#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    vector<vector<int>> findPrimePairs(int n) {
        auto [x1, y1] = linear_sieve(n);
        vector<int> spf = x1, primes = y1;

        vector<int> use(spf.size(), 0);
        vector<vector<int>> ans;

        for (auto elem : primes) {
            if (spf[n - elem] == n - elem && !use[elem]) {
                use[elem] = use[n - elem] = 1;
                ans.push_back({min(elem, n - elem), max(elem, n - elem)});
            }
        }

        return ans;
    }

    pair<vector<int>, vector<int>> linear_sieve(int n) {
        vector<int> spf(n + 1, 0), primes;
        spf[0] = spf[1] = -1;
        for (int i = 2; i <= n; i++) {
            if (spf[i] == 0) {
                spf[i] = i;
                primes.push_back(i);
            }
            for (int j = 0; j < (int) primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++)
                spf[i * primes[j]] = primes[j];
        }

        return {spf, primes};
    }
};

signed main() {
    int n = 10;
    auto res = Solution().findPrimePairs(n);
    for (auto& p : res) cout << p[0] << ' ' << p[1] << '\n';
}