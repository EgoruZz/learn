#ifndef LINEAR_ALGEBRA_M_CPP
#define LINEAR_ALGEBRA_M_CPP

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_M
#define INSIDE_LINEAR_ALGEBRA_M
#include "../l/l.cpp"
#endif

struct AppliedLinearAlgebra : NormedSpaces {

using DMatrix = vector<vector<double>>;

// =============================================================
// A. LU-РАЗЛОЖЕНИЕ
// =============================================================
// A = LU (L нижнетр. с 1 на диаг., U верхнетр.)
// O(n³) построение, O(n²) разрешение.
pair<DMatrix, DMatrix> lu_decomposition(DMatrix A) {
    int n = A.size();
    DMatrix L(n, vector<double>(n, 0));
    DMatrix U = A;
    for (int i = 0; i < n; i++) L[i][i] = 1;
    for (int col = 0; col < n; col++) {
        for (int row = col + 1; row < n; row++) {
            if (abs(U[col][col]) < 1e-12) continue;
            double factor = U[row][col] / U[col][col];
            L[row][col] = factor;
            for (int j = col; j < n; j++)
                U[row][j] -= U[col][j] * factor;
        }
    }
    return {L, U};
}

// Решение Ly = b (прямой ход)
vector<double> forward_sub(const DMatrix& L, const vector<double>& b) {
    int n = L.size();
    vector<double> y(n);
    for (int i = 0; i < n; i++) {
        y[i] = b[i];
        for (int j = 0; j < i; j++) y[i] -= L[i][j] * y[j];
    }
    return y;
}

// Решение Ux = y (обратный ход)
vector<double> backward_sub(const DMatrix& U, const vector<double>& y) {
    int n = U.size();
    vector<double> x(n);
    for (int i = n - 1; i >= 0; i--) {
        x[i] = y[i];
        for (int j = i + 1; j < n; j++) x[i] -= U[i][j] * x[j];
        x[i] /= U[i][i];
    }
    return x;
}

// =============================================================
// B. QR-РАЗЛОЖЕНИЕ
// =============================================================
// Через Грам-Шмидт: A = QR, Q ортогональная, R верхнетр.
// O(n²m) время.
pair<DMatrix, DMatrix> qr_decomposition(DMatrix A) {
    int m = A.size(), n = A[0].size();
    DMatrix Q(m, vector<double>(n, 0));
    DMatrix R(n, vector<double>(n, 0));

    for (int j = 0; j < n; j++) {
        // Копируем столбец j
        vector<double> v(m);
        for (int i = 0; i < m; i++) v[i] = A[i][j];

        for (int i = 0; i < j; i++) {
            double dot = 0;
            for (int k = 0; k < m; k++) dot += Q[k][i] * A[k][j];
            R[i][j] = dot;
            for (int k = 0; k < m; k++) v[k] -= dot * Q[k][i];
        }

        double norm = 0;
        for (int k = 0; k < m; k++) norm += v[k] * v[k];
        norm = sqrt(norm);
        R[j][j] = norm;
        if (norm > 1e-12)
            for (int k = 0; k < m; k++) Q[k][j] = v[k] / norm;
    }
    return {Q, R};
}

// =============================================================
// C. МНК
// =============================================================
// min ‖Ax − b‖₂; A_que = AᵀA, b_que = Aᵀb; решение A_que·x = b_que.
// O(n³) время.
vector<double> least_squares(const DMatrix& A, const vector<double>& b) {
    int m = A.size(), n = A[0].size();
    DMatrix AtA(n, vector<double>(n, 0));
    vector<double> Atb(n, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < m; k++)
                AtA[i][j] += A[k][i] * A[k][j];
    for (int i = 0; i < n; i++)
        for (int k = 0; k < m; k++)
            Atb[i] += A[k][i] * b[k];
    // Решаем через Гаусс
    vector<double> x(n, 0);
    DMatrix aug(n, vector<double>(n + 1));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) aug[i][j] = AtA[i][j];
        aug[i][n] = Atb[i];
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
    for (int i = 0; i < n; i++) x[i] = aug[i][n];
    return x;
}

// =============================================================
// D. ШЕРМАН-МОРРИСОН
// =============================================================
// (A + uvᵀ)⁻¹ = A⁻¹ − (A⁻¹u)(vᵀA⁻¹) / (1 + vᵀA⁻¹u)
// O(n²) время.
DMatrix sherman_morrison(const DMatrix& Ainv, const vector<double>& u,
                         const vector<double>& v) {
    int n = Ainv.size();
    // Ainv_u = A⁻¹ * u
    vector<double> Au(n, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) Au[i] += Ainv[i][j] * u[j];
    // vT * Ainv_u
    double vTAu = 0;
    for (int i = 0; i < n; i++) vTAu += v[i] * Au[i];
    if (fabs(1 + vTAu) < 1e-12) return {};  // вырожденный случай
    // A⁻¹u * vᵀA⁻¹
    DMatrix result = Ainv;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            result[i][j] -= Au[i] * v[j] / (1 + vTAu);
    return result;
}

// =============================================================
// E. ИТЕРАЦИОННЫЕ МЕТОДЫ
// =============================================================

// --- E.1. Метод Якоби ---
// x⁽ᵏ⁺¹⁾ᵢ = (bᵢ − Σ_{j≠i} aᵢⱼx⁽ᵏ⁾ⱼ) / aᵢᵢ
vector<double> jacobi(const DMatrix& A, const vector<double>& b,
                      int max_iter = 1000, double eps = 1e-10) {
    int n = A.size();
    vector<double> x(n, 0), x_new(n);
    for (int iter = 0; iter < max_iter; iter++) {
        for (int i = 0; i < n; i++) {
            double sum = 0;
            for (int j = 0; j < n; j++)
                if (j != i) sum += A[i][j] * x[j];
            x_new[i] = (b[i] - sum) / A[i][i];
        }
        double diff = 0;
        for (int i = 0; i < n; i++) diff += fabs(x_new[i] - x[i]);
        x = x_new;
        if (diff < eps) break;
    }
    return x;
}

// --- E.2. Метод Зейделя ---
vector<double> gauss_seidel(const DMatrix& A, const vector<double>& b,
                            int max_iter = 1000, double eps = 1e-10) {
    int n = A.size();
    vector<double> x(n, 0);
    for (int iter = 0; iter < max_iter; iter++) {
        double diff = 0;
        for (int i = 0; i < n; i++) {
            double old = x[i];
            double sum = 0;
            for (int j = 0; j < n; j++)
                if (j != i) sum += A[i][j] * x[j];
            x[i] = (b[i] - sum) / A[i][i];
            diff += fabs(x[i] - old);
        }
        if (diff < eps) break;
    }
    return x;
}

}; // struct AppliedLinearAlgebra

#endif // LINEAR_ALGEBRA_M_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_M_MAIN
int main() {
    AppliedLinearAlgebra al;

    cout << "=== LU ===" << endl;
    NormedSpaces::DMatrix A = {{2,1,-1},{-3,-1,2},{-2,1,2}};
    auto [L, U] = al.lu_decomposition(A);
    cout << "L: "; for (auto& row : L) { for (auto x : row) cout << x << " "; cout << endl; }
    cout << "U: "; for (auto& row : U) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "\n=== QR ===" << endl;
    NormedSpaces::DMatrix M = {{1,1},{1,2},{1,3}};
    auto [Q, R] = al.qr_decomposition(M);
    cout << "Q: "; for (auto& row : Q) { for (auto x : row) cout << x << " "; cout << endl; }
    cout << "R: "; for (auto& row : R) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "\n=== МНК ===" << endl;
    NormedSpaces::DMatrix A2 = {{1,1},{2,2},{3,3}};
    vector<double> b2 = {2,4,6};
    auto x = al.least_squares(A2, b2);
    cout << "x = "; for (auto v : x) cout << v << " "; cout << endl;

    cout << "\n=== Sherman-Morrison ===" << endl;
    NormedSpaces::DMatrix Ainv = {{1,0},{0,1}};
    vector<double> u = {1,0}, v = {0,1};
    auto result = al.sherman_morrison(Ainv, u, v);
    cout << "(I + e₁e₂ᵀ)⁻¹: ";
    for (auto& row : result) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "\n=== Якоби ===" << endl;
    NormedSpaces::DMatrix A3 = {{4,1},{1,3}};
    vector<double> b3 = {1,2};
    auto xj = al.jacobi(A3, b3);
    cout << "x = "; for (auto v : xj) cout << v << " "; cout << endl;

    cout << "\n=== Зейдель ===" << endl;
    auto xs = al.gauss_seidel(A3, b3);
    cout << "x = "; for (auto v : xs) cout << v << " "; cout << endl;

    return 0;
}
#endif
