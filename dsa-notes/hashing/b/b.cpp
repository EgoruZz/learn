#ifndef HASHING_B_CPP
#define HASHING_B_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <functional>
using namespace std;

// =============================================================
// B. МАТЕМАТИЧЕСКИЕ ОСНОВЫ И ТЕОРИЯ
// =============================================================
// Структура md: A. Теоретико-числовые основы
//               → B. Теория вероятностей и статистика
//               → C. Алгебраические структуры
//
// HashingMath — наследует HashingBasics (a.cpp).
// Модульная арифметика, конечные поля, простые числа,
// парадокс дней рождения, χ²-тест, автокорреляция,
// группы/кольца/поля, линейная алгебра, булева алгебра.

#ifndef INSIDE_HASHING_B
#define INSIDE_HASHING_B
#include "../a/a.cpp"
#endif

struct HashingMath : HashingBasics {

// =============================================================
// A. ТЕОРЕТИКО-ЧИСЛОВЫЕ ОСНОВЫ
// =============================================================

// --- A.1. Быстрое возведение в степень по модулю ---
// a^n mod m за O(log n).
long long mod_pow(long long a, long long n, long long m) {
    a %= m;
    long long result = 1;
    while (n > 0) {
        if (n & 1) result = result * a % m;
        a = a * a % m;
        n >>= 1;
    }
    return result;
}

// --- A.2. Обратный элемент по модулю (расширенный Евклид) ---
// a⁻¹ mod m. Возвращает -1 если обратного нет.
long long mod_inverse(long long a, long long m) {
    long long m0 = m, t, q;
    long long x0 = 0, x1 = 1;
    if (m == 1) return 0;
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m0;
    return (a == 1) ? x1 : -1;
}

// --- A.3. GCD (алгоритм Евклида) ---
long long gcd(long long a, long long b) {
    while (b) { a %= b; swap(a, b); }
    return a;
}

// --- A.4. Проверка на простоту (пробное деление) ---
bool is_prime_trial(long long n) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

// --- A.5. Тест Миллера-Рабина ---
// composite → true с вероятностью >= 1 - 4^{-k}.
bool miller_rabin(long long n, int k = 10) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;
    long long d = n - 1;
    int r = 0;
    while (d % 2 == 0) { d /= 2; r++; }
    for (int i = 0; i < k; i++) {
        long long a = 2 + rand() % (n - 3);
        long long x = mod_pow(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool found = false;
        for (int j = 0; j < r - 1; j++) {
            x = x * x % n;
            if (x == n - 1) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

// --- A.6. Следующее простое число ---
long long next_prime(long long n) {
    while (!is_prime_trial(n)) n++;
    return n;
}

// --- A.7. Вероятность коллизии (точная) ---
// P(коллизия) = 1 - prod_{i=0}^{n-1} (1 - i/M).
double exact_collision_prob(int n, long long M) {
    if (n > M) return 1.0;
    double p = 1.0;
    for (int i = 0; i < n; i++)
        p *= (1.0 - (double)i / M);
    return 1.0 - p;
}

// --- A.8. Китайская теорема об остатках ---
// Решение системы x ≡ a[i] (mod m[i]) для попарно взаимно простых m[i].
// Возвращает x mod M, где M = prod m[i]. Возвращает -1 при ошибке.
long long crt(const vector<long long>& a, const vector<long long>& m) {
    int k = (int)a.size();
    long long M = 1;
    for (int i = 0; i < k; i++) M *= m[i];
    long long result = 0;
    for (int i = 0; i < k; i++) {
        long long Mi = M / m[i];
        long long inv = mod_inverse(Mi, m[i]);
        if (inv == -1) return -1;
        result = (result + a[i] * Mi % M * inv) % M;
    }
    return result;
}

// =============================================================
// B. ТЕОРИЯ ВЕРОЯТНОСТЕЙ И СТАТИСТИКА
// =============================================================

// --- B.1. χ²-тест равномерности ---
// observed[i] — наблюдаемое число, expected[i] — ожидаемое.
// Возвращает χ²-статистику.
double chi_squared_test(const vector<int>& observed,
                        const vector<double>& expected) {
    double chi2 = 0;
    for (int i = 0; i < (int)observed.size(); i++) {
        if (expected[i] > 1e-12)
            chi2 += (observed[i] - expected[i]) * (observed[i] - expected[i]) / expected[i];
    }
    return chi2;
}

// --- B.2. Автокорреляция ---
// r(k) = Σ (x_i - mean)(x_{i+k} - mean) / Σ (x_i - mean)^2.
// lag — сдвиг k.
double autocorrelation(const vector<double>& x, int lag) {
    int n = (int)x.size();
    double mean = 0;
    for (double v : x) mean += v;
    mean /= n;
    double num = 0, den = 0;
    for (int i = 0; i < n; i++) {
        double d = x[i] - mean;
        den += d * d;
        if (i + lag < n)
            num += d * (x[i + lag] - mean);
    }
    return (den > 1e-300) ? num / den : 0.0;
}

// --- B.3. Энтропия Шеннона ---
// H = -Σ p_i log2(p_i).
double shannon_entropy(const vector<int>& counts) {
    int total = 0;
    for (int c : counts) total += c;
    double H = 0;
    for (int c : counts) {
        if (c > 0) {
            double p = (double)c / total;
            H -= p * log2(p);
        }
    }
    return H;
}

// --- B.4. Генерация случайных данных для тестирования хешей ---
// Генерирует n случайных long long значений.
vector<long long> random_data(int n, long long max_val) {
    vector<long long> data(n);
    for (int i = 0; i < n; i++)
        data[i] = rand() % max_val;
    return data;
}

// =============================================================
// C. АЛГЕБРАИЧЕСКИЕ СТРУКТУРЫ
// =============================================================

// --- C.1. Проверка: является ли (Z/mZ, +, ·) полем ---
// Поле ⟺ m простое.
bool is_field(long long m) {
    return is_prime_trial(m);
}

// --- C.2. Порядок элемента в группе (Z/mZ, +) ---
// Наименьшее k > 0: k*a ≡ 0 (mod m).
long long element_order(long long a, long long m) {
    a = ((a % m) + m) % m;
    if (a == 0) return 1;
    long long current = a;
    for (long long k = 1; k <= m; k++) {
        if (current == 0) return k;
        current = (current + a) % m;
    }
    return -1;  // не должно произойти
}

// --- C.3. Количество первообразных корней по модулю p ---
// Для простого p: φ(p-1) первообразных корней.
long long count_primitive_roots(long long p) {
    if (!is_prime_trial(p) || p <= 2) return 0;
    long long phi = p - 1;
    long long result = phi;
    for (long long i = 2; i * i <= phi; i++) {
        if (phi % i == 0) {
            while (phi % i == 0) phi /= i;
            result -= result / i;
        }
    }
    if (phi > 1) result -= result / phi;
    return result;
}

// --- C.4. Матричное умножение по модулю ---
// C = A * B по модулю m.
vector<vector<long long>> mat_mul_mod(const vector<vector<long long>>& A,
                                       const vector<vector<long long>>& B,
                                       long long m) {
    int n = (int)A.size();
    vector<vector<long long>> C(n, vector<long long>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                C[i][j] = (C[i][j] + A[i][k] * B[k][j]) % m;
    return C;
}

// --- C.5. Возведение матрицы в степень по модулю ---
// A^n mod m за O(n^3 log p).
vector<vector<long long>> mat_pow_mod(vector<vector<long long>> A,
                                       long long p, long long m) {
    int n = (int)A.size();
    vector<vector<long long>> result(n, vector<long long>(n, 0));
    for (int i = 0; i < n; i++) result[i][i] = 1;  // единичная
    while (p > 0) {
        if (p & 1) result = mat_mul_mod(result, A, m);
        A = mat_mul_mod(A, A, m);
        p >>= 1;
    }
    return result;
}

}; // struct HashingMath

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_B_MAIN
int main() {
    HashingMath hm;
    srand(42);

    cout << "=== A. ТЕОРЕТИКО-ЧИСЛОВЫЕ ОСНОВЫ ===" << endl;

    cout << "--- Модульная арифметика ---" << endl;
    cout << "  2^10 mod 1000000007 = " << hm.mod_pow(2, 10, 1000000007) << endl;
    cout << "  3^-1 mod 7 = " << hm.mod_inverse(3, 7) << " (проверка: 3*" << hm.mod_inverse(3, 7) << " mod 7 = " << (3 * hm.mod_inverse(3, 7)) % 7 << ")" << endl;
    cout << "  gcd(48, 18) = " << hm.gcd(48, 18) << endl;

    cout << "\n--- Простые числа ---" << endl;
    for (long long n : {2, 7, 15, 97, 100, 1000003}) {
        cout << "  " << n << ": " << (hm.is_prime_trial(n) ? "PRIME" : "composite") << endl;
    }
    cout << "  Следующее простое после 100: " << hm.next_prime(100) << endl;
    cout << "  Первых 10 простых: ";
    int cnt = 0;
    for (long long p = 2; cnt < 10; p++) {
        if (hm.is_prime_trial(p)) { cout << p << " "; cnt++; }
    }
    cout << endl;

    cout << "\n--- Китайская теорема об остатках ---" << endl;
    // x ≡ 2 (mod 3), x ≡ 3 (mod 5), x ≡ 2 (mod 7)
    // Решение: x = 23
    vector<long long> a = {2, 3, 2}, m = {3, 5, 7};
    long long x = hm.crt(a, m);
    cout << "  x ≡ 2 (mod 3), x ≡ 3 (mod 5), x ≡ 2 (mod 7)" << endl;
    cout << "  x = " << x << " (ожидаем 23)" << endl;

    cout << "\n--- Вероятность коллизии ---" << endl;
    long long M = 1LL << 16;  // 65536
    for (int n : {10, 100, 256, 500, 1000}) {
        double exact = hm.exact_collision_prob(n, M);
        double approx = 1.0 - exp(-(double)n * (n - 1) / (2.0 * M));
        cout << "  n=" << n << ": exact=" << exact << " approx=" << approx << endl;
    }

    cout << "\n=== B. СТАТИСТИКА ===" << endl;

    cout << "--- χ²-тест для хеш-таблицы ---" << endl;
    // 1000 элементов, 10 бакетов: ожидаемо 100 в каждом
    int n_buckets = 10, n_elements = 1000;
    vector<int> counts(n_buckets, 0);
    for (int i = 0; i < n_elements; i++) {
        long long h = hm.simple_polynomial_hash(to_string(i)) % n_buckets;
        counts[h]++;
    }
    vector<double> expected(n_buckets, (double)n_elements / n_buckets);
    double chi2 = hm.chi_squared_test(counts, expected);
    cout << "  χ² = " << chi2 << " (при df=" << n_buckets - 1 << ")" << endl;
    cout << "  Энтропия = " << hm.shannon_entropy(counts)
         << " бит (макс = " << log2(n_buckets) << ")" << endl;

    cout << "\n--- Автокорреляция хешей ---" << endl;
    vector<double> hashes;
    for (int i = 0; i < 1000; i++)
        hashes.push_back(hm.simple_polynomial_hash(to_string(i)) % 1000);
    for (int lag : {1, 5, 10, 50}) {
        cout << "  r(" << lag << ") = " << hm.autocorrelation(hashes, lag) << endl;
    }

    cout << "\n=== C. АЛГЕБРАИЧЕСКИЕ СТРУКТУРЫ ===" << endl;

    cout << "--- Проверка полей ---" << endl;
    for (long long m : {7, 11, 15, 17, 100}) {
        cout << "  Z/" << m << "Z: " << (hm.is_field(m) ? "ПОЛЕ" : "не поле") << endl;
    }

    cout << "--- Первыеобразные корни ---" << endl;
    for (long long p : {7, 11, 13, 17, 19, 23}) {
        cout << "  mod " << p << ": " << hm.count_primitive_roots(p)
             << " первообразных корней" << endl;
    }

    cout << "\n--- Матричное возведение в степень ---" << endl;
    // Числа Фибоначчи через матрицу: [[1,1],[1,0]]^n
    vector<vector<long long>> F = {{1, 1}, {1, 0}};
    long long MOD = 1000000007;
    for (int n : {5, 10, 20, 50}) {
        auto Fn = hm.mat_pow_mod(F, n, MOD);
        cout << "  F(" << n << ") = " << Fn[0][1] << endl;
    }

    return 0;
}
#endif

#endif // HASHING_B_CPP
