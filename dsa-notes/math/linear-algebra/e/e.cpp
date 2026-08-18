#ifndef LINEAR_ALGEBRA_E_CPP
#define LINEAR_ALGEBRA_E_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_E
#define INSIDE_LINEAR_ALGEBRA_E
#include "../d/d.cpp"
#endif

struct LinearSystems : InverseAndRank {

using DMatrix = vector<vector<double>>;

// Гаусс для double (ранг расширенной матрицы)
int gaussian_rank(DMatrix A) {
    int n = A.size(), m = A[0].size();
    int rank = 0;
    for (int col = 0; col < m && rank < n; col++) {
        int pivot = -1;
        for (int row = rank; row < n; row++)
            if (abs(A[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) continue;
        swap(A[rank], A[pivot]);
        for (int row = 0; row < n; row++) {
            if (row == rank || abs(A[row][col]) < 1e-12) continue;
            double f = A[row][col];
            for (int j = col; j < m; j++) A[row][j] -= A[rank][j] * f;
        }
        rank++;
    }
    return rank;
}

// --- B.1. Решение СЛУ методом Гаусса ---
vector<double> solve_gauss(DMatrix A, vector<double> b) {
    int n = A.size();
    vector<vector<double>> aug(n, vector<double>(n + 1));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) aug[i][j] = A[i][j];
        aug[i][n] = b[i];
    }
    for (int col = 0; col < n; col++) {
        int pivot = -1;
        for (int row = col; row < n; row++)
            if (abs(aug[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) continue;
        swap(aug[col], aug[pivot]);
        double lead = aug[col][col];
        for (int j = col; j <= n; j++) aug[col][j] /= lead;
        for (int row = 0; row < n; row++) {
            if (row == col || abs(aug[row][col]) < 1e-12) continue;
            double f = aug[row][col];
            for (int j = col; j <= n; j++) aug[row][j] -= aug[col][j] * f;
        }
    }
    vector<double> x(n, 0);
    for (int i = 0; i < n; i++)
        if (abs(aug[i][i]) > 1e-12) x[i] = aug[i][n];
    return x;
}

// --- B.2. Метод обратной матрицы ---
vector<double> solve_inverse(const DMatrix& A, const vector<double>& b) {
    DMatrix Ainv = inverse_gauss(A);
    if (Ainv.empty()) return {};
    int n = A.size();
    vector<double> x(n, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            x[i] += Ainv[i][j] * b[j];
    return x;
}

// --- C.1. Совместность ---
int check_consistency(DMatrix A, vector<double> b) {
    int n = A.size();
    int rank_a = gaussian_rank(A);
    DMatrix aug(n, vector<double>(n + 1));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) aug[i][j] = A[i][j];
        aug[i][n] = b[i];
    }
    int rank_ab = gaussian_rank(aug);
    if (rank_a != rank_ab) return 0;
    return (rank_a == n) ? 1 : 2;
}

// --- C.2. ФСР однородной ---
vector<vector<double>> kernel_basis(DMatrix A) {
    int n = A.size(), m = A[0].size();
    for (int col = 0; col < m; col++) {
        int pivot = -1;
        for (int row = col; row < n; row++)
            if (abs(A[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) continue;
        swap(A[col], A[pivot]);
        double lead = A[col][col];
        for (int j = col; j < m; j++) A[col][j] /= lead;
        for (int row = 0; row < n; row++) {
            if (row == col || abs(A[row][col]) < 1e-12) continue;
            double f = A[row][col];
            for (int j = col; j < m; j++) A[row][j] -= A[col][j] * f;
        }
    }
    vector<bool> is_pivot(m, false);
    for (int i = 0; i < min(n, m); i++)
        if (abs(A[i][i]) > 1e-12) is_pivot[i] = true;
    vector<vector<double>> basis;
    for (int j = 0; j < m; j++) {
        if (is_pivot[j]) continue;
        vector<double> v(m, 0);
        v[j] = 1.0;
        for (int i = 0; i < min(n, m); i++)
            if (is_pivot[i] && abs(A[i][j]) > 1e-12)
                v[i] = -A[i][j];
        basis.push_back(v);
    }
    return basis;
}

}; // struct LinearSystems

#endif // LINEAR_ALGEBRA_E_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_E_MAIN
int main() {
    LinearSystems ls;
    cout << "=== Гаусс ===" << endl;
    LinearSystems::DMatrix A = {{2,1,-1},{-3,-1,2},{-2,1,2}};
    vector<double> b = {8, -11, -3};
    auto x = ls.solve_gauss(A, b);
    cout << "x = "; for (auto v : x) cout << v << " "; cout << endl;

    cout << "\n=== Обратная ===" << endl;
    auto x2 = ls.solve_inverse(A, b);
    cout << "x = "; for (auto v : x2) cout << v << " "; cout << endl;

    cout << "\n=== Совместность ===" << endl;
    cout << "status = " << ls.check_consistency(A, b) << endl;

    cout << "\n=== ФСР ===" << endl;
    LinearSystems::DMatrix A2 = {{1,2,3},{4,5,6}};
    auto basis = ls.kernel_basis(A2);
    cout << "dim(ker) = " << basis.size() << endl;
    for (auto& v : basis) { for (auto x : v) cout << x << " "; cout << endl; }

    return 0;
}
#endif
