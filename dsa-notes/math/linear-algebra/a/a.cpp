#ifndef LINEAR_ALGEBRA_A_CPP
#define LINEAR_ALGEBRA_A_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <cmath>
using namespace std;

// =============================================================
// A. АЛГЕБРАИЧЕСКИЕ СТРУКТУРЫ И ОСНОВЫ
// =============================================================
// Структура md: A. Множества и операции
//               -> B. Алгебраические структуры (группы, кольца, поля)
//               -> C. Арифметические операции (модулярная арифметика)
//               -> D. Конечные суммы и произведения
//
// AlgebraicBasics — базовый класс всей ветки linear-algebra.
// Все последующие разделы (B-M) строятся на этих примитивах.

// =============================================================
// C. ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ (вне структуры)
// =============================================================

// --- C.1. Быстрое возведение в степень по модулю ---
// a^k mod m за O(log k) умножений.
long long mod_pow(long long a, long long k, long long m) {
    a %= m;
    long long result = 1;
    while (k > 0) {
        if (k & 1) result = result * a % m;
        a = a * a % m;
        k >>= 1;
    }
    return result;
}

// --- C.2. Расширенный алгоритм Евклида ---
// Находит gcd(a, b) и коэффициенты x, y: ax + by = gcd(a, b).
long long ext_gcd(long long a, long long b, long long& x, long long& y) {
    if (b == 0) { x = 1; y = 0; return a; }
    long long x1, y1;
    long long g = ext_gcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

// --- C.3. Обратный элемент по модулю ---
long long mod_inv(long long a, long long m) {
    long long x, y;
    ext_gcd(a, m, x, y);
    return (x % m + m) % m;
}

// --- C.4. НОД (обобщённый для long long) ---
long long gcd_ll(long long a, long long b) {
    while (b) { a %= b; swap(a, b); }
    return a;
}

// =============================================================
// ОСНОВНАЯ СТРУКТУРА
// =============================================================

struct AlgebraicBasics {

// =============================================================
// B. АЛГЕБРАИЧЕСКИЕ СТРУКТУРЫ
// =============================================================

// --- B.1. Модулярная арифметика: группа Z_n ---
struct ModGroup {
    long long n;
    ModGroup(long long n) : n(n) {}
    long long add(long long a, long long b) const { return (a % n + b % n + n) % n; }
    long long sub(long long a, long long b) const { return (a % n - b % n + n) % n; }
    long long mul(long long a, long long b) const { return a % n * (b % n) % n; }
    long long inv(long long a) const { return mod_inv(a, n); }
    long long pow(long long a, long long k) const { return mod_pow(a, k, n); }
};

// --- B.2. Конечное поле GF(p) ---
struct GF {
    long long p;
    GF(long long p) : p(p) { assert(p > 1); }
    long long add(long long a, long long b) const { return (a % p + b % p + p) % p; }
    long long sub(long long a, long long b) const { return (a % p - b % p + p) % p; }
    long long mul(long long a, long long b) const { return a % p * (b % p) % p; }
    long long div(long long a, long long b) const { return mul(a, inv(b)); }
    long long inv(long long a) const { return mod_inv(a, p); }
    long long pow(long long a, long long k) const { return mod_pow(a, k, p); }
};

// =============================================================
// D. КОНЕЧНЫЕ СУММЫ И ПРОИЗВЕДЕНИЯ
// =============================================================

// --- D.1. Сумма арифметической прогрессии ---
long long arithmetic_sum(long long a, long long d, long long n) {
    return n * a + d * n * (n - 1) / 2;
}

// --- D.2. Сумма геометрической прогрессии (по модулю) ---
long long geometric_sum(long long a, long long r, long long n, long long mod) {
    if (r % mod == 1) return n % mod * (a % mod) % mod;
    long long rn = mod_pow(r, n, mod);
    long long inv_r1 = mod_inv((r - 1 + mod) % mod, mod);
    return ((rn - 1 + mod) % mod) * inv_r1 % mod * (a % mod) % mod;
}

// --- D.3. Сумма первых n натуральных ---
long long sum_natural(long long n) { return n * (n + 1) / 2; }

// --- D.4. Сумма квадратов ---
long long sum_squares(long long n) { return n * (n + 1) * (2 * n + 1) / 6; }

// --- D.5. Бином Ньютона: C(n, k) ---
vector<long long> fact_cache = {1};
vector<long long> inv_fact_cache = {1};

void precompute_fact(int max_n, long long mod = (long long)1e9 + 7) {
    fact_cache.resize(max_n + 1);
    inv_fact_cache.resize(max_n + 1);
    fact_cache[0] = 1;
    for (int i = 1; i <= max_n; i++)
        fact_cache[i] = fact_cache[i - 1] * i % mod;
    inv_fact_cache[max_n] = mod_inv(fact_cache[max_n], mod);
    for (int i = max_n - 1; i >= 0; i--)
        inv_fact_cache[i] = inv_fact_cache[i + 1] * (i + 1) % mod;
}

long long binom(int n, int k, long long mod = (long long)1e9 + 7) {
    if (k < 0 || k > n) return 0;
    return fact_cache[n] * inv_fact_cache[k] % mod * inv_fact_cache[n - k] % mod;
}

// --- D.6. Степень матрицы (быстрое возведение) ---
vector<vector<long long>> mat_mul(const vector<vector<long long>>& A,
                                  const vector<vector<long long>>& B,
                                  long long mod) {
    int n = A.size();
    vector<vector<long long>> C(n, vector<long long>(n, 0));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < n; j++)
                C[i][j] = (C[i][j] + A[i][k] * B[k][j]) % mod;
    return C;
}

vector<vector<long long>> mat_pow(vector<vector<long long>> A, long long k,
                                  long long mod) {
    int n = A.size();
    vector<vector<long long>> result(n, vector<long long>(n, 0));
    for (int i = 0; i < n; i++) result[i][i] = 1;
    while (k > 0) {
        if (k & 1) result = mat_mul(result, A, mod);
        A = mat_mul(A, A, mod);
        k >>= 1;
    }
    return result;
}

}; // struct AlgebraicBasics

// =============================================================
// MAIN
// =============================================================
#ifdef LINEAR_ALGEBRA_A_MAIN
int main() {
    AlgebraicBasics ab;

    cout << "=== ModPow ===" << endl;
    cout << "2^10 mod 1e9+7 = " << mod_pow(2, 10, 1000000007) << endl;

    cout << "\n=== ModInv ===" << endl;
    cout << "3^(-1) mod 7 = " << mod_inv(3, 7) << endl;

    cout << "\n=== ModGroup Z_7 ===" << endl;
    AlgebraicBasics::ModGroup g7(7);
    cout << "3 + 5 mod 7 = " << g7.add(3, 5) << endl;
    cout << "3 * 5 mod 7 = " << g7.mul(3, 5) << endl;

    cout << "\n=== GF(7) ===" << endl;
    AlgebraicBasics::GF gf7(7);
    cout << "3 / 2 mod 7 = " << gf7.div(3, 2) << endl;

    cout << "\n=== Суммы ===" << endl;
    cout << "sum_natural(10) = " << ab.sum_natural(10) << endl;
    cout << "sum_squares(10) = " << ab.sum_squares(10) << endl;
    cout << "arith_sum(1,2,5) = " << ab.arithmetic_sum(1, 2, 5) << endl;

    cout << "\n=== Биномы ===" << endl;
    ab.precompute_fact(20);
    cout << "C(10,3) = " << ab.binom(10, 3) << endl;

    cout << "\n=== Матричное возведение ===" << endl;
    vector<vector<long long>> F = {{1,1},{1,0}};
    auto F5 = ab.mat_pow(F, 5, 1000000007);
    cout << "F(5) = " << F5[0][1] << endl;

    return 0;
}
#endif

#endif // LINEAR_ALGEBRA_A_CPP
