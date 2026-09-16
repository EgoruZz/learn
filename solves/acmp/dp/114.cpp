#include <iostream>
#include <vector>
using namespace std;

int solve(int n, int k) {
    vector<int> number(n, 0);
    number[0] = k - 1;
    number[1] = number[0] * k;
    for (int i = 2; i < n; i++) {
        number[i] = number[i - 1] * (k - 1) + number[i - 2] * (k - 1);
    }
    return number[n - 1];
}

signed main() {
    int n, k;
    cin >> n >> k;
    auto res = solve(n, k);
    cout << res << endl;
}
