#include <iostream>
using namespace std;

void solve() {
    int n = 100;

    int x = 0; // n^2 - слой, ставим n слоев друг за другом
    // n^2 * n - n^2 - (n-1)^2 - (n-2)^2 - ... - 1^2 = x
    // тогда ответ n^3 - x
    for (int i = 0; i <= n - 1; i++) x += i * (2*n - i);
    int sum_squares = n * n * n - x;

    int square_sum = (n * (n + 1)/ 2) * (n * (n + 1)/ 2);

    cout << square_sum - sum_squares;
    cout << endl;
}

signed main() {
    solve();
}
