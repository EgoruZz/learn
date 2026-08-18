#ifndef ANALYSIS_E_CPP
#define ANALYSIS_E_CPP

#include "../d/d.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>
#include <limits>
#include <cassert>
using namespace std;

// =============================================================
// E. РЯДЫ
// =============================================================
// Series : MultivariableCalculus

struct Series : MultivariableCalculus {

// =============================================================
// A–B. ЧИСЛОВЫЕ РЯДЫ
// =============================================================

// --- A.1. Частичная сумма ряда ---
// Σ(k=0..n) aₖ
double partial_sum(function<double(int)> a, int n) {
    double s = 0;
    for (int k = 0; k <= n; ++k) s += a(k);
    return s;
}

// --- A.2. Проверка сходимости (критерий Коши) ---
// Проверяет, что остаток |Σ(k=n+1..∞) aₖ| < eps
bool series_converges(function<double(int)> a, int n_start = 1000,
                       double eps = 1e-10, int check = 100) {
    double prev = 0;
    for (int n = n_start; n < n_start + check; ++n) {
        double curr = abs(a(n));
        if (curr > eps) return false;
        prev = curr;
    }
    return true;
}

// --- B.1. Сумма геометрического ряда: Σqᵏ = 1/(1−q) ---
double geometric_series_sum(double q, int n = 1000) {
    return (abs(q) < 1) ? 1.0 / (1.0 - q) : partial_sum([q](int k) { return pow(q, k); }, n);
}

// --- B.2. P-ряд: Σ1/nᵖ ---
double p_series_sum(double p, int n = 10000) {
    double s = 0;
    for (int k = 1; k <= n; ++k) s += 1.0 / pow(k, p);
    return s;
}

// --- B.3. Признак Даламбера ---
// lim |aₙ₊₁/aₙ| = L → L < 1 converge, L > 1 diverge
double dalamber_ratio(function<double(int)> a, int n = 10000) {
    return abs(a(n + 1)) / abs(a(n));
}

// --- B.4. Признак Коши (радикальный) ---
double cauchy_root(function<double(int)> a, int n = 10000) {
    return pow(abs(a(n)), 1.0 / n);
}

// =============================================================
// C. ЗНАКОПЕРЕМЕННЫЕ РЯДЫ
// =============================================================

// --- C.1. Ряд Лейбница: Σ(−1)ⁿ/n ---
double leibniz_series(int n = 10000) {
    double s = 0;
    for (int k = 1; k <= n; ++k) s += ((k % 2 == 1) ? 1.0 : -1.0) / k;
    return s;
}

// =============================================================
// E. СТЕПЕННЫЕ РЯДЫ
// =============================================================

// --- E.1. Радиус сходимости: R = lim |aₙ/aₙ₊₁| ---
double radius_of_convergence(function<double(int)> a, int n = 10000) {
    double r1 = abs(a(n)), r2 = abs(a(n + 1));
    return (r2 < 1e-15) ? INFINITY : r1 / r2;
}

// --- E.2. Оценка степенного ряда: Σ aₖ(x−a)ᵏ ---
double power_series_eval(const vector<double>& coeff, double a, double x) {
    double r = 0, p = 1;
    for (size_t k = 0; k < coeff.size(); ++k) { r += coeff[k]*p; p *= (x-a); }
    return r;
}

// =============================================================
// F. РАЗЛОЖЕНИЯ
// =============================================================

// --- F.1. Стандартные разложения (повтор из b.cpp для полноты) ---
double series_exp(double x, int n = 20) {
    double s = 0, t = 1; for (int k = 0; k <= n; ++k) { s += t; t *= x / (k + 1); } return s;
}
double series_sin(double x, int n = 20) {
    double s = 0, t = x; for (int k = 0; k <= n; ++k) { s += t; t *= -x*x / ((2*k+2)*(2*k+3)); } return s;
}
double series_cos(double x, int n = 20) {
    double s = 0, t = 1; for (int k = 0; k <= n; ++k) { s += t; t *= -x*x / ((2*k+1)*(2*k+2)); } return s;
}
double series_ln(double x, int n = 20) {
    assert(x > -1 && x <= 1); double s = 0, t = x;
    for (int k = 1; k <= n; ++k) { s += ((k%2==1)?1:-1)*t/k; t *= x; } return s;
}

// --- F.2. Ряд Фурье: коэффициенты ---
// aₙ = (1/π)∫₋π^π f(x)cos(nx)dx, bₙ = (1/π)∫₋π^π f(x)sin(nx)dx
tuple<double, double> fourier_coeff(function<double(double)> f,
                                     int n, int N = 10000) {
    auto cos_integrand = [&](double x) { return f(x) * cos(n * x); };
    auto sin_integrand = [&](double x) { return f(x) * sin(n * x); };
    double a_n = integral_simpson(cos_integrand, -M_PI, M_PI, N) / M_PI;
    double b_n = integral_simpson(sin_integrand, -M_PI, M_PI, N) / M_PI;
    return {a_n, b_n};
}

// --- F.3. Оценка ряда Фурье в точке ---
double fourier_eval(function<double(double)> f, double x,
                     int terms = 10, int N = 10000) {
    auto [a0, b0] = fourier_coeff(f, 0, N);
    double sum = a0 / 2.0;
    for (int k = 1; k <= terms; ++k) {
        auto [ak, bk] = fourier_coeff(f, k, N);
        sum += ak * cos(k * x) + bk * sin(k * x);
    }
    return sum;
}

}; // struct Series

#ifdef ANALYSIS_E_MAIN
int main() {
    Series s;
    cout << "=== E. Ряды ===" << endl;

    // Геометрический ряд: Σ(1/2)^k = 2
    cout << "Σ(1/2)^k = " << s.geometric_series_sum(0.5) << " (ожидается 2.0)" << endl;

    // Ряд Лейбница: Σ(-1)^{n+1}/n → ln 2 ≈ 0.6931
    cout << "Ряд Лейбница = " << s.leibniz_series() << " (ожидается ln2 ≈ 0.6931)" << endl;

    // Ряд Фурье f(x)=x на [-π,π], b₁=2
    auto f_line = [](double x) { return x; };
    auto [a1, b1] = s.fourier_coeff(f_line, 1);
    cout << "Fourier b₁ для f(x)=x: " << b1 << " (ожидается ≈2.0)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_E_CPP
