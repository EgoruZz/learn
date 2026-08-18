#ifndef LINEAR_ALGEBRA_G_CPP
#define LINEAR_ALGEBRA_G_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_G
#define INSIDE_LINEAR_ALGEBRA_G
#include "../f/f.cpp"
#endif

struct LinearOperators : VectorSpaces {

using DMatrix = vector<vector<double>>;

// --- A.1. Матрица линейного отображения ---
DMatrix linear_map_matrix(const vector<vector<double>>& v_basis,
                          const vector<vector<double>>& w_basis,
                          const vector<vector<double>>& images) {
    int m = w_basis.size(), n = v_basis.size();
    DMatrix A(m, vector<double>(n));
    for (int j = 0; j < n; j++) {
        auto coords = coordinates_in_basis(images[j], w_basis);
        for (int i = 0; i < m; i++) A[i][j] = coords[i];
    }
    return A;
}

// --- A.2. Ядро ---
vector<vector<double>> kernel_basis_op(const DMatrix& A) {
    int n = A.size(), m = A[0].size();
    DMatrix R = A;
    for (int col = 0; col < m; col++) {
        int pivot = -1;
        for (int row = col; row < n; row++)
            if (abs(R[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) continue;
        swap(R[col], R[pivot]);
        double lead = R[col][col];
        for (int j = col; j < m; j++) R[col][j] /= lead;
        for (int row = 0; row < n; row++) {
            if (row == col || abs(R[row][col]) < 1e-12) continue;
            double f = R[row][col];
            for (int j = col; j < m; j++) R[row][j] -= R[col][j] * f;
        }
    }
    vector<bool> is_pivot(m, false);
    for (int i = 0; i < min(n, m); i++)
        if (abs(R[i][i]) > 1e-12) is_pivot[i] = true;
    vector<vector<double>> basis;
    for (int j = 0; j < m; j++) {
        if (is_pivot[j]) continue;
        vector<double> v(m, 0);
        v[j] = 1.0;
        for (int i = 0; i < min(n, m); i++)
            if (is_pivot[i] && abs(R[i][j]) > 1e-12) v[i] = -R[i][j];
        basis.push_back(v);
    }
    return basis;
}

int dim_kernel(const DMatrix& A) { return kernel_basis_op(A).size(); }
int dim_image(const DMatrix& A) { return gaussian_rank(A); }
bool is_injective(const DMatrix& A) { return dim_kernel(A) == 0; }
bool is_surjective(const DMatrix& A) { return dim_image(A) == (int)A.size(); }

// --- B.1. Замена базиса: A' = P^(-1) * A * P ---
DMatrix change_basis(const DMatrix& A, const DMatrix& P) {
    DMatrix Pinv = inverse_gauss(P);
    if (Pinv.empty()) return {};
    DMatrix AP = DMatrix(A.size(), vector<double>(A[0].size()));
    for (int i = 0; i < (int)A.size(); i++)
        for (int j = 0; j < (int)A[0].size(); j++)
            for (int k = 0; k < (int)A.size(); k++)
                AP[i][j] += A[i][k] * P[k][j];
    DMatrix result = DMatrix(Pinv.size(), vector<double>(Pinv[0].size()));
    for (int i = 0; i < (int)Pinv.size(); i++)
        for (int j = 0; j < (int)Pinv[0].size(); j++)
            for (int k = 0; k < (int)Pinv.size(); k++)
                result[i][j] += Pinv[i][k] * AP[k][j];
    return result;
}

}; // struct LinearOperators

#endif // LINEAR_ALGEBRA_G_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_G_MAIN
int main() {
    LinearOperators lo;
    cout << "=== Линейное отображение ===" << endl;
    vector<vector<double>> v_b = {{1,0},{0,1}}, w_b = {{1,1},{1,-1}};
    vector<vector<double>> imgs = {{1,2},{3,4}};
    auto A = lo.linear_map_matrix(v_b, w_b, imgs);
    for (auto& row : A) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "\n=== Ядро ===" << endl;
    vector<vector<double>> mat = {{1,2,3},{4,5,6}};
    cout << "dim(ker) = " << lo.dim_kernel(mat) << endl;

    cout << "\n=== Инъективность ===" << endl;
    vector<vector<double>> I = {{1,0,0},{0,1,0},{0,0,1}};
    cout << "I injective? " << lo.is_injective(I) << endl;

    cout << "\n=== Замена базиса ===" << endl;
    vector<vector<double>> A_op = {{2,1},{1,2}};
    vector<vector<double>> P = {{1,1},{1,-1}};
    auto A_new = lo.change_basis(A_op, P);
    for (auto& row : A_new) { for (auto x : row) cout << x << " "; cout << endl; }

    return 0;
}
#endif
