#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class Solution {
public:
    int distinctPrimeFactors(vector<int>& nums) {
        const int MAX_VAL = 1e3;
        auto spf = linear_sieve(MAX_VAL);

        unordered_set<int> distinct_primes;
        for (auto val : nums) {
            factorize(val, spf, distinct_primes);
        }

        return distinct_primes.size();
    }

    vector<int> linear_sieve(int n) {
        vector<int> spf(n + 1, 0);
        vector<int> primes;
        for (int i = 2; i <= n; i++) {
            if (spf[i] == 0) {
                spf[i] = i;
                primes.push_back(i);
            }
            for (int j = 0; j < (int)primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++) {
                spf[i * primes[j]] = primes[j];
            }
        }
        return spf;
    }

    void factorize(int n, const vector<int>& spf,
        unordered_set<int>& distinct_primes) {
        
        while (n > 1) {
            int p = spf[n];
            while (n % p == 0) n /= p;
            distinct_primes.insert(p);
        }
    }
};

signed main() {
    vector<int> v = {2, 4, 8, 16, 96};
    auto res = Solution().distinctPrimeFactors(v);
    cout << res << endl;
}
