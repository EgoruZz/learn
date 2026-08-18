#ifndef LINEAR_ALGEBRA_B_CPP
#define LINEAR_ALGEBRA_B_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// B. МАТРИЦЫ И ОПЕРАЦИИ
// =============================================================
// Структура md: A. Определения и типы матриц
//               → B. Операции (сумма, умножение, транспонирование, след)
//               → C. Блочные матрицы, Кронекер
//               → D. Элементарные преобразования, метод Гаусса
//
// MatrixOperations — наследует AlgebraicBasics (a.cpp).
// Все методы параметризованы размерами m x n.

#ifndef INSIDE_LINEAR_ALGEBRA_B
#define INSIDE_LINEAR_ALGEBRA_B
#include "../a/a.cpp"
#endif

struct MatrixOperations : AlgebraicBasics {

// =============================================================
// ТИП МАТРИЦЫ
// =============================================================

using Matrix = vector<vector<long long>>;

// Создание нулевой матрицы m x n.
Matrix zeros(int m, int n) {
    return Matrix(m, vector<long long>(n, 0));
}

// Единичная матрица n x n.
Matrix identity(int n) {
    Matrix I = zeros(n, n);
    for (int i = 0; i < n; i++) I[i][i] = 1;
    return I;
}

// Диагональная матрица из вектора.
Matrix diag(const vector<long long>& d) {
    int n = d.size();
    Matrix D = zeros(n, n);
    for (int i = 0; i < n; i++) D[i][i] = d[i];
    return D;
}

// =============================================================
// B. ОПЕРАЦИИ НАД МАТРИЦАМИ
// =============================================================

// --- B.1. Сумма матриц ---
// O(m*n) время.
Matrix mat_add(const Matrix& A, const Matrix& B) {
    int m = A.size(), n = A[0].size();
    Matrix C = zeros(m, n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

// --- B.2. Вычитание матриц ---
Matrix mat_sub(const Matrix& A, const Matrix& B) {
    int m = A.size(), n = A[0].size();
    Matrix C = zeros(m, n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] - B[i][j];
    return C;
}

// --- B.3. Умножение на скаляр ---
Matrix mat_scale(const Matrix& A, long long alpha) {
    int m = A.size(), n = A[0].size();
    Matrix C = zeros(m, n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] * alpha;
    return C;
}

// --- B.4. Произведение матриц ---
// O(m*n*p) время.
Matrix mat_mul(const Matrix& A, const Matrix& B) {
    int m = A.size(), n = A[0].size(), p = B[0].size();
    Matrix C = zeros(m, p);
    for (int i = 0; i < m; i++)
        for (int k = 0; k < n; k++) {
            if (A[i][k] == 0) continue;
            for (int j = 0; j < p; j++)
                C[i][j] += A[i][k] * B[k][j];
        }
    return C;
}

// --- B.5. Произведение по модулю ---
Matrix mat_mul_mod(const Matrix& A, const Matrix& B, long long mod) {
    int m = A.size(), n = A[0].size(), p = B[0].size();
    Matrix C = zeros(m, p);
    for (int i = 0; i < m; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < p; j++)
                C[i][j] = (C[i][j] + A[i][k] * B[k][j]) % mod;
    return C;
}

// --- B.6. Транспонирование ---
Matrix transpose(const Matrix& A) {
    int m = A.size(), n = A[0].size();
    Matrix T = zeros(n, m);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            T[j][i] = A[i][j];
    return T;
}

// --- B.7. След ---
// O(n) время для квадратной матрицы.
long long trace(const Matrix& A) {
    long long tr = 0;
    int n = min(A.size(), A[0].size());
    for (int i = 0; i < n; i++) tr += A[i][i];
    return tr;
}

// --- B.8. Сравнение матриц ---
bool mat_equal(const Matrix& A, const Matrix& B) {
    if (A.size() != B.size() || A[0].size() != B[0].size()) return false;
    for (int i = 0; i < (int)A.size(); i++)
        for (int j = 0; j < (int)A[0].size(); j++)
            if (A[i][j] != B[i][j]) return false;
    return true;
}

// =============================================================
// C. КРОНЕКЕРОВСКОЕ ПРОИЗВЕДЕНИЕ
// =============================================================
// A (m1 x n1) ⊗ B (m2 x n2) = матрица (m1*m2 x n1*n2).
// O(m1*n1*m2*n2) время.
Matrix kronecker(const Matrix& A, const Matrix& B) {
    int m1 = A.size(), n1 = A[0].size();
    int m2 = B.size(), n2 = B[0].size();
    Matrix C = zeros(m1 * m2, n1 * n2);
    for (int i1 = 0; i1 < m1; i1++)
        for (int j1 = 0; j1 < n1; j1++)
            for (int i2 = 0; i2 < m2; i2++)
                for (int j2 = 0; j2 < n2; j2++)
                    C[i1 * m2 + i2][j1 * n2 + j2] = A[i1][j1] * B[i2][j2];
    return C;
}

// =============================================================
// D. ЭЛЕМЕНТАРНЫЕ ПРЕОБРАЗОВАНИЯ И ГАУСС
// =============================================================

// --- D.1. Приведение к ступенчатому виду ---
// Возвращает число ненулевых строк (ранг).
// O(m*n^2) время.
int gaussian_elimination(Matrix& A) {
    int m = A.size(), n = A[0].size();
    int rank = 0;
    for (int col = 0; col < n && rank < m; col++) {
        // Ищем ведущий элемент
        int pivot = -1;
        for (int row = rank; row < m; row++) {
            if (A[row][col] != 0) { pivot = row; break; }
        }
        if (pivot == -1) continue;

        // Перестановка строк
        swap(A[rank], A[pivot]);

        // Обнуление столбца
        for (int row = 0; row < m; row++) {
            if (row == rank || A[row][col] == 0) continue;
            long long factor = A[row][col];
            for (int j = col; j < n; j++)
                A[row][j] = A[row][j] * A[rank][col] - A[rank][j] * factor;
        }
        rank++;
    }
    return rank;
}

// --- D.2. Улучшенный ступенчатый вид (RREF) ---
// Приводит матрицу к улучшённому ступенчатому виду.
// O(m*n*min(m,n)) время.
void rref(Matrix& A) {
    int m = A.size(), n = A[0].size();
    int row = 0;
    for (int col = 0; col < n && row < m; col++) {
        int pivot = -1;
        for (int r = row; r < m; r++) {
            if (A[r][col] != 0) { pivot = r; break; }
        }
        if (pivot == -1) continue;

        swap(A[row], A[pivot]);

        // Нормализация строки (делим на ведущий элемент)
        long long lead = A[row][col];
        for (int j = col; j < n; j++)
            A[row][j] = A[row][j] / lead;  // целочисленное деление

        // Обнуление остальных строк
        for (int r = 0; r < m; r++) {
            if (r == row || A[r][col] == 0) continue;
            long long factor = A[r][col];
            for (int j = col; j < n; j++)
                A[r][j] -= A[row][j] * factor;
        }
        row++;
    }
}

// --- D.3. Вычисление ранга через Гаусс ---
int rank_via_gauss(Matrix A) {
    return gaussian_elimination(A);
}

}; // struct MatrixOperations

// =============================================================
// MAIN
// =============================================================
#ifdef LINEAR_ALGEBRA_B_MAIN
int main() {
    MatrixOperations mo;

    cout << "=== Матрицы ===" << endl;
    MatrixOperations::Matrix A = {{1,2},{3,4}};
    MatrixOperations::Matrix B = {{5,6},{7,8}};

    cout << "A + B:" << endl;
    auto C = mo.mat_add(A, B);
    for (auto& row : C) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "A * B:" << endl;
    auto D = mo.mat_mul(A, B);
    for (auto& row : D) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "tr(A) = " << mo.trace(A) << endl;

    cout << "\n=== Транспонирование ===" << endl;
    auto At = mo.transpose(A);
    for (auto& row : At) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "\n=== Кронекер ===" << endl;
    MatrixOperations::Matrix I2 = mo.identity(2);
    auto K = mo.kronecker(I2, A);
    for (auto& row : K) { for (auto x : row) cout << x << " "; cout << endl; }

    cout << "\n=== Гаусс (ранг) ===" << endl;
    MatrixOperations::Matrix M = {{1,2,3},{4,5,6},{7,8,9}};
    cout << "rank = " << mo.rank_via_gauss(M) << endl;

    return 0;
}
#endif

#endif // LINEAR_ALGEBRA_B_CPP
