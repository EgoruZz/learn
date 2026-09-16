#include <iostream>
#include <vector>
using namespace std;

int solve(int n) {
    vector<int> pizza(n + 1, 0);
    pizza[0] = 1;
    for (int i = 1; i <= n; i++) {
        pizza[i] = pizza[i - 1] + i;
    }
    
    return pizza[n];
}

signed main() {
    int n;
    cin >> n;
    auto res = solve(n);
    cout << res << endl;
}
