#ifndef LINEAR_ALGEBRA_H_CPP
#define LINEAR_ALGEBRA_H_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_H
#define INSIDE_LINEAR_ALGEBRA_H
#include "../g/g.cpp"
#endif

struct BilinearForms : LinearOperators {

using DMatrix = vector<vector<double>>;

// --- B.2. Ранг билинейной формы ---
int bilinear_rank(const DMatrix& B) { return gaussian_rank(B); }

// --- B.3. Симметризация ---
DMatrix symmetrize(const DMatrix& B) {
    int n = B.size();
    DMatrix S(n, vector<double>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            S[i][j] = (B[i][j] + B[j][i]) / 2.0;
    return S;
}

// --- C.1. Значение квадратичной формы ---
double quadratic_form(const DMatrix& A, const vector<double>& x) {
    int n = A.size();
    double r = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) r += x[i] * A[i][j] * x[j];
    return r;
}

// --- C.2. Индексы инерции ---
pair<int,int> inertia_indices(DMatrix A) {
    int n = A.size(), pos = 0, neg = 0;
    for (int i = 0; i < n; i++) {
        int pivot = i;
        for (int j = i + 1; j < n; j++)
            if (abs(A[j][i]) > abs(A[pivot][i])) pivot = j;
        if (abs(A[pivot][i]) < 1e-12) continue;
        if (pivot != i) swap(A[i], A[pivot]);
        for (int j = i + 1; j < n; j++) {
            double f = A[j][i] / A[i][i];
            for (int k = i; k < n; k++) A[j][k] -= A[i][k] * f;
            for (int k = i; k < n; k++) A[k][j] -= A[k][i] * f;
        }
    }
    for (int i = 0; i < n; i++) {
        if (abs(A[i][i]) > 1e-12) {
            if (A[i][i] > 0) pos++; else neg++;
        }
    }
    return {pos, neg};
}

// --- C.3. Положительная определённость ---
bool is_positive_definite(DMatrix A) {
    int n = A.size();
    for (int i = 0; i < n; i++) {
        double det = 1;
        for (int j = 0; j <= i; j++) det *= A[j][j];
        if (det <= 1e-12) return false;
        for (int j = i + 1; j < n; j++) {
            if (abs(A[j][i]) < 1e-12) continue;
            double f = A[j][i] / A[i][i];
            for (int k = i; k < n; k++) A[j][k] -= A[i][k] * f;
            for (int k = i; k < n; k++) A[k][j] -= A[k][i] * f;
        }
    }
    return true;
}

// --- D.1. Матрица Грама ---
DMatrix gram_matrix(const vector<vector<double>>& vectors) {
    int k = vectors.size();
    DMatrix G(k, vector<double>(k, 0));
    for (int i = 0; i < k; i++)
        for (int j = 0; j < k; j++)
            for (int d = 0; d < (int)vectors[0].size(); d++)
                G[i][j] += vectors[i][d] * vectors[j][d];
    return G;
}

// --- D.2. Объём через Грам ---
double gram_volume(const vector<vector<double>>& vectors) {
    DMatrix G = gram_matrix(vectors);
    // Определитель через Гаусс (double версия)
    int n = G.size();
    double det = 1;
    for (int col = 0; col < n; col++) {
        int pivot = -1;
        for (int row = col; row < n; row++)
            if (abs(G[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) return 0;
        if (pivot != col) { swap(G[col], G[pivot]); det = -det; }
        det *= G[col][col];
        for (int row = col + 1; row < n; row++) {
            if (abs(G[row][col]) < 1e-12) continue;
            double f = G[row][col] / G[col][col];
            for (int j = col; j < n; j++) G[row][j] -= G[col][j] * f;
        }
    }
    return sqrt(abs(det));
}

}; // struct BilinearForms

#endif // LINEAR_ALGEBRA_H_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_H_MAIN
int main() {
    BilinearForms bf;
    cout << "=== Квадратичная форма ===" << endl;
    vector<vector<double>> A = {{2,1},{1,2}};
    vector<double> x = {1,1};
    cout << "Q(1,1) = " << bf.quadratic_form(A, x) << endl;

    cout << "\n=== Индексы инерции ===" << endl;
    auto [pos, neg] = bf.inertia_indices(A);
    cout << "pos=" << pos << " neg=" << neg << endl;

    cout << "\n=== Положительная определённость ===" << endl;
    cout << "A: " << (bf.is_positive_definite(A) ? "да" : "нет") << endl;
    vector<vector<double>> B = {{1,2},{2,1}};
    cout << "B: " << (bf.is_positive_definite(B) ? "да" : "нет") << endl;

    cout << "\n=== Грамматрица ===" << endl;
    vector<vector<double>> v = {{1,0},{0,1},{1,1}};
    auto G = bf.gram_matrix(v);
    for (auto& row : G) { for (auto x : row) cout << x << " "; cout << endl; }
    cout << "volume = " << bf.gram_volume(v) << endl;

    return 0;
}
#endif
