#include <iostream>
#include <vector>
using namespace std;

int gcd_substract(int a, int b) {
    if (a > b) swap(a, b);
    while (b > 0) (a > b) ? a -= b : b -= a;
    return a;
}

int gcd_mod_rec(int a, int b) {
    return (b == 0) ? gcd_mod_rec(b, a % b) : a;
}

int gcd_mod(int a, int b) {
    while (b) swap(a %= b, b);
    return a;
}

int lcm(int a, int b) {
    return a / gcd_mod(a, b) * b;
}

pair<vector<int>, vector<int> > fact_sqrt(int a) {
    vector<int> num((int) log2(a), 0), deg((int) log2(a), 0);
    int x = 2, pos = 0;
    while (a > 1) {
        if (a % x == 0) num[pos] = x;
        while (a % x == 0) {
            deg[pos]++;
            a /= x;
        }
        x++;
        pos++;
    }
    return make_pair(num, deg);
}

signed main() {
    int a, b;
    // a = 5; b = 32;
    cin >> a;

    //cout << gcd_substract(a, b) << '\n';
    //cout << gcd_mod(a, b) << '\n';
    pair<vector<int>, vector<int> > r = fact_sqrt(a);
    vector<int> v1 = r.first, v2 = r.second;
    for (int i = 0; i < (int) v1.size(); i++) cout << v1[i] << " ";
    cout << '\n';
    for (int i = 0; i < (int) v2.size(); i++) cout << v2[i] << " ";
}
