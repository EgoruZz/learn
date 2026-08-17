#ifndef GRAPH_C_CPP
#define GRAPH_C_CPP

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#include <climits>
#include <set>
#include <cmath>
using namespace std;

// =============================================================
// C. КРАТЧАЙШИЕ ПУТИ В ГРАФАХ
// =============================================================
// Структура md: A. Одноисточниковые кратчайшие пути (SSSP)
//               → B. Кратчайшие пути между всеми парами (APSP)
//               → C. Эвристические алгоритмы поиска пути
//               → D. Специальные виды путей
//
// ShortestPaths — наследует ConnectivityAlgorithms (b.cpp).
// Реализует: Dijkstra, Bellman-Ford, SPFA, DAG shortest path,
// Floyd-Warshall, Johnson, A* search.

#ifndef INSIDE_GRAPH_C
#define INSIDE_GRAPH_C
#include "../b/b.cpp"
#undef INSIDE_GRAPH_C
#endif

struct ShortestPaths : ConnectivityAlgorithms {

// =============================================================
// A. ОДНОИСТОЧНИКОВЫЕ КРАТЧАЙШИЕ ПУТИ (SSSP)
// =============================================================

// --- A.1. Dijkstra (binary heap, без decrease-key) ---
// adj — взвешенный список смежности: adj[v] = {(u, w), ...}.
// s — исток. dist[v] = кратчайшее расстояние, prev[v] — предок.
// O((n + m) log n) время, O(n + m) память.
void dijkstra(int s, const vector<vector<pair<int,int>>>& adj,
              vector<int>& dist, vector<int>& prev) {
    int n = adj.size();
    dist.assign(n, INT_MAX);
    prev.assign(n, -1);
    dist[s] = 0;
    // PQ: (расстояние, вершина); без decrease-key — вставляем дубликаты
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    pq.push({0, s});

    while (!pq.empty()) {
        auto [d, v] = pq.top(); pq.pop();
        if (d > dist[v]) continue;  // устаревшая запись
        for (auto& [u, w] : adj[v]) {
            if (dist[u] > dist[v] + w) {
                dist[u] = dist[v] + w;
                prev[u] = v;
                pq.push({dist[u], u});
            }
        }
    }
}

// --- A.2. Dijkstra с decrease-key (через позиционный массив) ---
// Требует: priority queue с поддержкой decrease-key (или indexed pq).
// Здесь: упрощённый вариант через set (аналог decrease-key).
// O((n + m) log n) время.
void dijkstra_decrease_key(int s, const vector<vector<pair<int,int>>>& adj,
                           vector<int>& dist, vector<int>& prev) {
    int n = adj.size();
    dist.assign(n, INT_MAX);
    prev.assign(n, -1);
    dist[s] = 0;
    // set как PriorityQueue с decrease-key: удаляем старую запись, вставляем новую
    set<pair<int,int>> pq;
    pq.insert({0, s});

    while (!pq.empty()) {
        auto [d, v] = *pq.begin(); pq.erase(pq.begin());
        if (d > dist[v]) continue;
        for (auto& [u, w] : adj[v]) {
            if (dist[u] > dist[v] + w) {
                if (dist[u] != INT_MAX)
                    pq.erase({dist[u], u});
                dist[u] = dist[v] + w;
                prev[u] = v;
                pq.insert({dist[u], u});
            }
        }
    }
}

// --- A.3. Bellman-Ford ---
// Произвольные веса, обнаружение отрицательных циклов.
// edges — список рёбер (u, v, w); n — число вершин.
// dist[v] = кратчайшее расстояние от s; prev[v] — предок.
// Возвращает false если достижим отрицательный цикл.
bool bellman_ford(int n, int s, const vector<tuple<int,int,int>>& edges,
                  vector<int>& dist, vector<int>& prev) {
    dist.assign(n, INT_MAX);
    prev.assign(n, -1);
    dist[s] = 0;

    // n−1 проходов по всем рёбрам
    for (int i = 0; i < n - 1; i++) {
        bool updated = false;
        for (auto& [u, v, w] : edges) {
            if (dist[u] != INT_MAX && dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                updated = true;
            }
        }
        if (!updated) break;  // ранняя остановка
    }

    // Проверка на отрицательный цикл
    for (auto& [u, v, w] : edges) {
        if (dist[u] != INT_MAX && dist[v] > dist[u] + w)
            return false;  // отрицательный цикл
    }
    return true;
}

// --- A.4. SPFA (Shortest Path Faster Algorithm) ---
// Оптимизация Bellman-Ford: в очередь только изменённые вершины.
// В среднем O(m), худший O(nm).
// Через adjacency list.
bool spfa(int n, int s, const vector<vector<pair<int,int>>>& adj,
          vector<int>& dist, vector<int>& prev) {
    dist.assign(n, INT_MAX);
    prev.assign(n, -1);
    vector<int> count_in_queue(n, 0);
    vector<bool> in_queue(n, false);
    queue<int> q;
    dist[s] = 0;
    q.push(s);
    in_queue[s] = true;
    count_in_queue[s] = 1;

    while (!q.empty()) {
        int v = q.front(); q.pop();
        in_queue[v] = false;
        for (auto& [u, w] : adj[v]) {
            if (dist[u] > dist[v] + w) {
                dist[u] = dist[v] + w;
                prev[u] = v;
                if (!in_queue[u]) {
                    q.push(u);
                    in_queue[u] = true;
                    count_in_queue[u]++;
                    if (count_in_queue[u] > n) return false;  // отриц. цикл
                }
            }
        }
    }
    return true;
}

// --- A.5. DAG Shortest Path ---
// Топологическая сортировка + релаксация в топологическом порядке.
// O(n + m) время.
bool dag_shortest_path(int n, const vector<vector<pair<int,int>>>& adj,
                       int s, vector<int>& dist, vector<int>& prev) {
    // Топологическая сортировка (Kahn's algorithm из a.cpp)
    vector<int> in_degree(n, 0);
    for (int v = 0; v < n; v++)
        for (auto& [u, w] : adj[v])
            in_degree[u]++;

    queue<int> q;
    for (int i = 0; i < n; i++)
        if (in_degree[i] == 0) q.push(i);

    vector<int> topo;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        topo.push_back(v);
        for (auto& [u, w] : adj[v]) {
            in_degree[u]--;
            if (in_degree[u] == 0) q.push(u);
        }
    }

    if ((int)topo.size() != n) return false;  // цикл

    dist.assign(n, INT_MAX);
    prev.assign(n, -1);
    dist[s] = 0;

    for (int v : topo) {
        if (dist[v] == INT_MAX) continue;
        for (auto& [u, w] : adj[v]) {
            if (dist[u] > dist[v] + w) {
                dist[u] = dist[v] + w;
                prev[u] = v;
            }
        }
    }
    return true;
}

// =============================================================
// B. КРАТЧАЙШИЕ ПУТИ МЕЖДУ ВСЕМИ ПАРАМИ (APSP)
// =============================================================

// --- B.1. Floyd-Warshall ---
// dp[i][j] = кратчайшее расстояние из i в j.
// dp и next уже инициализированы (начальные расстояния).
// Восстановление пути через next[i][j].
// O(n³) время, O(n²) память.
void floyd_warshall(vector<vector<int>>& dp, vector<vector<int>>& next,
                    int n) {
    // k — промежуточная вершина (внешний цикл для оптимизации памяти)
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dp[i][k] == INT_MAX) continue;
            for (int j = 0; j < n; j++) {
                if (dp[k][j] == INT_MAX) continue;
                if (dp[i][j] > dp[i][k] + dp[k][j]) {
                    dp[i][j] = dp[i][k] + dp[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }
}

// Восстановление пути из i в j через next[].
vector<int> floyd_reconstruct_path(int i, int j, const vector<vector<int>>& next) {
    if (next[i][j] == -1) return {};
    vector<int> path = {i};
    while (i != j) {
        i = next[i][j];
        path.push_back(i);
    }
    return path;
}

// --- B.2. Floyd-Warshall для взвешенного графа (из adjacency list) ---
// Возвращает {dp, next}.
pair<vector<vector<int>>, vector<vector<int>>> floyd_from_adj(
    int n, const vector<vector<pair<int,int>>>& adj) {
    vector<vector<int>> dp(n, vector<int>(n, INT_MAX));
    vector<vector<int>> next(n, vector<int>(n, -1));
    for (int i = 0; i < n; i++) {
        dp[i][i] = 0;
        next[i][i] = i;
    }
    for (int v = 0; v < n; v++)
        for (auto& [u, w] : adj[v]) {
            dp[v][u] = w;
            next[v][u] = u;
        }
    floyd_warshall(dp, next, n);
    return {dp, next};
}

// --- B.3. Johnson's Algorithm ---
// Для разреженных графов с отрицательными весами.
// Возвращает {dp, next} или пустые векторы при отрицательном цикле.
pair<vector<vector<int>>, vector<vector<int>>> johnson(
    int n, const vector<vector<pair<int,int>>>& adj) {
    // Шаг 1: добавляем виртуальную вершину s (индекс n)
    // edges от s к каждой вершине с весом 0
    vector<int> h(n, 0);
    vector<int> dist_bf, prev_bf;
    vector<tuple<int,int,int>> edges_bf;
    for (int v = 0; v < n; v++)
        edges_bf.push_back({n, v, 0});  // s → v с весом 0
    for (int v = 0; v < n; v++)
        for (auto& [u, w] : adj[v])
            edges_bf.push_back({v, u, w});

    // Bellman-Ford от s
    vector<int> dist_s(n + 1, INT_MAX);
    dist_s[n] = 0;
    for (int i = 0; i < n; i++) {
        bool updated = false;
        for (auto& [u, v, w] : edges_bf) {
            if (dist_s[u] != INT_MAX && dist_s[v] > dist_s[u] + w) {
                dist_s[v] = dist_s[u] + w;
                updated = true;
            }
        }
        if (!updated) break;
    }
    // Проверка отрицательного цикла
    for (auto& [u, v, w] : edges_bf)
        if (dist_s[u] != INT_MAX && dist_s[v] > dist_s[u] + w)
            return {{}, {}};  // отрицательный цикл

    for (int i = 0; i < n; i++) h[i] = (dist_s[i] == INT_MAX) ? 0 : dist_s[i];

    // Шаг 2: перезахват весов w'(u,v) = w(u,v) + h(u) − h(v) ≥ 0
    vector<vector<pair<int,int>>> adj_reweighted(n);
    for (int v = 0; v < n; v++)
        for (auto& [u, w] : adj[v])
            adj_reweighted[v].push_back({u, w + h[v] - h[u]});

    // Шаг 3: n запусков Dijkstra
    vector<vector<int>> dp(n, vector<int>(n, INT_MAX));
    vector<vector<int>> next(n, vector<int>(n, -1));
    for (int s = 0; s < n; s++) {
        vector<int> dist, prev;
        dijkstra(s, adj_reweighted, dist, prev);
        for (int t = 0; t < n; t++) {
            if (dist[t] == INT_MAX) { dp[s][t] = INT_MAX; continue; }
            dp[s][t] = dist[t] - h[s] + h[t];
            // Восстановление next
            if (s == t) { next[s][t] = t; continue; }
            // Идём по prev от t к s
            vector<int> path;
            for (int v = t; v != -1; v = prev[v]) path.push_back(v);
            reverse(path.begin(), path.end());
            if (!path.empty() && path[0] == s) {
                next[s][t] = path.size() > 1 ? path[1] : t;
            }
        }
    }

    return {dp, next};
}

// =============================================================
// C. ЭВРИСТИЧЕСКИЕ АЛГОРИТМЫ ПОИСКА ПУТИ
// =============================================================

// --- C.1. A* Search Algorithm ---
// h(v) — эвристика (адмиссибли для оптимальности).
// Возвращает путь от start до goal или пустой вектор.
vector<int> a_star(int n, const vector<vector<pair<int,int>>>& adj,
                   int start, int goal,
                   const function<int(int)>& h) {
    vector<int> dist(n, INT_MAX), prev(n, -1);
    vector<bool> closed(n, false);
    dist[start] = 0;
    // PQ: (f = g + h, вершина)
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    pq.push({h(start), start});

    while (!pq.empty()) {
        auto [f, v] = pq.top(); pq.pop();
        if (v == goal) break;
        if (closed[v]) continue;
        closed[v] = true;

        for (auto& [u, w] : adj[v]) {
            int new_dist = dist[v] + w;
            if (new_dist < dist[u]) {
                dist[u] = new_dist;
                prev[u] = v;
                pq.push({new_dist + h(u), u});
            }
        }
    }

    if (dist[goal] == INT_MAX) return {};
    vector<int> path;
    for (int v = goal; v != -1; v = prev[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

// --- C.2. Greedy Best First Search ---
// f(v) = h(v); не оптимален, но быстр.
vector<int> greedy_best_first(int n, const vector<vector<pair<int,int>>>& adj,
                               int start, int goal,
                               const function<int(int)>& h) {
    vector<int> prev(n, -1);
    vector<bool> visited(n, false);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    pq.push({h(start), start});
    visited[start] = true;

    while (!pq.empty()) {
        auto [f, v] = pq.top(); pq.pop();
        if (v == goal) break;
        for (auto& [u, w] : adj[v]) {
            if (!visited[u]) {
                visited[u] = true;
                prev[u] = v;
                pq.push({h(u), u});
            }
        }
    }

    if (!visited[goal]) return {};
    vector<int> path;
    for (int v = goal; v != -1; v = prev[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

// --- C.3. Восстановление пути (общее) ---
vector<int> reconstruct_path(int s, int t, const vector<int>& prev) {
    vector<int> path;
    if (prev[t] == -1 && s != t) return {};
    for (int v = t; v != -1; v = prev[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

}; // struct ShortestPaths

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_C_MAIN
int main() {
    ShortestPaths sp;

    cout << "=== Dijkstra ===" << endl;
    int n = 5;
    vector<vector<pair<int,int>>> adj(n);
    vector<tuple<int,int,int>> edges = {
        {0,1,4},{0,2,1},{1,3,1},{2,1,2},{2,3,5},{3,4,3}
    };
    for (auto& [u, v, w] : edges) adj[u].push_back({v, w});

    vector<int> dist, prev;
    sp.dijkstra(0, adj, dist, prev);
    for (int i = 0; i < n; i++)
        cout << "dist(0," << i << ") = " << dist[i] << endl;

    cout << "\n=== Bellman-Ford ===" << endl;
    vector<int> dist_bf, prev_bf;
    bool no_neg = sp.bellman_ford(n, 0, edges, dist_bf, prev_bf);
    cout << "No negative cycle: " << (no_neg ? "YES" : "NO") << endl;
    for (int i = 0; i < n; i++)
        cout << "dist(0," << i << ") = " << dist_bf[i] << endl;

    cout << "\n=== DAG Shortest Path ===" << endl;
    vector<vector<pair<int,int>>> adj_dag(5);
    adj_dag[0].push_back({1, 2});
    adj_dag[0].push_back({2, 4});
    adj_dag[1].push_back({3, 1});
    adj_dag[2].push_back({3, 3});
    adj_dag[3].push_back({4, 2});
    vector<int> dist_dag, prev_dag;
    sp.dag_shortest_path(5, adj_dag, 0, dist_dag, prev_dag);
    for (int i = 0; i < 5; i++)
        cout << "dist(0," << i << ") = " << dist_dag[i] << endl;

    cout << "\n=== Floyd-Warshall ===" << endl;
    auto [dp, next] = sp.floyd_from_adj(n, adj);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (dp[i][j] == INT_MAX) cout << "INF ";
            else cout << dp[i][j] << "   ";
        }
        cout << endl;
    }
    auto path = sp.floyd_reconstruct_path(0, 4, next);
    cout << "Path 0->4: ";
    for (int v : path) cout << v << " ";
    cout << endl;

    cout << "\n=== A* ===" << endl;
    // Эвристика: h(v) = 0 (для теста = Dijkstra)
    auto h_zero = [](int v) { return 0; };
    auto astar_path = sp.a_star(n, adj, 0, 4, h_zero);
    cout << "Path 0->4: ";
    for (int v : astar_path) cout << v << " ";
    cout << endl;

    return 0;
}
#endif

#endif // GRAPH_C_CPP
