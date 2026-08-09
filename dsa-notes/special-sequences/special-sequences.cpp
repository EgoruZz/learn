#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
using namespace std;
typedef long long ll;

// Сначала включаем конспект комбинаторики (a.cpp) ПОЛНОСТЬЮ — до цепочки
// number-theory. Иначе a.cpp попадёт только в режиме INSIDE_NUMBER_THEORY
// (только C_iter, без struct Combinatorics), и c.cpp не сможет наследовать.
// Дерево Штерна-Броко описано в конспекте комбинаторики (C, c.cpp):
// реализация уровня дерева — RationalCombinatorics::stern_brocot_level
// (c.cpp, A.2.3) — здесь заимствуется как факт, а не дублируется.
#define COMBINATORICS_MAIN
#include "../math/combinatorics/a/a.cpp"

// Подключаем g.cpp → e.cpp → b.cpp → a.cpp, и g.cpp уже включает c.cpp
#define SPECIAL_NUMBERS_STANDALONE
#include "../special-numbers/special-numbers.cpp"

#define RATIONAL_COMBINATORICS_MAIN
#include "../math/combinatorics/c/c.cpp"

// =============================================================
// H. СПЕЦИАЛЬНЫЕ ПОСЛЕДОВАТЕЛЬНОСТИ
// =============================================================
// Наследует SpecialNumbers (g.cpp → DigitOps (c.cpp) → PrimeNumbers → ...)
//
// Содержит:
//   A. Фибоначчи и Лукас          — fast doubling O(log n), Pisano period
//   B. Обобщённые Фибоначчи        — matrix exponentiation
//   C. Якобсталь                   — closed form
//   D. Падован и Перрин            — fast doubling / matrix
//   E. Каталаны                    — product formula, mod p
//   F. Мотцкина                    — recurrence O(n)
//   G. Стирлинга (I и II рода)     — DP O(n·k)
//   H. Белла                       — треугольник Айткена O(n²)
//   I. Шрёдера                     — recurrence O(n²)
//   J. Эйлера (Eulerian)           — DP O(n²)
//   K. Деланнуа                    — DP O(n·m)
//   L. Вайтоф / Бити               — O(1) per query
//   M. Стерн-Броко / диатомическая — recursive
//   N. Коллац                      — iteration + memoization
//   O. Джагглер                    — iteration
//   P. Силвестер                   — product (limited by overflow)

struct SpecialSequences : SpecialNumbers {

    // =========================================================
    // ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ МАТРИЦ
    // =========================================================

    struct Mat { ll a, b, c, d; };

    Mat mat_mul(Mat A, Mat B) {
        return {A.a*B.a + A.b*B.c, A.a*B.b + A.b*B.d,
                A.c*B.a + A.d*B.c, A.c*B.b + A.d*B.d};
    }

    Mat mat_pow(Mat M, ll n) {
        Mat R = {1, 0, 0, 1};
        while (n > 0) {
            if (n & 1) R = mat_mul(R, M);
            M = mat_mul(M, M);
            n >>= 1;
        }
        return R;
    }

    // =========================================================
    // A. ЧИСЛА ФИБОНАЧЧИ И ЛУКАСА
    // =========================================================

    // Fast Doubling: возвращает {F(n), F(n+1)}
    pair<ll,ll> fib_pair(ll n) {
        if (n == 0) return {0, 1};
        auto [a, b] = fib_pair(n >> 1);
        ll c = a * (2 * b - a);
        ll d = a * a + b * b;
        if (n & 1) return {d, c + d};
        else return {c, d};
    }

    // F(n) — n-е число Фибоначчи, O(log n)
    ll fib(ll n) {
        if (n < 0) return 0;
        return fib_pair(n).first;
    }

    // L(n) — n-е число Лукаса, O(log n)
    // L(n) = F(n-1) + F(n+1) = 2·F(n+1) - F(n)
    ll lucas(ll n) {
        if (n == 0) return 2;
        auto [fn, fn1] = fib_pair(n);
        return 2 * fn1 - fn;
    }

    // F(n) mod m — быстрое вычисление по модулю
    ll fib_mod(ll n, ll m) {
        if (n < 0) return 0;
        // Fast doubling по модулю
        function<pair<ll,ll>(ll)> go = [&](ll k) -> pair<ll,ll> {
            if (k == 0) return {0 % m, 1 % m};
            auto [a, b] = go(k >> 1);
            ll c = (a % m * ((2 * b % m - a % m + m) % m)) % m;
            ll d = (a % m * a % m + b % m * b % m) % m;
            if (k & 1) return {d, (c + d) % m};
            else return {c, d};
        };
        return go(n).first;
    }

    // Pisano period π(m) — период F(n) mod m
    ll pisano_period(ll m) {
        ll prev = 0, curr = 1;
        for (ll i = 0; i <= m * m + 1; i++) {
            ll next = (prev + curr) % m;
            prev = curr;
            curr = next;
            if (prev == 0 && curr == 1) return i + 1;
        }
        return -1; // не должно произойти
    }

    // Индекс Зекендорфа:唯一 представление n как суммы различных F(k) с k ≥ 2
    // Возвращает индексы (k₁ > k₂ > ... > k_r ≥ 2)
    vector<ll> zeckendorf(ll n) {
        vector<ll> indices;
        // Генерируем Фибоначчи до n
        vector<ll> fibs = {1, 2}; // F(2)=1, F(3)=2
        while (fibs.back() <= n) {
            fibs.push_back(fibs.back() + fibs[fibs.size()-2]);
        }
        for (int i = (int)fibs.size() - 1; i >= 2 && n > 0; i--) {
            if (fibs[i] <= n) {
                indices.push_back(i + 1); // F(i+1) = fibs[i] (сдвиг: fibs[0]=F(2))
                n -= fibs[i];
            }
        }
        return indices;
    }

    // =========================================================
    // B. ОБОБЩЁННЫЕ ПОСЛЕДОВАТЕЛЬНОСТИ ФИБОНАЧЧИ
    // =========================================================

    // G(n) при G(0) = a, G(1) = b, G(n) = G(n-1) + G(n-2)
    ll generalized_fib(ll n, ll a, ll b) {
        if (n == 0) return a;
        if (n == 1) return b;
        // |b a| · |1 1|^n → первый элемент = G(n+1), второй = G(n)
        // Или: G(n) = a·F(n-1) + b·F(n)
        ll fn = fib(n);
        ll fn1 = fib(n - 1);
        return a * fn1 + b * fn;
    }

    // G(n) mod m
    ll generalized_fib_mod(ll n, ll a, ll b, ll m) {
        if (n == 0) return a % m;
        if (n == 1) return b % m;
        ll fn = fib_mod(n, m);
        ll fn1 = fib_mod(n - 1, m);
        return ((a % m) * (fn1 % m) + (b % m) * (fn % m)) % m;
    }

    // =========================================================
    // C. ЧИСЛА ЯКОБСТАЛЯ
    // =========================================================

    // J(n) = (2^n - (-1)^n) / 3 — closed form
    // n ≤ 62, чтобы 2^n влезало в long long
    ll jacobsthal(ll n) {
        if (n == 0) return 0;
        ll pow2 = 1LL << min(n, 62LL);
        ll sign = (n % 2 == 0) ? 1 : -1;
        return (pow2 + sign) / 3;
    }

    // J(n) по рекурренту: J(n) = J(n-1) + 2·J(n-2), O(n) по времени, O(1) по памяти
    ll jacobsthal_rec(ll n) {
        if (n == 0) return 0;
        if (n == 1) return 1;
        ll prev2 = 0, prev1 = 1;
        for (int i = 2; i <= n; i++) {
            ll cur = prev1 + 2 * prev2;
            prev2 = prev1;
            prev1 = cur;
        }
        return prev1;
    }

    // =========================================================
    // D. ЧИСЛА ПАДОВАНА И ПЕРРИНА
    // =========================================================

    // Падован: P(0)=P(1)=P(2)=1, P(n) = P(n-2) + P(n-3)
    ll padovan(ll n) {
        if (n <= 2) return 1;
        ll p0 = 1, p1 = 1, p2 = 1;
        for (int i = 3; i <= n; i++) {
            ll p3 = p0 + p2;
            p0 = p1; p1 = p2; p2 = p3;
        }
        return p2;
    }

    // Перрин: Pr(0)=3, Pr(1)=0, Pr(2)=2, Pr(n) = Pr(n-1) + Pr(n-3)
    // Матричный метод O(log n):
    // Состояние: [Pr(n), Pr(n-1), Pr(n-2)]
    // M = |1 0 1|   Pr(n+1) = Pr(n) + Pr(n-2)
    //     |1 0 0|   Pr(n)   → Pr(n-1)
    //     |0 1 0|   Pr(n-1) → Pr(n-2)
    // M^n · [2, 0, 3]^T = [Pr(n+2), Pr(n+1), Pr(n)]^T
    struct Mat3 {
        ll a[3][3];
    };
    Mat3 mat3_mul(Mat3 A, Mat3 B, ll mod) {
        Mat3 C = {{{0}}};
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++)
                    C.a[i][j] = (C.a[i][j] + A.a[i][k] * B.a[k][j]) % mod;
        return C;
    }
    Mat3 mat3_pow(Mat3 M, ll n, ll mod) {
        Mat3 R = {{{1,0,0},{0,1,0},{0,0,1}}};
        while (n > 0) {
            if (n & 1) R = mat3_mul(R, M, mod);
            M = mat3_mul(M, M, mod);
            n >>= 1;
        }
        return R;
    }

    ll perrin(ll n) {
        if (n == 0) return 3;
        if (n == 1) return 0;
        if (n == 2) return 2;
        Mat3 M = {{{1,0,1},{1,0,0},{0,1,0}}};
        Mat3 Mn = mat3_pow(M, n - 2, (ll)4e18);
        // M^(n-2) · [2, 0, 3] = [Pr(n), Pr(n-1), Pr(n-2)]
        return (Mn.a[0][0] * 2 + Mn.a[0][2] * 3);
    }

    // Перрин mod m: Pr(n) mod m, O(log n)
    ll perrin_mod(ll n, ll m) {
        if (n == 0) return 3 % m;
        if (n == 1) return 0;
        if (n == 2) return 2 % m;
        Mat3 M = {{{1,0,1},{1,0,0},{0,1,0}}};
        Mat3 Mn = mat3_pow(M, n - 2, m);
        return (Mn.a[0][0] * 2 + Mn.a[0][2] * 3) % m;
    }

    // Перрин: проверка на псевдопростоту (n | Pr(n))
    bool is_perrin_pseudoprime(ll n) {
        if (n <= 2) return false;
        return perrin_mod(n, n) == 0;
    }

    // =========================================================
    // E. ЧИСЛА КАТАЛАНЫ
    // =========================================================

    // C(n) = C(2n, n) / (n+1) — точное вычисление
    // Рекуррент: C(n+1) = C(n) · 2(2n+1)/(n+2)
    ll catalan(ll n) {
        if (n <= 1) return 1;
        ll res = 1;
        for (int k = 1; k < n; k++) {
            res = res * 2 * (2 * k + 1) / (k + 2);
        }
        return res;
    }

    // C(n) mod p — через биномиальные коэффициенты и теорему Люка
    // Используем: C(2n, n) mod p через factorize_rho и теорему Люка
    ll catalan_mod(ll n, ll p) {
        if (n <= 1) return 1 % p;
        // C(n) = C(2n, n) / (n+1)
        // Через произведение: C(n) = product(2k/(k+1)) mod p
        ll res = 1;
        for (int k = 1; k <= n; k++) {
            res = (res * (2 * k % p)) % p;
            res = (res * modinv(k + 1, p)) % p;
        }
        return res;
    }

    // Все числа Каталана ≤ limit (для малых n, т.к. растут быстро)
    vector<ll> catalan_numbers(ll limit) {
        vector<ll> res;
        ll c = 1;
        for (int n = 0; c <= limit; n++) {
            res.push_back(c);
            c = c * 2 * (2 * n + 1) / (n + 2);
        }
        return res;
    }

    // =========================================================
    // F. ЧИСЛА МОТЦКИНА
    // =========================================================

    // M(n) = M(n-1) + Σ M(k)·M(n-2-k), O(n²)
    ll motzkin(ll n) {
        if (n <= 1) return 1;
        vector<ll> m(n + 1);
        m[0] = 1; m[1] = 1;
        for (int i = 2; i <= n; i++) {
            m[i] = m[i - 1];
            for (int k = 0; k <= i - 2; k++) {
                m[i] += m[k] * m[i - 2 - k];
            }
        }
        return m[n];
    }

    // M(n) mod p — O(n²) но с mod
    ll motzkin_mod(ll n, ll p) {
        if (n <= 1) return 1 % p;
        vector<ll> m(n + 1);
        m[0] = 1 % p; m[1] = 1 % p;
        for (int i = 2; i <= n; i++) {
            m[i] = m[i - 1];
            for (int k = 0; k <= i - 2; k++) {
                m[i] = (m[i] + m[k] * m[i - 2 - k]) % p;
            }
        }
        return m[n];
    }

    // =========================================================
    // G. ЧИСЛА СТИРЛИНГА (I И II РОДА)
    // =========================================================

    // S(n,k) — II рода: разбиения n-множества на k непустых подмножеств
    // S(n,k) = S(n-1,k-1) + k·S(n-1,k), O(n·k)
    ll stirling2(ll n, ll k) {
        if (n == 0 && k == 0) return 1;
        if (n == 0 || k == 0) return 0;
        vector<vector<ll>> S(n + 1, vector<ll>(k + 1));
        S[0][0] = 1;
        for (ll i = 1; i <= n; i++)
            for (ll j = 1; j <= min(i, k); j++)
                S[i][j] = S[i-1][j-1] + j * S[i-1][j];
        return S[n][k];
    }

    // c(n,k) — I рода (без знака): перестановки n ровно из k циклов
    // c(n,k) = c(n-1,k-1) + (n-1)·c(n-1,k), O(n·k)
    ll stirling1(ll n, ll k) {
        if (n == 0 && k == 0) return 1;
        if (n == 0 || k == 0) return 0;
        vector<vector<ll>> S(n + 1, vector<ll>(k + 1));
        S[0][0] = 1;
        for (ll i = 1; i <= n; i++)
            for (ll j = 1; j <= min(i, k); j++)
                S[i][j] = S[i-1][j-1] + (i - 1) * S[i-1][j];
        return S[n][k];
    }

    // =========================================================
    // H. ЧИСЛА БЕЛЛА
    // =========================================================

    // B(n) — все разбиения n-множества; треугольник Айткена, O(n²)
    ll bell(ll n) {
        if (n <= 1) return 1;
        vector<ll> prev = {1};
        for (ll i = 1; i < n; i++) {
            vector<ll> cur(i + 1);
            cur[0] = prev.back();
            for (ll j = 1; j <= i; j++)
                cur[j] = cur[j-1] + prev[j-1];
            prev = cur;
        }
        return prev.back();
    }

    // =========================================================
    // I. ЧИСЛА ШРЁДЕРА
    // =========================================================

    // Большие: S(0) = 1, S(n) = S(n-1) + Σ S(k)·S(n-1-k), O(n²)
    ll schroder(ll n) {
        vector<ll> s(n + 1);
        s[0] = 1;
        for (ll i = 1; i <= n; i++) {
            ll cur = s[i-1];
            for (ll k = 0; k <= i - 1; k++)
                cur += s[k] * s[i - 1 - k];
            s[i] = cur;
        }
        return s[n];
    }

    // Малые: s(0) = 1, s(n) = S(n)/2
    ll schroder_small(ll n) {
        if (n == 0) return 1;
        return schroder(n) / 2;
    }

    // =========================================================
    // J. ЧИСЛА ЭЙЛЕРА (EULERIAN)
    // =========================================================

    // A(n,k) — перестановки n с k подъёмами (ascents)
    // A(n,k) = (k+1)·A(n-1,k) + (n-k)·A(n-1,k-1), O(n²)
    ll eulerian(ll n, ll k) {
        if (k < 0 || k >= n) return 0;
        vector<vector<ll>> E(n + 1, vector<ll>(n + 1));
        E[0][0] = 1;
        for (ll i = 1; i <= n; i++)
            for (ll j = 0; j < i; j++)
                E[i][j] = (j + 1) * E[i-1][j]
                        + (j >= 1 ? (i - j) * E[i-1][j-1] : 0);
        return E[n][k];
    }

    // =========================================================
    // K. ЧИСЛА ДЕЛАННУА
    // =========================================================

    // D(m,n) — пути с шагами (1,0), (0,1), (1,1); D(0,n)=D(m,0)=1, O(m·n)
    ll delannoy(ll m, ll n) {
        vector<vector<ll>> D(m + 1, vector<ll>(n + 1, 1));
        for (ll i = 1; i <= m; i++)
            for (ll j = 1; j <= n; j++)
                D[i][j] = D[i-1][j] + D[i][j-1] + D[i-1][j-1];
        return D[m][n];
    }

    // =========================================================
    // L. МАССИВ ВАЙТОФА И ПОСЛЕДОВАТЕЛЬНОСТЬ БИТИ
    // =========================================================

    // Золотое сечение (приближение)
    static constexpr double PHI = 1.6180339887498948482;
    static constexpr double PHI2 = 2.6180339887498948482;

    // Бити-последовательность: B_r(n) = ⌊n·r⌋
    ll beatty(ll n, double r) {
        return (ll)floor(n * r);
    }

    // Массив Вайтофа: A(0, n) = ⌊n·φ⌋, A(1, n) = ⌊n·φ²⌋
    ll wythoff_a(ll n) { // A(0, n) — "проигрышные" позиции
        return (ll)floor(n * PHI);
    }

    ll wythoff_b(ll n) { // A(1, n) — вторая строка
        return (ll)floor(n * PHI2);
    }

    // Проверка: (a, b) — позиция Вайтофа (a = ⌊n·φ⌋, b = ⌊n·φ²⌋)
    bool is_wythoff_position(ll a, ll b) {
        if (a > b) swap(a, b);
        ll n = (ll)round(a / PHI);
        return wythoff_a(n) == a && wythoff_b(n) == b;
    }

    // =========================================================
    // M. ДЕРЕВО СТЕРНА-БРОКО И ПОСЛЕДОВАТЕЛЬНОСТЬ СТЕРНА
    // =========================================================

    // Диатомическая (последовательность Стерна): s(0)=0, s(1)=1,
    // s(2n) = s(n), s(2n+1) = s(n) + s(n+1)
    ll stern_diatomic(ll n) {
        if (n <= 1) return n;
        if (n % 2 == 0) return stern_diatomic(n / 2);
        return stern_diatomic(n / 2) + stern_diatomic(n / 2 + 1);
    }

    // Диатомическая: итеративная версия O(log n)
    ll stern_diatomic_iter(ll n) {
        ll a = 0, b = 1;
        for (int i = 31; i >= 0; i--) {
            if ((n >> i) & 1) {
                a = a + b;
            } else {
                b = a + b;
            }
        }
        return (n & 1) ? b : a;
    }

    // Рациональные на n-м уровне дерева Стерна-Броко (breadth-first)
    // Реализация — в конспекте комбинаторики (C, c.cpp, A.2.3):
    // RationalCombinatorics::stern_brocot_level; здесь — как факт.
    vector<pair<ll,ll>> stern_brocot_level(int n) {
        RationalCombinatorics rc;
        return rc.stern_brocot_level(n);
    }

    // =========================================================
    // N. ПОСЛЕДОВАТЕЛЬНОСТЬ КОЛЛАЦА
    // =========================================================

    // Одна итерация: T(n)
    ll collatz_step(ll n) {
        return (n % 2 == 0) ? n / 2 : 3 * n + 1;
    }

    // Время остановки t(n): количество шагов до 1
    // С кэшированием (memoization)
    unordered_map<ll,ll> collatz_cache;
    ll collatz_stopping_time(ll n) {
        if (n == 1) return 0;
        if (collatz_cache.count(n)) return collatz_cache[n];
        ll steps = 1 + collatz_stopping_time(collatz_step(n));
        collatz_cache[n] = steps;
        return steps;
    }

    // Орбита Коллаца (последовательность до 1)
    vector<ll> collatz_orbit(ll n) {
        vector<ll> orbit = {n};
        while (n != 1) {
            n = collatz_step(n);
            orbit.push_back(n);
        }
        return orbit;
    }

    // Максимальное t(n) для n ≤ limit
    pair<ll,ll> collatz_max_stopping(ll limit) {
        ll max_steps = 0, max_n = 1;
        collatz_cache.clear();
        for (ll i = 1; i <= limit; i++) {
            ll s = collatz_stopping_time(i);
            if (s > max_steps) {
                max_steps = s;
                max_n = i;
            }
        }
        return {max_n, max_steps};
    }

    // =========================================================
    // O. ПОСЛЕДОВАТЕЛЬНОСТЬ ДЖАГГЛЕРА
    // =========================================================

    // Одна итерация: J(n) = ⌊n^{1/2}⌋ если n чётное, ⌊n^{3/2}⌋ если нечётное
    ll juggler_step(ll n) {
        if (n <= 1) return n;
        if (n % 2 == 0) return isqrt_newton(n);
        // ⌊n^{3/2}⌋ = ⌊√(n^3)⌋
        __int128 n3 = (__int128)n * n * n;
        return isqrt_newton((ll)n3);
    }

    // Время остановки Джагглера
    ll juggler_stopping_time(ll n) {
        ll steps = 0;
        while (n > 1) {
            n = juggler_step(n);
            steps++;
        }
        return steps;
    }

    // Орбита Джагглера
    vector<ll> juggler_orbit(ll n) {
        vector<ll> orbit = {n};
        while (n > 1) {
            n = juggler_step(n);
            orbit.push_back(n);
        }
        return orbit;
    }

    // =========================================================
    // P. ПОСЛЕДОВАТЕЛЬНОСТЬ СИЛВЕСТЕРА
    // =========================================================

    // Первые несколько членов (ограничено overflow)
    // a(0)=2, a(n) = product(a(0)..a(n-1)) + 1
    // a(7) > 10^54 — не влезает в long long
    vector<ll> sylvester_sequence(int count) {
        vector<ll> res = {2};
        for (int i = 1; i < count; i++) {
            __int128 prod = 1;
            for (ll x : res) prod *= x;
            res.push_back((ll)(prod + 1));
        }
        return res;
    }

    // Проверка: все ли члены до n взаимно простые (всегда true для Силвестера)
    bool sylvester_coprime_check(int n) {
        auto seq = sylvester_sequence(n);
        for (int i = 0; i < (int)seq.size(); i++) {
            for (int j = i + 1; j < (int)seq.size(); j++) {
                if (gcd_mod(seq[i], seq[j]) != 1) return false;
            }
        }
        return true;
    }

    // Сумма обратных: Σ 1/a(i) от i=0 до n
    // = 1 - 1/(a(0)·a(1)·...·a(n))
    double sylvester_sum_inv(int n) {
        auto seq = sylvester_sequence(n + 1);
        __int128 prod = 1;
        for (ll x : seq) prod *= x;
        return 1.0 - 1.0 / (double)prod;
    }
};

// =============================================================
// MAIN — ДЕМОНСТРАЦИЯ
// =============================================================
#ifndef SPECIAL_SEQUENCES_STANDALONE
signed main() {
    SpecialSequences ss;

    cout << "=== A. ФИБОНАЧЧИ И ЛУКАСА ===" << endl;
    cout << "F(10) = " << ss.fib(10) << endl;
    cout << "F(20) = " << ss.fib(20) << endl;
    cout << "L(10) = " << ss.lucas(10) << endl;
    cout << "F(100) mod 1000000007 = " << ss.fib_mod(100, 1000000007) << endl;
    cout << "Pisano period mod 10 = " << ss.pisano_period(10) << endl;
    cout << "Pisano period mod 7 = " << ss.pisano_period(7) << endl;
    cout << "Zeckendorf(100) indices: ";
    for (ll idx : ss.zeckendorf(100)) cout << idx << " ";
    cout << endl;

    cout << "\n=== B. ОБОБЩЁННЫЕ ФИБОНАЧЧИ ===" << endl;
    cout << "G(10) with a=3,b=2: " << ss.generalized_fib(10, 3, 2) << endl;
    cout << "G(10) with a=0,b=1 (standard): " << ss.generalized_fib(10, 0, 1) << endl;
    cout << "G(10) with a=2,b=1 (Lucas): " << ss.generalized_fib(10, 2, 1) << endl;

    cout << "\n=== C. ЧИСЛА ЯКОБСТАЛЯ ===" << endl;
    cout << "J(0..10): ";
    for (int i = 0; i <= 10; i++) cout << ss.jacobsthal_rec(i) << " ";
    cout << endl;
    cout << "J(20) closed form: " << ss.jacobsthal(20) << endl;
    cout << "J(20) recurrence: " << ss.jacobsthal_rec(20) << endl;

    cout << "\n=== D. ПАДОВАН И ПЕРРИН ===" << endl;
    cout << "P(0..12): ";
    for (int i = 0; i <= 12; i++) cout << ss.padovan(i) << " ";
    cout << endl;
    cout << "Pr(0..12): ";
    for (int i = 0; i <= 12; i++) cout << ss.perrin(i) << " ";
    cout << endl;
    cout << "Perrin pseudoprime 271441: " << ss.is_perrin_pseudoprime(271441) << endl;

    cout << "\n=== E. ЧИСЛА КАТАЛАНЫ ===" << endl;
    cout << "C(0..10): ";
    for (int i = 0; i <= 10; i++) cout << ss.catalan(i) << " ";
    cout << endl;
    auto cnums = ss.catalan_numbers(1e6);
    cout << "Catalan ≤ 10^6: ";
    for (ll x : cnums) cout << x << " ";
    cout << endl;

    cout << "\n=== F. ЧИСЛА МОТЦКИНА ===" << endl;
    cout << "M(0..10): ";
    for (int i = 0; i <= 10; i++) cout << ss.motzkin(i) << " ";
    cout << endl;

    cout << "\n=== G. ЧИСЛА СТИРЛИНГА ===" << endl;
    cout << "S(4,2) II рода = " << ss.stirling2(4, 2) << endl;
    cout << "S(5,2) II рода = " << ss.stirling2(5, 2) << endl;
    cout << "S(5,3) II рода = " << ss.stirling2(5, 3) << endl;
    cout << "s(4,2) I рода = " << ss.stirling1(4, 2) << endl;
    cout << "s(5,2) I рода = " << ss.stirling1(5, 2) << endl;

    cout << "\n=== H. ЧИСЛА БЕЛЛА ===" << endl;
    cout << "B(0..7): ";
    for (int i = 0; i <= 7; i++) cout << ss.bell(i) << " ";
    cout << endl;

    cout << "\n=== I. ЧИСЛА ШРЁДЕРА ===" << endl;
    cout << "S(0..6) большие: ";
    for (int i = 0; i <= 6; i++) cout << ss.schroder(i) << " ";
    cout << endl;
    cout << "S(0..6) малые: ";
    for (int i = 0; i <= 6; i++) cout << ss.schroder_small(i) << " ";
    cout << endl;

    cout << "\n=== J. ЧИСЛА ЭЙЛЕРА ===" << endl;
    cout << "A(5,k): ";
    for (int k = 0; k < 5; k++) cout << ss.eulerian(5, k) << " ";
    cout << endl;

    cout << "\n=== K. ЧИСЛА ДЕЛАННУА ===" << endl;
    cout << "D(3,3) = " << ss.delannoy(3, 3) << endl;
    cout << "D(4,4) = " << ss.delannoy(4, 4) << endl;

    cout << "\n=== L. ВАЙТОФ / БИТИ ===" << endl;
    cout << "Wythoff A(0..9): ";
    for (int i = 0; i <= 9; i++) cout << ss.wythoff_a(i) << " ";
    cout << endl;
    cout << "Wythoff B(0..9): ";
    for (int i = 0; i <= 9; i++) cout << ss.wythoff_b(i) << " ";
    cout << endl;
    cout << "(1,2) is Wythoff: " << ss.is_wythoff_position(1, 2) << endl;
    cout << "(3,5) is Wythoff: " << ss.is_wythoff_position(3, 5) << endl;
    cout << "(4,7) is Wythoff: " << ss.is_wythoff_position(4, 7) << endl;

    cout << "\n=== M. СТЕРН-БРОКО ===" << endl;
    cout << "Stern diatomic 0..15: ";
    for (int i = 0; i <= 15; i++) cout << ss.stern_diatomic(i) << " ";
    cout << endl;
    cout << "Stern level 3: ";
    for (auto [a, b] : ss.stern_brocot_level(3)) cout << a << "/" << b << " ";
    cout << endl;

    cout << "\n=== N. КОЛЛАЦ ===" << endl;
    cout << "Collatz orbit 6: ";
    for (ll x : ss.collatz_orbit(6)) cout << x << " ";
    cout << endl;
    auto [maxn, maxs] = ss.collatz_max_stopping(10000);
    cout << "Max stopping time ≤ 10000: n=" << maxn << " steps=" << maxs << endl;

    cout << "\n=== O. ДЖАГГЛЕР ===" << endl;
    cout << "Juggler orbit 3: ";
    for (ll x : ss.juggler_orbit(3)) cout << x << " ";
    cout << endl;
    cout << "Juggler stopping time 3: " << ss.juggler_stopping_time(3) << endl;

    cout << "\n=== P. СИЛВЕСТЕР ===" << endl;
    auto sylv = ss.sylvester_sequence(7);
    cout << "Sylvester 0..6: ";
    for (ll x : sylv) cout << x << " ";
    cout << endl;
    cout << "All coprime: " << ss.sylvester_coprime_check(7) << endl;
    cout << "Sum of inverses (0..6) ≈ " << ss.sylvester_sum_inv(6) << endl;

    return 0;
}
#endif
