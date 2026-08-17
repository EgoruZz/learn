#ifndef GRAPH_D_CPP
#define GRAPH_D_CPP

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <set>
#include <numeric>
#include <cmath>
using namespace std;

// =============================================================
// D. ОСТОВНЫЕ ДЕРЕВЬЯ (SPANNING TREES)
// =============================================================
// Структура md: A. Минимальное остовное дерево (MST)
//               → B. Второе по минимальности остовное дерево
//               → C. Теоретические основы (Кирхгоф, Прюфер, Кэли)
//               → D. Обратные задачи и оптимизации
//
// SpanningTrees — наследует ShortestPaths (c.cpp).
// Реализует: Kruskal, Prim, Boruvka, матрицу Кирхгофа,
// код Прюфера (кодирование/декодирование).

#ifndef INSIDE_GRAPH_D
#define INSIDE_GRAPH_D
#include "../c/c.cpp"
#undef INSIDE_GRAPH_D
#endif

struct SpanningTrees : ShortestPaths {

// =============================================================
// A. МИНИМАЛЬНОЕ ОСТОВНОЕ ДЕРЕВО (MST)
// =============================================================

// --- A.1. Алгоритм Крускала (Kruskal) ---
// edges — список рёбер (u, v, w); n — число вершин.
// Возвращает: MST как список рёбер + суммарный вес.
// O(m log m) время (сортировка), O(n + m) память.
struct KruskalResult {
    vector<tuple<int,int,int>> mst_edges;
    long long total_weight;
};

KruskalResult kruskal(int n, vector<tuple<int,int,int>> edges) {
    // Сортировка рёбер по весу
    sort(edges.begin(), edges.end(),
         [](const auto& a, const auto& b) {
             return get<2>(a) < get<2>(b);
         });

    DSU dsu(n);
    KruskalResult result;
    result.total_weight = 0;

    for (auto& [u, v, w] : edges) {
        if (dsu.unite(u, v)) {
            result.mst_edges.push_back({u, v, w});
            result.total_weight += w;
            if ((int)result.mst_edges.size() == n - 1) break;
        }
    }

    return result;
}

// --- A.2. Алгоритм Прима (Prim) ---
// adj — взвешенный список смежности.
// Возвращает: MST как список рёбер + суммарный вес.
// O((n + m) log n) время с binary heap.
KruskalResult prim(int n, const vector<vector<pair<int,int>>>& adj) {
    KruskalResult result;
    result.total_weight = 0;
    vector<bool> visited(n, false);
    // PQ: (вес_рёбра, from, to)
    priority_queue<tuple<int,int,int>, vector<tuple<int,int,int>>,
                   greater<tuple<int,int,int>>> pq;

    // Начинаем с вершины 0
    visited[0] = true;
    for (auto& [u, w] : adj[0])
        pq.push({w, 0, u});

    while (!pq.empty() && (int)result.mst_edges.size() < n - 1) {
        auto [w, from, to] = pq.top(); pq.pop();
        if (visited[to]) continue;
        visited[to] = true;
        result.mst_edges.push_back({from, to, w});
        result.total_weight += w;
        for (auto& [u, w2] : adj[to]) {
            if (!visited[u])
                pq.push({w2, to, u});
        }
    }

    return result;
}

// --- A.3. Алгоритм Борувки (Boruvka) ---
// edges — список рёбер (u, v, w); n — число вершин.
// O(m log n) время.
KruskalResult boruvka(int n, const vector<tuple<int,int,int>>& edges) {
    DSU dsu(n);
    KruskalResult result;
    result.total_weight = 0;

    while (dsu.components > 1) {
        // Для каждой компоненты: минимальное исходящее ребро
        vector<tuple<int,int,int>> best(n, {-1, -1, INT_MAX});
        // best[comp] = (u, v, w) — лучшее ребро из comp

        for (auto& [u, v, w] : edges) {
            int cu = dsu.find(u), cv = dsu.find(v);
            if (cu == cv) continue;
            if (get<2>(best[cu]) > w)
                best[cu] = {u, v, w};
            if (get<2>(best[cv]) > w)
                best[cv] = {u, v, w};
        }

        bool merged = false;
        for (int i = 0; i < n; i++) {
            auto [u, v, w] = best[i];
            if (u == -1) continue;
            if (dsu.unite(u, v)) {
                result.mst_edges.push_back({u, v, w});
                result.total_weight += w;
                merged = true;
            }
        }

        if (!merged) break;  // граф не связен
    }

    return result;
}

// =============================================================
// B. ВТОРОЕ ПО МИНИМАЛЬНОСТИ ОСТОВНОЕ ДЕРЕВО
// =============================================================

// --- B.1. Second-best MST ---
// Находит максимальное ребро на пути от u до v в дереве (MST).
// mst_adj — список смежности MST.
// O(n²) предподсчёт, O(1) запрос через brute-force для малых n.
// Для больших n — LCA + Sparse Table (struct.md).
int max_on_path_bruteforce(int u, int v, int n,
                           const vector<vector<pair<int,int>>>& mst_adj,
                           const vector<int>& parent,
                           const vector<int>& depth) {
    // Поднимаем глубокую вершину наверх, запоминая максимум
    int max_w = 0;
    if (depth[u] < depth[v]) swap(u, v);
    // Поднимаем u до глубины v
    while (depth[u] > depth[v]) {
        int p = parent[u];
        for (auto& [to, w] : mst_adj[u])
            if (to == p) { max_w = max(max_w, w); break; }
        u = p;
    }
    // Поднимаем обе до LCA
    while (u != v) {
        int pu = parent[u], pv = parent[v];
        for (auto& [to, w] : mst_adj[u])
            if (to == pu) { max_w = max(max_w, w); break; }
        for (auto& [to, w] : mst_adj[v])
            if (to == pv) { max_w = max(max_w, w); break; }
        u = pu; v = pv;
    }
    return max_w;
}

// Второе по минимальности MST: для каждого не-MST-ребра
// проверяем замену максимального ребра на пути.
// O(n·m) для brute-force (n запросов max_on_path на m рёбер).
pair<long long, tuple<int,int,int>> second_best_mst(
    int n, const vector<tuple<int,int,int>>& mst_edges,
    const vector<tuple<int,int,int>>& all_edges) {
    // Строим MST adjacency list
    vector<vector<pair<int,int>>> mst_adj(n);
    long long mst_weight = 0;
    for (auto& [u, v, w] : mst_edges) {
        mst_adj[u].push_back({v, w});
        mst_adj[v].push_back({u, w});
        mst_weight += w;
    }

    // BFS для определения depth и parent
    vector<int> parent(n, -1), depth(n, 0);
    queue<int> q;
    q.push(0);
    parent[0] = -1;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (auto& [u, w] : mst_adj[v]) {
            if (u == parent[v]) continue;
            parent[u] = v;
            depth[u] = depth[v] + 1;
            q.push(u);
        }
    }

    // Проверяем каждое не-MST-ребро
    set<pair<int,int>> mst_set;
    for (auto& [u, v, w] : mst_edges) {
        mst_set.insert({min(u,v), max(u,v)});
    }

    long long best_diff = LLONG_MAX;
    tuple<int,int,int> best_edge = {-1, -1, -1};

    for (auto& [u, v, w] : all_edges) {
        if (mst_set.count({min(u,v), max(u,v)})) continue;
        int max_w = max_on_path_bruteforce(u, v, n, mst_adj, parent, depth);
        long long diff = (long long)w - max_w;
        if (diff > 0 && diff < best_diff) {
            best_diff = diff;
            best_edge = {u, v, w};
        }
    }

    return {mst_weight + best_diff, best_edge};
}

// =============================================================
// C. ТЕОРЕТИЧЕСКИЕ ОСНОВЫ
// =============================================================

// --- C.1. Матрица Кирхгофа: число остовных деревьев ---
// Строит Laplacian L = D − A, удаляет строку и столбец,
// вычисляет определитель через Гаусс (modular или double).
// Для целочисленных графов: O(n³).
long long kirchhoff_tree_count(int n, const vector<pair<int,int>>& edges) {
    // Laplacian matrix
    vector<vector<long long>> L(n, vector<long long>(n, 0));
    for (int i = 0; i < n; i++) L[i][i] = 0;
    for (auto& [u, v] : edges) {
        L[u][u]++; L[v][v]++;
        L[u][v]--; L[v][u]--;
    }

    // Удаляем последнюю строку и столбец → матрица (n-1)×(n-1)
    int m = n - 1;
    vector<vector<long long>> A(m, vector<long long>(m));
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            A[i][j] = L[i][j];

    // Определитель через Гаусс (приведение к верхнетреугольной)
    long long det = 1;
    for (int i = 0; i < m; i++) {
        // Ищем ведущий элемент
        int pivot = -1;
        for (int j = i; j < m; j++) {
            if (A[j][i] != 0) { pivot = j; break; }
        }
        if (pivot == -1) return 0;  // det = 0
        if (pivot != i) {
            swap(A[i], A[pivot]);
            det = -det;
        }
        det *= A[i][i];
        if (A[i][i] == 0) return 0;
        // Нормализуем строку i (необязательно для det, но нужно для обнуления)
        long long inv = A[i][i];  // для целых: расширенный Гаусс
        for (int j = i + 1; j < m; j++) {
            if (A[j][i] == 0) continue;
            long long factor = A[j][i];
            for (int k = i; k < m; k++) {
                A[j][k] = A[j][k] * A[i][i] - A[i][k] * factor;
            }
        }
    }

    // Исправление: Гаусс для целых через деление
    // Пересчитаем через double для простоты
    vector<vector<double>> Ad(m, vector<double>(m));
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            Ad[i][j] = (double)A[i][j];

    double det_d = 1;
    for (int i = 0; i < m; i++) {
        int pivot = i;
        for (int j = i + 1; j < m; j++)
            if (abs(Ad[j][i]) > abs(Ad[pivot][i])) pivot = j;
        if (pivot != i) { swap(Ad[i], Ad[pivot]); det_d = -det_d; }
        if (abs(Ad[i][i]) < 1e-12) return 0;
        det_d *= Ad[i][i];
        for (int j = i + 1; j < m; j++) {
            double f = Ad[j][i] / Ad[i][i];
            for (int k = i; k < m; k++)
                Ad[j][k] -= f * Ad[i][k];
        }
    }

    return (long long)round(abs(det_d));
}

// --- C.2. Код Прюфера: дерево → последовательность ---
// Возвращает последовательность Прюфера длины n−2.
// O(n log n) время через set.
vector<int> prufer_encode(int n, const vector<pair<int,int>>& tree_edges) {
    vector<vector<int>> adj(n);
    for (auto& [u, v] : tree_edges) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // Множество листьев (sorted)
    set<int> leaves;
    vector<int> degree(n);
    for (int i = 0; i < n; i++) {
        degree[i] = adj[i].size();
        if (degree[i] == 1) leaves.insert(i);
    }

    vector<int> code;
    vector<bool> removed(n, false);

    for (int step = 0; step < n - 2; step++) {
        // Берём лист с наименьшей маркировкой
        int leaf = *leaves.begin();
        leaves.erase(leaves.begin());
        removed[leaf] = true;

        // Находим его соседа (единственный не удалённый)
        int neighbor = -1;
        for (int u : adj[leaf]) {
            if (!removed[u]) { neighbor = u; break; }
        }

        code.push_back(neighbor);
        degree[neighbor]--;

        if (degree[neighbor] == 1)
            leaves.insert(neighbor);
    }

    return code;
}

// --- C.3. Код Прюфера: последовательность → дерево ---
// Возвращает список рёбер дерева.
// O(n log n) время через priority queue.
vector<pair<int,int>> prufer_decode(const vector<int>& code) {
    int n = code.size() + 2;
    vector<int> degree(n, 1);
    for (int x : code) degree[x]++;

    priority_queue<int, vector<int>, greater<int>> leaves;
    for (int i = 0; i < n; i++)
        if (degree[i] == 1) leaves.push(i);

    vector<pair<int,int>> edges;
    for (int x : code) {
        int leaf = leaves.top(); leaves.pop();
        edges.push_back({leaf, x});
        degree[x]--;
        if (degree[x] == 1) leaves.push(x);
    }

    // Последние две вершины
    int a = leaves.top(); leaves.pop();
    int b = leaves.top(); leaves.pop();
    edges.push_back({a, b});

    return edges;
}

}; // struct SpanningTrees

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_D_MAIN
int main() {
    SpanningTrees st;

    int n = 5;
    vector<tuple<int,int,int>> edges = {
        {0,1,2},{0,3,6},{1,2,3},{1,3,8},{1,4,5},{2,4,7},{3,4,9}
    };

    cout << "=== Kruskal ===" << endl;
    auto kr = st.kruskal(n, edges);
    for (auto& [u, v, w] : kr.mst_edges)
        cout << u << " - " << v << " : " << w << endl;
    cout << "Total: " << kr.total_weight << endl;

    cout << "\n=== Prim ===" << endl;
    vector<vector<pair<int,int>>> adj(n);
    for (auto& [u, v, w] : edges) {
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }
    auto pr = st.prim(n, adj);
    for (auto& [u, v, w] : pr.mst_edges)
        cout << u << " - " << v << " : " << w << endl;
    cout << "Total: " << pr.total_weight << endl;

    cout << "\n=== Boruvka ===" << endl;
    auto br = st.boruvka(n, edges);
    for (auto& [u, v, w] : br.mst_edges)
        cout << u << " - " << v << " : " << w << endl;
    cout << "Total: " << br.total_weight << endl;

    cout << "\n=== Second-best MST ===" << endl;
    auto [sb_weight, sb_edge] = st.second_best_mst(n, kr.mst_edges, edges);
    cout << "Second-best weight: " << sb_weight << endl;
    auto [u, v, w] = sb_edge;
    cout << "Replace with: " << u << " - " << v << " : " << w << endl;

    cout << "\n=== Kirchhoff ===" << endl;
    vector<pair<int,int>> und_edges;
    for (auto& [u, v, w] : edges) und_edges.push_back({u, v});
    long long trees = st.kirchhoff_tree_count(n, und_edges);
    cout << "Number of spanning trees: " << trees << endl;

    cout << "\n=== Prufer ===" << endl;
    vector<pair<int,int>> tree_edges = {{0,1},{1,2},{1,3},{3,4}};
    auto code = st.prufer_encode(n, tree_edges);
    cout << "Code: ";
    for (int x : code) cout << x << " ";
    cout << endl;

    auto decoded = st.prufer_decode(code);
    cout << "Decoded edges: ";
    for (auto& [u, v] : decoded)
        cout << "(" << u << "," << v << ") ";
    cout << endl;

    cout << "Cayley check: " << n << "^" << n-2 << " = "
         << (long long)pow(n, n-2) << endl;

    return 0;
}
#endif

#endif // GRAPH_D_CPP
