#ifndef LINEAR_ALGEBRA_D_CPP
#define LINEAR_ALGEBRA_D_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_D
#define INSIDE_LINEAR_ALGEBRA_D
#include "../c/c.cpp"
#endif

struct InverseAndRank : Determinants {

using Matrix = vector<vector<long long>>;
using DMatrix = vector<vector<double>>;

// --- A.1. Обратная через Гаусс (double) ---
DMatrix inverse_gauss(DMatrix A) {
    int n = A.size();
    DMatrix inv(n, vector<double>(n, 0));
    for (int i = 0; i < n; i++) inv[i][i] = 1.0;
    for (int col = 0; col < n; col++) {
        int pivot = -1;
        for (int row = col; row < n; row++)
            if (abs(A[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) return {};
        if (pivot != col) { swap(A[col], A[pivot]); swap(inv[col], inv[pivot]); }
        double lead = A[col][col];
        for (int j = 0; j < n; j++) { A[col][j] /= lead; inv[col][j] /= lead; }
        for (int row = 0; row < n; row++) {
            if (row == col || abs(A[row][col]) < 1e-12) continue;
            double f = A[row][col];
            for (int j = 0; j < n; j++) { A[row][j] -= A[col][j]*f; inv[row][j] -= inv[col][j]*f; }
        }
    }
    return inv;
}

bool check_inverse(const DMatrix& A, const DMatrix& Ainv) {
    int n = A.size();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double s = 0;
            for (int k = 0; k < n; k++) s += A[i][k]*Ainv[k][j];
            if (abs(s - (i==j ? 1.0 : 0.0)) > 1e-6) return false;
        }
    return true;
}

int rank_gauss(Matrix A) { return gaussian_elimination(A); }
bool is_nonsingular(const Matrix& A) { return det_gauss(A) != 0; }

}; // struct InverseAndRank

#endif // LINEAR_ALGEBRA_D_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_D_MAIN
int main() {
    cout << "=== Обратная (Гаусс) ===" << endl;
    InverseAndRank::DMatrix A = {{2,1,0},{1,3,1},{0,1,2}};
    auto Ainv = InverseAndRank().inverse_gauss(A);
    for (auto& row : Ainv) { for (auto x : row) cout << x << " "; cout << endl; }
    cout << "Check: " << (InverseAndRank().check_inverse(A, Ainv) ? "OK" : "FAIL") << endl;

    cout << "\n=== Ранг ===" << endl;
    InverseAndRank::Matrix B = {{1,2,3},{4,5,6},{7,8,9}};
    cout << "rank = " << InverseAndRank().rank_gauss(B) << endl;

    cout << "\n=== Невырожденность ===" << endl;
    InverseAndRank::Matrix A2 = {{2,1,0},{1,3,1},{0,1,2}};
    cout << "det = " << InverseAndRank().det_gauss(A2) << " => "
         << (InverseAndRank().is_nonsingular(A2) ? "невырожденна" : "вырожденна") << endl;

    return 0;
}
#endif
