#ifndef GEOMETRY_H_CPP
#define GEOMETRY_H_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <tuple>
using namespace std;

#ifndef INSIDE_GEOMETRY_H
#define INSIDE_GEOMETRY_H
#include "../g/g.cpp"
#endif

struct Triangulations : SpatialStructures {

// =============================================================
// 1. ТРИАНГУЛЯЦИЯ ДЕЛОНЕ
// =============================================================

bool in_circle(const Point& a, const Point& b, const Point& c, const Point& d) {
    double ax = a.x - d.x, ay = a.y - d.y;
    double bx = b.x - d.x, by = b.y - d.y;
    double cx = c.x - d.x, cy = c.y - d.y;
    double det = ax * (by * (cx*cx + cy*cy) - cy * (bx*bx + by*by))
               - ay * (bx * (cx*cx + cy*cy) - cx * (bx*bx + by*by))
               + (ax*ax + ay*ay) * (bx * cy - by * cx);
    return det > EPS;
}

vector<tuple<int,int,int>> delaunay_triangulation(vector<Point> points) {
    int n = points.size();
    if (n < 3) return {};
    struct LiftedPoint { double x, y, z; int idx; };
    vector<LiftedPoint> lifted(n);
    for (int i = 0; i < n; i++) {
        lifted[i] = {points[i].x, points[i].y,
                     points[i].x * points[i].x + points[i].y * points[i].y, i};
    }
    vector<int> hull3d;
    for (int i = 0; i < min(3, n); i++) hull3d.push_back(i);
    for (int i = 3; i < n; i++) {
        auto orient3 = [&](int a, int b, int c, int d) -> double {
            LiftedPoint& A = lifted[a], &B = lifted[b], &C = lifted[c], &D = lifted[d];
            double ux = B.x-A.x, uy = B.y-A.y, uz = B.z-A.z;
            double vx = C.x-A.x, vy = C.y-A.y, vz = C.z-A.z;
            double wx = D.x-A.x, wy = D.y-A.y, wz = D.z-A.z;
            return ux*(vy*wz - vz*wy) - uy*(vx*wz - vz*wx) + uz*(vx*wy - vy*wx);
        };
        vector<int> visible;
        for (int j = 0; j < (int)hull3d.size(); j++) {
            int a = hull3d[j], b = hull3d[(j+1) % hull3d.size()];
            if (orient3(a, b, hull3d[(j+2)%hull3d.size()], i) > 0)
                visible.push_back(j);
        }
        vector<int> new_hull;
        for (int j = 0; j < (int)hull3d.size(); j++) {
            bool left = false, right = false;
            for (int v : visible) {
                if (hull3d[v] == hull3d[j]) left = true;
                if (hull3d[(v+1) % hull3d.size()] == hull3d[j]) right = true;
            }
            if (!left && !right) new_hull.push_back(hull3d[j]);
        }
        for (int v : visible) {
            new_hull.push_back(hull3d[v]);
            new_hull.push_back(i);
        }
        hull3d = new_hull;
    }
    auto lift_point = [&](const Point& p) -> LiftedPoint {
        return {p.x, p.y, p.x*p.x + p.y*p.y, -1};
    };
    auto orient3d = [&](const LiftedPoint& a, const LiftedPoint& b,
                        const LiftedPoint& c, const LiftedPoint& d) -> double {
        double ux = b.x-a.x, uy = b.y-a.y, uz = b.z-a.z;
        double vx = c.x-a.x, vy = c.y-a.y, vz = c.z-a.z;
        double wx = d.x-a.x, wy = d.y-a.y, wz = d.z-a.z;
        return ux*(vy*wz - vz*wy) - uy*(vx*wz - vz*wx) + uz*(vx*wy - vy*wx);
    };
    vector<tuple<int,int,int>> triangles;
    for (int i = 0; i + 2 < (int)hull3d.size(); i += 3) {
        int a = hull3d[i], b = hull3d[i+1], c = hull3d[i+2];
        LiftedPoint lp = lift_point({0, 0});
        if (orient3d(lifted[a], lifted[b], lifted[c], lp) > 0)
            triangles.push_back({lifted[a].idx, lifted[b].idx, lifted[c].idx});
        else
            triangles.push_back({lifted[a].idx, lifted[c].idx, lifted[b].idx});
    }
    return triangles;
}

// =============================================================
// 2. ТРИАНГУЛЯЦИЯ МОНОТОННЫХ МНОГОУГОЛЬНИКОВ
// =============================================================

bool is_monotone_y(const vector<Point>& poly) {
    int n = poly.size();
    int min_y = 0, max_y = 0;
    for (int i = 1; i < n; i++) {
        if (poly[i].y < poly[min_y].y) min_y = i;
        if (poly[i].y > poly[max_y].y) max_y = i;
    }
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int k = (j + 1) % n;
        if (i != min_y && i != max_y) {
            if (orient(poly[i], poly[j], poly[k]) < 0) return false;
        }
    }
    return true;
}

vector<tuple<int,int,int>> triangulate_monotone(const vector<Point>& poly) {
    int n = poly.size();
    if (n < 3) return {};
    int min_y = 0, max_y = 0;
    for (int i = 1; i < n; i++) {
        if (poly[i].y < poly[min_y].y) min_y = i;
        if (poly[i].y > poly[max_y].y) max_y = i;
    }
    vector<int> order(n);
    int idx = 0;
    int cur = min_y;
    while (cur != max_y) {
        order[idx++] = cur;
        cur = (cur + 1) % n;
    }
    order[idx++] = max_y;
    vector<int> right_chain;
    cur = max_y;
    while (cur != min_y) {
        right_chain.push_back(cur);
        cur = (cur + n - 1) % n;
    }
    right_chain.push_back(min_y);
    vector<int> stack;
    stack.push_back(order[0]);
    stack.push_back(order[1]);
    vector<tuple<int,int,int>> triangles;
    int ri = 1;
    for (int i = 2; i < idx; i++) {
        int top = stack.back();
        bool same_chain = false;
        for (int j = 0; j < (int)right_chain.size() - 1; j++) {
            if ((order[i] == right_chain[j] || order[i] == right_chain[j+1]) &&
                (top == right_chain[j] || top == right_chain[j+1])) {
                same_chain = true;
                break;
            }
        }
        if (same_chain) {
            while (stack.size() > 1) {
                int b = stack.back(); stack.pop_back();
                int a = stack.back();
                triangles.push_back({a, b, order[i]});
            }
            stack.push_back(top);
            stack.push_back(order[i]);
        } else {
            int bot = stack.front();
            stack.clear();
            triangles.push_back({bot, order[i-1], order[i]});
            for (int j = 0; j < (int)right_chain.size(); j++) {
                if (right_chain[j] == order[i-1]) { ri = j + 1; break; }
            }
            stack.push_back(order[i-1]);
            stack.push_back(order[i]);
        }
    }
    return triangles;
}

// =============================================================
// 3. АЛГОРИТМ ДУГЛАСА-ПЕКЕРА
// =============================================================

vector<Point> douglas_peucker(const vector<Point>& points, double epsilon) {
    if (points.size() <= 2) return points;
    double max_dist = 0;
    int max_idx = 0;
    for (int i = 1; i < (int)points.size() - 1; i++) {
        double d = dist_point_segment(points[i], points[0], points.back());
        if (d > max_dist) { max_dist = d; max_idx = i; }
    }
    if (max_dist > epsilon) {
        vector<Point> left(points.begin(), points.begin() + max_idx + 1);
        vector<Point> right(points.begin() + max_idx, points.end());
        auto l = douglas_peucker(left, epsilon);
        auto r = douglas_peucker(right, epsilon);
        l.insert(l.end(), r.begin() + 1, r.end());
        return l;
    }
    return {points[0], points.back()};
}

}; // struct Triangulations

#endif // GEOMETRY_H_CPP
