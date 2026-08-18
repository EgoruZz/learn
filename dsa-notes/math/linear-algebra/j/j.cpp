#ifndef LINEAR_ALGEBRA_J_CPP
#define LINEAR_ALGEBRA_J_CPP

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <set>
#include <string>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_J
#define INSIDE_LINEAR_ALGEBRA_J
#include "../i/i.cpp"
#endif

struct JordanForm : ComplexAndPolynomials {

using DMatrix = vector<vector<double>>;

double det_matrix(DMatrix A) {
    int n = A.size();
    double det = 1;
    for (int col = 0; col < n; col++) {
        int pivot = -1;
        for (int row = col; row < n; row++)
            if (abs(A[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) return 0;
        if (pivot != col) { swap(A[col], A[pivot]); det = -det; }
        det *= A[col][col];
        for (int row = col + 1; row < n; row++) {
            if (abs(A[row][col]) < 1e-12) continue;
            double f = A[row][col] / A[col][col];
            for (int j = col; j < n; j++) A[row][j] -= A[col][j] * f;
        }
    }
    return det;
}

int rank_matrix(DMatrix A) {
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

vector<double> characteristic_polynomial(DMatrix A) {
    int n = A.size();
    if (n == 1) return {1, -A[0][0]};
    if (n == 2) return {A[0][0]*A[1][1]-A[0][1]*A[1][0], -(A[0][0]+A[1][1]), 1};
    // Общий случай: интерполяция
    vector<double> xs(n + 1), ys(n + 1);
    for (int i = 0; i <= n; i++) {
        xs[i] = i * 1.0;
        DMatrix M = A;
        for (int j = 0; j < n; j++) M[j][j] -= i;
        ys[i] = det_matrix(M);
    }
    DMatrix V(n + 1, vector<double>(n + 1));
    for (int i = 0; i <= n; i++)
        for (int j = 0; j <= n; j++)
            V[i][j] = pow(xs[i], j);
    DMatrix aug(n + 1, vector<double>(n + 2));
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= n; j++) aug[i][j] = V[i][j];
        aug[i][n + 1] = ys[i];
    }
    for (int col = 0; col <= n; col++) {
        int pivot = -1;
        for (int row = col; row <= n; row++)
            if (abs(aug[row][col]) > 1e-12) { pivot = row; break; }
        if (pivot == -1) continue;
        swap(aug[col], aug[pivot]);
        double lead = aug[col][col];
        for (int j = col; j <= n + 1; j++) aug[col][j] /= lead;
        for (int row = 0; row <= n; row++) {
            if (row == col || abs(aug[row][col]) < 1e-12) continue;
            double f = aug[row][col];
            for (int j = col; j <= n + 1; j++) aug[row][j] -= aug[col][j] * f;
        }
    }
    vector<double> result(n + 1);
    for (int i = 0; i <= n; i++) result[i] = aug[i][n + 1];
    return result;
}

vector<complex<double>> eigenvalues(const DMatrix& A) {
    int n = A.size();
    auto poly = characteristic_polynomial(A);
    int deg = n;
    vector<complex<double>> roots;
    vector<double> cur_poly = poly;
    for (int k = 0; k < deg; k++) {
        int cur_deg = cur_poly.size() - 1;
        if (cur_deg <= 0) break;
        // Ищем один корень текущего полинома
        complex<double> best_z;
        double best_val = 1e18;
        for (double r = 0.5; r <= 10.0; r *= 2) {
            for (double ang = 0; ang < 2 * M_PI; ang += M_PI / 6) {
                complex<double> z(r * cos(ang), r * sin(ang));
                for (int iter = 0; iter < 200; iter++) {
                    complex<double> val = 0;
                    for (int i = cur_deg; i >= 0; i--) val = val * z + cur_poly[i];
                    if (abs(val) < 1e-9) break;
                    complex<double> dval = 0;
                    for (int i = 1; i <= cur_deg; i++) dval = dval * z + i * cur_poly[i];
                    if (abs(dval) < 1e-12) break;
                    z = z - val / dval;
                }
                complex<double> val = 0;
                for (int i = cur_deg; i >= 0; i--) val = val * z + cur_poly[i];
                if (abs(val) < best_val) { best_val = abs(val); best_z = z; }
            }
        }
        roots.push_back(best_z);
        // Делим полином на (x - root): синтетическое деление
        if (cur_deg > 0) {
            vector<double> new_poly(cur_deg);
            new_poly[0] = cur_poly[0];
            for (int i = 1; i < cur_deg; i++)
                new_poly[i] = cur_poly[i] + best_z.real() * new_poly[i-1];
            cur_poly = new_poly;
        }
    }
    return roots;
}

struct JNFResult {
    vector<complex<double>> eigenvalues;
    vector<int> block_sizes;
    vector<complex<double>> lambdas;
};

bool is_diagonalizable(const DMatrix& A) {
    auto eigen = eigenvalues(A);
    int n = A.size();
    set<string> vals;
    for (auto& v : eigen) vals.insert(to_string(real(v)) + "_" + to_string(imag(v)));
    return (int)vals.size() == n;
}

JNFResult determine_jnf(const DMatrix& A) {
    int n = A.size();
    auto eigen = eigenvalues(A);
    JNFResult result;
    result.eigenvalues = eigen;

    if (n == 2) {
        if (abs(eigen[0] - eigen[1]) > 1e-9) {
            result.block_sizes = {1, 1};
            result.lambdas = eigen;
        } else {
            DMatrix M = A;
            M[0][0] -= real(eigen[0]); M[1][1] -= real(eigen[0]);
            result.block_sizes = (abs(det_matrix(M)) < 1e-9) ? vector<int>{2} : vector<int>{1, 1};
            result.lambdas = {eigen[0]};
        }
    } else {
        // Упрощённо: определяем через количество различных корней
        vector<complex<double>> unique_vals;
        for (auto& v : eigen) {
            bool found = false;
            for (auto& u : unique_vals)
                if (abs(v - u) < 1e-9) { found = true; break; }
            if (!found) unique_vals.push_back(v);
        }
        result.block_sizes.assign(unique_vals.size(), 1);
        result.lambdas = unique_vals;
    }
    return result;
}

}; // struct JordanForm

#endif // LINEAR_ALGEBRA_J_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_J_MAIN
int main() {
    JordanForm jf;
    cout << "=== Характеристический многочлен ===" << endl;
    vector<vector<double>> A = {{2,1},{1,2}};
    auto poly = jf.characteristic_polynomial(A);
    // poly[i] = коэффициент при x^i; печатаем от x^n к x^0
    cout << "χ(x) = ";
    for (int i = (int)poly.size() - 1; i >= 0; i--) {
        if (i < (int)poly.size() - 1) cout << (poly[i] >= 0 ? " + " : " - ");
        else if (poly[i] < 0) cout << "-";
        double v = fabs(poly[i]);
        if (v != 1.0 || i == 0) cout << v;
        if (i > 0) cout << "x" << (i > 1 ? "^" + to_string(i) : "");
    }
    cout << endl;

    cout << "\n=== Собственные значения ===" << endl;
    auto eigen = jf.eigenvalues(A);
    for (auto& v : eigen) cout << v << " "; cout << endl;

    cout << "\n=== Диагонализуемость ===" << endl;
    cout << "A = {{2,1},{1,2}}: " << (jf.is_diagonalizable(A) ? "да" : "нет") << endl;

    cout << "\n=== ЖНФ ===" << endl;
    auto jnf = jf.determine_jnf(A);
    cout << "Клетки: "; for (int s : jnf.block_sizes) cout << s << " "; cout << endl;

    vector<vector<double>> B = {{3,1},{0,3}};
    auto jnf2 = jf.determine_jnf(B);
    cout << "B={{3,1},{0,3}}: "; for (int s : jnf2.block_sizes) cout << s << " "; cout << endl;

    return 0;
}
#endif
