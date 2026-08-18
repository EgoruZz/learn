#ifndef LINEAR_ALGEBRA_I_CPP
#define LINEAR_ALGEBRA_I_CPP

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_I
#define INSIDE_LINEAR_ALGEBRA_I
#include "../h/h.cpp"
#endif

struct ComplexAndPolynomials : BilinearForms {

// --- A.1. Корень степени n ---
vector<complex<double>> nth_root(complex<double> z, int n) {
    double r = abs(z), phi = arg(z), rn = pow(r, 1.0 / n);
    vector<complex<double>> roots(n);
    for (int k = 0; k < n; k++)
        roots[k] = polar(rn, (phi + 2 * M_PI * k) / n);
    return roots;
}

// --- B.1. Многочлен: вычисление через Горнера ---
struct Polynomial {
    vector<double> c;
    Polynomial() {}
    Polynomial(const vector<double>& v) : c(v) {
        while (c.size() > 1 && fabs(c.back()) < 1e-12) c.pop_back();
    }
    int degree() const { return (int)c.size() - 1; }
    double eval(double x) const {
        double r = 0;
        for (int i = (int)c.size() - 1; i >= 0; i--) r = r * x + c[i];
        return r;
    }
};

// --- B.3. Интерполяция (Лагранж) ---
Polynomial lagrange_interpolation(const vector<double>& x, const vector<double>& y) {
    int n = x.size();
    vector<double> result(n, 0);
    for (int i = 0; i < n; i++) {
        vector<double> Li = {1.0};
        for (int j = 0; j < n; j++) {
            if (j == i) continue;
            vector<double> newLi(Li.size() + 1, 0);
            double denom = x[i] - x[j];
            for (int k = 0; k < (int)Li.size(); k++) {
                newLi[k] += Li[k] * (-x[j]) / denom;
                newLi[k + 1] += Li[k] / denom;
            }
            Li = newLi;
        }
        for (int k = 0; k < n; k++) result[k] += Li[k] * y[i];
    }
    return Polynomial(result);
}

}; // struct ComplexAndPolynomials

#endif // LINEAR_ALGEBRA_I_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_I_MAIN
int main() {
    ComplexAndPolynomials cp;
    cout << "=== Комплексные числа ===" << endl;
    complex<double> z(1, sqrt(3));
    cout << "|z| = " << abs(z) << ", arg = " << arg(z) * 180 / M_PI << " deg" << endl;

    cout << "\n=== Корни ===" << endl;
    auto roots = cp.nth_root(complex<double>(0, 1), 4);
    for (auto& r : roots) cout << r << " "; cout << endl;

    cout << "\n=== Многочлены ===" << endl;
    ComplexAndPolynomials::Polynomial P({1, -3, 2});
    cout << "P(1) = " << P.eval(1) << ", P(2) = " << P.eval(2) << endl;

    cout << "\n=== Интерполяция ===" << endl;
    vector<double> xs = {0, 1, 2, 3}, ys = {1, 4, 9, 16};
    auto interp = cp.lagrange_interpolation(xs, ys);
    cout << "P(0)=" << interp.eval(0) << " P(1)=" << interp.eval(1)
         << " P(2)=" << interp.eval(2) << " P(3)=" << interp.eval(3) << endl;

    return 0;
}
#endif
