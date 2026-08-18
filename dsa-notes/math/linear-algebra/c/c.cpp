#ifndef LINEAR_ALGEBRA_C_CPP
#define LINEAR_ALGEBRA_C_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

// =============================================================
// C. ОПРЕДЕЛИТЕЛИ И ОБЪЁМ СИСТЕМЫ ВЕКТОРОВ
// =============================================================
// Структура md: A. Определение определителя
//               → B. Свойства определителя
//               → C. Вычисление определителей (Гаусс, Лаплас)
//               → D. Миноры и алгебраические дополнения
//               → E. Объём системы векторов
//
// Determinants — наследует MatrixOperations (b.cpp).

#ifndef INSIDE_LINEAR_ALGEBRA_C
#define INSIDE_LINEAR_ALGEBRA_C
#include "../b/b.cpp"
#endif

struct Determinants : MatrixOperations {

// =============================================================
// C. ВЫЧИСЛЕНИЕ ОПРЕДЕЛИТЕЛЕЙ
// =============================================================

// --- C.1. Определитель через метод Гаусса ---
// Приведение к верхнетреугольному виду; определитель = произведение диагонали.
// O(n³) время.
long long det_gauss(Matrix A) {
    int n = A.size();
    long long det = 1;
    for (int col = 0; col < n; col++) {
        // Поиск ведущего элемента
        int pivot = -1;
        for (int row = col; row < n; row++) {
            if (A[row][col] != 0) { pivot = row; break; }
        }
        if (pivot == -1) return 0;  // вырожденная матрица

        // Перестановка строк
        if (pivot != col) {
            swap(A[col], A[pivot]);
            det = -det;  // изменение знака
        }

        det *= A[col][col];

        // Обнуление столбца
        for (int row = col + 1; row < n; row++) {
            if (A[row][col] == 0) continue;
            long long factor = A[row][col];
            for (int j = col; j < n; j++)
                A[row][j] = A[row][j] * A[col][col] - A[col][j] * factor;
        }
    }
    return det;
}

// --- C.2. Определитель через разложение Лапласа (рекурсивно) ---
// O(n!) время — только для малых n.
long long det_laplace(const Matrix& A) {
    int n = A.size();
    if (n == 1) return A[0][0];
    if (n == 2) return A[0][0] * A[1][1] - A[0][1] * A[1][0];

    long long det = 0;
    for (int j = 0; j < n; j++) {
        // Минор M[0][j]
        Matrix minor(n - 1, vector<long long>(n - 1));
        for (int i = 1; i < n; i++) {
            int col_idx = 0;
            for (int jj = 0; jj < n; jj++) {
                if (jj == j) continue;
                minor[i - 1][col_idx++] = A[i][jj];
            }
        }
        long long sign = (j % 2 == 0) ? 1 : -1;
        det += sign * A[0][j] * det_laplace(minor);
    }
    return det;
}

// =============================================================
// D. ОБЪЁМ СИСТЕМЫ ВЕКТОРОВ
// =============================================================

// --- D.1. Ориентированный объём n векторов в R^n ---
// Через определитель матрицы из векторов-столбцов.
// O(n³) время.
long long oriented_volume(const vector<vector<long long>>& vectors) {
    int n = vectors.size();
    Matrix mat(n, vector<long long>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            mat[i][j] = vectors[i][j];
    return det_gauss(mat);
}

// --- D.2. Площадь параллелограмма (2 вектора в R²) ---
// |det([v1|v2])|
long long parallelogram_area(long long x1, long long y1,
                              long long x2, long long y2) {
    return abs(x1 * y2 - x2 * y1);
}

// --- D.3. Объём параллелепипеда (3 вектора в R³) ---
// |(u × v) · w| = |det([u|v|w])|
long long parallelepiped_volume(long long ux, long long uy, long long uz,
                                 long long vx, long long vy, long long vz,
                                 long long wx, long long wy, long long wz) {
    return abs(ux * (vy * wz - vz * wy) -
               uy * (vx * wz - vz * wx) +
               uz * (vx * wy - vy * wx));
}

// --- D.4. Векторное произведение (u × v) в R³ ---
tuple<long long, long long, long long> cross_product(
    long long ux, long long uy, long long uz,
    long long vx, long long vy, long long vz) {
    return {uy * vz - uz * vy,
            uz * vx - ux * vz,
            ux * vy - uy * vx};
}

// --- D.5. Определитель через матрицу Грама ---
// Для k векторов в R^n: объём = √det(G) где G[i][j] = ⟨vᵢ,vⱼ⟩.
double gram_determinant_volume(const vector<vector<double>>& vectors) {
    int k = vectors.size();
    int n = vectors[0].size();
    vector<vector<double>> G(k, vector<double>(k, 0));
    for (int i = 0; i < k; i++)
        for (int j = 0; j < k; j++)
            for (int d = 0; d < n; d++)
                G[i][j] += vectors[i][d] * vectors[j][d];

    // Определитель через Гаусс (double)
    double det = 1;
    for (int col = 0; col < k; col++) {
        int pivot = -1;
        for (int row = col; row < k; row++)
            if (abs(G[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) return 0;
        if (pivot != col) { swap(G[col], G[pivot]); det = -det; }
        det *= G[col][col];
        for (int row = col + 1; row < k; row++) {
            if (abs(G[row][col]) < 1e-12) continue;
            double factor = G[row][col] / G[col][col];
            for (int j = col; j < k; j++)
                G[row][j] -= G[col][j] * factor;
        }
    }
    return sqrt(abs(det));
}

}; // struct Determinants

// =============================================================
// MAIN
// =============================================================
#ifdef LINEAR_ALGEBRA_C_MAIN
int main() {
    Determinants det;

    cout << "=== Определитель через Гаусс ===" << endl;
    MatrixOperations::Matrix A = {{1,2,3},{4,5,6},{7,8,9}};
    cout << "det(3x3) = " << det.det_gauss(A) << endl;

    MatrixOperations::Matrix B = {{2,1,3},{1,0,2},{3,2,1}};
    cout << "det(3x3) = " << det.det_gauss(B) << endl;

    cout << "\n=== Определитель через Лаплас ===" << endl;
    MatrixOperations::Matrix C = {{1,2},{3,4}};
    cout << "det(2x2) = " << det.det_laplace(C) << endl;

    cout << "\n=== Объём системы векторов ===" << endl;
    cout << "Площадь (1,0)x(0,1) = " << det.parallelogram_area(1,0,0,1) << endl;
    cout << "Объём (1,0,0)x(0,1,0)x(0,0,1) = "
         << det.parallelepiped_volume(1,0,0, 0,1,0, 0,0,1) << endl;

    auto [cx, cy, cz] = det.cross_product(1,0,0, 0,1,0);
    cout << "Cross (1,0,0)x(0,1,0) = (" << cx << "," << cy << "," << cz << ")" << endl;

    cout << "\n=== Матрица Грама ===" << endl;
    vector<vector<double>> v = {{1,0,0},{0,1,0},{1,1,0}};
    cout << "Volume (3 вектора в R^3) = " << det.gram_determinant_volume(v) << endl;
    vector<vector<double>> v2 = {{1,0},{0,1}};
    cout << "Volume (2 вектора в R^2) = " << det.gram_determinant_volume(v2) << endl;

    return 0;
}
#endif

#endif // LINEAR_ALGEBRA_C_CPP
