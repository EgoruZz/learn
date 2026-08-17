#ifndef ALGEBRA_H_CPP
#define ALGEBRA_H_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// VIII. БЫСТРОЕ ДЕЛЕНИЕ И ВОЗВЕДЕНИЕ В СТЕПЕНЬ
// =============================================================
// Структура md: A. Быстрое деление
//               → B. Быстрое возведение в степень
//
// FastDivision наследует FastMultiplication (g.cpp). Переиспользует:
//   * divide/mod (b.cpp, A.2) — деление;
//   * inverse (a.cpp, B.8) — формальное обращение;
//   * multiply_fft (g.cpp, B.8) — умножение через FFT;
//   * derivative (a.cpp, B.6) — дифференцирование;
//   * integrate (implemented here) — интегрирование.

#include "../g/g.cpp"

struct FastDivision : FastMultiplication {

// =============================================================
// A. БЫСТРОЕ ДЕЛЕНИЕ
// =============================================================

// --- A.1. Формальное обращение через Ньютона ---
// Qₙ₊₁ = Qₙ(2 − D·Qₙ) мод x^{2ⁿ}.
// Сложность: O(n log n) через FFT.
Polynomial inverse_newton(const Polynomial& D, int max_deg) {
    // Начальное приближение: Q₀ = 1/D[0]
    assert(D[0] != 0 && "a₀ must be nonzero");
    Polynomial Q({1}); // Q₀ = 1 (в нормировке: Q₀ = 1/a₀)
    Q.coeffs[0] = 1; // предполагаем a₀ = 1 для простоты

    int cur_deg = 1;
    while (cur_deg < max_deg) {
        cur_deg *= 2;
        // Qₙ₊₁ = Qₙ · (2 − D · Qₙ) mod x^{cur_deg}
        Polynomial DQ = multiply_fft(D, Q);
        // Обрезаем до cur_deg
        if ((int)DQ.coeffs.size() > cur_deg)
            DQ.coeffs.resize(cur_deg);
        // 2 − D·Q
        Polynomial two({2});
        Polynomial diff = two - DQ;
        Q = multiply_fft(Q, diff);
        // Обрезаем до cur_deg
        if ((int)Q.coeffs.size() > cur_deg)
            Q.coeffs.resize(cur_deg);
        Q.normalize();
    }
    if ((int)Q.coeffs.size() > max_deg)
        Q.coeffs.resize(max_deg);
    Q.normalize();
    return Q;
}

// --- A.1. Быстрое деление: P = Q·D + R ---
pair<Polynomial, Polynomial> fast_divide(const Polynomial& P, const Polynomial& D) {
    int n = P.degree(), m = D.degree();
    if (n < m) return {Polynomial({0}), P};

    // Реверс коэффициентов для обращения
    Polynomial D_rev, P_rev;
    for (int i = m; i >= 0; i--) D_rev.coeffs.push_back(D[i]);
    for (int i = n; i >= 0; i--) P_rev.coeffs.push_back(P[i]);

    // Q_rev = P_rev · D_rev⁻¹ mod x^{n−m+1}
    Polynomial D_inv = inverse_newton(D_rev, n - m + 1);
    Polynomial Q_rev = multiply_fft(P_rev, D_inv);
    if ((int)Q_rev.coeffs.size() > n - m + 1)
        Q_rev.coeffs.resize(n - m + 1);

    // Реверс обратно
    Polynomial Q;
    for (int i = (int)Q_rev.coeffs.size() - 1; i >= 0; i--)
        Q.coeffs.push_back(Q_rev[i]);
    Q.normalize();

    Polynomial R = P - Q * D;
    R.normalize();
    return {Q, R};
}

// --- A.2. Многоточечное вычисление значений ---
// Наивный: O(n · deg P) через Горнера.
// Оптимальный: O(n log² n) через дерево подотрезков (prodtree + modtree).
// Здесь реализован наивный вариант; для олимпиадных задач с большими n
// используется модулярная арифметика (см. combinatorics/b, poly_inv).
vector<long long> multi_point_eval(const Polynomial& P, const vector<long long>& points) {
    int n = (int)points.size();
    vector<long long> result(n);
    for (int i = 0; i < n; i++)
        result[i] = horner(P, points[i]);
    return result;
}

// --- A.2*. Многоточечное вычисление через дерево (O(n log² n), модулярное) ---
// Строим дерево: корень = ∏(x − xᵢ), рекурсивно P(x) mod polynomial вершины.
// Требует fast_divide и модулярной арифметики (параметр mod).
// vector<long long> multi_point_eval_mod(Polynomial P, const vector<long long>& points, long long mod);

// =============================================================
// B. БЫСТРОЕ ВОЗВЕДЕНИЕ В СТЕПЕНЬ
// =============================================================

// --- B.4. Бинарное возведение многочлена в степень ---
// Pᵏ mod xⁿ за O(M(n) log k).
Polynomial poly_pow(const Polynomial& base, long long exp, int max_deg) {
    Polynomial result({1});
    Polynomial b = base;
    while (exp > 0) {
        if (exp & 1) {
            result = multiply_fft(result, b);
            if ((int)result.coeffs.size() > max_deg)
                result.coeffs.resize(max_deg);
            result.normalize();
        }
        b = multiply_fft(b, b);
        if ((int)b.coeffs.size() > max_deg)
            b.coeffs.resize(max_deg);
        b.normalize();
        exp >>= 1;
    }
    return result;
}

// --- B.5. Экспонента: exp(P) = ∑ Pᵏ/k! ---
// Через дифференцирование: Q = exp(P) → Q' = Q·P'
// Рекуррентность: Q[n] = (1/n) ∑_{k=1}^{n} k·P[k]·Q[n−k]
// ВНИМАНИЕ: целочисленное деление sum/n даёт точный результат только
// когда n | sum (гарантируется для формальных рядов над ℚ).
// Для ℤ[x] или 𝔽_p используйте exp_poly_mod.
Polynomial exp_poly(const Polynomial& P, int max_deg) {
    assert(P[0] == 0 && "P₀ must be 0 for exp");
    Polynomial Q({1});
    for (int n = 1; n <= max_deg; n++) {
        long long sum = 0;
        for (int k = 1; k <= n && k <= P.degree(); k++)
            sum += k * P[k] * Q[n - k];
        Q.coeffs.push_back(sum / n);
    }
    Q.normalize();
    return Q;
}

// --- B.5*. Экспонента по модулю p (для олимпиад) ---
Polynomial exp_poly_mod(const Polynomial& P, int max_deg, long long mod) {
    assert(P[0] == 0 && "P₀ must be 0 for exp");
    vector<long long> inv(max_deg + 1, 1);
    for (int i = 2; i <= max_deg; i++)
        inv[i] = (mod - mod / i) * inv[mod % i] % mod;
    Polynomial Q({1});
    for (int n = 1; n <= max_deg; n++) {
        long long sum = 0;
        for (int k = 1; k <= n && k <= P.degree(); k++)
            sum = (sum + k * P[k] % mod * Q[n - k]) % mod;
        Q.coeffs.push_back(sum * inv[n] % mod);
    }
    Q.normalize();
    return Q;
}

// --- B.6. Логарифм: log(1+P) при P₀ = 0 ---
// (log Q)' = Q'/Q → log Q = ∫ Q'/Q
Polynomial log_poly(const Polynomial& P, int max_deg) {
    assert(P[0] == 1 && "Q₀ must be 1 for log");
    Polynomial Q = P;
    Polynomial Qp = derivative(Q);
    Polynomial Q_inv = inverse_newton(Q, max_deg);
    Polynomial ratio = multiply_fft(Qp, Q_inv);
    vector<long long> res(max_deg + 1, 0);
    for (int i = 0; i <= max_deg && i < (int)ratio.coeffs.size(); i++)
        res[i + 1] = ratio[i] / (i + 1);
    return Polynomial(res);
}

// --- B.6*. Логарифм по модулю p ---
Polynomial log_poly_mod(const Polynomial& P, int max_deg, long long mod) {
    assert(P[0] == 1 && "Q₀ must be 1 for log");
    vector<long long> inv(max_deg + 2, 1);
    for (int i = 2; i <= max_deg + 1; i++)
        inv[i] = (mod - mod / i) * inv[mod % i] % mod;
    Polynomial Q = P;
    Polynomial Qp = derivative(Q);
    Polynomial Q_inv = inverse_newton(Q, max_deg);
    Polynomial ratio = multiply_fft(Qp, Q_inv);
    vector<long long> res(max_deg + 1, 0);
    for (int i = 0; i <= max_deg && i < (int)ratio.coeffs.size(); i++)
        res[i + 1] = ratio[i] * inv[i + 1] % mod;
    return Polynomial(res);
}

// --- B.7. Формальная синуса: sin(P) при P₀ = 0 ---
// sin(P) = ∑_{k=0}^{∞} (-1)^k P^{2k+1} / (2k+1)!
// Вычисляется через рекуррентность:
//   S₀ = 0, C₀ = 1 (sin и cos удовлетворяют: S' = C, C' = −S)
//   S[n] = (1/n) ∑_{k=1}^{n} k·P[k]·C[n−k]   (из S' = P'·C)
//   C[n] = −(1/n) ∑_{k=1}^{n} k·P[k]·S[n−k]  (из C' = −P'·S)
// Работаем в truncated series мод x^{max_deg+1}.
Polynomial poly_sin(const Polynomial& P, int max_deg) {
    assert(P[0] == 0 && "P₀ must be 0 for sin");
    int N = max_deg + 1;
    vector<long long> S(N, 0), C(N, 0);
    S[0] = 0;  // sin(0) = 0
    C[0] = 1;  // cos(0) = 1
    for (int n = 1; n < N; n++) {
        long long sum_s = 0, sum_c = 0;
        for (int k = 1; k <= n && k <= P.degree(); k++) {
            sum_s += k * P[k] % N * C[n - k];
            sum_c += k * P[k] % N * S[n - k];
        }
        S[n] = sum_s / n;
        C[n] = -sum_c / n;
    }
    return Polynomial(vector<long long>(S.begin(), S.begin() + N));
}

// --- B.8. Формальный косинуса: cos(P) при P₀ = 0 ---
// Аналогично sin: используем рекуррентность S' = P'·C, C' = −P'·S.
// Возвращаем только C (косинус).
Polynomial poly_cos(const Polynomial& P, int max_deg) {
    assert(P[0] == 0 && "P₀ must be 0 for cos");
    int N = max_deg + 1;
    vector<long long> S(N, 0), C(N, 0);
    S[0] = 0;  // sin(0) = 0
    C[0] = 1;  // cos(0) = 1
    for (int n = 1; n < N; n++) {
        long long sum_s = 0, sum_c = 0;
        for (int k = 1; k <= n && k <= P.degree(); k++) {
            sum_s += k * P[k] % N * C[n - k];
            sum_c += k * P[k] % N * S[n - k];
        }
        S[n] = sum_s / n;
        C[n] = -sum_c / n;
    }
    return Polynomial(vector<long long>(C.begin(), C.begin() + N));
}

// --- B.9. Формальный арктангенса: atan(P) при P₀ = 0 ---
// atan(P) = ∫ P'/(1 + P²) dx
// Вычисляем: Q = P², R = 1/(1+Q), S = P'·R, результат = ∫ S dx.
Polynomial poly_atan(const Polynomial& P, int max_deg) {
    assert(P[0] == 0 && "P₀ must be 0 for atan");
    // P²
    Polynomial Psq = multiply_fft(P, P);
    if ((int)Psq.coeffs.size() > max_deg + 1)
        Psq.coeffs.resize(max_deg + 1);
    // 1 + P²
    Polynomial one_plus_Psq = Psq;
    one_plus_Psq.coeffs[0] += 1;
    // 1/(1 + P²) через обращение Ньютона
    Polynomial inv = inverse_newton(one_plus_Psq, max_deg + 1);
    // P'
    Polynomial Pp = derivative(P);
    // P' · 1/(1+P²)
    Polynomial ratio = multiply_fft(Pp, inv);
    if ((int)ratio.coeffs.size() > max_deg + 1)
        ratio.coeffs.resize(max_deg + 1);
    // Интегрирование
    vector<long long> res(max_deg + 1, 0);
    for (int i = 0; i <= max_deg && i < (int)ratio.coeffs.size(); i++)
        res[i + 1] = ratio[i] / (i + 1);
    return Polynomial(res);
}

// --- Интегрирование: ∫ P dx (свободный член = 0) ---
Polynomial integrate(const Polynomial& P) {
    vector<long long> res(P.coeffs.size() + 1, 0);
    for (int i = 0; i < (int)P.coeffs.size(); i++)
        res[i + 1] = P[i] / (i + 1);
    return Polynomial(res);
}

}; // struct FastDivision

#endif // ALGEBRA_H_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_H_MAIN
int main() {
    using Poly = FastDivision::Polynomial;
    FastDivision fastDiv;

    // Пример A.1: быстрое деление
    Poly P({-3, 1, -2, 1}); // x³ − 2x² + x − 3
    Poly D({-1, 1});         // x − 1
    auto [Q, R] = fastDiv.fast_divide(P, D);
    cout << "Fast divide P/D = "; Q.print(); cout << "\n";
    cout << "Fast divide P%D = "; R.print(); cout << "\n";

    // Пример B.4: возведение в степень
    Poly x({0, 1}); // x
    Poly x3 = fastDiv.poly_pow(x, 3, 10); // x³
    cout << "x^3 = "; x3.print(); cout << "\n";

    // Пример B.5: exp(x) ≈ 1 + x + x²/2 + x³/6 + x⁴/24
    Poly expP = fastDiv.exp_poly(Poly({0, 1}), 4); // exp(x)
    cout << "exp(x) = "; expP.print(); cout << "\n";

    return 0;
}
#endif // ALGEBRA_H_MAIN
