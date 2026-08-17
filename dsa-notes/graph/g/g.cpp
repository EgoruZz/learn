#ifndef GRAPH_G_CPP
#define GRAPH_G_CPP

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
using namespace std;

// =============================================================
// G. ПАРОСОЧЕТАНИЯ В ГРАФАХ
// =============================================================
// Структура md: A. Двудольные паросочетания
//               → B. Паросочетания в общих графах
//               → C. Взвешенные паросочетания
//               → D. Смежные понятия
//
// MatchingAlgorithms — наследует TreeAlgorithms (f.cpp).
// Реализует: Kuhn, Hopcroft-Karp, Hungarian, min path cover.

#ifndef INSIDE_GRAPH_G
#define INSIDE_GRAPH_G
#include "../f/f.cpp"
#undef INSIDE_GRAPH_G
#endif

struct MatchingAlgorithms : TreeAlgorithms {

// =============================================================
// A. ДВУДОЛЬНЫЕ ПАРОСОЧЕТАНИЯ
// =============================================================

// --- A.1. Алгоритм Куна (DFS-based) ---
// adj — список смежности из L в R; |L| = n, |R| = m.
// match_r[j] = вершина из L, сопоставленная j из R (-1 если свободна).
// Возвращает размер максимального паросочетания.
// O(VE) время.
int kuhn(int n, int m, const vector<vector<int>>& adj,
         vector<int>& match_r) {
    match_r.assign(m, -1);
    vector<bool> used(n);
    int result = 0;

    function<bool(int)> try_kuhn = [&](int u) -> bool {
        for (int v : adj[u]) {
            if (used[v]) continue;
            used[v] = true;
            if (match_r[v] == -1 || try_kuhn(match_r[v])) {
                match_r[v] = u;
                return true;
            }
        }
        return false;
    };

    for (int u = 0; u < n; u++) {
        fill(used.begin(), used.end(), false);
        if (try_kuhn(u)) result++;
    }
    return result;
}

// --- A.2. Алгоритм Хопкрофта-Карпа (BFS + DFS) ---
// adj — список смежности из L в R.
// match_l[i] = вершина из R, сопоставленная i из L (-1 если свободна).
// match_r[j] = вершина из L, сопоставленная j из R (-1 если свободна).
// Возвращает размер максимального паросочетания.
// O(E√V) время.
int hopcroft_karp(int n, int m, const vector<vector<int>>& adj,
                  vector<int>& match_l, vector<int>& match_r) {
    match_l.assign(n, -1);
    match_r.assign(m, -1);
    vector<int> dist(n);

    auto bfs = [&]() -> bool {
        queue<int> q;
        for (int u = 0; u < n; u++) {
            if (match_l[u] == -1) { dist[u] = 0; q.push(u); }
            else dist[u] = INT_MAX;
        }
        bool found = false;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                int u2 = match_r[v];
                if (u2 == -1) found = true;
                else if (dist[u2] == INT_MAX) {
                    dist[u2] = dist[u] + 1;
                    q.push(u2);
                }
            }
        }
        return found;
    };

    function<bool(int)> dfs = [&](int u) -> bool {
        for (int v : adj[u]) {
            int u2 = match_r[v];
            if (u2 == -1 || (dist[u2] == dist[u] + 1 && dfs(u2))) {
                match_l[u] = v;
                match_r[v] = u;
                return true;
            }
        }
        dist[u] = INT_MAX;
        return false;
    };

    int result = 0;
    while (bfs()) {
        for (int u = 0; u < n; u++)
            if (match_l[u] == -1 && dfs(u)) result++;
    }
    return result;
}

// =============================================================
// C. ВЗВЕШЕННЫЕ ПАРОСОЧЕТАНИЯ (Hungarian Algorithm)
// =============================================================

// --- C.1. Венгерский алгоритм ---
// cost[i][j] — матрица стоимостей (i ∈ L, j ∈ R), |L| = n, |R| = n.
// Возвращает: {минимальная_стоимость, match_l, match_r}.
// O(n³) время.
struct HungarianResult {
    long long cost;
    vector<int> match_l;  // match_l[i] = j
    vector<int> match_r;  // match_r[j] = i
};

HungarianResult hungarian(const vector<vector<int>>& cost) {
    int n = cost.size();
    int m = cost[0].size();
    // Дополняем до квадратной матрицы если нужно
    int sz = max(n, m);
    vector<vector<long long>> a(sz, vector<long long>(sz, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            a[i][j] = cost[i][j];

    vector<long long> u(sz + 1, 0), v(sz + 1, 0);
    vector<int> p(sz + 1, 0), way(sz + 1, 0);

    for (int i = 1; i <= sz; i++) {
        p[0] = i;
        int j0 = 0;
        vector<long long> minv(sz + 1, LLONG_MAX);
        vector<bool> used(sz + 1, false);
        do {
            used[j0] = true;
            int i0 = p[j0], j1 = 0;
            long long delta = LLONG_MAX;
            for (int j = 1; j <= sz; j++) {
                if (used[j]) continue;
                long long cur = a[i0 - 1][j - 1] - u[i0] - v[j];
                if (cur < minv[j]) { minv[j] = cur; way[j] = j0; }
                if (minv[j] < delta) { delta = minv[j]; j1 = j; }
            }
            for (int j = 0; j <= sz; j++) {
                if (used[j]) { u[p[j]] += delta; v[j] -= delta; }
                else minv[j] -= delta;
            }
            j0 = j1;
        } while (p[j0] != 0);
        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }

    HungarianResult res;
    res.cost = -v[0];
    res.match_l.assign(n, -1);
    res.match_r.assign(m, -1);
    for (int j = 1; j <= sz; j++) {
        if (p[j] > 0 && p[j] <= n && j <= m) {
            res.match_l[p[j] - 1] = j - 1;
            res.match_r[j - 1] = p[j] - 1;
        }
    }
    return res;
}

// =============================================================
// D. СМЕЖНЫЕ ПОНЯТИЯ
// =============================================================

// --- D.1. Minimum Path Cover в DAG ---
// n — число вершин; adj — ориентированный граф (список смежности).
// min_path_cover = n − max matching (сведение к двудольному).
// O(VE) время.
int min_path_cover_dag(int n, const vector<vector<int>>& adj) {
    // Строим двудольный граф: v_in (0..n-1) → v_out (n..2n-1)
    // Ребро (u_out, v_in) если (u, v) ∈ E
    vector<vector<int>> bip_adj(n);
    for (int u = 0; u < n; u++)
        for (int v : adj[u])
            bip_adj[u].push_back(v);

    vector<int> match_r;
    int max_match = kuhn(n, n, bip_adj, match_r);
    return n - max_match;
}

// --- D.2. Minimum Vertex Cover для двудольного графа ---
// Через теорему Кёнига: |VC| = |M*|.
// Построение: BFS от свободных вершин в L через alternating graph;
// VC = (L \ Reachable) ∪ (R ∩ Reachable).
// O(VE) время.
pair<int, vector<int>> min_vertex_cover_bipartite(
    int n, int m, const vector<vector<int>>& adj) {
    vector<int> match_r;
    int max_match = kuhn(n, m, adj, match_r);

    // Построение matching из L
    vector<int> match_l(n, -1);
    for (int j = 0; j < m; j++)
        if (match_r[j] != -1) match_l[match_r[j]] = j;

    // BFS от свободных вершин в L через alternating edges
    vector<bool> visited_l(n, false), visited_r(m, false);
    queue<int> q;
    for (int u = 0; u < n; u++)
        if (match_l[u] == -1) { q.push(u); visited_l[u] = true; }

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) {
            if (!visited_r[v]) {
                visited_r[v] = true;
                // Ребро (u, v) — не в matching (u свободен);
                // Если v покрыта matching'ом — идём в matched vertex
                int u2 = match_r[v];
                if (u2 != -1 && !visited_l[u2]) {
                    visited_l[u2] = true;
                    q.push(u2);
                }
            }
        }
    }

    // VC = (L \ Reachable_L) ∪ (R ∩ Reachable_R)
    vector<int> vc;
    for (int u = 0; u < n; u++)
        if (!visited_l[u]) vc.push_back(u);
    for (int v = 0; v < m; v++)
        if (visited_r[v]) vc.push_back(n + v);

    return {max_match, vc};
}

}; // struct MatchingAlgorithms

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_G_MAIN
int main() {
    MatchingAlgorithms ma;

    cout << "=== Kuhn (Bipartite Matching) ===" << endl;
    // L = {0,1,2}, R = {0,1,2}
    // 0 — 0, 0 — 1
    // 1 — 1, 1 — 2
    // 2 — 0
    int n = 3, m = 3;
    vector<vector<int>> adj = {{0,1},{1,2},{0}};
    vector<int> match_r;
    int sz = ma.kuhn(n, m, adj, match_r);
    cout << "Max matching size: " << sz << endl;
    for (int j = 0; j < m; j++)
        if (match_r[j] != -1)
            cout << match_r[j] << " -- " << j << endl;

    cout << "\n=== Hopcroft-Karp ===" << endl;
    vector<int> match_l, match_r2;
    int sz2 = ma.hopcroft_karp(n, m, adj, match_l, match_r2);
    cout << "Max matching size: " << sz2 << endl;
    for (int i = 0; i < n; i++)
        if (match_l[i] != -1)
            cout << i << " -- " << match_l[i] << endl;

    cout << "\n=== Hungarian Algorithm ===" << endl;
    vector<vector<int>> cost = {
        {4, 1, 3},
        {2, 0, 5},
        {3, 2, 2}
    };
    auto hr = ma.hungarian(cost);
    cout << "Min cost: " << hr.cost << endl;
    for (int i = 0; i < 3; i++)
        if (hr.match_l[i] != -1)
            cout << i << " -- " << hr.match_l[i]
                 << " (cost=" << cost[i][hr.match_l[i]] << ")" << endl;

    cout << "\n=== Min Path Cover in DAG ===" << endl;
    int n_dag = 6;
    vector<vector<int>> dag_adj(n_dag);
    // 0→1, 1→2, 3→4, 4→5, 0→3
    dag_adj[0] = {1, 3};
    dag_adj[1] = {2};
    dag_adj[3] = {4};
    dag_adj[4] = {5};
    int mpc = ma.min_path_cover_dag(n_dag, dag_adj);
    cout << "Min path cover: " << mpc << endl;

    cout << "\n=== Min Vertex Cover (Bipartite) ===" << endl;
    auto [mvc_size, vc] = ma.min_vertex_cover_bipartite(n, m, adj);
    cout << "Min vertex cover size: " << mvc_size << endl;
    cout << "VC: ";
    for (int v : vc) {
        if (v < n) cout << "L" << v << " ";
        else cout << "R" << (v - n) << " ";
    }
    cout << endl;

    return 0;
}
#endif

#endif // GRAPH_G_CPP
