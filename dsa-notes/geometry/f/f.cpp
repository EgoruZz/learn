#ifndef GEOMETRY_F_CPP
#define GEOMETRY_F_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

#ifndef INSIDE_GEOMETRY_F
#define INSIDE_GEOMETRY_F
#include "../e/e.cpp"
#endif

struct ComputationalGeometry : ConvexPolygons {

// =============================================================
// 1. АЛГОРИТМЫ НА ПРЯМОЙ
// =============================================================

double union_length(vector<pair<double,double>> segments) {
    sort(segments.begin(), segments.end());
    double total = 0, cur_start = segments[0].first, cur_end = segments[0].second;
    for (int i = 1; i < (int)segments.size(); i++) {
        if (segments[i].first <= cur_end)
            cur_end = max(cur_end, segments[i].second);
        else {
            total += cur_end - cur_start;
            cur_start = segments[i].first;
            cur_end = segments[i].second;
        }
    }
    total += cur_end - cur_start;
    return total;
}

// =============================================================
// 2. РЕШЁТЧАТАЯ ГЕОМЕТРИЯ
// =============================================================

int pick_interior(int area_x2, int boundary) {
    return (area_x2 - boundary + 2) / 2;
}

// =============================================================
// 3. ПРИНАДЛЕЖНОСТЬ ТОЧКИ МНОГОУГОЛЬНИКУ
// =============================================================

bool point_in_polygon(const Point& p, const vector<Point>& poly) {
    int n = poly.size();
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) /
                    (poly[j].y - poly[i].y) + poly[i].x))
            inside = !inside;
    }
    return inside;
}

int winding_number(const Point& p, const vector<Point>& poly) {
    int wn = 0, n = poly.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        if (poly[i].y <= p.y) {
            if (poly[j].y > p.y && orient(poly[i], poly[j], p) > 0) wn++;
        } else {
            if (poly[j].y <= p.y && orient(poly[i], poly[j], p) < 0) wn--;
        }
    }
    return wn;
}

// =============================================================
// 4. ПЕРЕСЕЧЕНИЕ ПОЛУПЛОСКОСТЕЙ
// =============================================================

vector<Point> halfplane_intersection(vector<tuple<double,double,double>> halfplanes) {
    int n = halfplanes.size();
    if (n == 0) return {};
    struct HP { double A, B, C; };
    vector<HP> hps(n);
    for (int i = 0; i < n; i++) {
        hps[i] = {get<0>(halfplanes[i]), get<1>(halfplanes[i]), get<2>(halfplanes[i])};
    }
    auto normal_angle = [](double A, double B) { return atan2(B, A); };
    sort(hps.begin(), hps.end(), [&](const HP& a, const HP& b) {
        double anga = normal_angle(a.A, a.B);
        double angb = normal_angle(b.A, b.B);
        if (fabs(anga - angb) > EPS) return anga < angb;
        return a.C < b.C;
    });
    vector<HP> unique_hps;
    for (int i = 0; i < n; i++) {
        if (i == 0 || fabs(normal_angle(hps[i].A, hps[i].B) -
                           normal_angle(hps[i-1].A, hps[i-1].B)) > EPS)
            unique_hps.push_back(hps[i]);
    }
    n = unique_hps.size();
    if (n == 0) return {};
    auto intersect = [](const HP& h1, const HP& h2) -> pair<bool, Point> {
        double det = h1.A * h2.B - h2.A * h1.B;
        if (fabs(det) < EPS) return {false, {0, 0}};
        return {true, {(h1.B * h2.C - h2.B * h1.C) / det,
                       (h2.A * h1.C - h1.A * h2.C) / det}};
    };
    auto inside = [](const HP& h, const Point& p) {
        return h.A * p.x + h.B * p.y <= h.C + EPS;
    };
    vector<Point> dq;
    vector<Point> result;
    for (int i = 0; i < n; i++) {
        while (dq.size() > 1 && !inside(unique_hps[i], dq.back()))
            dq.pop_back();
        while (dq.size() > 1 && !inside(unique_hps[i], dq.front()))
            dq.erase(dq.begin());
        dq.push_back({});
        if (dq.size() > 1) {
            auto [ok, p] = intersect(unique_hps[i], unique_hps[i == 0 ? n - 1 : i - 1]);
            if (ok) dq.back() = p;
        }
    }
    while (dq.size() > 1 && !inside(unique_hps[0], dq.back()))
        dq.pop_back();
    if (dq.size() > 2) {
        auto [ok, p] = intersect(unique_hps[0], unique_hps[n - 1]);
        if (ok) dq[0] = p;
    }
    return dq;
}

// =============================================================
// 5. ОПТИМИЗАЦИЯ В МНОГОУГОЛЬНИКАХ
// =============================================================

pair<Point, double> largest_inscribed_circle(const vector<Point>& hull) {
    int n = hull.size();
    Point best_center;
    double best_r = 0;
    for (int i = 0; i < n; i++) {
        Point center = {(hull[i].x + hull[(i+1)%n].x) / 2.0,
                        (hull[i].y + hull[(i+1)%n].y) / 2.0};
        double min_r = 1e18;
        for (int j = 0; j < n; j++)
            min_r = min(min_r, dist_point_line(center, hull[j], hull[(j+1)%n]));
        if (min_r > best_r) { best_r = min_r; best_center = center; }
    }
    return {best_center, best_r};
}

pair<Point, double> min_enclosing_circle(const vector<Point>& points) {
    if (points.empty()) return {{0, 0}, 0};
    if (points.size() == 1) return {points[0], 0};
    Point c = points[0];
    double r = 0;
    for (int iter = 0; iter < 100; iter++) {
        int farthest = 0;
        double max_d = 0;
        for (int i = 0; i < (int)points.size(); i++) {
            double d = dist(c, points[i]);
            if (d > max_d) { max_d = d; farthest = i; }
        }
        if (max_d <= r + EPS) break;
        r = max_d;
        double step = r;
        for (int j = 9; j >= 0; j--) {
            Point best = c;
            for (double dx = -step; dx <= step; dx += step) {
                for (double dy = -step; dy <= step; dy += step) {
                    Point cand = {c.x + dx, c.y + dy};
                    double d = dist(cand, points[farthest]);
                    if (d < dist(best, points[farthest]))
                        best = cand;
                }
            }
            c = best;
            step *= 0.4;
        }
    }
    return {c, r};
}

// =============================================================
// 6. ПОКРЫТИЕ ОТРЕЗКОВ ТОЧКАМИ
// =============================================================

vector<double> cover_segments_with_points(vector<pair<double,double>> segments, int k) {
    sort(segments.begin(), segments.end());
    vector<double> points;
    int i = 0;
    while (i < (int)segments.size()) {
        double cur_end = segments[i].second;
        int j = i + 1;
        while (j < (int)segments.size() && segments[j].first <= cur_end + EPS) {
            cur_end = max(cur_end, segments[j].second);
            j++;
        }
        points.push_back(cur_end);
        i = j;
    }
    return points;
}

}; // struct ComputationalGeometry

#endif // GEOMETRY_F_CPP
