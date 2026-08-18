#ifndef LINEAR_ALGEBRA_L_CPP
#define LINEAR_ALGEBRA_L_CPP

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_L
#define INSIDE_LINEAR_ALGEBRA_L
#include "../k/k.cpp"
#endif

struct NormedSpaces : FunctionalMatrices {

using DMatrix = vector<vector<double>>;

// =============================================================
// A. НОРМЫ
// =============================================================

double norm_l1(const vector<double>& x) {
    double r = 0; for (double v : x) r += fabs(v); return r;
}
double norm_l2(const vector<double>& x) {
    double r = 0; for (double v : x) r += v*v; return sqrt(r);
}
double norm_linf(const vector<double>& x) {
    double r = 0; for (double v : x) r = max(r, fabs(v)); return r;
}
double norm_frobenius(const DMatrix& A) {
    double r = 0; for (auto& row : A) for (double v : row) r += v*v; return sqrt(r);
}

// =============================================================
// C. НОРМА ОПЕРАТОРА
// =============================================================
// Spectral norm: ‖A‖₂ = σ₁ (максимальное сингулярное число).
// Через итерационный метод: ‖A‖₂² ≈ max x^T A^T A x / x^T x.
double spectral_norm(const DMatrix& A, int iterations = 100) {
    int n = A.size();
    vector<double> x(n, 1.0 / sqrt(n));
    for (int iter = 0; iter < iterations; iter++) {
        // y = A^T * (A * x)
        vector<double> y(n, 0);
        for (int i = 0; i < n; i++) {
            double ax = 0;
            for (int j = 0; j < n; j++) ax += A[i][j] * x[j];
            for (int k = 0; k < n; k++) y[k] += A[k][i] * ax;
        }
        double norm = 0;
        for (double v : y) norm += v * v;
        norm = sqrt(norm);
        for (int i = 0; i < n; i++) x[i] = y[i] / norm;
    }
    double ax = 0;
    for (int i = 0; i < n; i++) {
        double s = 0;
        for (int j = 0; j < n; j++) s += A[i][j] * x[j];
        ax += s * s;
    }
    return sqrt(ax);
}

// =============================================================
// D. ОРТОГОНАЛИЗАЦИЯ ГРАМА-ШМИДТА
// =============================================================
// Из n векторов размера m → n ортонормальных.
// O(n²m) время.
pair<vector<vector<double>>, vector<vector<double>>> gram_schmidt(
    const vector<vector<double>>& vectors) {
    int n = vectors.size();
    int m = vectors[0].size();
    vector<vector<double>> u = vectors;
    vector<vector<double>> e(n, vector<double>(m));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            double dot_ej_vi = 0;
            for (int k = 0; k < m; k++) dot_ej_vi += e[j][k] * vectors[i][k];
            for (int k = 0; k < m; k++) u[i][k] -= dot_ej_vi * e[j][k];
        }
        double norm = 0;
        for (double v : u[i]) norm += v * v;
        norm = sqrt(norm);
        if (norm > 1e-12)
            for (int k = 0; k < m; k++) e[i][k] = u[i][k] / norm;
    }
    return {u, e};
}

}; // struct NormedSpaces

#endif // LINEAR_ALGEBRA_L_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_L_MAIN
int main() {
    NormedSpaces ns;
    cout << "=== Нормы ===" << endl;
    vector<double> x = {3, -4, 0};
    cout << "L1: " << ns.norm_l1(x) << endl;
    cout << "L2: " << ns.norm_l2(x) << endl;
    cout << "L∞: " << ns.norm_linf(x) << endl;

    cout << "\n=== Спектральная норма ===" << endl;
    NormedSpaces::DMatrix A = {{2,1},{1,2}};
    cout << "‖A‖₂ = " << ns.spectral_norm(A) << endl;

    cout << "\n=== Грам-Шмидт ===" << endl;
    vector<vector<double>> v = {{1,1,0},{1,0,1},{0,1,1}};
    auto [u, e] = ns.gram_schmidt(v);
    for (auto& row : e) { for (auto x : row) cout << x << " "; cout << endl; }

    return 0;
}
#endif
