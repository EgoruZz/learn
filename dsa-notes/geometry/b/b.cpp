#ifndef GEOMETRY_B_CPP
#define GEOMETRY_B_CPP

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#ifndef INSIDE_GEOMETRY_B
#define INSIDE_GEOMETRY_B
#include "../a/a.cpp"
#endif

struct VectorAlgebra : GeometryBasics {

// =============================================================
// 1. РАССТОЯНИЯ
// =============================================================

double dist_euclidean(const vector<double>& a, const vector<double>& b) {
    double r = 0;
    for (int i = 0; i < (int)a.size(); i++)
        r += (a[i] - b[i]) * (a[i] - b[i]);
    return sqrt(r);
}

double dist_manhattan(const vector<double>& a, const vector<double>& b) {
    double r = 0;
    for (int i = 0; i < (int)a.size(); i++)
        r += fabs(a[i] - b[i]);
    return r;
}

double dist_chebyshev(const vector<double>& a, const vector<double>& b) {
    double r = 0;
    for (int i = 0; i < (int)a.size(); i++)
        r = max(r, fabs(a[i] - b[i]));
    return r;
}

double dist_minkowski(const vector<double>& a, const vector<double>& b, double p) {
    double r = 0;
    for (int i = 0; i < (int)a.size(); i++)
        r += pow(fabs(a[i] - b[i]), p);
    return pow(r, 1.0 / p);
}

// =============================================================
// 2. СКАЛЯРНОЕ ПРОИЗВЕДЕНИЕ
// =============================================================

double dot_product(const vector<double>& u, const vector<double>& v) {
    double r = 0;
    for (int i = 0; i < (int)u.size(); i++) r += u[i] * v[i];
    return r;
}

double vec_length(const vector<double>& v) { return sqrt(dot_product(v, v)); }

double angle_between(const vector<double>& u, const vector<double>& v) {
    return acos(max(-1.0, min(1.0, dot_product(u, v) / (vec_length(u) * vec_length(v)))));
}

bool is_orthogonal(const vector<double>& u, const vector<double>& v) {
    return fabs(dot_product(u, v)) < EPS;
}

vector<double> project(const vector<double>& v, const vector<double>& u) {
    double duu = dot_product(u, u);
    if (fabs(duu) < EPS) return vector<double>(v.size(), 0);
    double s = dot_product(v, u) / duu;
    vector<double> p(v.size());
    for (int i = 0; i < (int)v.size(); i++) p[i] = s * u[i];
    return p;
}

// =============================================================
// 3. ВЕКТОРНОЕ ПРОИЗВЕДЕНИЕ (ℝ³)
// =============================================================

struct Vec3 { double x, y, z; };

Vec3 cross_product(const Vec3& u, const Vec3& v) {
    return {u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x};
}

double cross_product_2d(double ax, double ay, double bx, double by) {
    return ax * by - ay * bx;
}

// =============================================================
// 4. СМЕШАННОЕ ПРОИЗВЕДЕНИЕ (ℝ³)
// =============================================================

double triple_product(const Vec3& u, const Vec3& v, const Vec3& w) {
    return (u.y*v.z - u.z*v.y) * w.x +
           (u.z*v.x - u.x*v.z) * w.y +
           (u.x*v.y - u.y*v.x) * w.z;
}

// =============================================================
// 5. ПРИМЕНЕНИЯ
// =============================================================

double triangle_area_via_cross(const Point& a, const Point& b, const Point& c) {
    return fabs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) / 2.0;
}

double tetra_volume(const Point& a, const Point& b, const Point& c, const Point& d) {
    Vec3 ab = {b.x - a.x, b.y - a.y, 0};
    Vec3 ac = {c.x - a.x, c.y - a.y, 0};
    Vec3 ad = {d.x - a.x, d.y - a.y, 0};
    return fabs(triple_product(ab, ac, ad)) / 6.0;
}

bool collinear_check(const Point& a, const Point& b, const Point& c) {
    return orient(a, b, c) == 0;
}

bool coplanar_check(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) {
    Vec3 ab = {b.x - a.x, b.y - a.y, b.z - a.z};
    Vec3 ac = {c.x - a.x, c.y - a.y, c.z - a.z};
    Vec3 ad = {d.x - a.x, d.y - a.y, d.z - a.z};
    return fabs(triple_product(ab, ac, ad)) < EPS;
}

// =============================================================
// 6. НОРМАЛИЗАЦИЯ
// =============================================================

vector<double> normalize_vec(const vector<double>& v) {
    double len = vec_length(v);
    if (len < EPS) return vector<double>(v.size(), 0);
    vector<double> result(v.size());
    for (int i = 0; i < (int)v.size(); i++) result[i] = v[i] / len;
    return result;
}

}; // struct VectorAlgebra

#endif // GEOMETRY_B_CPP
