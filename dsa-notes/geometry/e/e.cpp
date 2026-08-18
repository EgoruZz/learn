#ifndef GEOMETRY_E_CPP
#define GEOMETRY_E_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

#ifndef INSIDE_GEOMETRY_E
#define INSIDE_GEOMETRY_E
#include "../d/d.cpp"
#endif

struct ConvexPolygons : AnalyticGeometry3D {

// =============================================================
// 1. ПЕРИМЕТР И ПЛОЩАДЬ
// =============================================================

double perimeter(const vector<Point>& poly) {
    int n = poly.size();
    double per = 0;
    for (int i = 0; i < n; i++)
        per += dist(poly[i], poly[(i + 1) % n]);
    return per;
}

double area(const vector<Point>& poly) {
    return polygon_area(poly);
}

// =============================================================
// 3. ВЫПУКЛАЯ ОБОЛОЧКА
// =============================================================

vector<Point> jarvis(const vector<Point>& points) {
    int n = points.size();
    if (n <= 1) return points;
    int l = 0;
    for (int i = 1; i < n; i++)
        if (points[i] < points[l]) l = i;
    vector<Point> hull;
    int p = l;
    do {
        hull.push_back(points[p]);
        int q = (p + 1) % n;
        for (int i = 0; i < n; i++) {
            if (orient(points[p], points[i], points[q]) > 0)
                q = i;
        }
        p = q;
    } while (p != l);
    return hull;
}

vector<Point> graham(vector<Point> points) {
    int n = points.size();
    if (n <= 2) return points;
    int l = 0;
    for (int i = 1; i < n; i++)
        if (points[i] < points[l]) l = i;
    swap(points[0], points[l]);
    Point p0 = points[0];
    sort(points.begin() + 1, points.end(), [&](const Point& a, const Point& b) {
        int o = orient(p0, a, b);
        if (o != 0) return o > 0;
        return dist(p0, a) < dist(p0, b);
    });
    vector<Point> hull = {points[0], points[1]};
    for (int i = 2; i < n; i++) {
        while (hull.size() > 1 && orient(hull[hull.size()-2], hull.back(), points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }
    return hull;
}

vector<Point> andrew(vector<Point> points) {
    int n = points.size();
    if (n <= 2) return points;
    sort(points.begin(), points.end());
    vector<Point> hull;
    for (int i = 0; i < n; i++) {
        while (hull.size() >= 2 && orient(hull[hull.size()-2], hull.back(), points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }
    int lower = hull.size();
    for (int i = n - 2; i >= 0; i--) {
        while (hull.size() > lower && orient(hull[hull.size()-2], hull.back(), points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }
    hull.pop_back();
    return hull;
}

// =============================================================
// 4. ПРОВЕРКА ВЫПУКЛОСТИ
// =============================================================

bool is_convex(const vector<Point>& poly) {
    int n = poly.size();
    if (n <= 2) return true;
    int sign = 0;
    for (int i = 0; i < n; i++) {
        int o = orient(poly[i], poly[(i+1)%n], poly[(i+2)%n]);
        if (o != 0) {
            if (sign == 0) sign = o;
            else if (o != sign) return false;
        }
    }
    return true;
}

// =============================================================
// 5. POINT-IN-CONVEX-POLYGON O(log n)
// =============================================================

bool point_in_convex(const Point& p, const vector<Point>& poly) {
    int n = poly.size();
    if (n < 3) return false;
    if (orient(poly[0], poly[1], p) < 0) return false;
    if (orient(poly[0], poly[n-1], p) > 0) return false;
    int lo = 1, hi = n - 1;
    while (hi - lo > 1) {
        int mid = (lo + hi) / 2;
        if (orient(poly[0], poly[mid], p) >= 0) hi = mid;
        else lo = mid;
    }
    return orient(poly[lo], poly[hi], p) >= 0;
}

// =============================================================
// 6. MINKOWSKI SUM
// =============================================================

vector<Point> minkowski_sum(const vector<Point>& A, const vector<Point>& B) {
    vector<Point> result;
    int na = A.size(), nb = B.size();
    int ia = 0, ib = 0;
    result.push_back({A[0].x + B[0].x, A[0].y + B[0].y});
    for (int step = 0; step < na + nb - 1; step++) {
        Point ea = {A[(ia+1)%na].x - A[ia].x, A[(ia+1)%na].y - A[ia].y};
        Point eb = {B[(ib+1)%nb].x - B[ib].x, B[(ib+1)%nb].y - B[ib].y};
        if (orient({0,0}, ea, eb) >= 0) {
            result.push_back({result.back().x + eb.x, result.back().y + eb.y});
            ib = (ib + 1) % nb;
        } else {
            result.push_back({result.back().x + ea.x, result.back().y + ea.y});
            ia = (ia + 1) % na;
        }
    }
    return result;
}

// =============================================================
// 7. ВРАЩАЮЩИЕСЯ КАЛИПЕРЫ
// =============================================================

pair<double, pair<Point,Point>> diameter(const vector<Point>& hull) {
    int n = hull.size();
    if (n <= 1) return {0, {hull[0], hull[0]}};
    double max_d = 0;
    Point best_a, best_b;
    int j = 1;
    for (int i = 0; i < n; i++) {
        int ni = (i + 1) % n;
        while (true) {
            int nj = (j + 1) % n;
            double cross = (hull[ni].x - hull[i].x) * (hull[nj].y - hull[i].y)
                         - (hull[ni].y - hull[i].y) * (hull[nj].x - hull[i].x);
            if (cross > 0) { j = nj; }
            else break;
        }
        double d = dist(hull[i], hull[j]);
        if (d > max_d) { max_d = d; best_a = hull[i]; best_b = hull[j]; }
    }
    return {max_d, {best_a, best_b}};
}

double min_bounding_rect_area(const vector<Point>& hull) {
    int n = hull.size();
    double min_area = 1e18;
    int j = 1;
    for (int i = 0; i < n; i++) {
        int ni = (i + 1) % n;
        double edge_len = dist(hull[i], hull[ni]);
        while (true) {
            int nj = (j + 1) % n;
            double cross = (hull[ni].x - hull[i].x) * (hull[nj].y - hull[i].y)
                         - (hull[ni].y - hull[i].y) * (hull[nj].x - hull[i].x);
            if (cross > 0) j = nj; else break;
        }
        double width = dist_point_line(hull[j], hull[i], hull[ni]);
        double area = edge_len * width;
        if (area < min_area) min_area = area;
    }
    return min_area;
}

// =============================================================
// 8. ВЫПУКЛОЕ ЯДРО
// =============================================================

vector<Point> convex_kernel(vector<Point> poly) {
    vector<Point> result = poly;
    int n = result.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        double A = result[j].y - result[i].y;
        double B = result[i].x - result[j].x;
        double C = -(A * result[i].x + B * result[i].y);
        vector<Point> clipped;
        int m = result.size();
        for (int k = 0; k < m; k++) {
            Point cur = result[k];
            Point nxt = result[(k + 1) % m];
            double vc = A * cur.x + B * cur.y + C;
            double vn = A * nxt.x + B * nxt.y + C;
            if (vc >= -EPS) {
                clipped.push_back(cur);
                if (vn < -EPS) {
                    double t = vc / (vc - vn);
                    clipped.push_back({cur.x + t * (nxt.x - cur.x),
                                       cur.y + t * (nxt.y - cur.y)});
                }
            } else if (vn >= -EPS) {
                double t = vc / (vc - vn);
                clipped.push_back({cur.x + t * (nxt.x - cur.x),
                                   cur.y + t * (nxt.y - cur.y)});
            }
        }
        result = clipped;
        if (result.empty()) break;
    }
    return result;
}

}; // struct ConvexPolygons

#endif // GEOMETRY_E_CPP
