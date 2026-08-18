#ifndef LINEAR_ALGEBRA_F_CPP
#define LINEAR_ALGEBRA_F_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

#ifndef INSIDE_LINEAR_ALGEBRA_F
#define INSIDE_LINEAR_ALGEBRA_F
#include "../e/e.cpp"
#endif

struct VectorSpaces : LinearSystems {

using DMatrix = vector<vector<double>>;

int rank_of_vectors(const vector<vector<double>>& vectors) {
    if (vectors.empty()) return 0;
    int m = vectors[0].size();
    DMatrix mat(m, vector<double>(vectors.size()));
    for (int j = 0; j < (int)vectors.size(); j++)
        for (int i = 0; i < m; i++) mat[i][j] = vectors[j][i];
    return gaussian_rank(mat);
}

bool is_linearly_independent(const vector<vector<double>>& vectors) {
    return rank_of_vectors(vectors) == (int)vectors.size();
}

bool is_basis(const vector<vector<double>>& vectors, int expected_dim) {
    return (int)vectors.size() == expected_dim && is_linearly_independent(vectors);
}

vector<double> coordinates_in_basis(const vector<double>& v,
                                     const vector<vector<double>>& basis) {
    int n = v.size();
    DMatrix E(n, vector<double>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) E[i][j] = basis[j][i];
    DMatrix Einv = inverse_gauss(E);
    if (Einv.empty()) return {};
    vector<double> x(n, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) x[i] += Einv[i][j] * v[j];
    return x;
}

DMatrix transition_matrix(const vector<vector<double>>& from_basis,
                           const vector<vector<double>>& to_basis) {
    int n = from_basis.size();
    DMatrix P(n, vector<double>(n));
    for (int j = 0; j < n; j++) {
        auto coords = coordinates_in_basis(from_basis[j], to_basis);
        for (int i = 0; i < n; i++) P[i][j] = coords[i];
    }
    return P;
}

double dot_product(const vector<double>& u, const vector<double>& v) {
    double r = 0;
    for (int i = 0; i < (int)u.size(); i++) r += u[i] * v[i];
    return r;
}

double vec_norm(const vector<double>& v) { return sqrt(dot_product(v, v)); }

bool is_orthogonal(const vector<double>& u, const vector<double>& v) {
    return abs(dot_product(u, v)) < 1e-12;
}

vector<double> project(const vector<double>& v, const vector<double>& u) {
    double duu = dot_product(u, u);
    if (abs(duu) < 1e-12) return vector<double>(v.size(), 0);
    double s = dot_product(v, u) / duu;
    vector<double> p(v.size());
    for (int i = 0; i < (int)v.size(); i++) p[i] = s * u[i];
    return p;
}

double angle_between(const vector<double>& u, const vector<double>& v) {
    double c = dot_product(u, v) / (vec_norm(u) * vec_norm(v));
    return acos(max(-1.0, min(1.0, c)));
}

}; // struct VectorSpaces

#endif // LINEAR_ALGEBRA_F_CPP

// =============================================================
#ifdef LINEAR_ALGEBRA_F_MAIN
int main() {
    VectorSpaces vs;
    cout << "=== Линейная зависимость ===" << endl;
    vector<vector<double>> v1 = {{1,0},{0,1},{1,1}};
    cout << "rank({(1,0),(0,1),(1,1)}) = " << vs.rank_of_vectors(v1) << endl;

    cout << "\n=== Координаты ===" << endl;
    vector<double> v = {3,5};
    vector<vector<double>> basis = {{1,1},{1,-1}};
    auto coords = vs.coordinates_in_basis(v, basis);
    cout << "(3,5) в базисе: "; for (auto x : coords) cout << x << " "; cout << endl;

    cout << "\n=== Скалярное произведение ===" << endl;
    cout << "<(1,2,3),(4,5,6)> = " << vs.dot_product({1,2,3},{4,5,6}) << endl;

    cout << "\n=== Проекция ===" << endl;
    auto proj = vs.project({3,4}, {1,0});
    cout << "proj_(1,0)(3,4) = (" << proj[0] << "," << proj[1] << ")" << endl;

    cout << "\n=== Ортогональность ===" << endl;
    cout << "(1,0) ⊥ (0,1)? " << vs.is_orthogonal({1,0},{0,1}) << endl;
    cout << "(1,1) ⊥ (1,-1)? " << vs.is_orthogonal({1,1},{1,-1}) << endl;

    return 0;
}
#endif
