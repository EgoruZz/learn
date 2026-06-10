#include <iostream>
#include <vector>
using namespace std;
class Solution {
public:
    int minimizeSet(int divisor1, int divisor2, int uniqueCnt1, int uniqueCnt2) {
        int ls = 0, rs = LLONG_MAX;
        while (rs - ls > 1) {
            int mid = ls + (rs - ls) / 2;
            if (f(mid, divisor1, divisor2, uniqueCnt1, uniqueCnt2)) rs = mid;
            else ls = mid;
        }
        return rs;
    }
    bool f(int mid, int d1, int d2, int u1, int u2) {
        int free = mid - (mid / d1 + mid / d2) + mid / lcm(d1, d2);
        int second = 0, first = 0;
        if (d1 % d2 != 0) second = mid / d1 - mid / lcm(d1, d2);
        if (d2 % d1 != 0) first = mid / d2 - mid / lcm(d1, d2);
        return (first + free >= u1) and (free + min(first - u1, 0) + second >= u2);
    }
    int lcm(int a, int b) {
        return a * b / gcd(a, b);
    }
    int gcd(int a, int b) {
        while (b > 0) swap(a%= b, b);
        return a;
    }
};

signed main() {
    cout << Solution().minimizeSet(2, 7, 1, 3) << endl;
}