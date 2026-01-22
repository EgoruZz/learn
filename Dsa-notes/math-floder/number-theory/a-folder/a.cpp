#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;


int gcd_diff(int a, int b) {
    if (a > b) swap(a, b);
    while (b > 0) (a > b) ? a -= b : b -= a;
    return a;
}

int gcd_mod(int a, int b) {
    while (b) swap(a %= b, b);
    return a;
}

int gcd_mod_rec(int a, int b) {
    return (b == 0) ? gcd_mod_rec(b, a % b) : a;
}

// Используйте эту версию, если компилятор GCC/Clang:
unsigned int stein(unsigned int a, unsigned int b) {
    if (a == 0) return b;
    if (b == 0) return a;

    int shift = __builtin_ctz(a | b);
    a >>= __builtin_ctz(a);

    do {
        b >>= __builtin_ctz(b);
        if (a > b) std::swap(a, b);
        b -= a;
    } while (b);

    return a << shift;
}

int lcm(int a, int b) {
    return a / gcd_mod(a, b) * b;
}

vector<int> divide(int a) {
    vector<int> divs, sort;
    for (int x = 1; x * x <= a; x++) {
        if (a % x == 0) divs.push_back(x);
        if (a % x == 0 && a / x != x) divs.push_back(a / x);
    }
    for (int i = 0; i < (int) divs.size(); i += 2) sort.push_back(divs[i]);
    for (int i = (int) divs.size() - 1; i >= 1; i -= 2) sort.push_back(divs[i]);
    return sort;
}

pair<vector<int>, vector<int> > fact(int a) {
    vector<int> primes((int) log2(a), 0), powers((int) log2(a), 0);
    int x = 2, i = 0;
    while (a > 1) {
        for (; a % x == 0; a /= x, powers[i]++);
        (powers[i] == 0) ? x++: primes[i++] = x++;
    }
    return make_pair((primes.resize(i), std::move(primes)), (powers.resize(i), std::move(powers)));
}

int mu(int n) {
    auto [primes, powers] = fact(n);
    for (auto ai : powers) if (ai >= 2) return 0;
    return (primes.size() % 2 == 0) ? 1 : -1;
}

int fi(int n) {
    int result = 0;
    for (auto d : divide(n)) result += mu(d) * (n / d);
    return result;
}

int d(int n, int k) {
    vector<int> keys = divide(n);
    unordered_map<int, vector<int> > info;
    for (auto key : keys) info[key] = divide(key);

    unordered_map<int, int> di;
    for (auto key : keys) di[key] = 1;

    while (k--) {
        unordered_map<int, int> dj;
        for (auto key : keys) {
            for (auto div : info[key]) {
                dj[key] += di[div];
            }
        }
        di = dj;
    }

    return di[n];
}

signed main() {
    int a, b;
    // cin >> a >> b;

    cout << d(12, 5) << endl;
}
