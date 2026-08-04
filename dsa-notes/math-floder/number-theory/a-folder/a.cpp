#include <iostream>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <cmath>
#include <algorithm>
using namespace std;

// =============================================================
// A. ДЕЛИМОСТЬ, НОД, НОК, ФУНКЦИИ ДЕЛИТЕЛЕЙ
// =============================================================
// Структура md: A. Что деление? → B. Разложение → C. Функции делителей
//               → D. НОК → E. НОД → F. Мультипликативные функции
//
// Содержит:
//   B. Решета и факторизация: sieve, linear_sieve, fact, factorize
//   C. Функции делителей: divide, all_divisors, d_count, sigma_sum, aliquot_sum
//   D. НОК: lcm
//   E. НОД: gcd_diff, gcd_mod, gcd_mod_rec, stein
//   F. Мультипликативные: mu, mobius_sieve, liouville, liouville_sieve,
//      fi, fi_single, euler_sieve, C, dk_via_fact, d, sigma_via_fact, sigma_sieve

struct Divisibility {

// =============================================================
// B. РАЗЛОЖЕНИЕ НА МНОЖИТЕЛИ
// =============================================================

// --- B.1. Решето Эратосфена O(N log log N) ---
// Идея: если p — простое, то все кратные p (от p² до N) — составные.
// Начинаем с i², потому что все меньшие кратные уже помечены
// предыдущими простыми (2, 3, 5, ...).
//
// Возвращает вектор is_prime, где is_prime[i] == true ⟺ i простое.
vector<bool> sieve(int n) {
    vector<bool> is_prime(n + 1, true);
    is_prime[0] = is_prime[1] = false;
    for (int i = 2; i * i <= n; i++)
        if (is_prime[i])
            for (int j = i * i; j <= n; j += i)
                is_prime[j] = false;
    return is_prime;
}

// --- B.2. Линейное решето O(N) со SPF ---
// SPF[i] — smallest prime factor (минимальный простой делитель i)
//
// Ключевая идея: каждое составное число i*p помечается своим
// минимальным простым делителем. Условие p <= spf[i] гарантирует,
// что i*p помечается делителем p (иначе i*p было бы помечено ранее).
//
// Возвращает вектор spf, где spf[i] = минимальный простой делитель i.
vector<int> linear_sieve(int n) {
    vector<int> spf(n + 1, 0);
    vector<int> primes;
    for (int i = 2; i <= n; i++) {
        if (spf[i] == 0) {
            spf[i] = i;
            primes.push_back(i);
        }
        for (int j = 0; j < (int)primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++)
            spf[i * primes[j]] = primes[j];
    }
    return spf;
}

// --- B.3. Факторизация наивная (без предподсчёта) O(√n) ---
// Разложение n на простые множители за O(√n).
// Перебираем делители от 2 до √n.
//
// Возвращает пару векторов (primes, powers):
//   n = primes[0]^powers[0] * primes[1]^powers[1] * ...
pair<vector<int>, vector<int>> fact(int a) {
    vector<int> primes((int) log2(a), 0), powers((int) log2(a), 0);
    int x = 2, i = 0;
    while (a > 1) {
        for (; a % x == 0; a /= x, powers[i]++);
        (powers[i] == 0) ? x++: primes[i++] = x++;
    }
    return make_pair((primes.resize(i), std::move(primes)), (powers.resize(i), std::move(powers)));
}

// --- B.4. Быстрая факторизация через SPF O(log n) ---
// После linear_sieve(n) вызываем factorize(x, spf) для любого x ≤ n.
//
// Пример: x = 60, spf[60]=2 → 60/2=30 → 30/2=15 → spf[15]=3 → 15/3=5 → spf[5]=5 → 5/5=1
//         → primes={2,3,5}, powers={2,1,1}
pair<vector<int>, vector<int>> factorize(int n, const vector<int>& spf) {
    vector<int> primes, powers;
    while (n > 1) {
        int p = spf[n], cnt = 0;
        while (n % p == 0) n /= p, cnt++;
        primes.push_back(p);
        powers.push_back(cnt);
    }
    return {primes, powers};
}

// =============================================================
// C. ФУНКЦИИ ДЕЛИТЕЛЕЙ
// =============================================================

// --- C.1. Все делители одного числа (отсортированные) за O(√n) ---
// Алгоритм: перебираем пары (x, n/x).
// Сначала собираем пары, затем сортируем "змейкой":
// чётные индексы → слева направо, нечётные → справа налево.
//
// ВАЖНО: второй цикл начинается с последнего НЕЧЁТНОГО индекса,
// иначе при нечётном размере divs (квадраты) пропускается элемент.
vector<int> divide(int a) {
    vector<int> divs, sorted;
    for (int x = 1; x * x <= a; x++) {
        if (a % x == 0) divs.push_back(x);
        if (a % x == 0 && a / x != x) divs.push_back(a / x);
    }
    for (int i = 0; i < (int) divs.size(); i += 2) sorted.push_back(divs[i]);
    for (int i = (int) divs.size() - 1 - (divs.size() % 2); i >= 1; i -= 2)
        sorted.push_back(divs[i]);
    return sorted;
}

// --- C.2. Все делители до N (предподсчёт) O(N log N) ---
// dels[i] — вектор всех делителей числа i.
// Хранение всех делителей до N: O(N log N) памяти, O(1) доступ.
vector<vector<int>> all_divisors(int n) {
    vector<vector<int>> dels(n + 1);
    for (int i = 1; i <= n; i++)
        for (int j = i; j <= n; j += i)
            dels[j].push_back(i);
    return dels;
}

// --- C.3. Количество делителей d(n) ---
// Формула: d(n) = ∏ (αᵢ + 1) где n = ∏ pᵢ^αᵢ
// Связь: d(n) = σ₀(n) — нулевая функция суммы делителей
//
// Пример: n = 12 = 2²·3¹ → d(12) = (2+1)(1+1) = 6
int d_count(int n, const vector<int>& spf) {
    auto [primes, powers] = factorize(n, spf);
    int res = 1;
    for (int p : powers) res *= (p + 1);
    return res;
}

// --- C.4. Сумма делителей σ(n) через факторизацию ---
// Формула: σ(n) = ∏ (pᵢ^{αᵢ+1} - 1)/(pᵢ - 1)
//
// Почему: σ(p^α) = 1 + p + p² + ... + p^α = (p^{α+1} - 1)/(p - 1)
// σ мультипликативна, поэтому σ(n) = ∏ σ(pᵢ^αᵢ)
//
// Пример: n = 12 = 2²·3¹ → σ(12) = (2³-1)/(2-1) · (3²-1)/(3-1) = 7·4 = 28
int sigma_sum(int n, const vector<int>& spf) {
    auto [primes, powers] = factorize(n, spf);
    int res = 1;
    for (int i = 0; i < (int)primes.size(); i++) {
        long long p = primes[i], a = powers[i];
        long long pa1 = 1;
        for (int t = 0; t <= a; t++) pa1 *= p;  // p^{a+1}
        res *= (pa1 - 1) / (p - 1);
    }
    return res;
}

// --- C.5. Сумма аликвот (собственных делителей) до N O(N log N) ---
// s(n) = σ(n) - n = сумма делителей n без самого n
//
// Предподсчёт s[1..N]:
//   для каждого i от 1 до N добавляем i ко всем кратным 2i, 3i, 4i, ...
//
// Использование:
//   Совершенные:  s(n) == n          (6, 28, 496, 8128)
//   Дружественные: s(a) == b && s(b) == a  (220, 284)
//   Социабельные: s(a₁)=a₂, s(a₂)=a₃, ..., s(aₖ)=a₁
vector<int> aliquot_sum(int n) {
    vector<int> s(n + 1, 0);
    for (int i = 1; i <= n; i++)
        for (int j = 2 * i; j <= n; j += i)
            s[j] += i;
    return s;
}

// =============================================================
// D. НАИМЕНЬШЕЕ ОБЩЕЕ КРАТНОЕ (НОК)
// =============================================================

// --- D. НОК через НОД ---
// Формула: lcm(a,b) = |a·b| / gcd(a,b)
// Свойство: lcm(a,b) · gcd(a,b) = |a·b|
int lcm(int a, int b) {
    return a / gcd_mod(a, b) * b;
}

// =============================================================
// E. НАИБОЛЬШИЙ ОБЩИЙ ДЕЛИТЕЛЬ (НОД)
// =============================================================

// --- E.1. НОД через вычитание (алгоритм Евклида, древняя версия) ---
// Идея: gcd(a,b) = gcd(a, b-a) если b > a, иначе gcd(a-b, b)
// Сложность: O(max(a,b)) — медленнее деления
int gcd_diff(int a, int b) {
    if (a > b) swap(a, b);
    while (b > 0) (a > b) ? a -= b : b -= a;
    return a;
}

// --- E.2. НОД через остаток (алгоритм Евклида, классический) ---
// Формула: gcd(a,b) = gcd(b, a % b)
// Сложность: O(log(min(a,b))) — каждое деление уменьшает число хотя бы вдвое
int gcd_mod(int a, int b) {
    while (b) swap(a %= b, b);
    return a;
}

// --- E.3. НОД через остаток (рекурсивная версия) ---
int gcd_mod_rec(int a, int b) {
    return (b == 0) ? a : gcd_mod_rec(b, a % b);
}

// --- E.4. НОД бинарный (алгоритм Стейна) ---
// Идея: используем свойства чётности:
//   gcd(2a, 2b) = 2·gcd(a,b)
//   gcd(2a, b) = gcd(a,b) если b нечётное
//   gcd(a, b) = gcd(|a-b|, min(a,b))
// Сложность: O(log(max(a,b))) — но только побитовые операции
unsigned int stein(unsigned int a, unsigned int b) {
    if (a == 0) return b;
    if (b == 0) return a;
    int shift = __builtin_ctz(a | b);
    a >>= __builtin_ctz(a);
    do {
        b >>= __builtin_ctz(b);
        if (a > b) std::swap(a, b);
        b -= a;
    } while (b);
    return a << shift;
}

// =============================================================
// F. МУЛЬТИПЛИКАТИВНЫЕ ФУНКЦИИ
// =============================================================

// --- F.2.1. Функция Мёбиуса μ(n) для одного числа ---
// Определение:
//   μ(1) = 1
//   μ(n) = 0 если p² | n для некоторого p ( есть квадрат простого)
//   μ(n) = (-1)^k если n = p₁·p₂·...·pₖ (k различных простых)
//
// Пример: μ(6) = μ(2·3) = (-1)² = 1
//         μ(12) = μ(2²·3) = 0 (есть квадрат)
//         μ(30) = μ(2·3·5) = (-1)³ = -1
int mu(int n) {
    auto [primes, powers] = fact(n);
    for (auto ai : powers) if (ai >= 2) return 0;
    return (primes.size() % 2 == 0) ? 1 : -1;
}

// --- F.2.1. Функция Мёбиуса μ(n) — линейное решето O(N) ---
// Логика:
//   mu[1] = 1
//   Для простых p: mu[p] = -1 (один простой множитель)
//   Для i*p: если spf[i] == p → mu[i*p] = 0 (квадрат простого)
//            иначе → mu[i*p] = -mu[i] (ещё один простой множитель)
//
// Ключевое тождество: Σ_{d|n} μ(d) = [n == 1]
vector<int> mobius_sieve(int n) {
    vector<int> spf(n + 1, 0), mu(n + 1, 0);
    vector<int> primes;
    mu[1] = 1;
    for (int i = 2; i <= n; i++) {
        if (spf[i] == 0) {
            spf[i] = i;
            primes.push_back(i);
            mu[i] = -1;
        }
        for (int j = 0; j < (int)primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++) {
            int ip = i * primes[j];
            spf[ip] = primes[j];
            if (spf[i] == primes[j])
                mu[ip] = 0;  // квадрат простого
            else
                mu[ip] = -mu[i];
        }
    }
    return mu;
}

// --- F.2.1.1. Функция Лиувилля λ(n) для одного числа ---
// Определение: λ(n) = (-1)^Ω(n) где Ω(n) — сумма степеней простых делителей
//
// Отличие от μ: λ учитывает кратность (2² вносит +2 в сумму), μ — нет.
//
// Свойства:
//   λ(ab) = λ(a)λ(b) для любых a,b (полная мультипликативность)
//   λ(n) = 1 если n — полный квадрат, -1 если свободно от квадратов, 0 иначе
//
// Пример: λ(12) = λ(2²·3) = (-1)^(2+1) = -1
//         λ(36) = λ(2²·3²) = (-1)^(2+2) = 1
//         λ(6)  = λ(2·3)   = (-1)^(1+1) = 1
int liouville(int n) {
    int omega = 0;
    for (int i = 2; i * i <= n; i++) {
        while (n % i == 0) { omega++; n /= i; }
    }
    if (n > 1) omega++;
    return (omega % 2 == 0) ? 1 : -1;
}

// --- F.2.1.1. Функция Лиувилля λ(n) — линейное решето O(N) ---
// Логика:
//   lambda[1] = 1
//   Для простых p: lambda[p] = -1 (один простой множитель, Ω=1)
//   Для i*p (p ∤ i): lambda[i*p] = lambda[i] * lambda[p] (мультипликативность)
//   Для i*p (p | i): lambda[i*p] = -lambda[i] (Ω увеличивается на 1)
//
// Отличие от μ: всегда ненулевой (μ может быть 0 для квадратов)
vector<int> liouville_sieve(int n) {
    vector<int> spf(n + 1, 0), lambda(n + 1, 0);
    vector<int> primes;
    lambda[1] = 1;
    for (int i = 2; i <= n; i++) {
        if (spf[i] == 0) {
            spf[i] = i;
            primes.push_back(i);
            lambda[i] = -1;
        }
        for (int j = 0; j < (int)primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++) {
            int ip = i * primes[j];
            spf[ip] = primes[j];
            if (spf[i] == primes[j])
                lambda[ip] = -lambda[i];  // p | i → Ω += 1 → знак меняется
            else
                lambda[ip] = lambda[i] * lambda[primes[j]];  // p ∤ i → мультипликативность
        }
    }
    return lambda;
}

// --- F.2.2. Функция Эйлера φ(n) через μ ---
// Формула: φ(n) = Σ_{d|n} μ(d)·(n/d)
//
// Связь с Мёбиусом: это обращение формулы Σ_{d|n} φ(d) = n.
// Если g(n) = Σ f(d), то f(n) = Σ μ(d)·g(n/d).
// Здесь g(n) = n, f(n) = φ(n).
//
// Пример: φ(12) = μ(1)·12 + μ(2)·6 + μ(3)·4 + μ(4)·3 + μ(6)·2 + μ(12)·1
//               = 12 - 6 - 4 + 0 + 2 + 0 = 4
//         Числа, взаимно простые с 12: 1, 5, 7, 11 → 4 штуки
int fi(int n) {
    int result = 0;
    for (auto d : divide(n)) result += mu(d) * (n / d);
    return result;
}

// --- F.2.2. Функция Эйлера φ(n) для одного числа O(√n) ---
// Формула: φ(n) = n × ∏ (1 - 1/pᵢ) по всем различным простым pᵢ
//
// Почему: из n чисел от 1 до n нужно исключить кратные каждому pᵢ.
// Множитель (1 - 1/pᵢ) исключает 1/pᵢ всех чисел.
//
// Пример: φ(12) = 12 × (1-1/2) × (1-1/3) = 12 × 1/2 × 2/3 = 4
int fi_single(int n) {
    int result = n;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            while (n % i == 0) n /= i;
            result -= result / i;
        }
    }
    if (n > 1) result -= result / n;
    return result;
}

// --- F.2.2. Функция Эйлера φ(n) — линейное решето O(N) ---
// Логика:
//   phi[1] = 1
//   Для простых p: phi[p] = p - 1 (все числа от 1 до p-1 взаимно просты)
//   Для i*p: если spf[i] == p → phi[i*p] = phi[i] * p
//            иначе → phi[i*p] = phi[i] * (p - 1)
vector<int> euler_sieve(int n) {
    vector<int> spf(n + 1, 0), phi(n + 1, 0);
    vector<int> primes;
    phi[1] = 1;
    for (int i = 2; i <= n; i++) {
        if (spf[i] == 0) {
            spf[i] = i;
            primes.push_back(i);
            phi[i] = i - 1;
        }
        for (int j = 0; j < (int)primes.size() && primes[j] <= spf[i] && i * primes[j] <= n; j++) {
            int ip = i * primes[j];
            spf[ip] = primes[j];
            if (spf[i] == primes[j])
                phi[ip] = phi[i] * primes[j];
            else
                phi[ip] = phi[i] * (primes[j] - 1);
        }
    }
    return phi;
}

// --- F.2.3. Биномиальный коэффициент C(n, k) ---
// C(n, k) = n! / (k! · (n-k)!) — количество k-элементных подмножеств из n.
//
// Вычисляем итеративно: res = res * (n-i) / (i+1)
// Деление на (i+1) всегда целочисленное, потому что произведение
// k последовательных чисел делится на k!.
//
// Используется в dₖ(n) для подсчёта количества способов
// разложить степень простого на k множителей.
long long C(int n, int k) {
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    long long res = 1;
    for (int i = 0; i < k; i++)
        res = res * (n - i) / (i + 1);
    return res;
}

// --- F.2.4. dₖ(n) для одного числа через факторизацию ---
// Формула: dₖ(n) = ∏ C(aᵢ + k - 1, k - 1)
// где n = ∏ pᵢ^aᵢ
//
// Почему: для каждого простого pᵢ нужно выбрать, сколько раз pᵢ
// входит в каждый из k множителей. Это разбиение aᵢ на k неотрицательных
// слагаемых: C(aᵢ + k - 1, k - 1) — формула "звезд и барьеров".
//
// Пример: n = 12 = 2²·3, k = 3
//   d₃(12) = C(2+3-1, 3-1) · C(1+3-1, 3-1) = C(4,2) · C(3,2) = 6 · 3 = 18
long long dk_via_fact(int n, int k, const vector<int>& spf) {
    auto [primes, powers] = factorize(n, spf);
    long long res = 1;
    for (int p : powers)
        res *= C(p + k - 1, k - 1);
    return res;
}

// --- F.2.4. dₖ(n) через k-кратную Дирихлеву свёртку ---
// dₖ = 1 * 1 * ... * 1 (k копий единичной функции).
//
// Алгоритм: начинаем с di[key] = 1 (= d₁), затем k раз выполняем
// свёртку: dj[key] += di[div] для всех div | key.
// Результат: d(n, k) = d_{k+1}(n) в стандартной нотации
// (потому что начальное состояние уже d₁, и k итераций дают d_{k+1}).
//
// Пример: d(12, 2) = d₃(12) = 18
int d(int n, int k) {
    vector<int> keys = divide(n);
    unordered_map<int, vector<int> > info;
    for (auto key : keys) info[key] = divide(key);

    unordered_map<int, int> di;
    for (auto key : keys) di[key] = 1;

    while (k--) {
        unordered_map<int, int> dj;
        for (auto key : keys) {
            for (auto div : info[key]) {
                dj[key] += di[div];
            }
        }
        di = dj;
    }

    return di[n];
}

// --- F.2.5. σₖ(n) для одного числа через факторизацию ---
// Формула: σₖ(n) = ∏ (pᵢ^{k(aᵢ+1)} - 1)/(pᵢᵏ - 1) при k ≠ 0
//
// Почему: σₖ(p^a) = 1 + p^k + p^{2k} + ... + p^{ak} = (p^{k(a+1)} - 1)/(p^k - 1)
// σₖ мультипликативна, поэтому σₖ(n) = ∏ σₖ(pᵢ^aᵢ)
//
// Пример: n = 12, k=2 → σ₂(12) = (2^{2·3}-1)/(2²-1) · (3^{2·2}-1)/(3²-1)
//                                 = 63/3 · 80/8 = 21 · 10 = 210
//         1²+2²+3²+4²+6²+12² = 1+4+9+16+36+144 = 210
long long sigma_via_fact(int n, int k, const vector<int>& spf) {
    auto [primes, powers] = factorize(n, spf);
    long long res = 1;
    for (int i = 0; i < (int)primes.size(); i++) {
        long long p = primes[i], a = powers[i];
        long long pk = 1;
        for (int t = 0; t < k; t++) pk *= p;
        long long pkap1 = 1;
        for (int t = 0; t < k * (a + 1); t++) pkap1 *= p;
        res *= (pkap1 - 1) / (pk - 1);
    }
    return res;
}

// --- F.2.5. σₖ(n) — линейное решето O(N) для предподсчёта всех n ≤ N ---
//
// Дополнительные массивы:
//   pe[i]  = spf[i]^e — максимальная степень spf[i] в i (например pe[12] = 4)
//   rest[i] = σₖ(m) где m = i / pe[i] — часть числа без spf
//
// Логика:
//   Для простых p: sigma[p] = 1 + p^k
//   Для i*p (p ∤ i): sigma[i*p] = sigma[i] · sigma[p]  (мультипликативность)
//   Для i*p (p | i): sigma[i*p] = rest[i] · σₖ(p^{e+1})
//
// Ключевой момент: ratio σₖ(p^{e+1})/σₖ(p^e) не всегда целочисленный,
// поэтому используем rest[i] (σₖ части без spf) и пересчитываем σₖ(p^{e+1}).
vector<long long> sigma_sieve(int n, int k) {
    vector<int> spf(n + 1, 0), pe(n + 1, 0);
    vector<long long> rest(n + 1, 0), sigma(n + 1, 0);
    vector<int> primes;
    spf[1] = pe[1] = 1; rest[1] = sigma[1] = 1;
    for (int i = 2; i <= n; i++) {
        if (spf[i] == 0) {
            spf[i] = i; pe[i] = i; rest[i] = 1;
            primes.push_back(i);
            long long pk = 1;
            for (int t = 0; t < k; t++) pk *= i;
            sigma[i] = 1 + pk;
        }
        for (int j = 0; j < (int)primes.size() && primes[j] <= spf[i] && (long long)i * primes[j] <= n; j++) {
            int p = primes[j], ip = i * p;
            spf[ip] = p;
            long long pk = 1;
            for (int t = 0; t < k; t++) pk *= p;
            if (p == spf[i]) {
                pe[ip] = pe[i] * p;
                rest[ip] = rest[i];
                long long pe_ip_k = 1;
                for (int t = 0; t < k; t++) pe_ip_k *= pe[ip];
                sigma[ip] = rest[ip] * (pe_ip_k * pk - 1) / (pk - 1);
            } else {
                pe[ip] = p;
                rest[ip] = sigma[i];
                sigma[ip] = sigma[i] * (1 + pk);
            }
        }
    }
    return sigma;
}

}; // конец struct Divisibility

#ifndef DIVISIBILITY_MAIN
signed main() {
    Divisibility d;
    int n = 100;
    auto spf = d.linear_sieve(n);

    cout << "=== B. Sieve (простые до " << n << ") ===" << endl;
    auto is_p = d.sieve(n);
    for (int i = 2; i <= 30; i++) if (is_p[i]) cout << i << " ";
    cout << endl;

    cout << "\n=== B. SPF (минимальные простые делители) ===" << endl;
    for (int i = 2; i <= 20; i++) cout << "spf[" << i << "]=" << spf[i] << " ";
    cout << endl;

    cout << "\n=== B. Факторизация 60 через SPF ===" << endl;
    auto [fp, fpw] = d.factorize(60, spf);
    for (int i = 0; i < (int)fp.size(); i++)
        cout << fp[i] << "^" << fpw[i] << " ";
    cout << endl;

    cout << "\n=== C. Все делители 60 ===" << endl;
    auto divs = d.divide(60);
    for (int x : divs) cout << x << " ";
    cout << endl;

    cout << "\n=== C. d_count(12) = " << d.d_count(12, spf) << " ===" << endl;
    cout << "=== C. sigma_sum(12) = " << d.sigma_sum(12, spf) << " ===" << endl;

    cout << "\n=== F. Мёбиус μ(1..20) ===" << endl;
    auto mu_s = d.mobius_sieve(20);
    for (int i = 1; i <= 20; i++) cout << "μ(" << i << ")=" << mu_s[i] << "  ";
    cout << endl;

    cout << "\n=== F. Эйлер φ(1..20) ===" << endl;
    auto phi = d.euler_sieve(20);
    for (int i = 1; i <= 20; i++) cout << "φ(" << i << ")=" << phi[i] << "  ";
    cout << endl;

    cout << "\n=== F. dₖ(12, k=3) = " << d.dk_via_fact(12, 3, spf) << " ===" << endl;
    cout << "=== F. d(12, k=2) = " << d.d(12, 2) << " (свёртка = d₃) ===" << endl;

    cout << "\n=== F. σ₁(12) = " << d.sigma_via_fact(12, 1, spf) << " ===" << endl;

    auto sig = d.sigma_sieve(100, 1);
    cout << "\n=== F. σ₁(1..10) через линейное решето ===" << endl;
    for (int i = 1; i <= 10; i++) cout << "σ₁(" << i << ")=" << sig[i] << "  ";
    cout << endl;

    auto sig2 = d.sigma_sieve(100, 2);
    cout << "\n=== F. σ₂(1..10) через линейное решето ===" << endl;
    for (int i = 1; i <= 10; i++) cout << "σ₂(" << i << ")=" << sig2[i] << "  ";
    cout << endl;
}
#endif
