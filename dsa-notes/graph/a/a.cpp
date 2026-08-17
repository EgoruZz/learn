#ifndef GRAPH_A_CPP
#define GRAPH_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <climits>
using namespace std;

// =============================================================
// A. БАЗОВЫЕ ПОНЯТИЯ, ПРЕДСТАВЛЕНИЯ И ОБХОДЫ
// =============================================================
// Структура md: A. Определения и классификация графов
//               → B. Представления графов в памяти
//               → C. Поиск в глубину (DFS)
//               → D. Поиск в ширину (BFS)
//
// GraphBasics — базовый класс всей ветки graph. Собственной
// арифметики не имеет. Вводит представления графов, DFS, BFS и
// их фундаментальные приложения: компоненты связности, проверка
// на циклы, топологическая сортировка, двухцветность, копирование.
// Все последующие разделы (B–M) строятся на этих примитивах.
//
// Переиспользование из других веток:
//   (в первом разделе пока не требуется)

struct GraphBasics {

// =============================================================
// B. ПРЕДСТАВЛЕНИЯ ГРАФОВ В ПАМЯТИ
// =============================================================

// --- B.1. Построение списка смежности из списка рёбер ---
// edges — вектор пар (u, v); n — число вершин.
// Для взвешенных графов: vector<tuple<int,int,W>> + pair<int,W>.
// O(n + m) время, O(n + m) память.
vector<vector<int>> build_adj_list(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& [u, v] : edges) {
        adj[u].push_back(v);
        adj[v].push_back(u);  // неориентированный граф
    }
    return adj;
}

// Ориентированный вариант: только одно направление.
vector<vector<int>> build_adj_list_directed(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& [u, v] : edges) {
        adj[u].push_back(v);
    }
    return adj;
}

// --- B.2. Построение взвешенного списка смежности ---
// edges — вектор (u, v, w); n — число вершин.
// O(n + m) время, O(n + m) память.
vector<vector<pair<int,int>>> build_weighted_adj_list(
    int n, const vector<tuple<int,int,int>>& edges) {
    vector<vector<pair<int,int>>> adj(n);
    for (auto& [u, v, w] : edges) {
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }
    return adj;
}

// --- B.3. Построение матрицы смежности ---
// edges — вектор пар (u, v); n — число вершин; INF — «нет ребра».
// O(n² + m) время, O(n²) память.
vector<vector<int>> build_adj_matrix(int n, const vector<pair<int,int>>& edges,
                                     int INF = INT_MAX) {
    vector<vector<int>> mat(n, vector<int>(n, INF));
    for (int i = 0; i < n; i++) mat[i][i] = 0;
    for (auto& [u, v] : edges) {
        mat[u][v] = 1;
        mat[v][u] = 1;
    }
    return mat;
}

// --- B.4. Построение CSR (Compressed Sparse Row) ---
// edges — вектор пар (u, v); n — число вершин.
// offsets[v] — начало списка соседей v в массиве edges.
// O(n + m) время, O(n + m) память.
pair<vector<int>, vector<int>> build_csr(int n, const vector<pair<int,int>>& edges) {
    vector<int> degree(n, 0);
    for (auto& [u, v] : edges) {
        degree[u]++;
        degree[v]++;
    }
    vector<int> offsets(n + 1, 0);
    for (int i = 0; i < n; i++)
        offsets[i + 1] = offsets[i] + degree[i];
    vector<int> adj(offsets[n]);
    vector<int> pos = offsets;  // копия для заполнения
    for (auto& [u, v] : edges) {
        adj[pos[u]++] = v;
        adj[pos[v]++] = u;
    }
    return {offsets, adj};
}

// =============================================================
// C. ПОИСК В ГЛУБИНУ (DFS)
// =============================================================

// --- C.1. Рекурсивный DFS ---
// Обходит все вершины, достижимые из s; visited[v] = true при входе.
// O(n + m) время, O(n) память (visited + стек рекурсии).
void dfs_recursive(int v, const vector<vector<int>>& adj,
                   vector<bool>& visited) {
    visited[v] = true;
    for (int u : adj[v]) {
        if (!visited[u])
            dfs_recursive(u, adj, visited);
    }
}

// --- C.2. Итеративный DFS (через стек) ---
// Аналог рекурсивного, но без риска переполнения стека.
// O(n + m) время, O(n) память (стек + visited).
void dfs_iterative(int s, const vector<vector<int>>& adj,
                   vector<bool>& visited) {
    stack<int> st;
    st.push(s);
    while (!st.empty()) {
        int v = st.top(); st.pop();
        if (visited[v]) continue;
        visited[v] = true;
        for (int u : adj[v]) {
            if (!visited[u])
                st.push(u);
        }
    }
}

// --- C.3. DFS с метками времени (timestamping) ---
// tin[v] — время входа, tout[v] — время выхода.
// Глобальный таймер увеличивается при входе и при выходе.
// O(n + m) время, O(n) память.
int timer_dfs;
void dfs_timestamp(int v, const vector<vector<int>>& adj,
                   vector<bool>& visited,
                   vector<int>& tin, vector<int>& tout) {
    visited[v] = true;
    tin[v] = timer_dfs++;
    for (int u : adj[v]) {
        if (!visited[u])
            dfs_timestamp(u, adj, visited, tin, tout);
    }
    tout[v] = timer_dfs++;
}

// Проверка «v — предок u» за O(1).
bool is_ancestor(int v, int u, const vector<int>& tin,
                 const vector<int>& tout) {
    return tin[v] < tin[u] && tout[u] < tout[v];
}

// =============================================================
// C. ПРИМЕНЕНИЯ DFS
// =============================================================

// --- C.4. Проверка на циклы (неориентированный граф) ---
// Цикл ⟺ при DFS встречается посещённый сосед, не являющийся родителем.
// O(n + m) время.
bool has_cycle_undirected(int n, const vector<vector<int>>& adj) {
    vector<bool> visited(n, false);
    function<bool(int, int)> dfs = [&](int v, int parent) {
        visited[v] = true;
        for (int u : adj[v]) {
            if (!visited[u]) {
                if (dfs(u, v)) return true;
            } else if (u != parent) {
                return true;
            }
        }
        return false;
    };
    for (int i = 0; i < n; i++) {
        if (!visited[i] && dfs(i, -1))
            return true;
    }
    return false;
}

// --- C.5. Проверка на циклы (ориентированный граф) ---
// Цикл ⟺ обратное ребро (вершина в состоянии «в процессе»).
// Три состояния: 0 — не посещена, 1 — в процессе, 2 — завершена.
// O(n + m) время.
bool has_cycle_directed(int n, const vector<vector<int>>& adj) {
    vector<int> state(n, 0);  // 0=unvisited, 1=in-progress, 2=done
    function<bool(int)> dfs = [&](int v) {
        state[v] = 1;
        for (int u : adj[v]) {
            if (state[u] == 1) return true;   // обратное ребро → цикл
            if (state[u] == 0 && dfs(u)) return true;
        }
        state[v] = 2;
        return false;
    };
    for (int i = 0; i < n; i++) {
        if (state[i] == 0 && dfs(i))
            return true;
    }
    return false;
}

// --- C.6. Компоненты связности ---
// Число запусков DFS = число компонент.
// O(n + m) время, O(n) память.
int count_connected_components(int n, const vector<vector<int>>& adj) {
    vector<bool> visited(n, false);
    int components = 0;
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            dfs_recursive(i, adj, visited);
            components++;
        }
    }
    return components;
}

// Маркировка компонент: component_id[v] = номер компоненты (0..k-1).
vector<int> label_components(int n, const vector<vector<int>>& adj) {
    vector<bool> visited(n, false);
    vector<int> component_id(n, -1);
    int comp = 0;
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            function<void(int)> dfs = [&](int v) {
                visited[v] = true;
                component_id[v] = comp;
                for (int u : adj[v])
                    if (!visited[u]) dfs(u);
            };
            dfs(i);
            comp++;
        }
    }
    return component_id;
}

// --- C.7. Топологическая сортировка (DFS-based) ---
// Обратный порядок tout → топологический порядок DAG.
// Если обнаружено обратное ребро (цикл) → возвращает пустой вектор.
// O(n + m) время.
vector<int> topo_sort_dfs(int n, const vector<vector<int>>& adj) {
    vector<int> state(n, 0);  // 0=unvisited, 1=in-progress, 2=done
    vector<int> order;
    bool has_cycle = false;

    function<void(int)> dfs = [&](int v) {
        state[v] = 1;
        for (int u : adj[v]) {
            if (state[u] == 1) { has_cycle = true; return; }
            if (state[u] == 0) dfs(u);
        }
        state[v] = 2;
        order.push_back(v);
    };

    for (int i = 0; i < n; i++)
        if (state[i] == 0) dfs(i);

    if (has_cycle) return {};
    reverse(order.begin(), order.end());
    return order;
}

// --- C.8. Проверка двудольности (двухцветность через DFS) ---
// Граф двудолен ⟺ нет нечётных циклов ⟺ двухцветка без конфликтов.
// O(n + m) время.
bool is_bipartite(int n, const vector<vector<int>>& adj) {
    vector<int> color(n, -1);
    function<bool(int, int)> dfs = [&](int v, int c) {
        color[v] = c;
        for (int u : adj[v]) {
            if (color[u] == -1) {
                if (!dfs(u, 1 - c)) return false;
            } else if (color[u] == c) {
                return false;
            }
        }
        return true;
    };
    for (int i = 0; i < n; i++) {
        if (color[i] == -1 && !dfs(i, 0))
            return false;
    }
    return true;
}

// =============================================================
// D. ПОИСК В ШИРИНУ (BFS)
// =============================================================

// --- D.1. Базовый BFS + кратчайшие пути ---
// dist[v] = кратчайшее расстояние (число рёбер) от s до v.
// prev[v] = предыдущая вершина на пути (для восстановления).
// O(n + m) время, O(n) память.
void bfs_shortest_path(int s, const vector<vector<int>>& adj,
                        vector<int>& dist, vector<int>& prev) {
    int n = adj.size();
    dist.assign(n, INT_MAX);
    prev.assign(n, -1);
    queue<int> q;
    dist[s] = 0;
    q.push(s);
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (int u : adj[v]) {
            if (dist[u] == INT_MAX) {
                dist[u] = dist[v] + 1;
                prev[u] = v;
                q.push(u);
            }
        }
    }
}

// Восстановление пути из s в t через prev[].
vector<int> reconstruct_path(int s, int t, const vector<int>& prev) {
    vector<int> path;
    if (prev[t] == -1 && s != t) return path;  // путь не найден
    for (int v = t; v != -1; v = prev[v])
        path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

// --- D.2. Kahn's Algorithm (Topological Sort через BFS) ---
// O(n + m) время. Возвращает пустой вектор при цикле.
vector<int> topo_sort_kahn(int n, const vector<vector<int>>& adj) {
    vector<int> in_degree(n, 0);
    for (int v = 0; v < n; v++)
        for (int u : adj[v])
            in_degree[u]++;

    queue<int> q;
    for (int i = 0; i < n; i++)
        if (in_degree[i] == 0) q.push(i);

    vector<int> order;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        order.push_back(v);
        for (int u : adj[v]) {
            in_degree[u]--;
            if (in_degree[u] == 0) q.push(u);
        }
    }

    if ((int)order.size() != n) return {};  // цикл
    return order;
}

// --- D.3. Проверка двудольности через BFS ---
// Двухцветка по уровням: нечётные уровни — цвет 1, чётные — цвет 0.
// O(n + m) время.
bool is_bipartite_bfs(int n, const vector<vector<int>>& adj) {
    vector<int> color(n, -1);
    for (int start = 0; start < n; start++) {
        if (color[start] != -1) continue;
        queue<int> q;
        q.push(start);
        color[start] = 0;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int u : adj[v]) {
                if (color[u] == -1) {
                    color[u] = 1 - color[v];
                    q.push(u);
                } else if (color[u] == color[v]) {
                    return false;
                }
            }
        }
    }
    return true;
}

// --- D.4. Копирование графа (Deep Clone) ---
// Копирует граф, заданный через adjacency list, с созданием новой
// структуры. O(n + m) время, O(n) память (маппинг).
vector<vector<int>> clone_graph(int n, const vector<vector<int>>& adj) {
    vector<vector<int>> cloned(n);
    for (int v = 0; v < n; v++) {
        for (int u : adj[v]) {
            cloned[v].push_back(u);
        }
    }
    return cloned;
}

}; // struct GraphBasics

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef GRAPH_A_MAIN
int main() {
    GraphBasics gb;

    // Пример графа:
    // 0 — 1 — 2
    // |       |
    // 3 — 4 — 5
    //     |
    //     6
    int n = 7;
    vector<pair<int,int>> edges = {
        {0,1}, {1,2}, {0,3}, {3,4}, {4,5}, {2,5}, {4,6}
    };

    auto adj = gb.build_adj_list(n, edges);
    cout << "=== Adjacency List ===" << endl;
    for (int i = 0; i < n; i++) {
        cout << i << ": ";
        for (int u : adj[i]) cout << u << " ";
        cout << endl;
    }

    // DFS
    vector<bool> visited(n, false);
    cout << "\n=== DFS (recursive) from 0 ===" << endl;
    gb.dfs_recursive(0, adj, visited);
    for (int i = 0; i < n; i++)
        if (visited[i]) cout << i << " ";
    cout << endl;

    // Timestamps
    vector<int> tin(n), tout(n);
    fill(visited.begin(), visited.end(), false);
    gb.timer_dfs = 0;
    gb.dfs_timestamp(0, adj, visited, tin, tout);
    cout << "\n=== Timestamps ===" << endl;
    for (int i = 0; i < n; i++)
        cout << "v" << i << ": tin=" << tin[i] << " tout=" << tout[i] << endl;

    // Cycle check
    cout << "\n=== Cycle Check ===" << endl;
    cout << "Undirected: " << (gb.has_cycle_undirected(n, adj) ? "YES" : "NO") << endl;

    // Connected components
    cout << "\n=== Connected Components ===" << endl;
    cout << "Count: " << gb.count_connected_components(n, adj) << endl;
    auto comp = gb.label_components(n, adj);
    for (int i = 0; i < n; i++)
        cout << "v" << i << " -> component " << comp[i] << endl;

    // Topological sort (DAG)
    vector<pair<int,int>> dag_edges = {{0,1},{0,2},{1,3},{2,3},{3,4}};
    int n_dag = 5;
    auto adj_dag = gb.build_adj_list_directed(n_dag, dag_edges);
    auto topo = gb.topo_sort_dfs(n_dag, adj_dag);
    cout << "\n=== Topological Sort (DFS) ===" << endl;
    for (int v : topo) cout << v << " ";
    cout << endl;

    auto topo2 = gb.topo_sort_kahn(n_dag, adj_dag);
    cout << "Topological Sort (Kahn): ";
    for (int v : topo2) cout << v << " ";
    cout << endl;

    // Bipartite check
    cout << "\n=== Bipartite Check ===" << endl;
    cout << "Graph: " << (gb.is_bipartite(n, adj) ? "YES" : "NO") << endl;

    // BFS shortest path
    vector<int> dist, prev;
    gb.bfs_shortest_path(0, adj, dist, prev);
    cout << "\n=== BFS Shortest Path from 0 ===" << endl;
    for (int i = 0; i < n; i++)
        cout << "dist(0," << i << ") = " << dist[i] << endl;

    auto path = gb.reconstruct_path(0, 6, prev);
    cout << "Path 0->6: ";
    for (int v : path) cout << v << " ";
    cout << endl;

    return 0;
}
#endif

#endif // GRAPH_A_CPP
