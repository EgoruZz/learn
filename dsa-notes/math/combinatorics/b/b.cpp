#ifndef COMBINATORICS_B_CPP
#define COMBINATORICS_B_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
using namespace std;

// =============================================================
// B. ПРОИЗВОДЯЩИЕ ФУНКЦИИ
// =============================================================
// Структура md: A. ОПФ (определение, операции, ряды, рекурренты)
//               → B. ЭПФ (помеченные структуры)
//               → C. Приложения (пути, разбиения, размен)
//
// C_iter — каноническая реализация C(n,k) живёт в a.cpp; здесь
// используются её, факториалы (fact_precompute, inv_fact_precompute)
// и база ModArithmetic (powmod, modinv).
//
// Содержит:
//   A. ОПФ-операции: poly_mul, poly_shift, poly_deriv, poly_integral,
//      inverses, poly_inv, poly_log, poly_exp, poly_pow
//   A. Рекурренты: kitamasa
//   B. ЭПФ: epgf_mul, bell_egf, stirling2_row
//   C. Приложения: lattice_paths, lattice_paths_obstacles,
//      partitions_pentagonal, bounded_sum_count, coin_change_ways
//
// ВАЖНО (модуль): все функции работают по простому модулю mod
// (нужны обратные элементы). Для составного модуля — факторизация
// + CRT (см. C_composite в a.cpp).

#define COMBINATORICS_MAIN
#include "../a/a.cpp"

struct GeneratingFunctions : Combinatorics {

// =============================================================
// A. ОПФ: ОСНОВНЫЕ ОПЕРАЦИИ
// =============================================================

// --- A.1.2. Умножение многочленов (свёртка Коши) O(n·m) ---
// (F·G)(x) = Σₙ (Σₖ aₖ·bₙ₋ₖ)·xⁿ — коэффициент при xⁿ = число способов
// собрать n из объекта A размера k и объекта B размера n−k.
// Промежуточные произведения берём по модулю: иначе переполнение int64.
vector<long long> poly_mul(const vector<long long>& a, const vector<long long>& b,
                           long long mod) {
    if (a.empty() || b.empty()) return {};
    vector<long long> c(a.size() + b.size() - 1, 0);
    for (int i = 0; i < (int)a.size(); i++)
        for (int j = 0; j < (int)b.size(); j++)
            c[i + j] = (c[i + j] + a[i] * b[j]) % mod;
    return c;
}

// --- A.1.3. Сдвиг: умножение на x^k ---
// xᵏ·F(x) = Σ aₙ·xⁿ⁺ᵏ — добавляет k нулевых коэффициентов в начало.
vector<long long> poly_shift(const vector<long long>& a, int k) {
    vector<long long> c(a.size() + k, 0);
    for (int i = 0; i < (int)a.size(); i++) c[i + k] = a[i];
    return c;
}

// --- A.1.3. Производная O(n) ---
// (aᵢ·xⁱ)' = (i+1)·aᵢ₊₁·xⁱ — сдвиг коэффициентов с умножением на степень.
vector<long long> poly_deriv(const vector<long long>& a, long long mod) {
    if (a.size() <= 1) return {};
    vector<long long> d(a.size() - 1);
    for (int i = 1; i < (int)a.size(); i++)
        d[i - 1] = a[i] * i % mod;
    return d;
}

// --- A.1.3. Интеграл O(n) ---
// ∫ aᵢ·xⁱ dx = aᵢ/(i+1)·xⁱ⁺¹ — требует обратные к 1..n (mod простой).
vector<long long> poly_integral(const vector<long long>& a, long long mod) {
    vector<long long> inv = inverses((int)a.size() + 1, mod);
    vector<long long> I(a.size() + 1, 0);
    for (int i = 0; i < (int)a.size(); i++)
        I[i + 1] = a[i] * inv[i + 1] % mod;
    return I;
}

// --- A.1.3. Обратные 1..n за O(n) (модуль простой) ---
// inv[i] = mod − (mod/i)·inv[mod%i]: mod = i·(mod/i) + mod%i,
// делим на i·(mod%i) по модулю.
vector<long long> inverses(int n, long long mod) {
    vector<long long> inv(n + 1, 1);
    for (int i = 2; i <= n; i++)
        inv[i] = (mod - (mod / i) * inv[mod % i] % mod) % mod;
    return inv;
}

// =============================================================
// A. ОПФ: ФОРМАЛЬНЫЕ СТЕПЕННЫЕ РЯДЫ
// =============================================================

// --- A.2.1. Обратный ряд g = 1/F по модулю xⁿ O(n²) ---
// Условие: a[0] ≠ 0. g[0] = 1/a[0];
// для i > 0: g[i] = −(1/a[0])·Σ_{j=1..i} a[j]·g[i−j]
// (коэффициент при xⁱ в F·g должен быть 0).
vector<long long> poly_inv(const vector<long long>& a, int n, long long mod) {
    vector<long long> g(n, 0);
    g[0] = modinv(a[0], mod);
    for (int i = 1; i < n; i++) {
        long long s = 0;
        for (int j = 1; j <= i && j < (int)a.size(); j++)
            s = (s + a[j] * g[i - j]) % mod;
        g[i] = (mod - s) * g[0] % mod;
    }
    return g;
}

// --- A.2.2. Логарифм ряда ln F по модулю xⁿ O(n²) ---
// ln F = ∫ F'/F. Условие: F(0) = 1 (иначе ln F(0) — отдельный член).
vector<long long> poly_log(const vector<long long>& a, int n, long long mod) {
    vector<long long> d = poly_deriv(a, mod);         // F'
    vector<long long> inv = poly_inv(a, n, mod);      // 1/F
    vector<long long> prod = poly_mul(d, inv, mod);   // F'/F
    prod.resize(n);
    return poly_integral(prod, mod);                  // ∫ F'/F
}

// --- A.2.3. Экспонента ряда exp F по модулю xⁿ (Ньютон) O(n²) ---
// Условие: F(0) = 0. Итерация Ньютона: g ← g·(1 − ln g + F) mod x^m,
// m удваивается; ln g считаем на текущей точности.
vector<long long> poly_exp(const vector<long long>& a, int n, long long mod) {
    vector<long long> g(1, 1);
    for (int m = 1; m < n; m = min(m * 2, n)) {
        int m2 = min(m * 2, n);
        g.resize(m2, 0);
        vector<long long> lg = poly_log(g, m2, mod);  // ln g (g[0]=1)
        vector<long long> h(m2);
        h[0] = (1 - lg[0] + mod) % mod;               // 1 − ln g + F
        for (int i = 1; i < m2; i++) h[i] = (mod - lg[i]) % mod;
        for (int i = 0; i < m2 && i < (int)a.size(); i++)
            h[i] = (h[i] + a[i]) % mod;
        g = poly_mul(g, h, mod);                      // g·(1 − ln g + F)
        g.resize(m2);
    }
    g.resize(n);
    return g;
}

// --- A.2.4. Степень ряда F^k = exp(k·ln F) O(n²) ---
// Условие: F(0) = 1. Показатель k берём по модулю mod (mod простой —
// по малой теореме Ферма для коэффициентов ряда работает).
vector<long long> poly_pow(const vector<long long>& a, long long k, int n, long long mod) {
    vector<long long> lg = poly_log(a, n, mod);
    for (auto& x : lg) x = x * (k % mod) % mod;
    return poly_exp(lg, n, mod);
}

// =============================================================
// A. РЕШЕНИЕ РЕКУРРЕНТНЫХ СООТНОШЕНИЙ
// =============================================================

// --- A.3.3. Китамаса: n-й член линейной рекурренты O(k²·log n) ---
// aₙ = c₁·aₙ₋₁ + c₂·aₙ₋₂ + ... + cₖ·aₙ₋ₖ, даны init[0..k−1].
// Идея: aₙ = Σ bᵢ·aᵢ, где xⁿ ≡ Σ bᵢ·xⁱ (mod Q),
// Q(x) = xᵏ − c₁·xᵏ⁻¹ − ... − cₖ — характеристический многочлен.
// Возводим многочлен x в степень n по модулю Q бинарным возведением:
// редукция xⁱ = Σ_{j=1..k} cⱼ·xⁱ⁻ʲ при i ≥ k.
long long kitamasa(const vector<long long>& init, const vector<long long>& coef,
                   long long n, long long mod) {
    int k = (int)coef.size();
    if (n < (long long)init.size()) return init[n] % mod;

    auto mul = [&](const vector<long long>& A, const vector<long long>& B) {
        vector<long long> C(2 * k, 0);
        for (int i = 0; i < k; i++)
            for (int j = 0; j < k; j++)
                C[i + j] = (C[i + j] + A[i] * B[j]) % mod;
        for (int i = 2 * k - 2; i >= k; i--)
            if (C[i])
                for (int j = 1; j <= k; j++)
                    C[i - j] = (C[i - j] + C[i] * coef[j - 1]) % mod;
        C.resize(k);
        return C;
    };

    vector<long long> res(k, 0), base(k, 0);
    res[0] = 1;                        // x⁰
    if (k == 1) base[0] = coef[0];     // x ≡ c₁ (mod Q)
    else base[1] = 1;                  // x¹
    while (n > 0) {
        if (n & 1) res = mul(res, base);
        base = mul(base, base);
        n >>= 1;
    }
    long long ans = 0;
    for (int i = 0; i < k; i++)
        ans = (ans + res[i] * init[i]) % mod;
    return ans;
}

// =============================================================
// B. ЭКСПОНЕНЦИАЛЬНЫЕ ПРОИЗВОДЯЩИЕ ФУНКЦИИ (ЭПФ)
// =============================================================

// --- B.2. Умножение ЭПФ: биномиальная свёртка O(n²) ---
// (A·B)(x) = Σ cₙ·xⁿ/n!, cₙ = Σₖ C(n,k)·aₖ·bₙ₋ₖ:
// разделить n помеченных элементов на два подмножества — C(n,k) способов.
// a, b — ЭПФ-коэффициенты (aₙ = число структур размера n).
vector<long long> epgf_mul(const vector<long long>& a, const vector<long long>& b,
                           long long mod,
                           const vector<long long>& fact,
                           const vector<long long>& inv_fact) {
    int n = (int)a.size() + (int)b.size() - 1;
    vector<long long> c(n, 0);
    for (int k = 0; k < (int)a.size(); k++)
        for (int j = 0; j < (int)b.size(); j++) {
            long long comb = fact[k + j] * inv_fact[k] % mod * inv_fact[j] % mod;
            c[k + j] = (c[k + j] + a[k] * b[j] % mod * comb) % mod;
        }
    return c;
}

// --- B.3.2. Числа Белла через ЭПФ: B(x) = e^(eˣ − 1) ---
// Разбиение = множество непустых блоков. eˣ − 1 — непустое множество,
// exp — множество блоков. Возвращает B(0..n) по модулю mod.
vector<long long> bell_egf(int n, long long mod,
                           const vector<long long>& fact,
                           const vector<long long>& inv_fact) {
    vector<long long> a(n + 1, 0);
    for (int i = 1; i <= n; i++) a[i] = inv_fact[i];   // eˣ − 1 (коэффициенты 1/i!)
    vector<long long> e = poly_exp(a, n + 1, mod);     // e^(eˣ−1): Σ B(n)/n!·xⁿ
    vector<long long> res(n + 1);
    for (int i = 0; i <= n; i++) res[i] = e[i] * fact[i] % mod;
    return res;
}

// --- B.3.3. Строка Стирлинга II рода: S(n, 0..n) через свёртку O(n²) ---
// S(n,k) = 1/k!·Σ (−1)^(k−i)·C(k,i)·iⁿ = (−1)^k·Σ f[i]·g[k−i],
// f[i] = (−1)^i·iⁿ/i!, g[j] = 1/j! — свёртка коэффициентов ЭПФ.
// Требует n < mod (степени iⁿ и факториалы по простому модулю).
vector<long long> stirling2_row(int n, long long mod,
                                const vector<long long>& inv_fact) {
    vector<long long> f(n + 1), g(n + 1);
    for (int i = 0; i <= n; i++) {
        long long pw = powmod(i, n, mod);              // iⁿ
        f[i] = pw * inv_fact[i] % mod;
        if (i & 1) f[i] = (mod - f[i]) % mod;          // (−1)^i
        g[i] = inv_fact[i];
    }
    vector<long long> h = poly_mul(f, g, mod);
    h.resize(n + 1);
    vector<long long> S(n + 1);
    for (int k = 0; k <= n; k++) {
        long long v = h[k];
        if (k & 1) v = (mod - v) % mod;                // (−1)^k
        S[k] = v;
    }
    return S;
}

// =============================================================
// C. КОМБИНАТОРНЫЕ ПРИЛОЖЕНИЯ
// =============================================================

// --- C.1.1. Пути в решётке из (0,0) в (m,n) ---
// Каждый путь — последовательность из m шагов «вправо» и n «вверх»:
// выбрать позиции «вверх» → C(m+n, n). Биекция путь ↔ подмножество.
long long lattice_paths(long long m, long long n) {
    return C_iter(m + n, n);
}

// --- C.1.3. Пути в решётке с препятствиями O(N·M) ---
// ДП: dp[r][c] = dp[r−1][c] + dp[r][c−1], запрещённые клетки = 0.
// obs — координаты запрещённых клеток (0-индексация).
long long lattice_paths_obstacles(int rows, int cols,
                                  const vector<pair<int, int>>& obs,
                                  long long mod) {
    vector<vector<bool>> blocked(rows + 1, vector<bool>(cols + 1, false));
    for (auto& p : obs) blocked[p.first][p.second] = true;
    vector<vector<long long>> dp(rows + 1, vector<long long>(cols + 1, 0));
    dp[0][0] = 1;
    for (int r = 0; r <= rows; r++)
        for (int c = 0; c <= cols; c++) {
            if (r == 0 && c == 0) continue;
            if (blocked[r][c]) continue;
            if (r > 0) dp[r][c] = (dp[r][c] + dp[r - 1][c]) % mod;
            if (c > 0) dp[r][c] = (dp[r][c] + dp[r][c - 1]) % mod;
        }
    return dp[rows][cols];
}

// --- C.2.2. Разбиения чисел: p(0..n) через пентагональную теорему ---
// Σ p(n)xⁿ = ∏ 1/(1−xᵏ). Обратный ряд ∏(1−xᵏ) = Σ (−1)ᵏx^{k(3k−1)/2}
// (пятиугольные числа) → p(n) = Σ (−1)^(k+1)·(p(n−gₖ) + p(n−g₋ₖ)),
// gₖ = k(3k−1)/2, g₋ₖ = k(3k+1)/2. Сложность O(n√n).
vector<long long> partitions_pentagonal(int n, long long mod) {
    vector<long long> p(n + 1, 0);
    p[0] = 1;
    for (int i = 1; i <= n; i++) {
        long long s = 0;
        for (int k = 1; ; k++) {
            int g1 = k * (3 * k - 1) / 2;
            int g2 = k * (3 * k + 1) / 2;
            if (g1 > i && g2 > i) break;
            long long sign = (k % 2 == 1) ? 1 : mod - 1;  // (−1)^(k+1)
            if (g1 <= i) s = (s + sign * p[i - g1]) % mod;
            if (g2 <= i) s = (s + sign * p[i - g2]) % mod;
        }
        p[i] = s;
    }
    return p;
}

// --- C.3. Звёзды и барьеры с верхними границами O(2ᵐ) ---
// Число решений x₁+...+xₘ = n, 0 ≤ xᵢ ≤ bᵢ:
// включения-исключения по нарушенным границам (a.md 2.7 — без границ).
// Σ_S (−1)^{|S|}·C(n − Σ(bᵢ+1) + m − 1, m − 1).
long long bounded_sum_count(long long n, const vector<long long>& b,
                            long long mod) {
    int m = (int)b.size();
    long long res = 0;
    for (int mask = 0; mask < (1 << m); mask++) {
        long long rem = n, sign = 1;
        for (int i = 0; i < m; i++)
            if (mask >> i & 1) { rem -= b[i] + 1; sign = -sign; }
        if (rem < 0) continue;
        long long term = C_iter(rem + m - 1, m - 1) % mod;
        res = (res + sign * term) % mod;
    }
    return (res % mod + mod) % mod;
}

// --- C.4. Размен монет: число способов набрать n монетами номиналов ---
// ОПФ ∏ 1/(1−x^{cᵢ}); ДП по номиналам (порядок циклов важен:
// «сначала номиналы» — наборы, а не композиции). Сложность O(m·n).
long long coin_change_ways(const vector<long long>& coins, long long n,
                           long long mod) {
    vector<long long> dp(n + 1, 0);
    dp[0] = 1;
    for (long long c : coins)
        for (long long s = c; s <= n; s++)
            dp[s] = (dp[s] + dp[s - c]) % mod;
    return dp[n];
}

// =============================================================
// D. МЕТОД КОЭФФИЦИЕНТОВ И ИЗВЛЕЧЕНИЕ [xⁿ]
// =============================================================

// --- D.2. Извлечение [xⁿ] для рациональной P(x)/Q(x) ---
// Частные дроби: если Q(x) = ∏(1 − αᵢx), все корни простые,
// то aₙ = Σ cᵢ·αᵢⁿ, где cᵢ = P(1/αᵢ) / Q'(1/αᵢ)·(−αᵢ).
// Возвращает первые n коэффициентов ряда P/Q.
vector<long long> rational_coeff_extract(const vector<long long>& P,
                                          const vector<long long>& Q,
                                          int n, long long mod) {
    // Наивное: полиномиальное деление P/Q
    vector<long long> result(n, 0);
    vector<long long> rem = P;
    rem.resize(n + Q.size(), 0);
    for (int i = 0; i < n; i++) {
        if ((int)rem.size() <= i) break;
        result[i] = rem[i];
        for (int j = 0; j < (int)Q.size(); j++)
            if (i + j < (int)rem.size())
                rem[i + j] = (rem[i + j] - result[i] * Q[j] % mod + mod) % mod;
    }
    return result;
}

// =============================================================
// E. DP ЧЕРЕЗ ПРОИЗВОДЯЩИЕ ФУНКЦИИ
// =============================================================

// --- E.1. Решение D = 1 + F·D: D(x) = 1/(1 − F(x)) ---
// Если dp[n] = Σ f[k]·dp[n−1−k], dp[0] = 1, то D = 1/(1 − F).
// Возвращает dp[0..n-1].
vector<long long> ogf_rational_inverse(const vector<long long>& F, int n, long long mod) {
    // D = 1/(1 − F) → poly_inv(1 − F)
    vector<long long> one_minus_F(n, 0);
    one_minus_F[0] = 1;
    for (int i = 0; i < (int)F.size() && i < n; i++)
        one_minus_F[i] = (one_minus_F[i] - F[i] + mod) % mod;
    return poly_inv(one_minus_F, n, mod);
}

// =============================================================
// F. ДЕРЕВЬЯ И СТРУКТУРЫ ЧЕРЕЗ OGF
// =============================================================

// --- F.1. Бинарные деревья: B(x) = 1 + x·B(x)² → Bₙ = C(n) ---
// Решаем итеративно: B₀ = 1, Bₙ₊₁ = 1 + x·Bₙ² (mod xⁿ).
vector<long long> binary_trees_ogf(int n, long long mod) {
    vector<long long> B(1, 1); // B₀ = 1
    for (int iter = 0; iter < n; iter++) {
        int m = min((int)B.size() * 2, n);
        vector<long long> B2 = poly_mul(B, B, mod);
        B2.resize(m, 0);
        vector<long long> Bnew(m, 0);
        Bnew[0] = 1; // +1
        for (int i = 1; i < m; i++)
            Bnew[i] = B2[i - 1]; // x·B²: сдвиг на 1
        B = Bnew;
    }
    B.resize(n);
    return B;
}

// --- F.2. Catalan через формулу: C(n) = C(2n,n)/(n+1) ---
vector<long long> catalan_from_equation(int n, long long mod,
                                         const vector<long long>& fact,
                                         const vector<long long>& inv_fact) {
    vector<long long> c(n, 0);
    for (int i = 0; i < n; i++) {
        // C(2i,i) / (i+1)
        long long comb = fact[2 * i] % mod * inv_fact[i] % mod * inv_fact[i] % mod;
        c[i] = comb * modinv(i + 1, mod) % mod;
    }
    return c;
}

// =============================================================
// G. ВЕРОЯТНОСТНЫЕ ПРОИЗВОДЯЩИЕ ФУНКЦИИ (PGF)
// =============================================================

// --- G.2. Математическое ожидание через PGF: E[X] = G'(1) ---
// PGF задана коэффициентами gₖ = P(X=k) (gₖ ≥ 0, Σ gₖ = 1).
long long pgf_expectation(const vector<long long>& pgf, long long mod) {
    // G'(1) = Σ k·gₖ
    long long result = 0;
    for (int k = 0; k < (int)pgf.size(); k++)
        result = (result + k * pgf[k]) % mod;
    return result;
}

// --- G.2. Дисперсия: D[X] = G''(1) + G'(1) − (G'(1))² ---
long long pgf_variance(const vector<long long>& pgf, long long mod) {
    long long E = pgf_expectation(pgf, mod);
    long long E2 = 0;
    for (int k = 0; k < (int)pgf.size(); k++)
        E2 = (E2 + (long long)k * k % mod * pgf[k]) % mod;
    return (E2 - E * E % mod + mod) % mod;
}

// --- G.3. PGF для суммы независимых: G_{X+Y} = G_X · G_Y ---
vector<long long> pgf_sum_independent(const vector<long long>& gx,
                                       const vector<long long>& gy,
                                       long long mod) {
    return poly_mul(gx, gy, mod);
}

// --- G.4. PGF для биномиального: G(t) = (1−p+pt)ⁿ ---
vector<long long> pgf_binomial(int n, long long p, long long q_inv, int sz, long long mod) {
    // (1−p + p·t)ⁿ через бином Ньютона
    vector<long long> base = { (1 - p + mod) % mod, p };
    vector<long long> result = { 1 };
    // Бинарное возведение
    vector<long long> b = base;
    int exp = n;
    while (exp > 0) {
        if (exp & 1) result = poly_mul(result, b, mod);
        b = poly_mul(b, b, mod);
        exp >>= 1;
    }
    result.resize(sz);
    return result;
}

}; // конец struct GeneratingFunctions

#ifdef GENERATING_FUNCTIONS_MAIN
signed main() {
    GeneratingFunctions gf;
    const long long MOD = 1000000007;

    cout << "=== A. ОПФ: операции ===" << endl;
    // (1+x)(1+x) = 1 + 2x + x²
    vector<long long> r = gf.poly_mul({1, 1}, {1, 1}, MOD);
    cout << "poly_mul(1+x, 1+x) = " << r[0] << " " << r[1] << " " << r[2]
         << " (ожид. 1 2 1)" << endl;
    // 1/(1−x) = 1 + x + x² + x³ + ...
    vector<long long> inv = gf.poly_inv({1, MOD - 1}, 5, MOD);
    cout << "poly_inv(1−x) = ";
    for (long long x : inv) cout << x << ' ';
    cout << " (ожид. 1 1 1 1 1)" << endl;
    // exp(x) = 1 + x + x²/2 + x³/6 + x⁴/24
    vector<long long> e = gf.poly_exp({0, 1}, 5, MOD);
    cout << "poly_exp(x) = " << e[0] << " " << e[1] << " " << e[2] << " "
         << e[3] << " " << e[4] << " (ожид. 1 1 500000004 166666668 41666667)" << endl;
    // ln(1/(1−x)) = x + x²/2 + x³/3 + x⁴/4
    vector<long long> lg = gf.poly_log(inv, 5, MOD);
    cout << "poly_log(1/(1−x)) = " << lg[0] << " " << lg[1] << " " << lg[2]
         << " " << lg[3] << " " << lg[4] << " (ожид. 0 1 500000004 333333336 250000002)" << endl;
    // (1−x)⁻² = 1 + 2x + 3x² + 4x³ + ...
    vector<long long> pw = gf.poly_pow({1, MOD - 1}, MOD - 2, 5, MOD);
    cout << "poly_pow((1−x), −2) = " << pw[0] << " " << pw[1] << " " << pw[2]
         << " " << pw[3] << " " << pw[4] << " (ожид. 1 2 3 4 5)" << endl;

    cout << "\n=== A. Рекурренты: Китамаса ===" << endl;
    // Фибоначчи: aₙ = aₙ₋₁ + aₙ₋₂, init {0,1}
    cout << "F(10) = " << gf.kitamasa({0, 1}, {1, 1}, 10, MOD)
         << " (ожид. 55), F(20) = " << gf.kitamasa({0, 1}, {1, 1}, 20, MOD)
         << " (ожид. 6765)" << endl;
    // Трибоначчи: aₙ = aₙ₋₁ + aₙ₋₂ + aₙ₋₃, init {0,0,1}
    cout << "T(10) = " << gf.kitamasa({0, 0, 1}, {1, 1, 1}, 10, MOD)
         << " (ожид. 81), T(30) = " << gf.kitamasa({0, 0, 1}, {1, 1, 1}, 30, MOD)
         << endl;
    // Китамаса для огромных n: F(1e18) mod p — сравниваем с F(50) mod p
    cout << "F(50) mod p = " << gf.kitamasa({0, 1}, {1, 1}, 50, MOD)
         << " (ожид. 586268941)" << endl;
    cout << "F(1e18) mod p = " << gf.kitamasa({0, 1}, {1, 1}, 1000000000000000000LL, MOD)
         << endl;

    cout << "\n=== B. ЭПФ ===" << endl;
    auto fact = gf.fact_precompute(30, MOD);
    auto inv_fact = gf.inv_fact_precompute(30, MOD);
    // eˣ · eˣ = e^{2x}: cₙ = Σ C(n,k) = 2ⁿ (оба ряда урезаны до 6
    // коэффициентов, поэтому печатаем только c₀..c₅)
    vector<long long> one(6, 1);
    vector<long long> ee = gf.epgf_mul(one, one, MOD, fact, inv_fact);
    ee.resize(6);
    cout << "epgf_mul(eˣ, eˣ) = ";
    for (long long x : ee) cout << x << ' ';
    cout << " (ожид. 1 2 4 8 16 32)" << endl;
    // Белл через ЭПФ: B(0..7) = 1 1 2 5 15 52 203 877
    vector<long long> bell = gf.bell_egf(7, MOD, fact, inv_fact);
    cout << "bell_egf(0..7) = ";
    for (long long x : bell) cout << x << ' ';
    cout << " (ожид. 1 1 2 5 15 52 203 877)" << endl;
    // Стирлинг II: S(5,k) = 0 1 15 25 10 1
    vector<long long> s2 = gf.stirling2_row(5, MOD, inv_fact);
    cout << "stirling2_row(5) = ";
    for (long long x : s2) cout << x << ' ';
    cout << " (ожид. 0 1 15 25 10 1)" << endl;

    cout << "\n=== C. Приложения ===" << endl;
    cout << "lattice_paths(3,3) = " << gf.lattice_paths(3, 3)
         << " (ожид. 20), (2,3) = " << gf.lattice_paths(2, 3) << " (ожид. 10)" << endl;
    cout << "пути 2×2 без препятствий = " << gf.lattice_paths_obstacles(2, 2, {}, MOD)
         << " (ожид. 6), с препятствием (1,1) = "
         << gf.lattice_paths_obstacles(2, 2, {{1, 1}}, MOD) << " (ожид. 2)" << endl;
    vector<long long> pp = gf.partitions_pentagonal(10, MOD);
    cout << "p(0..10) = ";
    for (long long x : pp) cout << x << ' ';
    cout << " (ожид. 1 1 2 3 5 7 11 15 22 30 42)" << endl;
    cout << "p(50) = " << gf.partitions_pentagonal(50, MOD)[50]
         << " (ожид. 204226), p(100) = " << gf.partitions_pentagonal(100, MOD)[100]
         << " (ожид. 190569292)" << endl;
    cout << "bounded_sum_count(3, {2,2}) = " << gf.bounded_sum_count(3, {2, 2}, MOD)
         << " (ожид. 2), (5, {2,3}) = " << gf.bounded_sum_count(5, {2, 3}, MOD)
         << " (ожид. 1)" << endl;
    cout << "coin_change({1,5,10,25}, 10) = " << gf.coin_change_ways({1, 5, 10, 25}, 10, MOD)
         << " (ожид. 4), ({2,3}, 6) = " << gf.coin_change_ways({2, 3}, 6, MOD)
         << " (ожид. 2)" << endl;

    cout << "\n=== D. Метод коэффициентов ===" << endl;
    // 1/(1−x) = 1 + x + x² + x³ + ...: P={1}, Q={1,-1}
    auto rc = gf.rational_coeff_extract({1}, {1, MOD - 1}, 5, MOD);
    cout << "1/(1−x) coeffs: ";
    for (long long x : rc) cout << x << ' ';
    cout << " (ожид. 1 1 1 1 1)" << endl;

    cout << "\n=== F. Деревья через OGF ===" << endl;
    // Бинарные деревья: Bₙ = C(n) = 1 1 2 5 14 42
    auto bt = gf.binary_trees_ogf(6, MOD);
    cout << "binary_trees(0..5) = ";
    for (long long x : bt) cout << x << ' ';
    cout << " (ожид. 1 1 2 5 14 42)" << endl;
    // Catalan через формулу: C(n) = C(2n,n)/(n+1)
    auto cat = gf.catalan_from_equation(6, MOD, fact, inv_fact);
    cout << "catalan(0..5) = ";
    for (long long x : cat) cout << x << ' ';
    cout << " (ожид. 1 1 2 5 14 42)" << endl;

    cout << "\n=== G. PGF ===" << endl;
    // Бернуллиевская: X ~ Ber(p=1/2), PGF = (1+p·t)/2 = 0.5 + 0.5t
    vector<long long> ber = {500000004, 500000004}; // mod inverse of 2
    cout << "E[Ber(1/2)] = " << gf.pgf_expectation(ber, MOD)
         << " (ожид. 500000004 = 1/2)" << endl;
    cout << "D[Ber(1/2)] = " << gf.pgf_variance(ber, MOD)
         << " (ожид. 250000002 = 1/4)" << endl;
}
#endif // GENERATING_FUNCTIONS_MAIN
#endif // COMBINATORICS_B_CPP
