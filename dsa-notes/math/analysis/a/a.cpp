#ifndef ANALYSIS_A_CPP
#define ANALYSIS_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>
#include <limits>
#include <numeric>
#include <complex>
#include <tuple>
#include <cassert>
using namespace std;

// =============================================================
// A. ВВЕДЕНИЕ В АНАЛИЗ: ПРЕДЕЛЫ И НЕПРЕРЫВНОСТЬ
// =============================================================
// LimitsAndContinuity — базовый класс всей ветки analysis.
// Переиспользование: (в первом разделе пока не требуется)

struct LimitsAndContinuity {

// =============================================================
// B. ТЕОРИЯ ПЕДЕЛОВ (ЧАСТЬ 1)
// =============================================================

// --- B.1. Предел последовательности ---
// Вычисляет lim(n→∞) aₙ итеративно, ища N где |aₙ − aₙ₋₁| < eps.
// O(N) время, O(1) память.
pair<double, int> limit_sequence(function<double(int)> f,
                                  double eps = 1e-10,
                                  int max_n = 100000) {
    double prev = f(max_n - 1);
    double curr = f(max_n);
    if (abs(curr - prev) < eps) return {curr, max_n};
    for (int n = max_n; n > 10; --n) {
        if (abs(f(n) - f(n - 1)) < eps) return {f(n), n};
    }
    return {numeric_limits<double>::quiet_NaN(), -1};
}

// --- B.2. Предел функции: ε-δ проверка ---
// Возвращает максимальный δ для данного ε.
double limit_function_check(function<double(double)> f, double a, double L,
                            double eps = 1e-8) {
    double delta = 1.0;
    for (int iter = 0; iter < 100; ++iter) {
        delta /= 2.0;
        bool ok = true;
        for (int k = -10; k <= 10; ++k) {
            if (k == 0) continue;
            if (abs(f(a + k * delta / 10.0) - L) >= eps) { ok = false; break; }
        }
        if (ok) return delta;
    }
    return delta;
}

// --- B.3. Критерий Коши для последовательности ---
bool is_cauchy_sequence(function<double(int)> f,
                        double eps = 1e-10, int max_n = 1000) {
    for (int n = max_n / 2; n <= max_n; ++n)
        for (int m = n + 1; m <= min(n + 100, max_n); ++m)
            if (abs(f(n) - f(m)) >= eps) return false;
    return true;
}

// =============================================================
// C. ТЕОРИЯ ПЕДЕЛОВ (ЧАСТЬ 2)
// =============================================================

// --- C.1. Число e через определение ---
double compute_e_definition(int n) { return pow(1.0 + 1.0 / n, n); }

// --- C.2. Число e через ряд Тейлора ---
double compute_e_series(int terms = 20) {
    double sum = 0.0, factorial = 1.0;
    for (int k = 0; k < terms; ++k) { if (k > 0) factorial *= k; sum += 1.0 / factorial; }
    return sum;
}

// --- C.3–C.6. Замечательные пределы ---
double remarkable_limit_sin(double x) { return (abs(x) < 1e-15) ? 1.0 : sin(x) / x; }
double remarkable_limit_cos(double x) { return (abs(x) < 1e-15) ? 0.5 : (1.0 - cos(x)) / (x * x); }
double remarkable_limit_exp(double a, double x) { return pow(1.0 + a / x, x); }
double remarkable_limit_ln(double x) { return (abs(x) < 1e-15) ? 1.0 : log(1.0 + x) / x; }

// =============================================================
// D. ПРИЁМЫ РАСКРЫТИЯ НЕОПРЕДЕЛЁННОСТЕЙ
// =============================================================

// --- D.1. L'Hôpital для 0/0 ---
double lhopital_0_0(function<double(double)> f, function<double(double)> g,
                     function<double(double)> df, function<double(double)> dg,
                     double a, double h = 1e-8) {
    double fa = f(a + h) - f(a - h), ga = g(a + h) - g(a - h);
    return (abs(ga) < 1e-15) ? NAN : fa / ga;
}

// --- D.2. 1^∞ через exp ---
double one_inf_limit(double f_val, double g_val) { return exp(g_val * (f_val - 1.0)); }

// --- D.3. Эквивалентные бесконечно малые ---
tuple<double,double,double,double,double> equivalent_infinitesimals(double x) {
    return {
        (abs(x) < 1e-15) ? 1.0 : sin(x) / x,
        (abs(x) < 1e-15) ? 1.0 : tan(x) / x,
        (abs(x) < 1e-15) ? 1.0 : log(1.0 + x) / x,
        (abs(x) < 1e-15) ? 0.5 : (1.0 - cos(x)) / (x * x),
        (abs(x) < 1e-15) ? 1.0 : (exp(x) - 1.0) / x
    };
}

// =============================================================
// E. НЕПРЕРЫВНОСТЬ ФУНКЦИИ
// =============================================================

// --- E.1. Проверка непрерывности в точке ---
bool is_continuous_at(function<double(double)> f, double a, double eps = 1e-8) {
    double fa = f(a);
    double delta = 1.0;
    for (int iter = 0; iter < 50; ++iter) {
        delta /= 2.0;
        bool ok = true;
        for (int k = -20; k <= 20; ++k) {
            if (k == 0) continue;
            if (abs(f(a + k * delta / 20.0) - fa) >= eps) { ok = false; break; }
        }
        if (ok) return true;
    }
    return false;
}

// --- E.2. Промежуточная теорема (бисекция) ---
double intermediate_value(function<double(double)> f, double a, double b, double c,
                          double eps = 1e-10) {
    assert(f(a) < c && c < f(b));
    while (b - a > eps) {
        double mid = (a + b) / 2.0;
        if (f(mid) < c) a = mid; else b = mid;
    }
    return (a + b) / 2.0;
}

// --- E.3. Вейерштрасс: min/max ---
pair<double,double> weierstrass_min_max(function<double(double)> f, double a, double b,
                                         double eps = 1e-10) {
    auto ternary = [&](function<double(double)> g, bool find_min) {
        double l = a, r = b;
        while (r - l > eps) {
            double m1 = l + (r - l) / 3.0, m2 = r - (r - l) / 3.0;
            bool cond = find_min ? g(m1) < g(m2) : g(m1) > g(m2);
            if (cond) r = m2; else l = m1;
        }
        return (l + r) / 2.0;
    };
    return {f(ternary(f, true)), f(ternary(f, false))};
}

// --- E.4. Классификация разрыва ---
// 0=непрерывна, 1=устранимый, 2=скачок, 3=II род
int discontinuity_type(function<double(double)> f, double a, double eps = 1e-8) {
    double fa = f(a), left = f(a - eps), right = f(a + eps);
    if (abs(left - fa) < eps && abs(right - fa) < eps) return 0;
    if (abs(left - right) < eps) return 1;
    if (isfinite(left) && isfinite(right)) return 2;
    return 3;
}

// =============================================================
// F. ЭЛЕМЕНТАРНЫЕ ФУНКЦИИ
// =============================================================

double power_function(double x, double alpha) {
    if (x > 0) return exp(alpha * log(x));
    if (x == 0 && alpha > 0) return 0.0;
    return NAN;
}
double exponential_function(double a, double x) { return exp(x * log(a)); }
double logarithmic_function(double a, double x) { return log(x) / log(a); }
double trig_function(string name, double x) {
    if (name == "sin") return sin(x); if (name == "cos") return cos(x);
    if (name == "tan") return tan(x); if (name == "arcsin") return asin(x);
    if (name == "arccos") return acos(x); if (name == "arctan") return atan(x);
    return NAN;
}
double hyperbolic_function(string name, double x) {
    if (name == "sh") return sinh(x); if (name == "ch") return cosh(x);
    if (name == "th") return tanh(x); return NAN;
}

}; // struct LimitsAndContinuity

#ifdef ANALYSIS_A_MAIN
int main() {
    LimitsAndContinuity lac;
    cout << "=== A. Пределы и непрерывность ===" << endl;

    // e через ряд
    cout << "e (ряд) = " << lac.compute_e_series(20) << endl;

    // Замечательные пределы
    cout << "sin(0.001)/0.001 = " << lac.remarkable_limit_sin(0.001) << endl;

    // Эквивалентные
    auto [s,t,l,c,e] = lac.equivalent_infinitesimals(0.001);
    cout << "Эквивалентные: sin/x=" << s << " tan/x=" << t << " ln(1+x)/x=" << l << endl;

    // Непрерывность
    auto fsin = [](double x) { return sin(x); };
    cout << "sin непрерывна в 0: " << (lac.is_continuous_at(fsin,0)?"ДА":"НЕТ") << endl;

    // Промежуточная
    auto fcube = [](double x) { return x*x*x-2; };
    cout << "x³=2: x=" << lac.intermediate_value(fcube,0,2,0) << endl;

    return 0;
}
#endif

#endif // ANALYSIS_A_CPP
