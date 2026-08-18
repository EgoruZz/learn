#ifndef ANALYSIS_D_CPP
#define ANALYSIS_D_CPP

#include "../c/c.cpp"
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
// D. МЕТРИЧЕСКИЕ ПРОСТРАНСТВА И ФНП
// =============================================================
// MultivariableCalculus : IntegralCalculus

struct MultivariableCalculus : IntegralCalculus {

// =============================================================
// B. ФУНКЦИИ НЕСКОЛЬКИХ ПЕРЕМЕННЫХ
// =============================================================

// --- B.1. Частная производная (numerical) ---
// ∂f/∂xᵢ ≈ (f(x+heᵢ) − f(x−heᵢ))/(2h)
double partial_derivative(function<double(vector<double>)> f,
                           const vector<double>& x, int i,
                           double h = 1e-8) {
    vector<double> xp = x, xm = x;
    xp[i] += h; xm[i] -= h;
    return (f(xp) - f(xm)) / (2.0 * h);
}

// --- B.2. Градиент (numerical) ---
// ∇f = (∂f/∂x₁, ..., ∂f/∂xₙ)
vector<double> gradient(function<double(vector<double>)> f,
                         const vector<double>& x, double h = 1e-8) {
    int n = x.size();
    vector<double> grad(n);
    for (int i = 0; i < n; ++i)
        grad[i] = partial_derivative(f, x, i, h);
    return grad;
}

// --- B.3. Производная по направлению ---
// ∂f/∂v = ∇f · v/|v|
double directional_derivative(function<double(vector<double>)> f,
                                const vector<double>& x,
                                const vector<double>& v,
                                double h = 1e-8) {
    auto g = gradient(f, x, h);
    double norm = 0;
    for (int i = 0; i < (int)g.size(); ++i) norm += v[i] * v[i];
    norm = sqrt(norm);
    double dot = 0;
    for (int i = 0; i < (int)g.size(); ++i) dot += g[i] * v[i];
    return dot / norm;
}

// =============================================================
// D. ВЫСШИЕ ПРОИЗВОДНЫЕ И ТЕЙЛОР
// =============================================================

// --- D.1. Матрица Гессе (numerical) ---
// H[i][j] = ∂²f/∂xᵢ∂xⱼ
vector<vector<double>> hessian(function<double(vector<double>)> f,
                                const vector<double>& x,
                                double h = 1e-6) {
    int n = x.size();
    vector<vector<double>> H(n, vector<double>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vector<double> xpp = x, xpm = x, xmp = x, xmm = x;
            xpp[i] += h; xpp[j] += h;
            xpm[i] += h; xpm[j] -= h;
            xmp[i] -= h; xmp[j] += h;
            xmm[i] -= h; xmm[j] -= h;
            H[i][j] = (f(xpp) - f(xpm) - f(xmp) + f(xmm)) / (4.0 * h * h);
        }
    }
    return H;
}

// --- D.2. Определитель матрицы 2×2 ---
double det2x2(const vector<vector<double>>& M) {
    return M[0][0]*M[1][1] - M[0][1]*M[1][0];
}

// =============================================================
// F. ЭКСТРЕМУМЫ ФНП
// =============================================================

// --- F.1. Проверка критической точки (g = 0) ---
// Возвращает: 0=нет экстр, 1=мин, 2=макс, 3=седло
int classify_critical_2d(function<double(vector<double>)> f,
                          const vector<double>& x0,
                          double h = 1e-6) {
    auto H = hessian(f, x0, h);
    double D = det2x2(H);
    if (D > 0 && H[0][0] > 0) return 1; // минимум
    if (D > 0 && H[0][0] < 0) return 2; // максимум
    if (D < 0) return 3; // седло
    return 0; // неопределённый
}

// --- F.2. Градиентный спуск ---
// xₙ₊₁ = xₙ − α∇f(xₙ)
// Возвращает {x_min, f(x_min), число итераций}.
tuple<vector<double>, double, int> gradient_descent(
    function<double(vector<double>)> f,
    vector<double> x0, double alpha = 0.01,
    int max_iter = 10000, double eps = 1e-10) {
    auto x = x0;
    for (int iter = 0; iter < max_iter; ++iter) {
        auto g = gradient(f, x);
        double gnorm = 0;
        for (int i = 0; i < (int)g.size(); ++i) gnorm += g[i]*g[i];
        gnorm = sqrt(gnorm);
        if (gnorm < eps) return {x, f(x), iter};
        for (int i = 0; i < (int)x.size(); ++i)
            x[i] -= alpha * g[i];
    }
    return {x, f(x), max_iter};
}

// --- F.3. Метод Ньютона для ФНП ---
// xₙ₊₁ = xₙ − H⁻¹∇f(xₙ)
tuple<vector<double>, double, int> newton_multivariable(
    function<double(vector<double>)> f,
    vector<double> x0, int max_iter = 100, double eps = 1e-10) {
    auto x = x0;
    for (int iter = 0; iter < max_iter; ++iter) {
        auto g = gradient(f, x);
        auto H = hessian(f, x);
        // Решаем H·Δx = −g (для 2D через обратную матрицу)
        double D = det2x2(H);
        if (abs(D) < 1e-15) break;
        double dx = -(H[1][1]*g[0] - H[0][1]*g[1]) / D;
        double dy = -(-H[1][0]*g[0] + H[0][0]*g[1]) / D;
        x[0] += dx; x[1] += dy;
        if (sqrt(dx*dx+dy*dy) < eps) return {x, f(x), iter};
    }
    return {x, f(x), max_iter};
}

// --- F.4. Метод множителей Лагранжа (numerical, 2D) ---
// ext f(x,y) при g(x,y) = 0
// ∇f = λ∇g, g = 0 → решаем систему.
// Простейший метод: перебор + бисекция по λ.
pair<vector<double>, double> lagrange_multiplier_2d(
    function<double(double,double)> f,
    function<double(double,double)> g,
    double x0, double y0, double eps = 1e-8) {
    // Начальное приближение: найдём точку на g=0 близко к (x0,y0)
    // Упрощённый: бисекция по одному параметру
    double lambda = 0;
    auto L = [&](double x, double y, double l) {
        return f(x,y) - l * g(x,y);
    };
    // Итеративно: минимизируем L по (x,y), корректируем λ
    for (int iter = 0; iter < 100; ++iter) {
        // Минимум L через градиентный спуск (упрощённо)
        double x = x0, y = y0;
        for (int k = 0; k < 50; ++k) {
            double Lx = (L(x+eps,y,lambda)-L(x-eps,y,lambda))/(2*eps);
            double Ly = (L(x,y+eps,lambda)-L(x,y-eps,lambda))/(2*eps);
            x -= 0.01 * Lx; y -= 0.01 * Ly;
        }
        double gval = g(x, y);
        lambda += 0.1 * gval;
        x0 = x; y0 = y;
    }
    return {{x0, y0}, f(x0, y0)};
}

}; // struct MultivariableCalculus

#ifdef ANALYSIS_D_MAIN
int main() {
    MultivariableCalculus mc;
    cout << "=== D. Метрические пространства и ФНП ===" << endl;

    // Градиент f(x,y)=x²+y²: ∇f = (2x,2y)
    auto f = [](vector<double> p) { return p[0]*p[0] + p[1]*p[1]; };
    auto grad = mc.gradient(f, {1.0, 2.0});
    cout << "∇(x²+y²) в (1,2) = (" << grad[0] << ", " << grad[1] << ")" << endl;

    // Градиентный спуск: минимум x²+y² в (0,0)
    auto [xmin, fmin, iters] = mc.gradient_descent(f, {3.0, 4.0}, 0.1);
    cout << "Минимум x²+y²: (" << xmin[0] << ", " << xmin[1] << "), f=" << fmin << endl;

    // Классификация: f(x,y)=x²+y² → минимум
    cout << "Крит. точка (0,0) для x²+y²: " << mc.classify_critical_2d(f, {0.0, 0.0}) << " (1=мин)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_D_CPP
