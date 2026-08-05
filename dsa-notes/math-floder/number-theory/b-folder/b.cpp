#include <iostream>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <cmath>
#include <algorithm>
using namespace std;

typedef long long ll;

// =============================================================
// B. МОДУЛЯРНАЯ АРИФМЕТИКА, ТЕОРИЯ СРАВНЕНИЙ И ДИОФАНТОВЫ УРАВНЕНИЯ
// =============================================================
// Структура md: A. Модулярная арифметика → B. Теоремы → C. Линейные сравнения
//               → D. Диофантовы → E. Бинарное возведение → F. Расширенный Евклид
//               → G. Функции → H. Квадратные вычеты → I. Продвинутые → J. Прикладные
//
// Наследует Divisibility из a.cpp (gcd_mod, lcm и т.д.)
// Используется как базовый класс для расширения (см. d.cpp, f.cpp)
//
// Содержит:
//   A. Основы + CRT: count_in_range, cancel_mod, crt, crt_general, garner
//   B. Теоремы — (теоретические, нет отдельных функций)
//   C. Линейные сравнения: solve_linear_congruence
//   D. Диофантовы: solve_linear_diophantine, pell, pythagorean_triples
//   E. Бинарное возведение: powmod, powll
//   F. Расширенный Евклид: egcd, modinv
//   G. Функции: carmichael, radical, mangoldt
//   H. Квадратные вычеты: legendre, jacobi, is_quadratic_residue

#define DIVISIBILITY_MAIN
#include "../a-folder/a.cpp"

struct ModArithmetic : Divisibility {

// =============================================================
// A. МОДУЛЯРНАЯ АРИФМЕТИКА — ОСНОВЫ
// =============================================================

// --- A.1.1. Количество чисел в [L,R] по модулю ---
// Формула: |{a ∈ [L, R] : a ≡ r (mod m)}| = ⌊(R-r)/m⌋ - ⌊(L-1-r)/m⌋
//
// Пример: count_in_range(10, 30, 2, 5) = 4
//   Числа: 12, 17, 22, 27 — все ≡ 2 (mod 5)
int count_in_range(ll L, ll R, ll r, ll m) {
    auto count = [&](ll x) { return x >= 0 ? (x - r) / m + 1 : 0; };
    return count(R) - count(L - 1);
}

// --- A.1.2. Отмена модуля ---
// Решает: a·x ≡ b (mod m) через делимость
//   1. d = gcd(a, m)
//   2. Если b не делится на d → нет решения
//   3. Иначе: a' = a/d, b' = b/d, m' = m/d → x ≡ b' · (a')⁻¹ (mod m')
//
// Пример: cancel_mod(6, 4, 8) → d=2, a'=3, b'=2, m'=4 → x ≡ 2 (mod 4)
void cancel_mod(ll a, ll c, ll m, ll &a1, ll &b1, ll &m1) {
    ll d = gcd_mod(a, m);
    if (c % d != 0) { a1 = b1 = m1 = -1; return; }
    a1 = a / d; b1 = c / d; m1 = m / d;
}

// --- A.3.1. КРТ — стандартная (взаимно простые модули) ---
// Решение системы: x ≡ aᵢ (mod mᵢ) где gcd(mᵢ, mⱼ) = 1
// Формула: x = ∑ aᵢ · Mᵢ · Nᵢ (mod M), где M = ∏ mᵢ, Mᵢ = M/mᵢ, Nᵢ = Mᵢ⁻¹ (mod mᵢ)
//
// Пример: x ≡ 2 (mod 3), x ≡ 3 (mod 5), x ≡ 2 (mod 7) → x = 23
ll crt(const vector<ll>& a, const vector<ll>& m) {
    int k = a.size();
    ll M = 1;
    for (int i = 0; i < k; i++) M *= m[i];

    ll result = 0;
    for (int i = 0; i < k; i++) {
        ll Mi = M / m[i];
        ll Ni = modinv(Mi, m[i]);
        result = (result + a[i] % m[i] * Mi % M * Ni % M) % M;
    }
    return (result % M + M) % M;
}

// --- A.3.2. КРТ — обобщённая (произвольные модули) ---
// Условие совместности: aᵢ ≡ aⱼ (mod gcd(mᵢ, mⱼ)) ∀i,j
// Алгоритм: последовательное слияние пар сравнений → одно итоговое
//
// Пример: x ≡ 2 (mod 4), x ≡ 4 (mod 6) → x = 10 (mod 12)
pair<ll, ll> crt_general(vector<ll> a, vector<ll> m) {
    ll x = a[0] % m[0], l = m[0];
    for (int i = 1; i < (int)a.size(); i++) {
        ll g = gcd_mod(l, m[i]);
        ll diff = ((a[i] - x) % m[i] + m[i]) % m[i];
        if (diff % g != 0) return {-1, -1};
        ll t = (diff / g) * modinv(l / g, m[i] / g) % (m[i] / g);
        x = x + l * t;
        l = l / g * m[i];
        x = (x % l + l) % l;
    }
    return {x, l};
}

// --- A.3.3. Алгоритм Гарнера ---
// Представляем x в смешанной системе: x = v₁ + v₂·m₁ + v₃·m₁·m₂ + ...
// Преимущество: модульная арифметика только на последнем шаге, числа не растут
// Сложность: O(k²)
//
// Пример: x ≡ 2 (mod 3), x ≡ 3 (mod 5), x ≡ 2 (mod 7) → x = 23
ll garner(vector<ll> a, vector<ll> m) {
    int k = a.size();
    vector<ll> v(k, 0);

    for (int i = 0; i < k; i++) {
        v[i] = a[i] % m[i];
        for (int j = 0; j < i; j++)
            v[i] = modinv(m[j], m[i]) * (v[i] - v[j]) % m[i];
        v[i] = (v[i] % m[i] + m[i]) % m[i];
    }

    ll result = 0;
    for (int i = k - 1; i >= 0; i--)
        result = result * m[i] + v[i];
    return result;
}

// =============================================================
// B. ТЕОРЕМЫ МОДУЛЯРНОЙ АРИФМЕТИКИ
// =============================================================
// Малая теорема Ферма: a^(p-1) ≡ 1 (mod p) при p ∤ a
// Теорема Эйлера: a^φ(m) ≡ 1 (mod m) при gcd(a,m) = 1
// Теорема Лагранжа: f(x) ≡ 0 (mod p) имеет ≤ n решений (степень n)
// Дирихле: в прогрессии a+nd при gcd(a,d)=1 бесконечно много простых
//
// Реализованы через powmod, fi_single, jacobi (в других секциях)

// =============================================================
// C. ЛИНЕЙНЫЕ СРАВНЕНИЯ
// =============================================================

// --- C.1. Решение линейного сравнения ---
// a·x ≡ b (mod m) → все решения x ∈ [0, m-1]
//
// Алгоритм:
//   1. d = gcd(a, m)
//   2. Если b не делится на d → нет решения
//   3. Иначе: x₀ = (b/d) · (a/d)⁻¹ mod (m/d)
//   5. Все решения: x₀, x₀ + m', x₀ + 2m', ..., x₀ + (d-1)m'
//
// Пример: solve_linear_congruence(3, 6, 9) → {2, 5, 8}
vector<ll> solve_linear_congruence(ll a, ll b, ll m) {
    ll d = gcd_mod(a, m);
    if (b % d != 0) return {};

    a /= d; b /= d; m /= d;
    ll x0 = b * modinv(a, m) % m;

    vector<ll> res;
    for (ll i = 0; i < d; i++)
        res.push_back((x0 + i * m) % (m * d));
    return res;
}

// =============================================================
// D. ДИОФАНТОВЫ УРАВНЕНИЯ
// =============================================================

// --- D.1. Линейное диофантово уравнение ---
// ax + by = c → находит (x, y) если решение есть
//
// Алгоритм: egcd(a,b) → gcd, x₁, y₁ → x₀ = x₁·c/g, y₀ = y₁·c/g
//
// Пример: solve_linear_diophantine(6, 15, 9) → x=9, y=-3 → 6·9+15·(-3)=9 ✓
pair<ll, ll> solve_linear_diophantine(ll a, ll b, ll c) {
    ll x, y;
    ll g = egcd(a, b, x, y);
    if (c % g != 0) return {-1, -1};
    return {x * c / g, y * c / g};
}

// --- D.2. Уравнение Пелля ---
// x² - Dy² = 1, D — не квадрат
// Алгоритм: непрерывные дроби √D → конвергенты → фундаментальное решение
//
// Пример: pell(2) → x=3, y=2 → 9 - 8 = 1
pair<ll, ll> pell(int D) {
    int sq = (int)sqrt(D);
    if (sq * sq == D) return {-1, -1};

    ll a0 = sq;
    ll m = 0, d = 1, a = a0;
    ll x_prev = 1, x_curr = a0;
    ll y_prev = 0, y_curr = 1;

    while (x_curr * x_curr - (ll)D * y_curr * y_curr != 1) {
        m = d * a - m;
        d = (D - m * m) / d;
        a = (a0 + m) / d;

        ll x_next = a * x_curr + x_prev;
        ll y_next = a * y_curr + y_prev;

        x_prev = x_curr; x_curr = x_next;
        y_prev = y_curr; y_curr = y_next;
    }

    return {x_curr, y_curr};
}

// --- D.3. Пифагоровы тройки ---
// x² + y² = z², gcd(x,y,z) = 1 (примитивные)
// Формула: x = m² - n², y = 2mn, z = m² + n²
//   где m > n > 0, gcd(m,n) = 1, m и n разной четности
//
// Пример: pythagorean_triples(5) → (3,4,5), (5,12,13), (15,8,17), (7,24,25)
void pythagorean_triples(int max_m) {
    for (int m = 2; m <= max_m; m++) {
        for (int n = 1; n < m; n++) {
            if ((m + n) % 2 == 0) continue;
            if (gcd_mod(m, n) != 1) continue;
            int x = m * m - n * n;
            int y = 2 * m * n;
            int z = m * m + n * n;
            cout << x << "² + " << y << "² = " << z << "²" << endl;
        }
    }
}

// =============================================================
// E. БИНАРНОЕ ВОЗВЕДЕНИЕ В СТЕПЕНЬ
// =============================================================

// --- E.1. Бинарное возведение в степень по модулю ---
// a^exp % mod за O(log exp) умножений
//
// Идея: exp в двоичном: exp = b₀ + b₁·2 + b₂·4 + ...
// a^exp = a^b₀ · (a²)^b₁ · (a⁴)^b₂ · ...
//
// Пример: powmod(2, 13, 1000) = 2^13 % 1000 = 8192 % 1000 = 192
ll powmod(ll a, ll exp, ll mod) {
    a %= mod;
    ll result = 1;
    while (exp > 0) {
        if (exp & 1) result = result * a % mod;
        a = a * a % mod;
        exp >>= 1;
    }
    return result;
}

// --- E.2. Быстрое возведение в степень без модуля ---
// a^exp за O(log exp) умножений
ll powll(ll a, ll exp) {
    ll result = 1;
    while (exp > 0) {
        if (exp & 1) result *= a;
        a *= a;
        exp >>= 1;
    }
    return result;
}

// =============================================================
// F. РАСШИРЕННЫЙ АЛГОРИТМ ЕВКЛИДА
// =============================================================

// --- F.1. Обычный Евклид (в a.cpp: gcd_mod) ---
// Формула: gcd(a, b) = gcd(b, a % b)
// Сложность: O(log(min(a,b)))

// --- F.2. Расширенный алгоритм Евклида ---
// Находит (gcd, x, y) такие что a·x + b·y = gcd(a, b)
//
// Идея: на каждом шаге Евклида запоминаем коэффициенты:
//   если a = b·q + r, и известно r·x₁ + b·y₁ = gcd,
//   то a·x₁ + b·(y₁ - q·x₁) = gcd
//
// Пример: egcd(30, 20) → gcd=10, x=1, y=-1 → 30·1 + 20·(-1) = 10
ll egcd(ll a, ll b, ll &x, ll &y) {
    if (b == 0) { x = 1; y = 0; return a; }
    ll x1, y1;
    ll g = egcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

// --- F.3. Обратный элемент по модулю ---
// a⁻¹ mod m — число x такое что a·x ≡ 1 (mod m)
// Алгоритм: egcd(a, m) → если gcd == 1 → x и есть a⁻¹
//
// Пример: modinv(3, 7) → egcd(3,7) → x=-2 → (-2 % 7 + 7) % 7 = 5 → 3·5=15≡1 (mod 7)
ll modinv(ll a, ll m) {
    ll x, y;
    ll g = egcd(a, m, x, y);
    if (g != 1) return -1;
    return (x % m + m) % m;
}

// =============================================================
// G. ТЕОРЕТИКО-ЧИСЛОВЫЕ ФУНКЦИИ
// =============================================================

// --- G.1. Функция Кармайкла λ(n) ---
// Наименьшее m такое что aᵐ ≡ 1 (mod n) для всех a с gcd(a,n) = 1
//
// Свойства:
//   λ(pᵏ) = φ(pᵏ) для нечётных простых
//   λ(2) = 1, λ(4) = 2, λ(2ᵏ) = 2ᵏ⁻² при k ≥ 3
//   λ(lcm(m,n)) = lcm(λ(m), λ(n)) при gcd(m,n) = 1
//
// Пример: carmichael(7) = 6, carmichael(15) = 4
ll carmichael(ll n) {
    if (n == 1) return 1;
    if (n == 2) return 1;
    if (n == 4) return 2;

    ll result = 1;
    ll temp = n;

    for (ll p = 2; p * p <= temp; p++) {
        if (temp % p == 0) {
            ll pk = 1;
            while (temp % p == 0) {
                temp /= p;
                pk *= p;
            }

            ll lambda_p;
            if (p == 2) {
                if (pk == 2) lambda_p = 1;
                else if (pk == 4) lambda_p = 2;
                else lambda_p = pk / 4;
            } else {
                lambda_p = pk - pk / p;
            }

            result = result / gcd_mod(result, lambda_p) * lambda_p;
        }
    }

    if (temp > 1) {
        ll lambda_p = temp - 1;
        result = result / gcd_mod(result, lambda_p) * lambda_p;
    }

    return result;
}

// --- G.2. Радикал числа rad(n) ---
// rad(n) = ∏_{p|n} p — произведение различных простых делителей
//
// Пример: rad(12) = rad(2²·3) = 6, rad(1) = 1
ll radical(ll n) {
    ll result = 1;
    for (ll p = 2; p * p <= n; p++) {
        if (n % p == 0) {
            result *= p;
            while (n % p == 0) n /= p;
        }
    }
    if (n > 1) result *= n;
    return result;
}

// --- G.3. Функция Мангольдта Λ(n) ---
// Λ(n) = ln p если n = pᵏ (степень простого), иначе 0
// Свойство: Σ_{d|n} Λ(d) = ln n
//
// Пример: mangoldt(8) = ln 2, mangoldt(6) = 0
double mangoldt(ll n) {
    if (n <= 1) return 0;

    ll p = -1;
    ll temp = n;
    for (ll i = 2; i * i <= temp; i++) {
        if (temp % i == 0) {
            p = i;
            while (temp % i == 0) temp /= i;
            if (temp > 1) return 0;
            break;
        }
    }
    if (p == -1) p = n;
    return log((double)p);
}

// =============================================================
// H. КВАДРАТНЫЕ ВЫЧЕТЫ И СИМВОЛ ЛЕЖАНДРА
// =============================================================

// --- H.1. Символ Лежандра (a/p) ---
// (a/p) = a^((p-1)/2) mod p ∈ {0, 1, -1}
//   (a/p) = 1  если a — квадратный вычет по mod p
//   (a/p) = -1 если a — невычет
//   (a/p) = 0  если p | a
//
// Пример: legendre(2, 7) = 1 (вычет), legendre(3, 7) = -1 (невычет)
int legendre(ll a, ll p) {
    if (p == 2) return (a % 2 == 0) ? 0 : 1;
    a %= p;
    if (a == 0) return 0;
    ll l = powmod(a, (p - 1) / 2, p);
    return (l <= 1) ? l : -1;
}

// --- H.2. Символ Якоби (a/n) ---
// Обобщение символа Лежандра на составные n
// Определение: (a/n) = ∏ (a/pᵢ)^{eᵢ} где n = ∏ pᵢ^eᵢ
// Вычисление через закон взаимности за O(log²n)
//
// Пример: jacobi(2, 15) = 1, jacobi(3, 15) = 0
int jacobi(ll a, ll n) {
    if (n <= 0 || n % 2 == 0) return 0;
    a %= n;
    int result = 1;
    while (a != 0) {
        while (a % 2 == 0) {
            a /= 2;
            if (n % 8 == 3 || n % 8 == 5) result = -result;
        }
        swap(a, n);
        if (a % 4 == 3 && n % 4 == 3) result = -result;
        a %= n;
    }
    return (n == 1) ? result : 0;
}

// --- H.3. Проверка квадратного вычета ---
// a — квадратный вычет по mod p ⟺ (a/p) = 1
//
// Пример: is_quadratic_residue(2, 7) = true
bool is_quadratic_residue(ll a, ll p) {
    return legendre(a, p) == 1;
}

// =============================================================
// I. ПРОДВИНУТЫЕ ТЕОРЕТИЧЕСКИЕ ТЕМЫ
// =============================================================
// p-адические числа, лемма Гензеля, кольца главных идеалов, теория дивизоров
// — теоретические темы, реализация не требуется

// =============================================================
// J. ПРИКЛАДНЫЕ ТЕМЫ
// =============================================================
// RSA, Диффи-Хеллман, эллиптические кривые, коды Рида-Соломона
// — прикладные темы, реализация не требуется

}; // конец struct ModArithmetic

#ifndef DISCRETE_LOG_MAIN
// =============================================================
// MAIN — ДЕМОНСТРАЦИЯ
// =============================================================

signed main() {
    ModArithmetic ma;

    cout << "=== E. Бинарное возведение в степень ===" << endl;
    cout << "2^13 % 1000 = " << ma.powmod(2, 13, 1000) << " (ожидается 192)" << endl;
    cout << "3^20 % 100 = " << ma.powmod(3, 20, 100) << endl;

    cout << "\n=== F. Расширенный Евклид ===" << endl;
    ll x, y;
    ll g = ma.egcd(30, 20, x, y);
    cout << "gcd(30,20) = " << g << ", x=" << x << ", y=" << y
         << " → 30·" << x << " + 20·" << y << " = " << 30*x + 20*y << endl;

    g = ma.egcd(6, 15, x, y);
    cout << "gcd(6,15) = " << g << ", x=" << x << ", y=" << y
         << " → 6·" << x << " + 15·" << y << " = " << 6*x + 15*y << endl;

    cout << "\n=== F. Обратный элемент ===" << endl;
    cout << "3⁻¹ mod 7 = " << ma.modinv(3, 7) << " (ожидается 5)" << endl;
    cout << "5⁻¹ mod 17 = " << ma.modinv(5, 17) << endl;

    cout << "\n=== A. CRT (стандартная) ===" << endl;
    vector<ll> a = {2, 3, 2}, m = {3, 5, 7};
    cout << "x ≡ 2 (mod 3), x ≡ 3 (mod 5), x ≡ 2 (mod 7)" << endl;
    cout << "x = " << ma.crt(a, m) << " (ожидается 23)" << endl;

    cout << "\n=== A. CRT (общая) ===" << endl;
    vector<ll> a2 = {2, 4}, m2 = {4, 6};
    auto [xg, lg] = ma.crt_general(a2, m2);
    cout << "x ≡ 2 (mod 4), x ≡ 4 (mod 6) → x = " << xg << " (mod " << lg << ")" << endl;

    cout << "\n=== A. Гарнер ===" << endl;
    cout << "x ≡ 2 (mod 3), x ≡ 3 (mod 5), x ≡ 2 (mod 7)" << endl;
    cout << "x = " << ma.garner({2, 3, 2}, {3, 5, 7}) << " (ожидается 23)" << endl;

    cout << "\n=== A. Количество чисел по модулю ===" << endl;
    cout << "|{a ∈ [10,30] : a ≡ 2 (mod 5)}| = " << ma.count_in_range(10, 30, 2, 5)
         << " (ожидается 4)" << endl;

    cout << "\n=== D. Линейное диофантово уравнение ===" << endl;
    auto [xd, yd] = ma.solve_linear_diophantine(6, 15, 9);
    cout << "6x + 15y = 9 → x=" << xd << ", y=" << yd
         << " → 6·" << xd << " + 15·" << yd << " = " << 6*xd + 15*yd << endl;

    cout << "\n=== C. Линейное сравнение ===" << endl;
    auto sol = ma.solve_linear_congruence(3, 6, 9);
    cout << "3x ≡ 6 (mod 9) → x ∈ {";
    for (int i = 0; i < (int)sol.size(); i++) {
        if (i) cout << ", ";
        cout << sol[i];
    }
    cout << "}" << endl;

    cout << "\n=== D. Уравнение Пелля ===" << endl;
    for (int D = 2; D <= 13; D++) {
        int sq = (int)sqrt(D);
        if (sq * sq == D) continue;
        auto [px, py] = ma.pell(D);
        cout << "D=" << D << ": x=" << px << ", y=" << py
             << " → " << px << "² - " << D << "·" << py << "² = "
             << px*px - (ll)D*py*py << endl;
    }

    cout << "\n=== D. Пифагоровы тройки ===" << endl;
    ma.pythagorean_triples(10);

    cout << "\n=== G. Радикал числа ===" << endl;
    for (int n = 1; n <= 20; n++)
        cout << "rad(" << n << ")=" << ma.radical(n) << "  ";
    cout << endl;

    cout << "\n=== G. Функция Мангольдта (проверка Σ Λ(d) = ln n) ===" << endl;
    for (int n = 2; n <= 20; n++) {
        double sum = 0;
        for (int d = 1; d <= n; d++)
            if (n % d == 0) sum += ma.mangoldt(d);
        cout << "n=" << n << ": Σ Λ(d)=" << sum << ", ln(n)=" << log((double)n)
             << (abs(sum - log((double)n)) < 0.01 ? " ✓" : " ✗") << endl;
    }

    cout << "\n=== G. Функция Кармайкла ===" << endl;
    for (int n = 1; n <= 20; n++)
        cout << "λ(" << n << ")=" << ma.carmichael(n) << "  ";
    cout << endl;

    cout << "\n=== H. Символ Лежандра ===" << endl;
    for (int a = 1; a <= 10; a++)
        cout << "(" << a << "/7) = " << ma.legendre(a, 7) << "  ";
    cout << endl;
    cout << "(2/7) = " << ma.legendre(2, 7) << " (ожидается 1)" << endl;
    cout << "(3/7) = " << ma.legendre(3, 7) << " (ожидается -1)" << endl;

    cout << "\n=== H. Символ Якоби ===" << endl;
    cout << "(2/15) = " << ma.jacobi(2, 15) << " (ожидается 1)" << endl;
    cout << "(3/15) = " << ma.jacobi(3, 15) << " (ожидается 0)" << endl;
    cout << "(5/15) = " << ma.jacobi(5, 15) << " (ожидается 0, т.к. gcd(5,15)>1)" << endl;

    cout << "\n=== H. Квадратные вычеты по mod 7 ===" << endl;
    for (int a = 1; a <= 6; a++)
        cout << a << " — " << (ma.is_quadratic_residue(a, 7) ? "вычет" : "невычет") << endl;
}
#endif
