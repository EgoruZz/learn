#ifndef ANALYSIS_F_CPP
#define ANALYSIS_F_CPP

#include "../e/e.cpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include <cassert>
using namespace std;

// =============================================================
// F. КРАТНЫЕ, КРИВОЛИНЕЙНЫЕ И ПОВЕРХНОСТНЫЕ ИНТЕГРАЛЫ
// =============================================================
// MultipleIntegrals : Series

struct MultipleIntegrals : Series {

// =============================================================
// A. ДВОЙНОЙ ИНТЕГРАЛ
// =============================================================

// --- A.1. Двойной интеграл (метод прямоугольников) ---
// ∬_D f(x,y) dA ≈ Σ f(xᵢ,yⱼ) · Δx · Δy
// O(nx·ny) время, O(1) память.
double double_integral(function<double(double,double)> f,
                       double ax, double bx, double ay, double by,
                       int nx = 100, int ny = 100) {
    double dx = (bx-ax)/nx, dy = (by-ay)/ny, sum = 0;
    for (int i = 0; i < nx; ++i)
        for (int j = 0; j < ny; ++j)
            sum += f(ax + (i+0.5)*dx, ay + (j+0.5)*dy);
    return sum * dx * dy;
}

// --- A.2. Двойной интеграл (метод Симпсона) ---
// O(nx·ny) время, O(1) память. Погрешность O(h⁴).
double double_integral_simpson(function<double(double,double)> f,
                                double ax, double bx, double ay, double by,
                                int nx = 100, int ny = 100) {
    assert(nx % 2 == 0 && ny % 2 == 0);
    double dx = (bx-ax)/nx, dy = (by-ay)/ny;
    double sum = 0;
    for (int i = 0; i <= nx; ++i) {
        for (int j = 0; j <= ny; ++j) {
            double x = ax + i*dx, y = ay + j*dy;
            double wi = (i == 0 || i == nx) ? 1 : (i % 2 == 0 ? 2 : 4);
            double wj = (j == 0 || j == ny) ? 1 : (j % 2 == 0 ? 2 : 4);
            sum += wi * wj * f(x, y);
        }
    }
    return sum * dx * dy / 9.0;
}

// =============================================================
// B. ДВОЙНОЙ ИНТЕГРАЛ В ПОЛЯРНЫХ
// =============================================================

// --- B.1. Двойной интеграл в полярных координатах ---
// ∬ f(r cos θ, r sin θ) · r dr dθ
double double_integral_polar(function<double(double,double)> f,
                              double rmax, int nr = 100, int nt = 100) {
    double dr = rmax/nr, dt = 2*M_PI/nt, sum = 0;
    for (int i = 0; i < nr; ++i)
        for (int j = 0; j < nt; ++j) {
            double r = (i+0.5)*dr, t = (j+0.5)*dt;
            sum += f(r*cos(t), r*sin(t)) * r;
        }
    return sum * dr * dt;
}

// =============================================================
// C. КРИВОЛИНЕЙНЫЕ ИНТЕГРАЛЫ
// =============================================================

// --- C.1. Криволинейный I род: ∫_C f ds ---
// ds = |r'(t)| dt = √(x'²+y'²) dt
double line_integral_1st(function<double(double,double)> f,
                          function<double(double)> x, function<double(double)> y,
                          double a, double b, int n = 1000) {
    double dt = (b-a)/n, sum = 0;
    for (int i = 0; i < n; ++i) {
        double t = a + (i+0.5)*dt;
        double dx = (x(t+dt/2)-x(t-dt/2))/dt;
        double dy = (y(t+dt/2)-y(t-dt/2))/dt;
        sum += f(x(t), y(t)) * sqrt(dx*dx + dy*dy);
    }
    return sum * dt;
}

// --- C.2. Криволинейный II род: ∫_C P dx + Q dy ---
double line_integral_2nd(function<double(double,double)> P,
                          function<double(double,double)> Q,
                          function<double(double)> x, function<double(double)> y,
                          double a, double b, int n = 1000) {
    double dt = (b-a)/n, sum = 0;
    for (int i = 0; i < n; ++i) {
        double t = a + (i+0.5)*dt;
        double dx = (x(t+dt/2)-x(t-dt/2))/dt;
        double dy = (y(t+dt/2)-y(t-dt/2))/dt;
        sum += P(x(t),y(t))*dx + Q(x(t),y(t))*dy;
    }
    return sum * dt;
}

// --- C.3. Длина дуги кривой ---
double arc_length_parametric(function<double(double)> x,
                              function<double(double)> y,
                              double a, double b, int n = 1000) {
    auto one = [](double,double) { return 1.0; };
    return line_integral_1st(one, x, y, a, b, n);
}

// =============================================================
// D. ТЕОРЕМА ГРИНА
// =============================================================

// --- D.1. Проверка условия независимости от пути ---
// ∂P/∂y = ∂Q/∂x ⟹ ∫ P dx + Q dy не зависит от пути
bool greens_independence(function<double(double,double)> P,
                          function<double(double,double)> Q,
                          double x, double y, double h = 1e-8) {
    double dPdy = (P(x,y+h)-P(x,y-h))/(2*h);
    double dQdx = (Q(x+h,y)-Q(x-h,y))/(2*h);
    return abs(dPdy - dQdx) < 1e-6;
}

// =============================================================
// E. ТРОЙНЫЕ ИНТЕГРАЛЫ
// =============================================================

// --- E.1. Тройной интеграл (метод прямоугольников) ---
double triple_integral(function<double(double,double,double)> f,
                       double ax, double bx, double ay, double by,
                       double az, double bz, int n = 30) {
    double dx=(bx-ax)/n, dy=(by-ay)/n, dz=(bz-az)/n, sum=0;
    for(int i=0;i<n;++i) for(int j=0;j<n;++j) for(int k=0;k<n;++k)
        sum += f(ax+(i+.5)*dx, ay+(j+.5)*dy, az+(k+.5)*dz);
    return sum*dx*dy*dz;
}

// =============================================================
// MAIN
// =============================================================

}; // struct MultipleIntegrals

#ifdef ANALYSIS_F_MAIN
int main() {
    MultipleIntegrals mi;
    cout << "=== F. Кратные интегралы ===" << endl;

    // Двойной интеграл: ∬ xy dA на [0,1]×[0,1] = 1/4
    auto f_xy = [](double x, double y) { return x*y; };
    double I = mi.double_integral(f_xy, 0, 1, 0, 1);
    cout << "∬ xy dA на [0,1]² = " << I << " (ожидается 0.25)" << endl;

    // Симпсон
    double Is = mi.double_integral_simpson(f_xy, 0, 1, 0, 1);
    cout << " Simpson = " << Is << endl;

    // Полярный: ∬ e^{-(x²+y²)} dA на полукруге → π/2·(1−e^{-R²})
    auto f_polar = [](double x, double y) { return exp(-(x*x+y*y)); };
    double Ip = mi.double_integral_polar(f_polar, 3.0);
    cout << "Полярный ∬ e^{-r²}: " << Ip << endl;

    // Тройной: ∬∫ xyz на [0,1]³ = 1/8
    auto f_xyz = [](double x, double y, double z) { return x*y*z; };
    double T = mi.triple_integral(f_xyz, 0, 1, 0, 1, 0, 1);
    cout << "Тройной ∬∫ xyz = " << T << " (ожидается 0.125)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_F_CPP
