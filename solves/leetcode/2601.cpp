#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class Solution {
public:
    bool primeSubOperation(vector<int>& nums) {
        bool ans = true;
        int n = (int) nums.size();
        int prev = 0;
        vector<int> primes = linear_sieve((int) 2e3);

        for (int i = 0; i < n - 1; i++) {
            int diff = abs(nums[i] - prev);
            int lower_prime = binary_search(diff, primes);
            nums[i] -= lower_prime;
            if (nums[i] <= prev) nums[i] += lower_prime;
            if (nums[i] >= nums[i + 1] || nums[i] == 0) {
                ans = false;
                break;
            }
            prev = nums[i];
        }
        return ans;
    }

    int binary_search(int diff, vector<int>& primes) {
        int l = 0, r = (int) primes.size();
        while (r - l > 1) {
            int mid = l + (r - l) / 2;
            if (primes[mid] < diff) l = mid;
            else r = mid;
        }
        return primes[l];
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
        return primes;
    }
};

signed main() {
    vector<int> v = {13,32,52,58,12,95,51,38,80,27};
    auto res = Solution().primeSubOperation(v);
    cout << res << '\n';
}