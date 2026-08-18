#ifndef ANALYSIS_G_CPP
#define ANALYSIS_G_CPP

#include "../f/f.cpp"
#include <iostream>
#include <complex>
#include <vector>
#include <cmath>
#include <functional>
using namespace std;

typedef complex<double> cd;

struct ComplexAnalysis : MultipleIntegrals {

// --- A. Комплексные числа: корни ---
vector<cd> nth_roots(cd z, int n) {
    double r = abs(z), theta = arg(z);
    vector<cd> roots(n);
    for (int k = 0; k < n; ++k)
        roots[k] = pow(r, 1.0/n) * cd(cos((theta+2*M_PI*k)/n), sin((theta+2*M_PI*k)/n));
    return roots;
}

// --- B. Проверка условий Коши-Римана ---
bool cauchy_riemann(function<cd(cd)> f, cd z, double h = 1e-8) {
    cd dz(h, 0), di(0, h);
    cd dfdx = (f(z+dz) - f(z-dz)) / (2.0*h);
    cd dfdy = (f(z+di) - f(z-di)) / (2.0*h);
    // Cauchy-Riemann: df/dx = -i·df/dy
    return abs(dfdx - cd(0,-1)*dfdy) < 1e-6;
}

// --- B. Производная комплексной функции (numerical) ---
cd complex_derivative(function<cd(cd)> f, cd z, double h = 1e-8) {
    return (f(z+h) - f(z-h)) / (2.0*h);
}

// --- C. Интеграл по контуру (数值) ---
// ∮ f(z) dz через дискретизацию контура
cd contour_integral(function<cd(cd)> f,
                     function<cd(double)> gamma,
                     double a, double b, int n = 1000) {
    double dt = (b-a)/n;
    cd sum = 0;
    for (int i = 0; i < n; ++i) {
        double t = a + (i+0.5)*dt;
        cd z = gamma(t);
        cd dz = (gamma(t+dt/2) - gamma(t-dt/2));
        sum += f(z) * dz;
    }
    return sum;
}

// --- C. Формула Коши: f(z₀) = 1/(2πi) ∮ f(z)/(z−z₀) dz ---
cd cauchy_formula(function<cd(cd)> f, cd z0, double R = 1.0, int n = 1000) {
    auto gamma = [&](double t) -> cd { return z0 + R * cd(cos(t), sin(t)); };
    auto integrand = [&](cd z) -> cd { return f(z) / (z - z0); };
    cd integral = contour_integral(integrand, gamma, 0, 2*M_PI, n);
    return integral / (2.0 * M_PI * cd(0, 1));
}

// --- C. Теорема Лиувилля (проверка) ---
// Если |f(z)| ≤ M для всех z → f = const
bool liouville_check(function<cd(cd)> f, double R_max = 100, int samples = 1000) {
    double M = 0;
    for (int i = 0; i < samples; ++i) {
        double r = R_max * (double)rand() / RAND_MAX;
        double theta = 2 * M_PI * (double)rand() / RAND_MAX;
        cd z = r * cd(cos(theta), sin(theta));
        M = max(M, abs(f(z)));
    }
    return M < 1e6; // приближённая проверка
}

};

#ifdef ANALYSIS_G_MAIN
int main() {
    ComplexAnalysis ca;
    cout << "=== G. Комплексный анализ ===" << endl;

    // Корни из i: i^{1/2} = (±1+i)/√2
    auto roots = ca.nth_roots(cd(0, 1), 2);
    cout << "√i = " << roots[0] << ", " << roots[1] << endl;

    // Каучи-Риман для f(z)=z²: выполняется
    auto z2 = [](cd z) { return z*z; };
    cout << "C-R для z²: " << (ca.cauchy_riemann(z2, cd(1, 1)) ? "да" : "нет") << endl;

    // Интеграл по контуру: ∮ z dz = 0 (полином)
    auto gamma = [](double t) -> cd { return exp(cd(0, t)); }; // |z|=1
    cd I = ca.contour_integral(z2, gamma, 0, 2*M_PI);
    cout << "∮ z² dz ≈ " << abs(I) << " (ожидается ≈0)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_G_CPP
