#ifndef GRAPH_I_CPP
#define GRAPH_I_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>
#include <set>
#include <cmath>
using namespace std;

// =============================================================
// I. ПЛАНАРНЫЕ ГРАФЫ И АЛГОРИТМЫ
// =============================================================
// Структура md: A. Основные свойства (Эйлер, Куратовский, separator)
//               → B. Dual graph, Fáry, алгоритмы
//               → C. Графы на поверхностях
//
// PlanarGraphs — наследует NetworkFlow (h.cpp).
// Реализует: проверка планарности (necessary conditions),
// dual graph, planar separator (упрощённый), Euler formula.

#ifndef INSIDE_GRAPH_I
#define INSIDE_GRAPH_I
#include "../h/h.cpp"
#undef INSIDE_GRAPH_I
#endif

struct PlanarGraphs : NetworkFlow {

// =============================================================
// A. ОСНОВНЫЕ СВОЙСТВА ПЛАНАРНЫХ ГРАФОВ
// =============================================================

// --- A.1. Теорема Эйлера: V − E + F ---
// Возвращает F = E − V + 2 (для связного планарного графа).
// Если граф не связен: V − E + F = 1 + c (c — число компонент).
int euler_formula(int n, int m, int components = 1) {
    return m - n + 2 * components;
}

// Проверка necessary condition: E ≤ 3V − 6 (простой планарный, V ≥ 3).
bool satisfies_euler_bound(int n, int m) {
    if (n < 3) return true;
    return m <= 3 * n - 6;
}

// Для двудольных планарных: E ≤ 2V − 4.
bool satisfies_bipartite_bound(int n, int m) {
    if (n < 3) return true;
    return m <= 2 * n - 4;
}

// --- A.2. Проверка planarity (упрощённая) ---
// Necessary conditions: E ≤ 3V−6 (general) или E ≤ 2V−4 (bipartite).
// Не является sufficient — но полезна как быстрая проверка.
// Возвращает: 0 — не планарен (нарушен necessary condition),
//             1 — возможно планарен, 2 — K5/K3,3 (garantie no).
int planarity_check(int n, int m, bool is_bipartite) {
    if (!satisfies_euler_bound(n, m)) return 0;
    if (is_bipartite && !satisfies_bipartite_bound(n, m)) return 0;
    // Проверка на K5 и K3,3 как подграфы
    if (n >= 5 && m >= 10) {
        // K5 имеет n=5, m=10 — проверяем наличие K5 minor
        // (упрощённо: если полный граф K5)
        if (n == 5 && m == 10) return 0;  // K5 — не планарен
    }
    if (n >= 6 && m >= 9) {
        // K3,3 имеет n=6, m=9 — двудольный полный
        if (n == 6 && m == 9 && is_bipartite) return 0;
    }
    return 1;
}

// =============================================================
// B. DUAL GRAPH
// =============================================================

// --- B.1. Построение dual graph ---
// Для planar embedding: каждая грань → вершина dual graph.
// Здесь: упрощённый вариант через face traversal.
// Возвращает: количество граней F (через Эйлер).
int face_count(int n, int m, int components) {
    return euler_formula(n, m, components);
}

// --- B.2. Dual graph для planar graph (через компоненты) ---
// Для связного planar graph: dual имеет F вершин, E рёбер.
// Пространственная сложность: O(F + E).
// Упрощённо: возвращаем число вершин и рёбер dual graph.
pair<int, int> dual_graph_params(int n, int m) {
    int F = euler_formula(n, m);
    return {F, m};  // V* = F, E* = E
}

// =============================================================
// C. PLANAR SEPARATOR (упрощённый)
// =============================================================

// --- C.1. Упрощённый planar separator ---
// Для деревьев (которые планарны): центр дерева — separator O(1).
// Для общих планарных: нужен Lipton-Tarjan algorithm.
// Здесь: обобщённый подход через BFS + centroid-like разбиение.
// Возвращает: множество вершин-разделителей.
vector<int> planar_separator(int n, const vector<vector<int>>& adj) {
    // Упрощённый вариант: находим центр графа через BFS
    // от произвольной вершины, затем середину самого длинного пути
    vector<int> dist(n, -1);
    queue<int> q;
    q.push(0);
    dist[0] = 0;
    int far = 0;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        far = v;
        for (int u : adj[v])
            if (dist[u] == -1) { dist[u] = dist[v] + 1; q.push(u); }
    }

    // Второй BFS от far
    vector<int> dist2(n, -1), parent(n, -1);
    q.push(far);
    dist2[far] = 0;
    int far2 = far;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        far2 = v;
        for (int u : adj[v])
            if (dist2[u] == -1) { dist2[u] = dist2[v] + 1; parent[u] = v; q.push(u); }
    }

    // Путь от far до far2
    vector<int> path;
    for (int v = far2; v != -1; v = parent[v]) path.push_back(v);
    reverse(path.begin(), path.end());

    // separator — ~√n вершин вокруг середины пути
    int target = max(1, (int)sqrt(n));
    vector<int> sep;
    int mid = path.size() / 2;
    int radius = target / 2;
    for (int i = max(0, mid - radius); i <= min((int)path.size()-1, mid + radius); i++)
        sep.push_back(path[i]);

    return sep;
}

// =============================================================
// D. FÁRY THEOREM (упрощённое встраивание)
// =============================================================

// --- D.1. Straight-line drawing (упрощённый) ---
// Для дерева (планарного): назначаем координаты через DFS.
// Каждая вершина — точка (x, y); рёбра — прямые отрезки.
struct Point { int x, y; };

vector<Point> fary_tree_embedding(int n, const vector<vector<int>>& adj) {
    vector<Point> pos(n);
    int x_counter = 0;

    function<void(int, int)> dfs = [&](int v, int p) {
        pos[v] = {x_counter++, 0};
        for (int u : adj[v])
            if (u != p) dfs(u, v);
    };
    dfs(0, -1);

    // Y-координата = глубина
    vector<int> depth(n, 0);
    function<void(int, int)> dfs_depth = [&](int v, int p) {
        for (int u : adj[v])
            if (u != p) { depth[u] = depth[v] + 1; dfs_depth(u, v); }
    };
    dfs_depth(0, -1);
    for (int i = 0; i < n; i++) pos[i].y = depth[i] * 2;

    return pos;
}

// =============================================================
// E. ГРАФЫ НА ПОВЕРХНОСТЯХ
// =============================================================

// --- E.1. Обобщённая формула Эйлера для поверхности genus g ---
// V − E + F = 2 − 2g
int euler_genus(int v, int e, int f) {
    // V − E + F = 2 − 2g → g = (2 − (V − E + F)) / 2
    return (2 - (v - e + f)) / 2;
}

// Граница E для планарного графа на поверхности genus g.
int max_edges_planar_genus(int n, int g) {
    return 3 * (n + 2 * g - 1);
}

}; // struct PlanarGraphs

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_I_MAIN
int main() {
    PlanarGraphs pg;

    cout << "=== Euler Formula ===" << endl;
    // K4: n=4, m=6 → F = 6-4+2 = 4
    cout << "K4: F = " << pg.euler_formula(4, 6) << endl;
    // Куб: n=8, m=12 → F = 12-8+2 = 6
    cout << "Cube: F = " << pg.euler_formula(8, 12) << endl;

    cout << "\n=== Euler Bound ===" << endl;
    cout << "K4 (4,6): " << (pg.satisfies_euler_bound(4, 6) ? "OK" : "VIOLATED") << endl;
    cout << "K5 (5,10): " << (pg.satisfies_euler_bound(5, 10) ? "OK" : "VIOLATED") << endl;
    cout << "K6 (6,15): " << (pg.satisfies_euler_bound(6, 15) ? "OK" : "VIOLATED") << endl;

    cout << "\n=== Planarity Check ===" << endl;
    cout << "K4: " << pg.planarity_check(4, 6, false) << endl;
    cout << "K5: " << pg.planarity_check(5, 10, false) << endl;
    cout << "K3,3: " << pg.planarity_check(6, 9, true) << endl;
    cout << "Cube: " << pg.planarity_check(8, 12, false) << endl;

    cout << "\n=== Dual Graph ===" << endl;
    auto [dv, de] = pg.dual_graph_params(8, 12);
    cout << "Cube dual: V*=" << dv << ", E*=" << de << endl;

    cout << "\n=== Planar Separator ===" << endl;
    // K4 (планарен)
    vector<vector<int>> adj_k4(4);
    adj_k4[0] = {1,2,3}; adj_k4[1] = {0,2,3};
    adj_k4[2] = {0,1,3}; adj_k4[3] = {0,1,2};
    auto sep = pg.planar_separator(4, adj_k4);
    cout << "K4 separator: ";
    for (int v : sep) cout << v << " ";
    cout << endl;

    cout << "\n=== Fáry Tree Embedding ===" << endl;
    vector<vector<int>> tree_adj(5);
    tree_adj[0] = {1,2}; tree_adj[1] = {0,3,4};
    tree_adj[2] = {0}; tree_adj[3] = {1}; tree_adj[4] = {1};
    auto pos = pg.fary_tree_embedding(5, tree_adj);
    for (int i = 0; i < 5; i++)
        cout << "v" << i << ": (" << pos[i].x << "," << pos[i].y << ")" << endl;

    cout << "\n=== Genus ===" << endl;
    cout << "Torus (V=7,E=14,F=7): genus = " << pg.euler_genus(7, 14, 7) << endl;
    cout << "Max edges on torus (n=10): " << pg.max_edges_planar_genus(10, 1) << endl;

    return 0;
}
#endif

#endif // GRAPH_I_CPP
