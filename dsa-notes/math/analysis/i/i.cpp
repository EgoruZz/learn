#ifndef ANALYSIS_I_CPP
#define ANALYSIS_I_CPP

#include "../h/h.cpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include <complex>
#include <cassert>
using namespace std;

// =============================================================
// I. ОПЕРАЦИОННОЕ ИСЧИСЛЕНИЕ
// =============================================================
// OperationalCalculus : DifferentialEquations

struct OperationalCalculus : DifferentialEquations {

// =============================================================
// A. ПРЕОБРАЗОВАНИЕ ЛАПЛАСА
// =============================================================

// --- A.1. Преобразование Лапласа (численно) ---
// L{f}(s) = ∫₀^∞ e^{-st}f(t)dt
// O(n) время (интеграл Симпсона), O(1) память.
double laplace(function<double(double)> f, double s,
               double T = 30, int n = 10000) {
    auto integrand = [&](double t) { return exp(-s*t) * f(t); };
    return integral_simpson(integrand, 0, T, n);
}

// --- A.2. Обратное преобразование (интеграл Коши, численно) ---
// f(t) = (1/2π) ∫ L{f}(c+iω) e^{st} dω
double inverse_laplace(function<double(double)> F, double t,
                        double c = 2.0, double w_max = 100, int n = 4000) {
    auto integrand = [&](double w) {
        cd s_val(c, w);
        cd exp_st = exp(s_val * t);
        cd Fs(F(c), F(sqrt(c*c + w*w))); // упрощённо
        cd val = exp_st * Fs;
        return val.real();
    };
    return integral_simpson(integrand, -w_max, w_max, n) / (2*M_PI);
}

// =============================================================
// B. СВОЙСТВА
// =============================================================

// --- B.1. Хевисайд ---
double heaviside(double t, double a = 0) { return (t >= a) ? 1.0 : 0.0; }

// --- B.2. Дирак (σ-последовательность: лоренцева) ---
double dirac_delta(double t, double epsilon = 0.01) {
    return epsilon / (M_PI * (t*t + epsilon*epsilon));
}

// --- B.3. Теорема подобия: L{f(at)} = (1/a)F(s/a) ---
double laplace_similarity(function<double(double)> f, double s, double a) {
    return (1.0/a) * laplace(f, s/a);
}

// --- B.4. Лаплас периодической функции ---
// L{f}(s) = (1/(1−e^{-sT})) ∫₀^T e^{-st}f(t)dt
double laplace_periodic(function<double(double)> f, double s, double T,
                         int n = 10000) {
    auto integrand = [&](double t) { return exp(-s*t) * f(t); };
    double int_FT = integral_simpson(integrand, 0, T, n);
    return int_FT / (1.0 - exp(-s*T));
}

// =============================================================
// C. РЕШЕНИЕ ДУ
// =============================================================

// --- C.1. Решение y''+ay'+by=0 через Лаплас ---
function<double(double)> solve_ode_laplace(double a, double b,
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
    return [=](double t){
        return exp(alpha*t)*(y0*cos(beta*t)+((y1-alpha*y0)/beta)*sin(beta*t));
    };
}

// --- C.2. Свёртка (Дюамель) ---
// y(t) = ∫₀ᵗ g(τ)h(t−τ)dτ
double convolution(function<double(double)> g, function<double(double)> h,
                    double t, int n = 10000) {
    auto integrand = [&](double tau) { return g(tau) * h(t - tau); };
    return integral_simpson(integrand, 0, t, n);
}

// --- C.3. Импульсная характеристика: L⁻¹{1/(s²+as+b)} ---
function<double(double)> impulse_response(double a, double b) {
    return solve_ode_laplace(a, b, 0, 1);
}

}; // struct OperationalCalculus

#ifdef ANALYSIS_I_MAIN
int main() {
    OperationalCalculus oc;
    cout << "=== I. Операционное исчисление ===" << endl;

    // Проверка: L{e^{2t}} = 1/(s−2), s=3 → 1/(3−2) = 1
    auto f_exp = [](double t) { return exp(2*t); };
    cout << "L{e^{2t}}(s=3) = " << oc.laplace(f_exp, 3.0) << " (ожидается ≈1.0)" << endl;

    // L{sin(t)} = 1/(s²+1), s=2 → 2/(4+1) = 0.4
    auto f_sin = [](double t) { return sin(t); };
    cout << "L{sin(t)}(s=2) = " << oc.laplace(f_sin, 2.0) << " (ожидается 0.4)" << endl;

    // Хевисайд
    cout << "H(−1) = " << oc.heaviside(-1) << ", H(0.5) = " << oc.heaviside(0.5) << endl;

    // Дирак
    cout << "δ(0) ≈ " << oc.dirac_delta(0) << " (огромное)" << endl;
    cout << "δ(0.1) ≈ " << oc.dirac_delta(0.1) << endl;

    // Решение y''+3y'+2y=0, y(0)=1, y'(0)=0
    auto sol = oc.solve_ode_laplace(3.0, 2.0, 1.0, 0.0);
    cout << "y(t) для y''+3y'+2y=0: y(0)=" << sol(0) << ", y(1)=" << sol(1) << endl;

    // Свёртка: (1*1)(t) = t
    auto ones = [](double) { return 1.0; };
    cout << "Свёртка (1*1)(2) = " << oc.convolution(ones, ones, 2.0) << " (ожидается 2.0)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_I_CPP
