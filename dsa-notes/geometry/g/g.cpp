#ifndef GEOMETRY_G_CPP
#define GEOMETRY_G_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <limits>
using namespace std;

#ifndef INSIDE_GEOMETRY_G
#define INSIDE_GEOMETRY_G
#include "../f/f.cpp"
#endif

struct SpatialStructures : ComputationalGeometry {

// =============================================================
// 1. KD-TREE
// =============================================================

struct KDNode {
    Point pt;
    int axis;
    KDNode *left = nullptr, *right = nullptr;
    KDNode(Point p, int a) : pt(p), axis(a) {}
};

KDNode* build_kdtree(vector<Point> points, int depth = 0) {
    if (points.empty()) return nullptr;
    int axis = depth % 2;
    sort(points.begin(), points.end(), [axis](const Point& a, const Point& b) {
        return axis == 0 ? a.x < b.x : a.y < b.y;
    });
    int mid = points.size() / 2;
    vector<Point> left_pts(points.begin(), points.begin() + mid);
    vector<Point> right_pts(points.begin() + mid + 1, points.end());
    KDNode* node = new KDNode(points[mid], axis);
    node->left = build_kdtree(left_pts, depth + 1);
    node->right = build_kdtree(right_pts, depth + 1);
    return node;
}

void kNN(KDNode* root, const Point& target, int k,
         priority_queue<pair<double,Point>>& heap) {
    if (!root) return;
    double d = dist(root->pt, target);
    if ((int)heap.size() < k) {
        heap.push({d, root->pt});
    } else if (d < heap.top().first) {
        heap.pop();
        heap.push({d, root->pt});
    }
    int axis = root->axis;
    double diff = (axis == 0 ? target.x - root->pt.x : target.y - root->pt.y);
    KDNode* first = diff < 0 ? root->left : root->right;
    KDNode* second = diff < 0 ? root->right : root->left;
    kNN(first, target, k, heap);
    if ((int)heap.size() < k || fabs(diff) < heap.top().first)
        kNN(second, target, k, heap);
}

vector<Point> k_nearest(KDNode* root, const Point& target, int k) {
    priority_queue<pair<double,Point>> heap;
    kNN(root, target, k, heap);
    vector<Point> result;
    while (!heap.empty()) {
        result.push_back(heap.top().second);
        heap.pop();
    }
    reverse(result.begin(), result.end());
    return result;
}

double nearest_neighbor(KDNode* root, const Point& target) {
    if (!root) return 1e18;
    double best = dist(root->pt, target);
    int axis = root->axis;
    double diff = (axis == 0 ? target.x - root->pt.x : target.y - root->pt.y);
    KDNode* first = diff < 0 ? root->left : root->right;
    KDNode* second = diff < 0 ? root->right : root->left;
    best = min(best, nearest_neighbor(first, target));
    if (fabs(diff) < best)
        best = min(best, nearest_neighbor(second, target));
    return best;
}

// =============================================================
// 2. БЛИЖАЙШАЯ ПАРА ТОЧЕК
// =============================================================

double closest_pair_brute(const vector<Point>& points) {
    double min_d = 1e18;
    for (int i = 0; i < (int)points.size(); i++)
        for (int j = i + 1; j < (int)points.size(); j++)
            min_d = min(min_d, dist(points[i], points[j]));
    return min_d;
}

double closest_pair_dc(vector<Point> points) {
    int n = points.size();
    if (n <= 3) return closest_pair_brute(points);
    sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
        return a.x < b.x;
    });
    int mid = n / 2;
    double mid_x = points[mid].x;
    vector<Point> left(points.begin(), points.begin() + mid);
    vector<Point> right(points.begin() + mid, points.end());
    double d = min(closest_pair_dc(left), closest_pair_dc(right));
    vector<Point> strip;
    for (auto& p : points)
        if (fabs(p.x - mid_x) < d) strip.push_back(p);
    sort(strip.begin(), strip.end(), [](const Point& a, const Point& b) {
        return a.y < b.y;
    });
    for (int i = 0; i < (int)strip.size(); i++)
        for (int j = i + 1; j < (int)strip.size() && (strip[j].y - strip[i].y) < d; j++)
            d = min(d, dist(strip[i], strip[j]));
    return d;
}

// =============================================================
// 3. ДИАГРАММА ВОРОНОГО (базовая)
// =============================================================

struct VoronoiResult {
    vector<Point> centers;
    vector<vector<int>> neighbors;
};

// =============================================================
// 4. QUADTREE
// =============================================================

struct QuadNode {
    Point pt;
    bool has_point;
    double x_min, x_max, y_min, y_max;
    QuadNode* children[4];
    QuadNode(double xn, double xx, double yn, double yx)
        : has_point(false), x_min(xn), x_max(xx), y_min(yn), y_max(yx) {
        for (int i = 0; i < 4; i++) children[i] = nullptr;
    }
    ~QuadNode() { for (int i = 0; i < 4; i++) delete children[i]; }
};

void quad_insert(QuadNode* node, const Point& p) {
    if (!node) return;
    if (!node->has_point && !node->children[0]) {
        node->pt = p;
        node->has_point = true;
        return;
    }
    double x_mid = (node->x_min + node->x_max) / 2.0;
    double y_mid = (node->y_min + node->y_max) / 2.0;
    int quadrant = (p.x >= x_mid ? 1 : 0) + (p.y >= y_mid ? 2 : 0);
    if (!node->children[quadrant]) {
        double cx0 = (quadrant & 1) ? x_mid : node->x_min;
        double cx1 = (quadrant & 1) ? node->x_max : x_mid;
        double cy0 = (quadrant & 2) ? y_mid : node->y_min;
        double cy1 = (quadrant & 2) ? node->y_max : y_mid;
        node->children[quadrant] = new QuadNode(cx0, cx1, cy0, cy1);
    }
    quad_insert(node->children[quadrant], p);
}

void quad_range_query(QuadNode* node, const Point& center, double radius,
                      vector<Point>& result) {
    if (!node) return;
    if (node->has_point) {
        if (dist(node->pt, center) <= radius + EPS)
            result.push_back(node->pt);
        return;
    }
    for (int i = 0; i < 4; i++) {
        if (!node->children[i]) continue;
        double dx = max(0.0, max(node->children[i]->x_min - center.x,
                                 center.x - node->children[i]->x_max));
        double dy = max(0.0, max(node->children[i]->y_min - center.y,
                                 center.y - node->children[i]->y_max));
        if (dx * dx + dy * dy <= radius * radius + EPS)
            quad_range_query(node->children[i], center, radius, result);
    }
}

}; // struct SpatialStructures

#endif // GEOMETRY_G_CPP
