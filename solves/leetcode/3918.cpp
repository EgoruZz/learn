#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    int sumOfPrimesInRange(int n) {
        int n_rev = reverse_base(n, 10);
        int mn = min(n, n_rev);
        int mx = max(n, n_rev);

        vector<int> spf = linear_sieve(mx);
        long long sum = 0;
        for (int i = mn; i <= mx; i++) if (spf[i] == i) sum += i;

        return sum;
    }

    int reverse_base(int n, int b) {
        int rev = 0;
        while (n > 0) rev = rev * b + n % b, n /= b;
        return rev;
    }

    vector<int> linear_sieve(int n) {
        vector<int> spf(n + 1, 0);
        vector<int> primes;
        for (int i = 2; i <= n; i++) {
            if (spf[i] == 0) {
                spf[i] = i;
                primes.push_back(i);
            }
            for (int j = 0; j < (int)primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++)
                spf[i * primes[j]] = primes[j];
        }
        return spf;
    }
};

signed main() {
    auto k = 13;
    auto res = Solution().sumOfPrimesInRange(k);
    cout << res << endl;
}
