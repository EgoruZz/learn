#ifndef ANALYSIS_C_CPP
#define ANALYSIS_C_CPP

#include "../b/b.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>
#include <limits>
#include <tuple>
#include <cassert>
using namespace std;

// =============================================================
// C. ИНТЕГРАЛЬНОЕ ИСЧИСЛЕНИЕ ФУНКЦИЙ ОДНОЙ ПЕРЕМЕННОЙ
// =============================================================
// IntegralCalculus : DifferentialCalculus
// Переиспользование: LimitsAndContinuity (a.cpp), DifferentialCalculus (b.cpp)

struct IntegralCalculus : DifferentialCalculus {

// =============================================================
// F. ОПРЕДЕЛЁННЫЙ ИНТЕГРАЛ: ЧИСЛЕННЫЕ МЕТОДЫ
// =============================================================

// --- F.1. Метод прямоугольников (левые) ---
// ∫ₐᵇ f(x)dx ≈ Σ f(xᵢ)·Δx
// O(n) время, O(1) память. Погрешность O(Δx).
double integral_rectangles(function<double(double)> f, double a, double b,
                            int n = 1000) {
    double dx = (b - a) / n, sum = 0;
    for (int i = 0; i < n; ++i) sum += f(a + i * dx);
    return sum * dx;
}

// --- F.2. Метод трапеций ---
// ∫f ≈ Σ (f(xᵢ)+f(xᵢ₊₁))/2 · Δx
// O(n) время, O(1) память. Погрешность O(Δx²).
double integral_trapezoid(function<double(double)> f, double a, double b,
                           int n = 1000) {
    double dx = (b - a) / n;
    double sum = (f(a) + f(b)) / 2.0;
    for (int i = 1; i < n; ++i) sum += f(a + i * dx);
    return sum * dx;
}

// --- F.3. Метод Симпсона (Simpson's Rule) ---
// ∫f ≈ (Δx/3)(f₀ + 4f₁ + 2f₂ + 4f₃ + ... + fₙ)
// O(n) время, O(1) память. Погрешность O(Δx⁴). n чётное.
double integral_simpson(function<double(double)> f, double a, double b,
                         int n = 1000) {
    assert(n % 2 == 0);
    double dx = (b - a) / n;
    double sum = f(a) + f(b);
    for (int i = 1; i < n; i += 2) sum += 4.0 * f(a + i * dx);
    for (int i = 2; i < n; i += 2) sum += 2.0 * f(a + i * dx);
    return sum * dx / 3.0;
}

// --- F.4. Определённый интеграл через Ньютон-Лейбница ---
// ∫ₐᵇ f(x)dx = F(b) − F(a), F' = f
// Численно: F(x) = ∫ₐˣ f(t)dt через Симпсона.
double integral_newton_leibnitz(function<double(double)> f, double a, double b,
                                 int n = 10000) {
    return integral_simpson(f, a, b, n);
}

// =============================================================
// G. ПРИЛОЖЕНИЯ
// =============================================================

// --- G.1. Площадь между кривыми ---
// S = ∫ₐᵇ |f(x) − g(x)|dx
double area_between_curves(function<double(double)> f,
                            function<double(double)> g,
                            double a, double b, int n = 10000) {
    auto diff = [&](double x) { return abs(f(x) - g(x)); };
    return integral_simpson(diff, a, b, n);
}

// --- G.2. Объём тела вращения (disk method) ---
// V = π∫ₐᵇ f²(x)dx
double volume_of_revolution(function<double(double)> f,
                             double a, double b, int n = 10000) {
    auto fsq = [&](double x) { double v=f(x); return v*v; };
    return M_PI * integral_simpson(fsq, a, b, n);
}

// --- G.3. Длина дуги ---
// L = ∫ₐᵇ √(1 + (f'(x))²) dx
double arc_length(function<double(double)> f, double a, double b,
                   int n = 10000) {
    auto integrand = [&](double x) {
        double fp = derivative_central(f, x);
        return sqrt(1.0 + fp * fp);
    };
    return integral_simpson(integrand, a, b, n);
}

// --- G.4. Площадь поверхности вращения ---
// S = 2π∫ₐᵇ f(x)√(1 + (f'(x))²) dx
double surface_of_revolution(function<double(double)> f,
                              double a, double b, int n = 10000) {
    auto integrand = [&](double x) {
        double fp = derivative_central(f, x);
        return f(x) * sqrt(1.0 + fp * fp);
    };
    return 2.0 * M_PI * integral_simpson(integrand, a, b, n);
}

// =============================================================
// H. НЕСОБСТВЕННЫЙ ИНТЕГРАЛ
// =============================================================

// --- H.1. Несобственный интеграл I рода: ∫ₐ^∞ f(x)dx ---
// Вычисляет lim(b→∞) ∫ₐᵇ f(x)dx.
// Метод: нарастание b до сходимости.
double improper_integral_first(double a, function<double(double)> f,
                                double eps = 1e-8, double max_b = 1e6) {
    double prev = 0;
    for (double b = a + 1; b <= max_b; b *= 2) {
        double curr = integral_simpson(f, a, b, 10000);
        if (abs(curr - prev) < eps) return curr;
        prev = curr;
    }
    return prev;
}

// --- H.2. Несобственный интеграл II рода: разрыв в c ---
// ∫ₐᵇ f(x)dx, f неограничена в c ∈ [a,b].
// Вычисляет lim(ε→0) (∫ₐ^{c−ε} + ∫_{c+ε}ᵇ)
double improper_integral_second(function<double(double)> f,
                                 double a, double c, double b,
                                 double eps = 1e-8) {
    double prev = 0;
    for (double e = 0.5; e > 1e-12; e /= 2) {
        double left = integral_simpson(f, a, c - e, 1000);
        double right = integral_simpson(f, c + e, b, 1000);
        double curr = left + right;
        if (abs(curr - prev) < eps) return curr;
        prev = curr;
    }
    return prev;
}

// --- H.3. Признак Даламбера для ∫ₐ^∞ f(x)dx ---
// Если lim(x→∞) f(x)/xᵖ = L, то:
//   p > 1, L < ∞ → сходится
//   p ≤ 1, L > 0 → расходится
bool dalamber_test(function<double(double)> f, double p,
                    double large = 1e6) {
    double L = f(large) / pow(large, p);
    return (p > 1 && L < 1e10); // приближённая проверка
}

}; // struct IntegralCalculus

#ifdef ANALYSIS_C_MAIN
int main() {
    IntegralCalculus ic;
    cout << "=== C. Интегральное исчисление ===" << endl;

    // ∫₀¹ x² dx = 1/3
    auto f1 = [](double x) { return x*x; };
    cout << "∫₀¹ x² dx = " << ic.integral_simpson(f1, 0, 1) << " (ожидается 0.3333)" << endl;

    // Объём вращения y=√x на [0,1]: π∫x dx = π/2
    auto f2 = [](double x) { return sqrt(x); };
    cout << "V = π∫₀¹ x dx = " << ic.volume_of_revolution(f2, 0, 1) << endl;

    // Несобственный: ∫₁^∞ 1/x² dx = 1
    auto f3 = [](double x) { return 1.0/(x*x); };
    cout << "∫₁^∞ 1/x² dx = " << ic.improper_integral_first(1.0, f3) << " (ожидается 1.0)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_C_CPP
