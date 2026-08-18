#ifndef LINEAR_ALGEBRA_K_CPP
#define LINEAR_ALGEBRA_K_CPP

#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_K
#define INSIDE_LINEAR_ALGEBRA_K
#include "../j/j.cpp"
#endif

struct FunctionalMatrices : JordanForm {

using DMatrix = vector<vector<double>>;

// --- B.1. Численный градиент ---
vector<double> gradient(const function<double(const vector<double>&)>& f,
                        const vector<double>& x, double h = 1e-7) {
    int n = x.size();
    vector<double> grad(n);
    for (int i = 0; i < n; i++) {
        vector<double> xp = x, xm = x;
        xp[i] += h; xm[i] -= h;
        grad[i] = (f(xp) - f(xm)) / (2 * h);
    }
    return grad;
}

// --- B.2. Численный якобиан ---
DMatrix jacobian(const function<vector<double>(const vector<double>&)>& f,
                 const vector<double>& x, int m, double h = 1e-7) {
    int n = x.size();
    DMatrix J(m, vector<double>(n));
    for (int j = 0; j < n; j++) {
        vector<double> xp = x, xm = x;
        xp[j] += h; xm[j] -= h;
        auto fp = f(xp), fm = f(xm);
        for (int i = 0; i < m; i++)
            J[i][j] = (fp[i] - fm[i]) / (2 * h);
    }
    return J;
}

// --- B.3. Численный гессиан ---
DMatrix hessian(const function<double(const vector<double>&)>& f,
                const vector<double>& x, double h = 1e-5) {
    int n = x.size();
    DMatrix H(n, vector<double>(n));
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            vector<double> xpp = x, xpm = x, xmp = x, xmm = x;
            xpp[i] += h; xpp[j] += h;
            xpm[i] += h; xpm[j] -= h;
            xmp[i] -= h; xmp[j] += h;
            xmm[i] -= h; xmm[j] -= h;
            H[i][j] = H[j][i] = (f(xpp) - f(xpm) - f(xmp) + f(xmm)) / (4*h*h);
        }
    }
    return H;
}

}; // struct FunctionalMatrices

#endif // LINEAR_ALGEBRA_K_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_K_MAIN
int main() {
    FunctionalMatrices fm;
    cout << "=== Градиент ===" << endl;
    auto f1 = [](const vector<double>& x) -> double { return x[0]*x[0]+x[1]*x[1]; };
    auto grad = fm.gradient(f1, {3.0, 4.0});
    cout << "∇(x²+y²)(3,4) = (" << grad[0] << ", " << grad[1] << ")" << endl;

    cout << "\n=== Якобиан ===" << endl;
    auto f2 = [](const vector<double>& x) -> vector<double> { return {x[0]*x[0]+x[1], x[0]*x[1]}; };
    auto J = fm.jacobian(f2, {2.0, 3.0}, 2);
    for (auto& row : J) { for (auto v : row) cout << v << " "; cout << endl; }

    cout << "\n=== Гессиан ===" << endl;
    auto f3 = [](const vector<double>& x) -> double { return x[0]*x[0]*x[1]; };
    auto H = fm.hessian(f3, {1.0, 2.0});
    for (auto& row : H) { for (auto v : row) cout << v << " "; cout << endl; }

    return 0;
}
#endif
