#ifndef GEOMETRY_C_CPP
#define GEOMETRY_C_CPP

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#ifndef INSIDE_GEOMETRY_C
#define INSIDE_GEOMETRY_C
#include "../b/b.cpp"
#endif

struct AnalyticGeometry2D : VectorAlgebra {

// =============================================================
// 1. ПРЯМАЯ НА ПЛОСКОСТИ
// =============================================================

double dist_point_line_abc(double A, double B, double C, const Point& p) {
    return fabs(A * p.x + B * p.y + C) / hypot(A, B);
}

pair<bool, Point> intersect_lines(double A1, double B1, double C1,
                                   double A2, double B2, double C2) {
    double det = A1 * B2 - A2 * B1;
    if (fabs(det) < EPS) return {false, {0, 0}};
    double x = (B1 * C2 - B2 * C1) / det;
    double y = (A2 * C1 - A1 * C2) / det;
    return {true, {x, y}};
}

tuple<double,double,double> line_through_points(const Point& a, const Point& b) {
    double A = a.y - b.y;
    double B = b.x - a.x;
    double C = a.x * b.y - b.x * a.y;
    return {A, B, C};
}

double angle_between_lines(double A1, double B1, double A2, double B2) {
    double num = fabs(A1 * A2 + B1 * B2);
    double den = hypot(A1, B1) * hypot(A2, B2);
    if (den < EPS) return 0;
    return acos(max(-1.0, min(1.0, num / den)));
}

// =============================================================
// 2. ОКРУЖНОСТЬ
// =============================================================

pair<Point, double> circumscribed_circle(const Point& a, const Point& b, const Point& c) {
    double D = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
    if (fabs(D) < EPS) return {{0, 0}, -1};
    double ux = ((a.x*a.x + a.y*a.y) * (b.y - c.y) +
                 (b.x*b.x + b.y*b.y) * (c.y - a.y) +
                 (c.x*c.x + c.y*c.y) * (a.y - b.y)) / D;
    double uy = ((a.x*a.x + a.y*a.y) * (c.x - b.x) +
                 (b.x*b.x + b.y*b.y) * (a.x - c.x) +
                 (c.x*c.x + c.y*c.y) * (b.x - a.x)) / D;
    return {{ux, uy}, dist({ux, uy}, a)};
}

pair<int, vector<Point>> intersect_circles(Point c1, double r1, Point c2, double r2) {
    double d = dist(c1, c2);
    if (d > r1 + r2 + EPS) return {0, {}};
    if (fabs(d - (r1 + r2)) < EPS) return {1, {c1 + (c2 - c1) * (r1 / d)}};
    if (d < fabs(r1 - r2) - EPS) return {0, {}};
    if (fabs(d - fabs(r1 - r2)) < EPS) return {1, {c1 + (c2 - c1) * (r1 / d)}};
    double a = (r1*r1 - r2*r2 + d*d) / (2 * d);
    double h = sqrt(max(0.0, r1*r1 - a*a));
    Point mid = c1 + (c2 - c1) * (a / d);
    Point perp = {(c2.y - c1.y) / d, -(c2.x - c1.x) / d};
    return {2, {mid + perp * h, mid - perp * h}};
}

pair<Point, double> inscribed_circle(const Point& a, const Point& b, const Point& c) {
    double A = dist(b, c), B = dist(a, c), C = dist(a, b);
    double P = A + B + C;
    if (fabs(P) < EPS) return {{0, 0}, -1};
    Point center = {(a.x * A + b.x * B + c.x * C) / P,
                    (a.y * A + b.y * B + c.y * C) / P};
    double r = fabs((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y)) / P;
    return {center, r};
}

pair<int, vector<Point>> tangent_from_point_to_circle(const Point& p, const Point& c, double r) {
    double d = dist(p, c);
    if (d < r - EPS) return {0, {}};
    if (fabs(d - r) < EPS) return {1, {p}};
    double a = (d * d - r * r);
    double h = r * sqrt(a) / d;
    double cx = (c.x - p.x) / d, cy = (c.y - p.y) / d;
    Point t1 = {p.x + (d * d - r * r) / d * cx - h * cy,
                p.y + (d * d - r * r) / d * cy + h * cx};
    Point t2 = {p.x + (d * d - r * r) / d * cx + h * cy,
                p.y + (d * d - r * r) / d * cy - h * cx};
    return {2, {t1, t2}};
}

// =============================================================
// 3. КРИВЫЕ ВТОРОГО ПОРЯДКА
// =============================================================

int conic_type(double A, double B, double C) {
    double disc = B * B - 4 * A * C;
    if (disc < -EPS) return -1;
    if (fabs(disc) < EPS) return 0;
    return 1;
}

double ellipse_eccentricity(double a, double b) {
    return sqrt(1 - (b * b) / (a * a));
}

double hyperbola_eccentricity(double a, double b) {
    return sqrt(1 + (b * b) / (a * a));
}

// =============================================================
// 4. ПРОВЕРКИ ПРЯМЫХ
// =============================================================

bool parallel_check(double A1, double B1, double A2, double B2) {
    return fabs(A1 * B2 - A2 * B1) < EPS;
}

bool perpendicular_check(double A1, double B1, double A2, double B2) {
    return fabs(A1 * A2 + B1 * B2) < EPS;
}

}; // struct AnalyticGeometry2D

#endif // GEOMETRY_C_CPP
