#ifndef GRAPH_H_CPP
#define GRAPH_H_CPP

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <functional>
using namespace std;

// =============================================================
// H. ПОТОКИ В СЕТЯХ (NETWORK FLOW)
// =============================================================
// Структура md: A. Максимальный поток (Ford-Fulkerson, Edmonds-Karp, Dinic)
//               → B. Минимальный разрез
//               → C. Поток минимальной стоимости
//               → D. Специальные виды потоков
//               → E. Применения
//
// NetworkFlow — наследует MatchingAlgorithms (g.cpp).
// Реализует: Ford-Fulkerson, Edmonds-Karp, Dinic,
// Min-Cost Max-Flow (Successive Shortest Path с потенциалами).

#ifndef INSIDE_GRAPH_H
#define INSIDE_GRAPH_H
#include "../g/g.cpp"
#undef INSIDE_GRAPH_H
#endif

struct NetworkFlow : MatchingAlgorithms {

// =============================================================
// ВНУТРЕННЕЕ ПРЕДСТАВЛЕНИЕ СЕТИ
// =============================================================

struct Edge {
    int to, rev;        // to — конечная вершина, rev — индекс обратного ребра
    int cap;            // остаточная ёмкость
    int cost;           // стоимость за единицу потока (для min-cost)
};

// Построение сети из рёбер (u, v, capacity, cost).
// add_edge双向: прямое + обратное ребро (с capacity=0).
vector<vector<Edge>> build_network(int n,
    const vector<tuple<int,int,int>>& edges) {
    vector<vector<Edge>> g(n);
    for (auto& [u, v, c] : edges) {
        g[u].push_back({v, (int)g[v].size(), c, 0});
        g[v].push_back({u, (int)g[u].size() - 1, 0, 0});
    }
    return g;
}

// Версия со стоимостями.
vector<vector<Edge>> build_network_cost(int n,
    const vector<tuple<int,int,int,int>>& edges) {
    vector<vector<Edge>> g(n);
    for (auto& [u, v, c, cost] : edges) {
        g[u].push_back({v, (int)g[v].size(), c, cost});
        g[v].push_back({u, (int)g[u].size() - 1, 0, -cost});
    }
    return g;
}

// =============================================================
// A. МАКСИМАЛЬНЫЙ ПОТОК
// =============================================================

// --- A.1. Ford-Fulkerson (DFS-based) ---
// s — source, t — sink.
// O(E · |f*|) время.
int ford_fulkerson(vector<vector<Edge>>& g, int s, int t) {
    int flow = 0;
    int n = g.size();
    vector<int> parent(n), parent_edge(n);

    auto bfs_find_path = [&]() -> bool {
        fill(parent.begin(), parent.end(), -1);
        queue<int> q;
        q.push(s);
        parent[s] = s;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int i = 0; i < (int)g[v].size(); i++) {
                Edge& e = g[v][i];
                if (parent[e.to] == -1 && e.cap > 0) {
                    parent[e.to] = v;
                    parent_edge[e.to] = i;
                    if (e.to == t) return true;
                    q.push(e.to);
                }
            }
        }
        return false;
    };

    while (bfs_find_path()) {
        // Находим bottleneck
        int bottleneck = INT_MAX;
        for (int v = t; v != s; v = parent[v]) {
            int i = parent_edge[v];
            bottleneck = min(bottleneck, g[parent[v]][i].cap);
        }
        // Увеличиваем поток
        for (int v = t; v != s; v = parent[v]) {
            int i = parent_edge[v];
            g[parent[v]][i].cap -= bottleneck;
            g[v][g[parent[v]][i].rev].cap += bottleneck;
        }
        flow += bottleneck;
    }
    return flow;
}

// --- A.2. Edmonds-Karp (BFS-based) ---
// Аналог Ford-Fulkerson, но с BFS для поиска кратчайшего augmenting path.
// O(VE²) время.
int edmonds_karp(vector<vector<Edge>>& g, int s, int t) {
    int flow = 0;
    int n = g.size();
    vector<int> parent(n), parent_edge(n);

    auto bfs_find_path = [&]() -> bool {
        fill(parent.begin(), parent.end(), -1);
        queue<int> q;
        q.push(s);
        parent[s] = s;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int i = 0; i < (int)g[v].size(); i++) {
                Edge& e = g[v][i];
                if (parent[e.to] == -1 && e.cap > 0) {
                    parent[e.to] = v;
                    parent_edge[e.to] = i;
                    if (e.to == t) return true;
                    q.push(e.to);
                }
            }
        }
        return false;
    };

    while (bfs_find_path()) {
        int bottleneck = INT_MAX;
        for (int v = t; v != s; v = parent[v])
            bottleneck = min(bottleneck, g[parent[v]][parent_edge[v]].cap);
        for (int v = t; v != s; v = parent[v]) {
            int i = parent_edge[v];
            g[parent[v]][i].cap -= bottleneck;
            g[v][g[parent[v]][i].rev].cap += bottleneck;
        }
        flow += bottleneck;
    }
    return flow;
}

// --- A.3. Dinic's Algorithm ---
// Level graph + blocking flow.
// O(V²E) время; O(√V · E) для unit capacities.
int dinic(vector<vector<Edge>>& g, int s, int t) {
    int n = g.size();
    vector<int> level(n), iter(n);
    int flow = 0;

    auto bfs_level = [&]() -> bool {
        fill(level.begin(), level.end(), -1);
        queue<int> q;
        level[s] = 0;
        q.push(s);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (Edge& e : g[v]) {
                if (level[e.to] == -1 && e.cap > 0) {
                    level[e.to] = level[v] + 1;
                    q.push(e.to);
                }
            }
        }
        return level[t] != -1;
    };

    function<int(int, int)> dfs_blocking = [&](int v, int f) -> int {
        if (v == t) return f;
        for (int& i = iter[v]; i < (int)g[v].size(); i++) {
            Edge& e = g[v][i];
            if (level[v] < level[e.to] && e.cap > 0) {
                int d = dfs_blocking(e.to, min(f, e.cap));
                if (d > 0) {
                    e.cap -= d;
                    g[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    };

    while (bfs_level()) {
        fill(iter.begin(), iter.end(), 0);
        int d;
        while ((d = dfs_blocking(s, INT_MAX)) > 0)
            flow += d;
    }
    return flow;
}

// =============================================================
// B. МИНИМАЛЬНЫЙ РАЗРЕЗ
// =============================================================

// --- B.1. s-t Min-Cut после max-flow ---
// Возвращает: {capacity, S (множество вершин source-side)}.
// O(V + E) после max-flow.
pair<int, vector<int>> min_st_cut(const vector<vector<Edge>>& g, int s, int t) {
    int n = g.size();
    vector<bool> visited(n, false);
    queue<int> q;
    q.push(s);
    visited[s] = true;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (const Edge& e : g[v])
            if (!visited[e.to] && e.cap > 0) {
                visited[e.to] = true;
                q.push(e.to);
            }
    }
    int cap = 0;
    vector<int> S;
    for (int v = 0; v < n; v++) {
        if (visited[v]) {
            S.push_back(v);
            for (const Edge& e : g[v])
                if (!visited[e.to])
                    cap += e.cap;
        }
    }
    return {cap, S};
}

// =============================================================
// C. ПОТОК МИНИМАЛЬНОЙ СТОИМОСТИ
// =============================================================

// --- C.1. Successive Shortest Path (с потенциалами) ---
// s — source, t — sink, desired_flow — искомая величина потока.
// Возвращает: {стоимость, величина потока}.
// O(F · (E + V log V)) время.
pair<long long, int> min_cost_max_flow(
    vector<vector<Edge>>& g, int s, int t, int desired_flow = INT_MAX) {
    int n = g.size();
    vector<long long> pot(n, 0);  // потенциалы
    long long total_cost = 0;
    int total_flow = 0;

    while (total_flow < desired_flow) {
        // Dijkstra с потенциалами
        vector<long long> dist(n, LLONG_MAX);
        vector<int> parent_v(n, -1), parent_e(n, -1);
        dist[s] = 0;
        priority_queue<pair<long long,int>, vector<pair<long long,int>>,
                       greater<pair<long long,int>>> pq;
        pq.push({0, s});

        while (!pq.empty()) {
            auto [d, v] = pq.top(); pq.pop();
            if (d > dist[v]) continue;
            for (int i = 0; i < (int)g[v].size(); i++) {
                Edge& e = g[v][i];
                if (e.cap <= 0) continue;
                long long nd = dist[v] + e.cost + pot[v] - pot[e.to];
                if (nd < dist[e.to]) {
                    dist[e.to] = nd;
                    parent_v[e.to] = v;
                    parent_e[e.to] = i;
                    pq.push({nd, e.to});
                }
            }
        }

        if (dist[t] == LLONG_MAX) break;  // нет пути

        // Обновляем потенциалы
        for (int v = 0; v < n; v++)
            if (dist[v] < LLONG_MAX) pot[v] += dist[v];

        // Находим bottleneck
        int bottleneck = desired_flow - total_flow;
        for (int v = t; v != s; v = parent_v[v])
            bottleneck = min(bottleneck, g[parent_v[v]][parent_e[v]].cap);

        // Увеличиваем поток
        for (int v = t; v != s; v = parent_v[v]) {
            int i = parent_e[v];
            g[parent_v[v]][i].cap -= bottleneck;
            g[v][g[parent_v[v]][i].rev].cap += bottleneck;
            total_cost += (long long)g[parent_v[v]][i].cost * bottleneck;
            // Восстанавливаем стоимость из ОРИГИНАЛЬНОГО ребра
            // (обратное ребро имеет -cost, но мы не трогаем его)
        }
        total_flow += bottleneck;
    }

    return {total_cost, total_flow};
}

// =============================================================
// E. ПРИМЕНЕНИЯ
// =============================================================

// --- E.1. Maximum Bipartite Matching через max-flow ---
// adj — список смежности из L (0..n-1) в R (0..m-1).
// O(VE²) время (через Edmonds-Karp) или O(E√V) (Dinic).
int bipartite_matching_via_flow(int n, int m,
    const vector<vector<int>>& adj) {
    int s = n + m, t = n + m + 1;
    int nodes = n + m + 2;
    vector<vector<Edge>> g(nodes);

    auto add_edge = [&](int u, int v, int c) {
        g[u].push_back({v, (int)g[v].size(), c, 0});
        g[v].push_back({u, (int)g[u].size() - 1, 0, 0});
    };

    // s → L (capacity 1)
    for (int i = 0; i < n; i++) add_edge(s, i, 1);
    // R → t (capacity 1)
    for (int j = 0; j < m; j++) add_edge(n + j, t, 1);
    // L → R (capacity 1)
    for (int i = 0; i < n; i++)
        for (int j : adj[i])
            add_edge(i, n + j, 1);

    return dinic(g, s, t);
}

}; // struct NetworkFlow

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_H_MAIN
int main() {
    NetworkFlow nf;

    cout << "=== Ford-Fulkerson ===" << endl;
    // Сеть: 0→1(10), 0→2(10), 1→2(2), 1→3(8), 2→3(10)
    auto g1 = nf.build_network(4, {
        {0,1,10},{0,2,10},{1,2,2},{1,3,8},{2,3,10}
    });
    int ff = nf.ford_fulkerson(g1, 0, 3);
    cout << "Max flow: " << ff << endl;

    cout << "\n=== Edmonds-Karp ===" << endl;
    auto g2 = nf.build_network(4, {
        {0,1,10},{0,2,10},{1,2,2},{1,3,8},{2,3,10}
    });
    int ek = nf.edmonds_karp(g2, 0, 3);
    cout << "Max flow: " << ek << endl;

    cout << "\n=== Dinic ===" << endl;
    auto g3 = nf.build_network(4, {
        {0,1,10},{0,2,10},{1,2,2},{1,3,8},{2,3,10}
    });
    int din = nf.dinic(g3, 0, 3);
    cout << "Max flow: " << din << endl;

    cout << "\n=== Min Cost Max Flow ===" << endl;
    // Сеть со стоимостями
    auto g4 = nf.build_network_cost(4, {
        {0,1,10,1},{0,2,10,2},{1,2,2,1},{1,3,8,3},{2,3,10,1}
    });
    auto [cost, flow] = nf.min_cost_max_flow(g4, 0, 3);
    cout << "Flow: " << flow << ", Cost: " << cost << endl;

    cout << "\n=== Bipartite Matching via Flow ===" << endl;
    vector<vector<int>> bip_adj = {{0,1},{1,2},{0}};
    int match = nf.bipartite_matching_via_flow(3, 3, bip_adj);
    cout << "Max bipartite matching: " << match << endl;

    return 0;
}
#endif

#endif // GRAPH_H_CPP
