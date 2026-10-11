#include <iostream>
using namespace std;

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

void solve() {
    int n = 1e6;
    vector<int> primes = linear_sieve(n);
    cout << primes[10'001 - 1];
    cout << endl;
}

signed main() {
    solve();
}