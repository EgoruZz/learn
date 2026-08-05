#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class Solution {
public:
    vector<int> closestPrimes(int left, int right) {
        vector<int> spf = linear_sieve(right);
        vector<int> best = {0, (int) 1e9};

        int l = 0, r = 0;
        for (int i = left; i <= right; i++) {
            if (spf[i] == i) {
                l = r, r = i;
                if (r - l < best[1] - best[0] && l != 0) best = {l , r};
            }
        }

         if (l == 0) return {-1, -1};
         else return best;
    }

    vector<int> linear_sieve(int n) {
        vector<int> spf(n + 1, 0);
        vector<int> primes;
        for (int i = 2; i <= n; i++) {
            if (spf[i] == 0) {
                spf[i] = i;
                primes.push_back(i);
            }
            for (int j = 0; j < (int) primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++) {
                spf[i * primes[j]] = primes[j];
            }
        }
        return spf;
    }
};

signed main() {
    auto res = Solution().closestPrimes(1, 7);
    for (int i = 0; i < (int) res.size(); i++) cout << res[i] << ' ';
}
