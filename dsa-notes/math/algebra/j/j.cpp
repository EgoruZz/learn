#ifndef ALGEBRA_J_CPP
#define ALGEBRA_J_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>
using namespace std;

// =============================================================
// X. СИММЕТРИЧЕСКИЕ МНОГОЧЛЕНЫ, РЕЗУЛЬТАНТ И ДИСКРИМИНАНТ
// =============================================================
// Структура md: A. Симметрические многочлены
//               → B. Результант
//               → C. Дискриминант
//               → D. Специальные многочлены
//
// SymmetricResultant наследует Recurrences (i.cpp). Переиспользует:
//   * Polynomial (a.cpp, A) — базовая структура;
//   * divide/mod (b.cpp, A.2) — деление;
//   * gcd_classic (c.cpp, B.4) — НОД;
//   * derivative (a.cpp, B.6) — производная;
//   * multiply_fft (g.cpp, B.8) — умножение через FFT;
//   * viete (e.cpp, B.3) — формулы Виета.

#include "../i/i.cpp"

struct SymmetricResultant : Recurrences {

// =============================================================
// A. СИММЕТРИЧЕСКИЕ МНОГОЧЛЕНЫ
// =============================================================

// --- A.1. Элементарные симметрические многочлены ---
// eₖ(x₁,...,xₙ) = ∑_{1≤i₁<...<iₖ≤n} x_{i₁}...x_{iₖ}
// Вычисление по рекуррентности: eₖ(S∪{x}) = eₖ(S) + x·eₖ₋₁(S)
vector<long long> elementary_symmetric(const vector<long long>& x) {
    int n = (int)x.size();
    vector<long long> e(n + 1, 0);
    e[0] = 1;
    for (int i = 0; i < n; i++)
        for (int k = n; k >= 1; k--)
            e[k] += x[i] * e[k - 1];
    return e; // e[0]=1, e[1], ..., e[n]
}

// --- A.2. Степенные суммы ---
// pₖ = ∑xᵢᵏ
vector<long long> power_sums(const vector<long long>& x, int max_k) {
    int n = (int)x.size();
    vector<long long> p(max_k + 1, 0);
    for (int i = 0; i < n; i++) {
        long long xi = x[i], xipow = 1;
        for (int k = 0; k <= max_k; k++) {
            p[k] += xipow;
            xipow *= xi;
        }
    }
    return p;
}

// --- A.3. Полные симметрические многочлены ---
// hₖ = ∑_{1≤i₁≤...≤iₖ≤n} x_{i₁}...x_{iₖ} (с повторениями)
vector<long long> complete_symmetric(const vector<long long>& x, int max_k) {
    int n = (int)x.size();
    vector<long long> h(max_k + 1, 0);
    h[0] = 1;
    for (int i = 0; i < n; i++)
        for (int k = 1; k <= max_k; k++)
            h[k] += x[i] * h[k - 1];
    return h;
}

// --- A.4. Основная теорема: выражение симметрического через eₖ ---
// Упрощённо: для однородного монома x₁^{a₁}...xₙ^{aₙ} (a₁≥...≥aₙ)
// возвращаем коэффициенты при e^{α} (полный алгоритм — лексикографическая замена)
//
// Алгоритм лексикографической замены:
//   1. Упорядочить показатели: a₁ ≥ a₂ ≥ ... ≥ aₙ
//   2. Вычесть ведущий мономиал: x₁^{a₁}...xₙ^{aₙ} = e₁^{a₁-a₂} · e₂^{a₂-a₃} · ... · eₙ^{aₙ} · (x₁^{a₁-a₂}...xₙ₋₁^{aₙ₋₁-aₙ})
//   3. Повторять для каждого оставшегося мономиала, пока не получим 0
//   4. Собрать коэффициенты при базисных мономиалах e^α
//
// Полная реализация требует работы с partitions и упорядоченными мономиалами.
// Ниже — упрощённая версия для n ≤ 4.

struct MonomialExponents {
    vector<int> exps; // a₁ ≥ a₂ ≥ ... ≥ aₙ
    MonomialExponents(const vector<int>& e) : exps(e) {
        sort(exps.begin(), exps.end(), greater<int>());
    }
    bool is_zero() const { return exps.empty() || exps[0] == 0; }
};

// Вычитание ведущего мономиала: возвращает коэффициент при e^α и остаток
// x₁^{a₁}...xₙ^{aₙ} = e₁^{a₁-a₂} · e₂^{a₂-a₃} · ... · eₙ^{aₙ}
// (упрощённо — только для случая, когда все aᵢ ≥ 0 и a₁ ≥ ... ≥ aₙ)
void replace_by_symmetric_helper(
    const MonomialExponents& mono,
    int n,
    vector<pair<long long, vector<int>>>& result  // {(коэффициент, показатели e)}
) {
    if (mono.is_zero()) {
        result.push_back({1, vector<int>(n, 0)});
        return;
    }
    // Ведущий мономиал: e₁^{a₁-a₂} · e₂^{a₂-a₃} · ... · eₙ^{aₙ}
    vector<int> e_exp(n, 0);
    for (int i = 0; i < n - 1; i++)
        e_exp[i] = mono.exps[i] - mono.exps[i + 1];
    e_exp[n - 1] = mono.exps[n - 1];
    result.push_back({1, e_exp});
    // Остаток: x₁^{a₁-a₂}·x₂^{a₂-a₃}·...·xₙ₋₁^{aₙ₋₁-aₙ} — рекурсия
    // Для полного алгоритма нужно вычесть этот мономиал и обработать остаток
}

// --- A.4. Публичная функция: выражает симметрический мономиал через eₖ ---
// Для n переменных, мономиал x₁^{a₁}...xₙ^{aₙ} (a₁≥...≥aₙ)
// Возвращает список {коэффициент, показатели e₁^{α₁}...eₙ^{αₙ}}
vector<pair<long long, vector<int>>> replace_by_symmetric(const vector<int>& exponents, int n) {
    vector<pair<long long, vector<int>>> result;
    MonomialExponents mono(exponents);
    replace_by_symmetric_helper(mono, n, result);
    return result;
}

// =============================================================
// B. РЕЗУЛЬТАНТ
// =============================================================

// --- B.5. Матрица Сильвестра ---
// Для P степени n, Q степени m: матрица (n+m)×(n+m).
vector<vector<long long>> sylvester_matrix(const Polynomial& P, const Polynomial& Q) {
    int n = P.degree(), m = Q.degree();
    int sz = n + m;
    vector<vector<long long>> M(sz, vector<long long>(sz, 0));

    // Строки из P: m строк
    for (int i = 0; i < m; i++)
        for (int j = 0; j <= n; j++)
            M[i][i + j] = P[j];
    // Строки из Q: n строк
    for (int i = 0; i < n; i++)
        for (int j = 0; j <= m; j++)
            M[m + i][i + j] = Q[j];
    return M;
}

// --- B.5. Вычисление определителя: алгоритм Bareiss O(sz³) ---
// Bareiss: M[i][j] = (M[i][j]·M[k][k] − M[i][k]·M[k][j]) / prev_pivot
// Гарантирует целочисленное деление на каждом шаге (при целых элементах).
long long determinant(vector<vector<long long>> M) {
    int sz = (int)M.size();
    if (sz == 0) return 1;
    long long prev_pivot = 1;
    long long sign = 1;
    for (int k = 0; k < sz; k++) {
        // Поиск ведущего элемента
        int pivot = -1;
        for (int i = k; i < sz; i++)
            if (M[i][k] != 0) { pivot = i; break; }
        if (pivot == -1) return 0;
        if (pivot != k) {
            swap(M[k], M[pivot]);
            sign = -sign;
        }
        for (int i = k + 1; i < sz; i++) {
            for (int j = k + 1; j < sz; j++)
                M[i][j] = (M[i][j] * M[k][k] - M[i][k] * M[k][j]) / prev_pivot;
        }
        prev_pivot = M[k][k];
    }
    return sign * M[sz - 1][sz - 1];
}

// --- B.5. Результант через матрицу Сильвестра ---
long long resultant(const Polynomial& P, const Polynomial& Q) {
    auto M = sylvester_matrix(P, Q);
    return determinant(M);
}

// =============================================================
// C. ДИСКРИМИНАНТ
// =============================================================

// --- C.8. Дискриминант: D(P) = (−1)^{n(n−1)/2} aₙ⁻¹ Res(P, P') ---
long long discriminant(const Polynomial& P) {
    int n = P.degree();
    if (n <= 1) return 1;
    Polynomial Pp = derivative(P);
    long long res = resultant(P, Pp);
    long long sign = (n * (n - 1) / 2 % 2 == 0) ? 1 : -1;
    long long an = P[n];
    return sign * res / an;
}

// =============================================================
// D. СПЕЦИАЛЬНЫЕ МНОГОЧЛЕНЫ
// =============================================================

// --- D.9. Многочлены Чебышёва 1-го рода (рекуррентность) ---
// T₀=1, T₁=x, Tₙ=2xTₙ₋₁−Tₙ₋₂
Polynomial chebyshev_T(int n) {
    if (n == 0) return Polynomial({1});
    if (n == 1) return Polynomial({0, 1});
    Polynomial T_prev({1}), T_cur({0, 1});
    for (int i = 2; i <= n; i++) {
        Polynomial T_next = Polynomial({0, 2}) * T_cur - T_prev;
        T_prev = T_cur;
        T_cur = T_next;
    }
    return T_cur;
}

// --- D.9. Многочлены Чебышёва 2-го рода ---
// U₀=1, U₁=2x, Uₙ=2xUₙ₋₁−Uₙ₋₂
Polynomial chebyshev_U(int n) {
    if (n == 0) return Polynomial({1});
    if (n == 1) return Polynomial({0, 2});
    Polynomial U_prev({1}), U_cur({0, 2});
    for (int i = 2; i <= n; i++) {
        Polynomial U_next = Polynomial({0, 2}) * U_cur - U_prev;
        U_prev = U_cur;
        U_cur = U_next;
    }
    return U_cur;
}

// --- D.10. Многочлены Бернулли (рекуррентность) ---
// B₀=1, ∑_{k=0}^{n} C(n+1,k)Bₖ=0
vector<long long> bernoulli(int n) {
    vector<long long> B(n + 1, 0);
    B[0] = 1;
    for (int m = 1; m <= n; m++) {
        long long sum = 0;
        for (int k = 0; k < m; k++) {
            // C(m+1, k)
            long long C = 1;
            for (int i = 0; i < k; i++)
                C = C * (m + 1 - i) / (i + 1);
            sum += C * B[k];
        }
        B[m] = -sum / (m + 1);
    }
    return B;
}

// --- D.12. Многочлены Бернштейна ---
// Bᵢⁿ(x) = C(n,i)xⁱ(1−x)ⁿ⁻ⁱ
Polynomial bernstein(int i, int n) {
    // C(n,i)
    long long C = 1;
    for (int j = 0; j < i; j++)
        C = C * (n - j) / (j + 1);
    // xⁱ
    Polynomial xi(i > 0 ? vector<long long>(i, 0) : vector<long long>({0}));
    if (i > 0) xi.coeffs.push_back(1);
    else xi = Polynomial({1});
    // (1−x)ⁿ⁻ⁱ
    Polynomial one_minus_x({1, -1});
    Polynomial omx_pow({1});
    for (int j = 0; j < n - i; j++)
        omx_pow = multiply_fft(omx_pow, one_minus_x);
    return multiply_fft(xi, omx_pow) * C;
}

// =============================================================
// E. МНОГОЧЛЕНЫ БЕЛЛА И ОРТОГОНАЛЬНЫЕ МНОГОЧЛЕНЫ
// =============================================================

// --- E.1. Многочлены Белла: B(n, k) ---
// B(n+1, k) = k·B(n, k) + B(n, k-1)
// B(0, 0) = 1
// Возвращаем B(n, k) по модулю mod.
// Также предоставляем функцию для вычисления всего столбца B(n, 0..n).
long long bell_bnk(int n, int k, long long mod) {
    if (k < 0 || k > n) return 0;
    if (n == 0 && k == 0) return 1;
    // ДП по строкам: dp[j] = B(i, j) для текущей строки i
    vector<long long> dp(n + 1, 0);
    dp[0] = 1; // B(0, 0) = 1
    for (int i = 1; i <= n; i++) {
        vector<long long> next(n + 1, 0);
        for (int j = 1; j <= i; j++) {
            next[j] = (j * dp[j] % mod + dp[j - 1]) % mod;
        }
        dp = next;
    }
    return dp[k];
}

// Возвращает вектор B(n, 0), B(n, 1), ..., B(n, n) по модулю mod.
vector<long long> bell_polynomial(int n, long long mod) {
    vector<long long> result(n + 1);
    for (int k = 0; k <= n; k++)
        result[k] = bell_bnk(n, k, mod);
    return result;
}

// --- E.2. Полиномы Лежандра: Pₙ(x) ---
// P₀ = 1, P₁ = x
// (n+1)·Pₙ₊₁(x) = (2n+1)·x·Pₙ(x) − n·Pₙ₋₁(x)
// Вычисляем коэффициенты Pₙ как многочлен от x.
// points — количество точек для evaluations (если нужно evaluating form).
Polynomial legendre_P(int n) {
    if (n == 0) return Polynomial({1});
    if (n == 1) return Polynomial({0, 1});
    Polynomial P_prev({1}), P_cur({0, 1});
    for (int i = 1; i < n; i++) {
        // (i+1)·P_{i+1} = (2i+1)·x·P_i − i·P_{i-1}
        // P_{i+1} = ((2i+1)·x·P_i − i·P_{i-1}) / (i+1)
        Polynomial xP = Polynomial({0, 1}) * P_cur;  // x · P_i
        Polynomial term1 = xP * (2 * i + 1);
        Polynomial term2 = P_prev * i;
        Polynomial P_next = term1 - term2;
        // Деление на (i+1): все коэффициенты делятся на (i+1) точно
        for (auto& c : P_next.coeffs)
            c /= (i + 1);
        P_prev = P_cur;
        P_cur = P_next;
    }
    return P_cur;
}

// Оценка Pₙ(x) в точке x (по модулю mod).
long long legendre_P_eval(int n, long long x, long long mod) {
    if (n == 0) return 1 % mod;
    if (n == 1) return ((x % mod) + mod) % mod;
    long long P_prev = 1, P_cur = (x % mod + mod) % mod;
    for (int i = 1; i < n; i++) {
        // (i+1)·P_{i+1} = (2i+1)·x·P_i − i·P_{i-1}
        long long P_next = (((2 * i + 1) % mod) * (x % mod) % mod * P_cur % mod
                           - (i % mod) * P_prev % mod + mod) % mod;
        // Деление на (i+1) по модулю: умножаем на обратный элемент
        long long inv = 1, base = (i + 1) % mod, exp = mod - 2;
        while (exp > 0) {
            if (exp & 1) inv = inv * base % mod;
            base = base * base % mod;
            exp >>= 1;
        }
        P_next = P_next * inv % mod;
        P_prev = P_cur;
        P_cur = P_next;
    }
    return P_cur;
}

// --- E.3. Полиномы Лагерра: Lₙ(x) ---
// L₀ = 1, L₁ = 1 − x
// (n+1)·Lₙ₊₁(x) = (2n + 1 − x)·Lₙ(x) − n·Lₙ₋₁(x)
Polynomial laguerre_L(int n) {
    if (n == 0) return Polynomial({1});
    if (n == 1) return Polynomial({1, -1});
    Polynomial L_prev({1}), L_cur({1, -1});
    for (int i = 1; i < n; i++) {
        // (i+1)·L_{i+1} = (2i+1−x)·L_i − i·L_{i-1}
        // (2i+1)·L_i
        Polynomial term1 = L_cur * (2 * i + 1);
        // x·L_i
        Polynomial xL = Polynomial({0, 1}) * L_cur;
        // (2i+1−x)·L_i = term1 − xL
        Polynomial term_left = term1 - xL;
        // i·L_{i-1}
        Polynomial term2 = L_prev * i;
        Polynomial L_next = term_left - term2;
        // Деление на (i+1)
        for (auto& c : L_next.coeffs)
            c /= (i + 1);
        L_prev = L_cur;
        L_cur = L_next;
    }
    return L_cur;
}

// Оценка Lₙ(x) в точке x по модулю mod.
long long laguerre_L_eval(int n, long long x, long long mod) {
    if (n == 0) return 1 % mod;
    if (n == 1) return ((1 - x) % mod + mod) % mod;
    long long L_prev = 1, L_cur = ((1 - x) % mod + mod) % mod;
    for (int i = 1; i < n; i++) {
        // (i+1)·L_{i+1} = (2i+1−x)·L_i − i·L_{i-1}
        long long xmod = (x % mod + mod) % mod;
        long long L_next = ((((2 * i + 1) % mod - xmod + mod) % mod) * L_cur % mod
                           - (i % mod) * L_prev % mod + mod) % mod;
        long long inv = 1, base = (i + 1) % mod, exp = mod - 2;
        while (exp > 0) {
            if (exp & 1) inv = inv * base % mod;
            base = base * base % mod;
            exp >>= 1;
        }
        L_next = L_next * inv % mod;
        L_prev = L_cur;
        L_cur = L_next;
    }
    return L_cur;
}

// --- E.4. Полиномы Эрмита: Hₙ(x) ---
// H₀ = 1, H₁ = 2x
// Hₙ₊₁ = 2x·Hₙ − 2n·Hₙ₋₁
Polynomial hermite_H(int n) {
    if (n == 0) return Polynomial({1});
    if (n == 1) return Polynomial({0, 2});
    Polynomial H_prev({1}), H_cur({0, 2});
    for (int i = 1; i < n; i++) {
        // H_{i+1} = 2x·H_i − 2i·H_{i-1}
        Polynomial xH = Polynomial({0, 1}) * H_cur * 2;
        Polynomial term2 = H_prev * (2 * i);
        Polynomial H_next = xH - term2;
        H_prev = H_cur;
        H_cur = H_next;
    }
    return H_cur;
}

// Оценка Hₙ(x) в точке x по модулю mod.
long long hermite_H_eval(int n, long long x, long long mod) {
    if (n == 0) return 1 % mod;
    if (n == 1) return (2 * (x % mod) % mod + mod) % mod;
    long long H_prev = 1, H_cur = (2 * (x % mod) % mod + mod) % mod;
    for (int i = 1; i < n; i++) {
        // H_{i+1} = 2x·H_i − 2i·H_{i-1}
        long long xmod = (x % mod + mod) % mod;
        long long H_next = (2 * xmod % mod * H_cur % mod
                           - (2 * i % mod) * H_prev % mod + mod) % mod;
        H_prev = H_cur;
        H_cur = H_next;
    }
    return H_cur;
}

}; // struct SymmetricResultant

#endif // ALGEBRA_J_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_J_MAIN
int main() {
    using Poly = SymmetricResultant::Polynomial;
    SymmetricResultant symRes;

    // Пример A.1: элементарные симметрические для {1,2,3}
    auto e = symRes.elementary_symmetric({1, 2, 3});
    cout << "e₀=" << e[0] << " e₁=" << e[1] << " e₂=" << e[2] << " e₃=" << e[3] << "\n";
    // Ожидаем: 1 6 11 6

    // Пример A.2: степенные суммы
    auto p = symRes.power_sums({1, 2, 3}, 4);
    cout << "p₁=" << p[1] << " p₂=" << p[2] << " p₃=" << p[3] << "\n";
    // Ожидаем: 6 14 36

    // Пример B.5: результант
    Poly P({-1, 0, 1}); // x² − 1
    Poly Q({-1, 1});     // x − 1
    cout << "Res(x²-1, x-1) = " << symRes.resultant(P, Q) << "\n"; // 0 (общий корень x=1)

    // Пример C.8: дискриминант
    Poly R({1, -3, 2}); // x² − 3x + 2 = (x−1)(x−2)
    cout << "Disc(x²-3x+2) = " << symRes.discriminant(R) << "\n"; // 1 (корни различны)

    // Пример D.9: Чебышёв T₃
    auto T3 = symRes.chebyshev_T(3);
    cout << "T₃(x) = "; T3.print(); cout << "\n"; // 4x³ − 3x

    return 0;
}
#endif // ALGEBRA_J_MAIN
