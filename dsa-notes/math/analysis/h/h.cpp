#ifndef ANALYSIS_H_CPP
#define ANALYSIS_H_CPP

#include "../g/g.cpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include <cassert>
using namespace std;

struct DifferentialEquations : ComplexAnalysis {

// =============================================================
// B. ДУ ПЕРВОГО ПОРЯДКА
// =============================================================

// --- B.1. Разделяющиеся: dy/dx = f(x)g(y) ---
// Решение: ∫dy/g(y) = ∫f(x)dx + C
// Возвращаем решение как функцию y(x) через численное обратное отображение.
function<double(double)> solve_separable(
    function<double(double)> f, function<double(double)> g_inv,
    double C_val, double x0, double y0) {
    // Численно: y(x) ≈ y₀ + ∫_{x₀}^{x} f(t)g(y(t)) dt (метод трапеций)
    return [=](double x) {
        double y = y0, dx = (x - x0) / 1000;
        for (int i = 0; i < 1000; ++i) {
            double t = x0 + i * dx;
            y += dx * f(t) * g_inv(y); // g_inv = g(y)
        }
        return y;
    };
}

// --- B.2. Линейные 1-го порядка: y' + P(x)y = Q(x) ---
// y = e^{-∫P dx}(C + ∫Q·e^{∫P dx} dx)
function<double(double)> solve_linear_1st(
    function<double(double)> P, function<double(double)> Q,
    double x0, double y0, int n = 10000) {
    // Численно: интегрируем интегральный множитель
    auto exp_intP = [&](double x) {
        auto integrand = [&](double t) { return P(t); };
        double intP = integral_simpson(integrand, x0, x, n);
        return exp(intP);
    };
    return [=](double x) {
        auto integrand = [&](double t) { return Q(t) * exp_intP(t); };
        double intQ = integral_simpson(integrand, x0, x, n);
        return (y0 + intQ) / exp_intP(x);
    };
}

// --- C.1. Бернулли: y' + P(x)y = Q(x)yⁿ ---
// z = y^{1−n} → z' + (1−n)Pz = (1−n)Q
function<double(double)> solve_bernoulli(
    function<double(double)> P, function<double(double)> Q, double n,
    double x0, double y0, int steps = 10000) {
    // Линеаризация: z = y^{1−n}
    double m = 1.0 - n;
    auto Pm = [&](double x) { return m * P(x); };
    auto Qm = [&](double x) { return m * Q(x); };
    double z0 = pow(y0, m);
    auto z = solve_linear_1st(Pm, Qm, x0, z0, steps);
    return [=](double x) { return pow(z(x), 1.0/m); };
}

// =============================================================
// E. ДУ ВТОРОГО ПОРЯДКА
// =============================================================

// --- E.1. Определитель Вронского ---
double wronskian(function<double(double)> y1, function<double(double)> y2,
                  double x, double h = 1e-8) {
    double y1v = y1(x), y2v = y2(x);
    double y1p = (y1(x+h)-y1(x-h))/(2*h);
    double y2p = (y2(x+h)-y2(x-h))/(2*h);
    return y1v*y2p - y2v*y1p;
}

// --- E.2. Характеристическое уравнение: a₂r²+a₁r+a₀=0 ---
pair<cd,cd> char_roots_2nd(double a2, double a1, double a0) {
    double D = a1*a1 - 4*a2*a0;
    if (D >= 0) return {cd((-a1+sqrt(D))/(2*a2),0), cd((-a1-sqrt(D))/(2*a2),0)};
    return {cd(-a1/(2*a2),sqrt(-D)/(2*a2)), cd(-a1/(2*a2),-sqrt(-D)/(2*a2))};
}

// --- E.3. Решение y''+ay'+by=0 через корни характеристического ---
function<double(double)> solve_2nd_const(double a, double b,
                                          double y0, double y1) {
    auto [r1, r2] = char_roots_2nd(1.0, a, b);
    if (imag(r1)==0 && imag(r2)==0 && abs(real(r1)-real(r2))>1e-10) {
        double r1r=real(r1), r2r=real(r2);
        double C1=(y1-r2r*y0)/(r1r-r2r), C2=y0-C1;
        return [=](double t){return C1*exp(r1r*t)+C2*exp(r2r*t);};
    }
    if (abs(r1-r2)<1e-10) {
        double r=real(r1);
        return [=](double t){return (y0+(y1-r*y0)*t)*exp(r*t);};
    }
    double alpha=real(r1), beta=imag(r1);
    double A=y0, B=(y1-alpha*y0)/beta;
    return [=](double t){return exp(alpha*t)*(A*cos(beta*t)+B*sin(beta*t));};
}

// =============================================================
// I. СИСТЕМЫ ДУ
// =============================================================

// --- I.1. Матричная экспонента: e^{At} (Padé-аппроксимация) ---
// Упрощённо: e^{At} ≈ I + At + (At)²/2! + (At)³/3! + (At)⁴/4!
typedef vector<vector<double>> Matrix;

Matrix mat_identity(int n) {
    Matrix I(n, vector<double>(n, 0));
    for (int i = 0; i < n; ++i) I[i][i] = 1;
    return I;
}

Matrix mat_add(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, vector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

Matrix mat_scale(const Matrix& A, double s) {
    int n = A.size();
    Matrix C(n, vector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] * s;
    return C;
}

Matrix mat_mul(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, vector<double>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

Matrix mat_pow(const Matrix& M, int p) {
    int n = M.size();
    Matrix result = mat_identity(n);
    Matrix base = M;
    while (p > 0) {
        if (p & 1) result = mat_mul(result, base);
        base = mat_mul(base, base);
        p >>= 1;
    }
    return result;
}

// e^{At} ≈ Σ (At)^k / k! (обрезка при 20 членах)
Matrix matrix_exponential(const Matrix& A, double t, int terms = 20) {
    int n = A.size();
    Matrix result = mat_identity(n);
    Matrix At = mat_scale(A, t);
    Matrix term = mat_identity(n);
    double factorial = 1.0;
    for (int k = 1; k <= terms; ++k) {
        term = mat_mul(term, At);
        factorial *= k;
        result = mat_add(result, mat_scale(term, 1.0/factorial));
    }
    return result;
}

// --- I.2. Решение dy/dt = Ay: y(t) = e^{At}y₀ ---
vector<double> solve_system_const(const Matrix& A, const vector<double>& y0,
                                   double t) {
    Matrix eAt = matrix_exponential(A, t);
    int n = A.size();
    vector<double> y(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += eAt[i][j] * y0[j];
    return y;
}

// =============================================================
// J. ЧИСЛЕННЫЕ МЕТОДЫ
// =============================================================

// --- J.1. Эйлер ---
vector<pair<double,double>> euler_method(
    function<double(double,double)> f, double t0, double y0,
    double t_end, double h = 0.01) {
    vector<pair<double,double>> sol = {{t0,y0}};
    double t=t0, y=y0;
    while (t < t_end-1e-12) { y+=h*f(t,y); t+=h; sol.push_back({t,y}); }
    return sol;
}

// --- J.2. RK4 ---
vector<pair<double,double>> rk4_method(
    function<double(double,double)> f, double t0, double y0,
    double t_end, double h = 0.01) {
    vector<pair<double,double>> sol = {{t0,y0}};
    double t=t0, y=y0;
    while (t < t_end-1e-12) {
        double k1=h*f(t,y), k2=h*f(t+h/2,y+k1/2);
        double k3=h*f(t+h/2,y+k2/2), k4=h*f(t+h,y+k3);
        y+=(k1+2*k2+2*k3+k4)/6; t+=h; sol.push_back({t,y});
    }
    return sol;
}

// --- J.3. Адамс 2-го порядка ---
vector<pair<double,double>> adams_bashforth_2(
    function<double(double,double)> f, double t0, double y0,
    double t_end, double h = 0.01) {
    double k1=h*f(t0,y0), k2=h*f(t0+h/2,y0+k1/2);
    double k3=h*f(t0+h/2,y0+k2/2), k4=h*f(t0+h,y0+k3);
    double y1=y0+(k1+2*k2+2*k3+k4)/6;
    vector<pair<double,double>> sol = {{t0,y0},{t0+h,y1}};
    double t=t0+h, y=y1, yp=y0;
    while (t < t_end-1e-12) {
        double yc=f(t,y), ypp=f(t-h,yp);
        double yn=y+h*(3*yc-ypp)/2; yp=y; y=yn; t+=h; sol.push_back({t,y});
    }
    return sol;
}

// --- J.4. Пикар: yₙ₊₁(t) = y₀ + ∫f(s,yₙ(s))ds ---
function<double(double)> picard_iteration(
    function<double(double,double)> f, double t0, double y0,
    int iterations = 20, int n = 10000) {
    // Начальное приближение: y₀(t) = y₀
    vector<function<double(double)>> ys;
    ys.push_back([=](double t) { return y0; });
    for (int iter = 0; iter < iterations; ++iter) {
        auto prev = ys.back();
        ys.push_back([=](double t) {
            auto integrand = [&](double s) { return f(s, prev(s)); };
            return y0 + integral_simpson(integrand, t0, t, n/iterations);
        });
    }
    return ys.back();
}

// =============================================================
// L. КРАЕВЫЕ ЗАДАЧИ
// =============================================================

// --- L.1. Метод прогонки: y''+p(x)y'+q(x)y=f(x), y(a)=α, y(b)=β ---
vector<double> shooting_method(
    function<double(double,double,double)> f,
    double a, double b, double alpha, double beta, int n = 1000) {
    double h = (b-a)/n;
    vector<double> x(n+1), y(n+1);
    for (int i = 0; i <= n; ++i) x[i] = a + i*h;
    // Стреляем: y''=f(x,y,y') → система 1-го порядка
    // Два решения: с y'(a)=0 и y'(a)=1, комбинация
    auto solve_ivp = [&](double yp0) {
        vector<double> yy(n+1), yp(n+1);
        yy[0]=alpha; yp[0]=yp0;
        for (int i = 0; i < n; ++i) {
            double k1y=h*yp[i], k1p=h*f(x[i],yy[i],yp[i]);
            double k2y=h*(yp[i]+k1p/2), k2p=h*f(x[i]+h/2,yy[i]+k1y/2,yp[i]+k1p/2);
            yy[i+1]=yy[i]+k2y; yp[i+1]=yp[i]+k2p;
        }
        return yy[n];
    };
    // Бисекция по y'(a)
    double l=-100, r=100;
    for (int iter=0; iter<100; ++iter) {
        double m=(l+r)/2;
        if (solve_ivp(m) < beta) l=m; else r=m;
    }
    // Финальное решение
    double yp0=(l+r)/2;
    vector<double> result(n+1);
    vector<double> yy(n+1), yp(n+1);
    yy[0]=alpha; yp[0]=yp0;
    for (int i=0;i<n;++i) {
        double k1y=h*yp[i], k1p=h*f(x[i],yy[i],yp[i]);
        double k2y=h*(yp[i]+k1p/2), k2p=h*f(x[i]+h/2,yy[i]+k1y/2,yp[i]+k1p/2);
        yy[i+1]=yy[i]+k2y; yp[i+1]=yp[i]+k2p;
    }
    return yy;
}

};

#ifdef ANALYSIS_H_MAIN
int main() {
    DifferentialEquations de;
    cout << "=== H. Дифференциальные уравнения ===" << endl;

    // Характеристическое уравнение: r²+3r+2=0 → r₁=-1, r₂=-2
    auto [r1, r2] = de.char_roots_2nd(1.0, 3.0, 2.0);
    cout << "r²+3r+2=0: r₁=" << r1 << ", r₂=" << r2 << endl;

    // y''+3y'+2y=0, y(0)=1, y'(0)=0 → y=e^{-t}-e^{-2t}
    auto sol = de.solve_2nd_const(3.0, 2.0, 1.0, 0.0);
    cout << "y(0)=" << sol(0) << ", y(1)=" << sol(1) << endl;

    // RK4: y'=-y, y(0)=1 → y=e^{-t}
    auto rk = de.rk4_method([](double t, double y) { return -y; }, 0, 1, 1, 0.1);
    cout << "RK4 y'=-y: y(1)=" << rk.back().second << " (ожидается e⁻¹≈0.3679)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_H_CPP
