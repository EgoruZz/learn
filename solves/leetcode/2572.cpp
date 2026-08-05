#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class Solution {
public:
    int squareFreeSubsets(vector<int>& nums) {
        vector<int> mu = mobius_sieve(30);
        for (auto elem : mu) cout << elem << ' ';
        cout << '\n';

        const int MOD = (int) 1e9 + 7;
        int answer = 0;
        for (int i = 0; i < (int) nums.size(); i++) {
            if (mu[nums[i]] != 0) {
                int prod = nums[i], cnt = 1;
                for (int j = i + 1; j < (int) nums.size(); j++) {
                    if (mu[prod * nums[j]] != 0) prod *= nums[i], cnt++;
                }
                answer = (powmod(2, cnt, MOD) - 1 + MOD) % MOD;
                break;
            }
        }

        return answer;
    }

    int powmod(int a, int exp, int mod) {
        a %= mod;
        int result = 1;
        while (exp > 0) {
            if (exp & 1) result = result * a % mod;
            a = a * a % mod;
            exp >>= 1;
        }
        return result;
    }

    vector<int> mobius_sieve(int n) {
        vector<int> spf(n + 1, 0), mu(n + 1, 0);
        vector<int> primes;
        mu[1] = 1;
        for (int i = 2; i <= n; i++) {
            if (spf[i] == 0) {
                spf[i] = i;
                primes.push_back(i);
                mu[i] = -1;
            }
            for (int j = 0; j < (int) primes.size() && primes[j] < spf[i] && i * primes[j] <= n; j++) {
                int ip = i * primes[j];
                spf[ip] = primes[j];
                if (spf[i] == primes[j]) mu[ip] = 0;
                else mu[ip] = -mu[i];
            }
        }
        return mu;
    }
};

signed main() {
    vector<int> v = {3, 7, 12, 14};
    auto res = Solution().squareFreeSubsets(v);
    cout << res << '\n';
}