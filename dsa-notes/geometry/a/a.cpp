#ifndef GEOMETRY_A_CPP
#define GEOMETRY_A_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

const double EPS = 1e-9;

struct Point {
    double x, y;
    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}
    Point operator+(const Point& p) const { return {x + p.x, y + p.y}; }
    Point operator-(const Point& p) const { return {x - p.x, y - p.y}; }
    Point operator*(double k) const { return {x * k, y * k}; }
    bool operator==(const Point& p) const { return fabs(x - p.x) < EPS && fabs(y - p.y) < EPS; }
    bool operator<(const Point& p) const {
        return fabs(x - p.x) > EPS ? x < p.x : y < p.y;
    }
};

struct GeometryBasics {

// =============================================================
// 1. ТОЧНОСТЬ
// =============================================================

int sign(double x) {
    if (x > EPS) return 1;
    if (x < -EPS) return -1;
    return 0;
}

// =============================================================
// 2. ПРЕДИКАТЫ ОРИЕНТАЦИИ
// =============================================================

int orient(const Point& a, const Point& b, const Point& c) {
    double det = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (fabs(det) < EPS) return 0;
    return (det > 0) ? 1 : -1;
}

// =============================================================
// 3. РАССТОЯНИЯ
// =============================================================

double dist(const Point& a, const Point& b) {
    return hypot(a.x - b.x, a.y - b.y);
}

double manhattan(const Point& a, const Point& b) {
    return fabs(a.x - b.x) + fabs(a.y - b.y);
}

double chebyshev(const Point& a, const Point& b) {
    return max(fabs(a.x - b.x), fabs(a.y - b.y));
}

double dist_point_line(const Point& p, const Point& a, const Point& b) {
    double dx = b.x - a.x, dy = b.y - a.y;
    return fabs(dx * (a.y - p.y) - dy * (a.x - p.x)) / hypot(dx, dy);
}

double dist_point_segment(const Point& p, const Point& a, const Point& b) {
    double dx = b.x - a.x, dy = b.y - a.y;
    double len2 = dx * dx + dy * dy;
    if (len2 < EPS) return dist(p, a);
    double t = max(0.0, min(1.0, ((p.x - a.x) * dx + (p.y - a.y) * dy) / len2));
    Point proj = {a.x + t * dx, a.y + t * dy};
    return dist(p, proj);
}

// =============================================================
// 4. ПРЕДИКАТЫ НА ОТРЕЗКАХ И ТОЧКАХ
// =============================================================

bool on_segment(const Point& p, const Point& a, const Point& b) {
    return orient(a, b, p) == 0 &&
           min(a.x, b.x) <= p.x + EPS && p.x <= max(a.x, b.x) + EPS &&
           min(a.y, b.y) <= p.y + EPS && p.y <= max(a.y, b.y) + EPS;
}

bool ccw(const Point& a, const Point& b, const Point& c) {
    return orient(a, b, c) > 0;
}

// =============================================================
// 5. КООРДИНАТЫ
// =============================================================

Point to_polar(const Point& p) {
    return {hypot(p.x, p.y), atan2(p.y, p.x)};
}

Point from_polar(double r, double theta) {
    return {r * cos(theta), r * sin(theta)};
}

Point rotate(const Point& p, const Point& center, double theta) {
    double dx = p.x - center.x, dy = p.y - center.y;
    return {center.x + dx * cos(theta) - dy * sin(theta),
            center.y + dx * sin(theta) + dy * cos(theta)};
}

Point rotate_complex(const Point& p, double theta) {
    double c = cos(theta), s = sin(theta);
    return {p.x * c - p.y * s, p.x * s + p.y * c};
}

// Треугольные (barycentric) координаты точки P относительно треугольника (A, B, C).
vector<double> barycentric(const Point& p, const Point& a, const Point& b, const Point& c) {
    double det = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
    if (fabs(det) < EPS) return {-1, -1, -1};
    double l1 = ((b.x * c.y - c.x * b.y) + (b.y - c.y) * p.x + (c.x - b.x) * p.y) / det;
    double l2 = ((c.x * a.y - a.x * c.y) + (c.y - a.y) * p.x + (a.x - c.x) * p.y) / det;
    double l3 = 1.0 - l1 - l2;
    return {l1, l2, l3};
}

bool point_in_triangle(const Point& p, const Point& a, const Point& b, const Point& c) {
    vector<double> bary = barycentric(p, a, b, c);
    return bary[0] >= -EPS && bary[1] >= -EPS && bary[2] >= -EPS;
}

// =============================================================
// 6. ПЛОЩАДИ И ОБЪЁМЫ
// =============================================================

double triangle_area(const Point& a, const Point& b, const Point& c) {
    return fabs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) / 2.0;
}

double polygon_area(const vector<Point>& poly) {
    int n = poly.size();
    double area = 0;
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        area += poly[i].x * poly[j].y - poly[j].x * poly[i].y;
    }
    return fabs(area) / 2.0;
}

double polygon_perimeter(const vector<Point>& poly) {
    int n = poly.size();
    double per = 0;
    for (int i = 0; i < n; i++)
        per += dist(poly[i], poly[(i + 1) % n]);
    return per;
}

double volume_sphere(double r) {
    return 4.0 / 3.0 * M_PI * r * r * r;
}

double volume_cylinder(double r, double h) {
    return M_PI * r * r * h;
}

double volume_cone(double r, double h) {
    return M_PI * r * r * h / 3.0;
}

double volume_ellipsoid(double a, double b, double c) {
    return 4.0 / 3.0 * M_PI * a * b * c;
}

// =============================================================
// 7. КОМПЕНСИРОВАННАЯ СУММАЦИЯ
// =============================================================

double kahan_sum(const vector<double>& v) {
    double sum = 0.0, comp = 0.0;
    for (double x : v) {
        double y = x - comp;
        double t = sum + y;
        comp = (t - sum) - y;
        sum = t;
    }
    return sum;
}

}; // struct GeometryBasics

#endif // GEOMETRY_A_CPP
