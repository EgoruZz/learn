#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <functional>
using namespace std;
typedef long long ll;
typedef __int128 lll;

// Подключаем b.cpp (наследует a.cpp через Divisibility)
#define DISCRETE_LOG_MAIN

#include "../b-folder/b.cpp"

// =============================================================
// F. ПРОСТЫЕ ЧИСЛА
// =============================================================
// Наследует ModArithmetic из b.cpp (который наследует Divisibility из a.cpp)
// Содержит:
//   A. Тесты простоты                 is_prime_naive, is_prime, is_prime_probabilistic,
//                                    solovay_strassen_test, bpsw_test
//   B. Факторизация                   fermat, pollard_p1, pollard_rho, factorize_rho,
//                                    bent_method, monte_carlo_pollard
//   C. Решета                         sieve_sundaram, segmented_sieve, sieve_atkin
//                                    (решето Эратосфена и линейное решето в a.cpp)
//   D. Распределение простых          prime_count_approx, bertrand_check
//   E. Специальные классы            is_sophie_germain, twin_primes, lucas_lehmer

struct PrimeNumbers : ModArithmetic {

// =============================================================
// A. ТЕСТЫ ПРОСТОТЫ
// =============================================================

// --- A.1. Наивный метод O(√n) ---
bool is_prime_naive(ll n) {
    if (n < 2) return false;
    for (ll i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

// A.2. Тест Вильсона — не реализован (теоретический, O(p) по времени)

// --- A.3. Тест Миллера-Рабина (вероятностный) ---
bool is_prime_probabilistic(ll n, int k = 20) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<ll> dist(2, n - 2);

    ll d = n - 1;
    int r = 0;
    while (d % 2 == 0) { d /= 2; r++; }

    for (int i = 0; i < k; i++) {
        ll a = dist(rng);
        ll x = powmod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool found = false;
        for (int j = 0; j < r - 1; j++) {
            x = (lll)x * x % n;
            if (x == n - 1) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

// --- A.4. Детерминированный Миллер-Рабин (n < 3·10¹⁸) ---
bool is_prime(ll n) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (ll p : {2,3,5,7,11,13,17,19,23,29,31,37})
        if (n == p) return true;
    if (n < 41) return false;

    ll d = n - 1;
    int r = 0;
    while (d % 2 == 0) { d /= 2; r++; }

    for (ll a : {2,3,5,7,11,13,17,19,23,29,31,37}) {
        if (a >= n) continue;
        ll x = powmod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool found = false;
        for (int i = 0; i < r - 1; i++) {
            x = (lll)x * x % n;
            if (x == n - 1) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

// --- A.5. Тест Соловея-Штрассена ---
bool solovay_strassen_test(ll n, ll a) {
    if (n % a == 0) return n == a;
    ll x = powmod(a, (n - 1) / 2, n);
    ll j = jacobi(a, n);
    if (j == 0) return false;
    return x == (j + n) % n;
}

// --- A.6. Тест Агравала-Каяла-Саксены (AKS) ---
// Первый детерминированный полиномиальный тест O(log^6 n)
// Теоретически важен, но на практике медленнее Миллера-Рабина
// Не реализован

// --- A.7. Тест BPSW (Baillie-PSW) ---
// Комбинация: Миллер-Рабин(a=2) + сильный тест Люка
// Нет известных counterexamples для n < 2⁶⁴
bool bpsw_test(ll n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    if (n == 3 || n == 5) return true;

    // Шаг 1: Миллер-Рабин с a=2
    ll d = n - 1;
    int r = 0;
    while (d % 2 == 0) { d /= 2; r++; }
    ll x = powmod(2, d, n);
    if (x != 1 && x != n - 1) {
        bool found = false;
        for (int i = 0; i < r - 1; i++) {
            x = (lll)x * x % n;
            if (x == n - 1) { found = true; break; }
        }
        if (!found) return false;
    }

    // Шаг 2: Найти D ≡ 1 (mod 4) such that (D/n) = -1
    ll D = 5;
    while (D < 4 * n) {
        if (D % 4 == 1) {
            ll j = jacobi(D, n);
            if (j == -1) break;
            if (j == 0) return false;
        }
        D += 4;
    }
    if (D >= 4 * n) return false;

    // Шаг 3: Сильный тест Люка с P=1, Q=(1-D)/4
    ll P = 1;
    ll Q = ((1 - D) / 4) % n; if (Q < 0) Q += n;

    // Разложить n+1 = dd * 2^ss, dd нечётное
    ll dd = n + 1;
    int ss = 0;
    while (dd % 2 == 0) { dd /= 2; ss++; }

    // Вычислить U_dd, V_dd через бинарное разложение dd
    int bits[64], len = 0;
    ll tmp = dd;
    while (tmp > 0) { bits[len++] = tmp & 1; tmp >>= 1; }

    ll U = 1, V = P;
    ll Qk = Q;
    for (int i = len - 2; i >= 0; i--) {
        // Удвоение: U_{2m} = U_m * V_m, V_{2m} = V_m² - 2*Q^m
        ll U2 = ((lll)U * V) % n;
        ll V2 = (((lll)V * V) % n - 2 * Qk) % n;
        if (V2 < 0) V2 += n;
        Qk = ((lll)Qk * Qk) % n;
        U = U2; V = V2;

        // Если бит установлен — прибавляем 1
        if (bits[i]) {
            ll Dval = (P * P - 4 * Q) % n; if (Dval < 0) Dval += n;
            ll Ua = P * U + V; if (Ua & 1) Ua += n; Ua /= 2;
            ll Va = Dval * U + P * V; if (Va & 1) Va += n; Va /= 2;
            U = Ua % n; V = Va % n;
            Qk = ((lll)Qk * Q) % n;
        }
    }

    // U_dd ≡ 0 (mod n) — проходит
    if (U % n == 0) return true;

    // Проверить V_{dd·2^r} ≡ 0 (mod n) для r = 0..ss-1
    ll Qk_check = powmod(Q, dd, n);
    for (int i = 0; i < ss; i++) {
        if (V % n == 0) return true;
        V = (((lll)V * V) % n - 2 * Qk_check + 2 * n) % n;
        Qk_check = ((lll)Qk_check * Qk_check) % n;
    }

    return false;
}

// =============================================================
// B. ФАКТОРИЗАЦИЯ
// =============================================================
// B.1. Тривиальное деление O(√n) — Divisibility::factorize(n, spf) из a.cpp

// --- B.2. Метод Ферма (n = a² - b²) ---
pair<ll, ll> fermat(ll n) {
    ll a = (ll)ceil(sqrt((double)n));
    ll b2 = a * a - n;
    while ((ll)sqrt((double)b2) * (ll)sqrt((double)b2) != b2) {
        a++;
        b2 = a * a - n;
    }
    ll b = (ll)sqrt((double)b2);
    return {a - b, a + b};
}

// --- B.3. Алгоритм Полларда p-1 ---
ll pollard_p1(ll n, ll B = 10000) {
    ll a = 2;
    for (ll j = 2; j <= B; j++)
        a = powmod(a, j, n);
    ll g = gcd_mod(a - 1, n);
    return (g > 1 && g < n) ? g : -1;
}

// --- B.4. Rho-алгоритм Полларда ---
ll pollard_rho(ll n) {
    if (n % 2 == 0) return 2;

    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<ll> dist(1, n - 1);

    while (true) {
        ll x = dist(rng), y = x, c = dist(rng), d = 1;
        auto f = [&](ll val) { return ((lll)val * val + c) % n; };
        while (d == 1) {
            x = f(x);
            y = f(f(y));
            d = gcd_mod(abs(x - y), n);
        }
        if (d != n) return d;
    }
}

// Вспомогательная функция: полная факторизация через Pollard Rho
vector<ll> factorize_rho(ll n) {
    vector<ll> factors;
    if (n <= 1) return factors;

    while (n % 2 == 0) { factors.push_back(2); n /= 2; }
    while (n % 3 == 0) { factors.push_back(3); n /= 3; }

    function<void(ll)> factor = [&](ll m) {
        if (m == 1) return;
        if (is_prime(m)) { factors.push_back(m); return; }
        ll d = pollard_rho(m);
        factor(d);
        factor(m / d);
    };

    if (n > 1) factor(n);
    sort(factors.begin(), factors.end());
    return factors;
}

// --- B.5. Метод Бента ---
// Модификация Rho с цепочкой Дики
// Использует h(x) = x² + c mod n, ищет gcd через цепочку
ll bent_method(ll n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<ll> dist(1, n - 1);

    while (true) {
        ll y = dist(rng), c = dist(rng), m = dist(rng);
        ll g = 1, q = 1, x = y, ys = 0;
        while (g == 1) {
            x = y;
            for (ll i = 0; i < m; i++)
                y = ((lll)y * y + c) % n;
            for (ll k = 0; k < m && g == 1; k += 128) {
                ys = y;
                for (ll i = 0; i < min(m - k, 128LL); i++) {
                    y = ((lll)y * y + c) % n;
                    ll d = abs(x - y);
                    if (d != 0) g = gcd_mod(d, n);
                }
                q *= g;
                q %= n;
            }
            g = gcd_mod(q, n);
        }
        if (g == n) {
            g = 1;
            y = ys;
            while (g == 1) {
                y = ((lll)y * y + c) % n;
                ll d = abs(x - y);
                if (d != 0) g = gcd_mod(d, n);
            }
        }
        if (g != n) return g;
    }
}

// --- B.6. Метод Монте-Карло Полларда ---
// Случайные точки в пространстве вычетов, gcd через случайные попарные разности
ll monte_carlo_pollard(ll n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<ll> dist(1, n - 1);

    const int TRIES = 100;
    vector<ll> x(TRIES);
    for (int i = 0; i < TRIES; i++)
        x[i] = dist(rng);

    for (int i = 0; i < TRIES; i++)
        for (int j = i + 1; j < TRIES; j++) {
            ll d = gcd_mod(abs(x[i] - x[j]), n);
            if (1 < d && d < n) return d;
        }
    return -1;
}

// =============================================================
// C. РЕШЕТА
// =============================================================
// C.1. Решето Эратосфена O(N log log N) — Divisibility::sieve(n) из a.cpp
// C.2. Линейное решето O(N) — Divisibility::linear_sieve(n) из a.cpp

// --- C.3. Решето Сундарама ---
vector<bool> sieve_sundaram(int n) {
    vector<bool> mark(n + 1, true);
    for (int i = 1; i <= n; i++)
        for (int j = i; i + j + 2 * i * j <= n; j++)
            mark[i + j + 2 * i * j] = false;
    return mark;
}

// --- C.4. Сегментированное решето ---
vector<bool> segmented_sieve(ll L, ll R) {
    ll len = R - L + 1;
    vector<bool> is_prime_seg(len, true);

    ll limit = (ll)sqrt((double)R) + 1;
    vector<bool> small_sieve(limit + 1, true);
    small_sieve[0] = small_sieve[1] = false;
    for (ll i = 2; i * i <= limit; i++)
        if (small_sieve[i])
            for (ll j = i * i; j <= limit; j += i)
                small_sieve[j] = false;

    for (ll p = 2; p <= limit; p++) {
        if (!small_sieve[p]) continue;
        ll start = max(p * p, ((L + p - 1) / p) * p);
        for (ll j = start; j <= R; j += p)
            is_prime_seg[j - L] = false;
    }

    if (L == 0) is_prime_seg[0] = false;
    if (L == 1) is_prime_seg[0] = false;

    return is_prime_seg;
}

// --- C.5. Нечётное решето (Odd Sieve) O(N log log N) ---
// Идея: хранить только нечётные числа → память в 2 раза меньше
// Индекс i соответствует числу 2i+1 (i=0 → 1, i=1 → 3, i=2 → 5, ...)
//
// Алгоритм:
//   is_odd[i] = true ⟹ 2i+1 простое (или не проверено)
//   Для каждого нечётного простого p = 2j+1:
//     Начинаем с p² (первое составное кратное p, не помеченное ранее)
//     Индекс p²: (p²-1)/2 = 2j²+2j
//     Шаг: p (в индексах, т.к. следующее нечётное кратное = p²+2p → индекс +p)
//
// Сложность: O(N log log N) — как Эратосфен, но память O(N/2)
//
// Возвращает is_prime где is_prime[i] == true ⟺ (2i+1) простое
// (для i=0: 2·0+1=1 — не простое, помечаем false)
vector<bool> odd_sieve(ll n) {
    ll size = n / 2;  // количество нечётных чисел от 1 до n
    vector<bool> is_prime(size, true);
    is_prime[0] = false;  // 1 — не простое

    // Перебираем нечётные простые p = 2j+1
    // p² ≤ n ⟺ j ≤ (√n - 1)/2
    for (ll j = 1; 2 * j + 1 <= n; j++) {
        if (!is_prime[j]) continue;

        ll p = 2 * j + 1;
        // Начинаем с p² — индекс (p²-1)/2 = 2j²+2j
        ll start = 2 * j * j + 2 * j;
        // Помечаем составные: p², p²+2p, p²+4p, ...
        // В индексах: start, start+p, start+2p, ...
        for (ll idx = start; idx < size; idx += p)
            is_prime[idx] = false;
    }

    return is_prime;
}

// --- C.6. Решето Аткина-Берлестайна ---
// Сложность: O(N / log log N) — медленнее Эратосфена на практике
//
// Основа: квадратичные формы:
//   n = 4x² + y²  — решение нечётное → инвертировать
//   n = 3x² + y²  — решение нечётное → инвертировать (для x нечётное)
//   n = 3x² - y²  — решение нечётное → инвертировать (для x > y)
//
// После подсчёта решений: простое ⟺ количество решений нечётное
vector<bool> sieve_atkin(ll n) {
    vector<bool> is_prime(n + 1, false);
    if (n >= 2) is_prime[2] = true;
    if (n >= 3) is_prime[3] = true;

    for (ll x = 1; x * x <= n; x++) {
        for (ll y = 1; y * y <= n; y++) {
            ll k;

            // 4x² + y²
            k = 4 * x * x + y * y;
            if (k <= n && (k % 12 == 1 || k % 12 == 5))
                is_prime[k] = !is_prime[k];

            // 3x² + y² (x нечётное)
            k = 3 * x * x + y * y;
            if (x % 2 == 0 && k <= n && k % 12 == 7)
                is_prime[k] = !is_prime[k];

            // 3x² - y² (x > y)
            if (x > y) {
                k = 3 * x * x - y * y;
                if (k <= n && k % 12 == 11)
                    is_prime[k] = !is_prime[k];
            }
        }
    }

    // Удалить кратные квадратов простых
    for (ll r = 5; r * r <= n; r++) {
        if (is_prime[r]) {
            for (ll k = r * r; k <= n; k += r * r)
                is_prime[k] = false;
        }
    }

    return is_prime;
}

// =============================================================
// D. РАСПРЕДЕЛЕНИЕ ПРОСТЫХ
// =============================================================

// --- D.1. Приближение π(x) ~ x/ln(x) ---
ll prime_count_approx(ll x) {
    if (x <= 1) return 0;
    return (ll)(x / log((double)x));
}

// --- D.2. Проверка постулата Бертрана (дополнительная функция, не в f.md) ---
bool bertrand_check(ll n) {
    for (ll p = n + 1; p < 2 * n; p++)
        if (is_prime(p)) return true;
    return false;
}

// D.2. Формула Римана (точная) — не реализована (теоретическая)
// D.3. Свойства простых — не реализованы (теоретические)

// =============================================================
// E. СПЕЦИАЛЬНЫЕ КЛАССЫ ПРОСТЫХ
// =============================================================

// --- E.1. Простые Софи Жермен ---
bool is_sophie_germain(ll p) {
    return is_prime(p) && is_prime(2 * p + 1);
}

// --- E.2. Простые-близнецы ---
vector<pair<ll, ll>> twin_primes(ll limit) {
    vector<pair<ll, ll>> twins;
    vector<bool> sieve_arr(limit + 1, true);
    sieve_arr[0] = sieve_arr[1] = false;
    for (ll i = 2; i * i <= limit; i++)
        if (sieve_arr[i])
            for (ll j = i * i; j <= limit; j += i)
                sieve_arr[j] = false;

    for (ll i = 2; i + 2 <= limit; i++)
        if (sieve_arr[i] && sieve_arr[i + 2])
            twins.push_back({i, i + 2});
    return twins;
}

// --- E.3. Тест Люка-Лемера для чисел Мерсенна M_p = 2^p - 1 ---
bool lucas_lehmer(ll p) {
    if (p == 2) return true;
    ll m = (1LL << p) - 1;
    ll s = 4;
    for (ll i = 0; i < p - 2; i++)
        s = ((lll)s * s - 2) % m;
    return s == 0;
}

}; // конец struct PrimeNumbers

// =============================================================
// ТЕСТЫ
// =============================================================
#ifndef PRIME_NUMBERS_STANDALONE
signed main() {
    PrimeNumbers pn;

    cout << "=== A. Тесты простоты ===" << endl;
    vector<ll> test_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    vector<ll> test_composites = {4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 22, 24, 25};
    for (ll p : test_primes)
        cout << p << " prime: " << (pn.is_prime(p) ? "YES" : "NO") << endl;
    for (ll c : test_composites)
        cout << c << " prime: " << (pn.is_prime(c) ? "YES" : "NO") << endl;

    cout << "\n=== A. Большие простые ===" << endl;
    ll big_prime = 1000000007;
    ll big_composite = 1000000009;
    cout << big_prime << " prime: " << (pn.is_prime(big_prime) ? "YES" : "NO") << endl;
    cout << big_composite << " prime: " << (pn.is_prime(big_composite) ? "YES" : "NO") << endl;

    cout << "\n=== B. Факторизация ===" << endl;
    ll n1 = 9973;
    auto [a1, b1] = pn.fermat(n1);
    cout << "fermat(" << n1 << ") = " << a1 << " * " << b1 << endl;

    ll n2 = 8051;
    cout << "pollard_p1(" << n2 << ") = " << pn.pollard_p1(n2) << endl;
    cout << "pollard_rho(" << n2 << ") = " << pn.pollard_rho(n2) << endl;
    cout << "bent(" << n2 << ") = " << pn.bent_method(n2) << endl;
    cout << "monte_carlo(" << n2 << ") = " << pn.monte_carlo_pollard(n2) << endl;

    ll n3 = 123456789;
    auto factors = pn.factorize_rho(n3);
    cout << "factorize_rho(" << n3 << "): ";
    for (ll f : factors) cout << f << " ";
    cout << endl;

    cout << "\n=== C. Решета ===" << endl;
    auto sundaram = pn.sieve_sundaram(10);
    cout << "Sundaram primes to 21: ";
    cout << "2 ";
    for (int i = 1; i <= 10; i++)
        if (sundaram[i]) cout << 2 * i + 1 << " ";
    cout << endl;

    auto odd = pn.odd_sieve(50);
    cout << "Odd sieve primes to 50: 2 ";
    for (int i = 1; i < (int)odd.size(); i++)
        if (odd[i]) cout << 2 * i + 1 << " ";
    cout << endl;

    auto segmented = pn.segmented_sieve(100, 120);
    cout << "Primes in [100,120]: ";
    for (int i = 0; i <= 20; i++)
        if (segmented[i]) cout << 100 + i << " ";
    cout << endl;

    cout << "\n=== D. Распределение ===" << endl;
    cout << "π(1000) ≈ " << pn.prime_count_approx(1000) << " (точное: 168)" << endl;
    cout << "π(1000000) ≈ " << pn.prime_count_approx(1000000) << " (точное: 78498)" << endl;
    cout << "Bertrand(10): " << (pn.bertrand_check(10) ? "YES" : "NO") << endl;

    cout << "\n=== E. Специальные классы ===" << endl;
    cout << "Sophie Germain primes ≤ 50: ";
    for (ll p = 2; p <= 50; p++)
        if (pn.is_sophie_germain(p)) cout << p << " ";
    cout << endl;

    auto twins = pn.twin_primes(50);
    cout << "Twin primes ≤ 50: ";
    for (auto [a, b] : twins) cout << "(" << a << "," << b << ") ";
    cout << endl;

    cout << "Lucas-Lehmer M7 (127): " << (pn.lucas_lehmer(7) ? "YES" : "NO") << endl;
    cout << "Lucas-Lehmer M11 (2047): " << (pn.lucas_lehmer(11) ? "YES" : "NO") << endl;

    cout << "\n=== A. BPSW Test ===" << endl;
    cout << "bpsw(1000000007): " << (pn.bpsw_test(1000000007) ? "YES" : "NO") << endl;
    cout << "bpsw(1000000009): " << (pn.bpsw_test(1000000009) ? "YES" : "NO") << endl;

    cout << "\n=== C. Решето Аткина ===" << endl;
    auto atkin = pn.sieve_atkin(100);
    cout << "Atkin primes to 100: ";
    for (int i = 2; i <= 100; i++) if (atkin[i]) cout << i << " ";
    cout << endl;
}
#endif
