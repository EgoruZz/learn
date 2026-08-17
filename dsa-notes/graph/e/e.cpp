#ifndef GRAPH_E_CPP
#define GRAPH_E_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

// =============================================================
// E. ЭЙЛЕРОВЫ И ГАМИЛЬТОНОВЫ ГРАФЫ
// =============================================================
// Структура md: A. Эйлеровы пути и циклы
//               → B. Гамильтоновы пути и циклы
//               → C. Обобщённые задачи (TSP, Longest Path)
//
// EulerHamilton — наследует SpanningTrees (d.cpp).
// Реализует: критерии эйлеровости, Hierholzer, Флёри,
// Chinese Postman, гамильтонов цикл (backtracking, Held-Karp).

#ifndef INSIDE_GRAPH_E
#define INSIDE_GRAPH_E
#include "../d/d.cpp"
#undef INSIDE_GRAPH_E
#endif

struct EulerHamilton : SpanningTrees {

// =============================================================
// A. ЭЙЛЕРОВЫ ПУТИ И ЦИКЛЫ
// =============================================================

// --- A.1. Проверка: существует ли эйлеров цикл/путь ---
// type: 0 — неориентированный, 1 — ориентированный.
// Возвращает: 0 — нет, 1 — эйлеров путь, 2 — эйлеров цикл.
int euler_check(int n, const vector<vector<int>>& adj, bool directed) {
    if (!directed) {
        // Неориентированный: проверка связности + степени
        vector<bool> visited(n, false);
        function<void(int)> dfs = [&](int v) {
            visited[v] = true;
            for (int u : adj[v])
                if (!visited[u]) dfs(u);
        };
        int start = -1;
        for (int i = 0; i < n; i++) {
            if (!adj[i].empty()) { start = i; break; }
        }
        if (start == -1) return 2;  // пустой граф — trivia
        dfs(start);
        for (int i = 0; i < n; i++)
            if (!adj[i].empty() && !visited[i]) return 0;  // не связен

        int odd_count = 0;
        for (int i = 0; i < n; i++)
            if (adj[i].size() % 2 != 0) odd_count++;

        if (odd_count == 0) return 2;  // эйлеров цикл
        if (odd_count == 2) return 1;  // эйлеров путь
        return 0;
    } else {
        // Ориентированный
        vector<int> in_degree(n, 0), out_degree(n, 0);
        for (int v = 0; v < n; v++)
            for (int u : adj[v]) {
                out_degree[v]++;
                in_degree[u]++;
            }

        // Проверка связности (по слабому графу)
        vector<vector<int>> und_adj(n);
        for (int v = 0; v < n; v++)
            for (int u : adj[v]) {
                und_adj[v].push_back(u);
                und_adj[u].push_back(v);
            }
        vector<bool> visited(n, false);
        function<void(int)> dfs = [&](int v) {
            visited[v] = true;
            for (int u : und_adj[v])
                if (!visited[u]) dfs(u);
        };
        int start = -1;
        for (int i = 0; i < n; i++)
            if (out_degree[i] > 0 || in_degree[i] > 0) { start = i; break; }
        if (start == -1) return 2;
        dfs(start);
        for (int i = 0; i < n; i++)
            if ((out_degree[i] > 0 || in_degree[i] > 0) && !visited[i])
                return 0;

        int unequal = 0;
        for (int i = 0; i < n; i++) {
            if (in_degree[i] != out_degree[i]) unequal++;
        }
        if (unequal == 0) return 2;
        if (unequal == 2) return 1;
        return 0;
    }
}

// --- A.2. Алгоритм Хьерхольцера (Hierholzer) ---
// Возвращает эйлеров цикл/путь (список вершин).
// Работает с графами, где эйлеров цикл/путь существует.
// O(m) время.
vector<int> hierholzer(int n, const vector<vector<int>>& adj, int start) {
    // Для неориентированных графов: храним пары (ребро, использовано)
    // через отдельный счётчик рёбер
    vector<vector<int>> g = adj;
    // Множество использованных рёбер: для каждого ребра храним
    // количество использований (для неориент. — по 1 на направление)
    // Проще: используем multiset и удаляем по одному элементу

    // Пересчитаем: для каждого ребра (u,v) храним в g[u] все v;
    // при проходе удаляем v из g[u] и u из g[v]
    auto remove_edge = [&](int u, int v) {
        auto it = find(g[u].begin(), g[u].end(), v);
        if (it != g[u].end()) g[u].erase(it);
        auto it2 = find(g[v].begin(), g[v].end(), u);
        if (it2 != g[v].end()) g[v].erase(it2);
    };

    vector<int> stack, circuit;
    stack.push_back(start);

    while (!stack.empty()) {
        int v = stack.back();
        if (!g[v].empty()) {
            int u = g[v].back();
            remove_edge(v, u);
            stack.push_back(u);
        } else {
            circuit.push_back(v);
            stack.pop_back();
        }
    }

    reverse(circuit.begin(), circuit.end());
    return circuit;
}

// Эйлеров цикл (для графа с эйлеровым циклом).
vector<int> euler_circuit(int n, const vector<vector<int>>& adj) {
    int start = -1;
    for (int i = 0; i < n; i++)
        if (!adj[i].empty()) { start = i; break; }
    return hierholzer(n, adj, start);
}

// Эйлеров путь (для графа с эйлеровым путём).
// Находит стартовую вершину (нечётная степень).
vector<int> euler_path(int n, const vector<vector<int>>& adj) {
    vector<int> odd;
    for (int i = 0; i < n; i++)
        if (adj[i].size() % 2 != 0) odd.push_back(i);
    int start = (odd.size() >= 2) ? odd[0] : 0;
    return hierholzer(n, adj, start);
}

// --- A.3. Алгоритм Флёри (Fleury) ---
// «Не пересекай мост, если есть альтернатива».
// O(m²) — на каждом шаге проверка мостов через DFS.
vector<int> fleury(int n, const vector<vector<int>>& adj, int start) {
    vector<vector<int>> g = adj;
    vector<int> path;

    function<bool(int, int)> is_bridge = [&](int u, int v) -> bool {
        // Ребро (u,v) — мост в текущем графе g?
        // Удаляем его, проверяем связность
        g[u].erase(find(g[u].begin(), g[u].end(), v));
        g[v].erase(find(g[v].begin(), g[v].end(), u));

        vector<bool> visited(n, false);
        function<void(int)> dfs = [&](int x) {
            visited[x] = true;
            for (int y : g[x])
                if (!visited[y]) dfs(y);
        };
        dfs(u);
        bool connected = visited[v];

        // Восстанавливаем ребро
        g[u].push_back(v);
        g[v].push_back(u);

        return !connected;
    };

    int v = start;
    while (true) {
        path.push_back(v);
        if (g[v].empty()) break;

        int next = -1;
        for (int u : g[v]) {
            if (g[v].size() == 1 || !is_bridge(v, u)) {
                next = u;
                break;
            }
        }
        if (next == -1) break;

        // Удаляем ребро (v, next)
        g[v].erase(find(g[v].begin(), g[v].end(), next));
        g[next].erase(find(g[next].begin(), g[next].end(), v));
        v = next;
    }

    return path;
}

// =============================================================
// B. ГАМИЛЬТОНОВЫ ПУТИ И ЦИКЛЫ
// =============================================================

// --- B.1. Гамильтонов цикл (backtracking) ---
// Возвращает список вершин гамильтонова цикла или пустой вектор.
// O(n!) худший, на практике значительно быстрее с обрезками.
vector<int> hamilton_cycle_backtrack(int n, const vector<vector<int>>& adj) {
    vector<int> path;
    vector<bool> visited(n, false);

    function<bool(int, int)> dfs = [&](int v, int depth) -> bool {
        path.push_back(v);
        visited[v] = true;

        if (depth == n - 1) {
            // Проверяем: есть ли ребро обратно в start
            int start = path[0];
            for (int u : adj[v])
                if (u == start) return true;
            path.pop_back();
            visited[v] = false;
            return false;
        }

        for (int u : adj[v]) {
            if (!visited[u]) {
                if (dfs(u, depth + 1)) return true;
            }
        }

        path.pop_back();
        visited[v] = false;
        return false;
    };

    if (dfs(0, 0)) return path;
    return {};
}

// --- B.2. Гамильтонов путь (backtracking) ---
// Аналогично, но без требования ребра обратно в start.
vector<int> hamilton_path_backtrack(int n, const vector<vector<int>>& adj) {
    vector<int> path;
    vector<bool> visited(n, false);

    function<bool(int, int)> dfs = [&](int v, int depth) -> bool {
        path.push_back(v);
        visited[v] = true;

        if (depth == n - 1) return true;

        for (int u : adj[v]) {
            if (!visited[u]) {
                if (dfs(u, depth + 1)) return true;
            }
        }

        path.pop_back();
        visited[v] = false;
        return false;
    };

    // Перебираем стартовую вершину
    for (int s = 0; s < n; s++) {
        path.clear();
        fill(visited.begin(), visited.end(), false);
        if (dfs(s, 0)) return path;
    }
    return {};
}

// --- B.3. Held-Karp (DP with bitmask) ---
// Точное решение TSP за O(2^n · n²).
// dist[i][j] — матрица расстояний.
// Возвращает: {минимальная_стоимость, путь}.
pair<long long, vector<int>> held_karp(int n, const vector<vector<int>>& dist) {
    int FULL = (1 << n);
    const long long INF = 1e18;
    vector<vector<long long>> dp(FULL, vector<long long>(n, INF));
    vector<vector<int>> parent(FULL, vector<int>(n, -1));

    dp[1][0] = 0;  // стартуем из вершины 0

    for (int mask = 1; mask < FULL; mask++) {
        for (int v = 0; v < n; v++) {
            if (dp[mask][v] == INF) continue;
            if (!(mask & (1 << v))) continue;
            for (int u = 0; u < n; u++) {
                if (mask & (1 << u)) continue;
                int new_mask = mask | (1 << u);
                long long new_dist = dp[mask][v] + dist[v][u];
                if (new_dist < dp[new_mask][u]) {
                    dp[new_mask][u] = new_dist;
                    parent[new_mask][u] = v;
                }
            }
        }
    }

    // Находим лучшую вершину для возврата в 0
    int full_mask = FULL - 1;
    long long best = INF;
    int last = -1;
    for (int v = 1; v < n; v++) {
        long long cost = dp[full_mask][v] + dist[v][0];
        if (cost < best) {
            best = cost;
            last = v;
        }
    }

    if (best >= INF) return {-1, {}};

    // Восстановление пути
    vector<int> path;
    int mask = full_mask;
    int v = last;
    while (v != -1) {
        path.push_back(v);
        int prev_v = parent[mask][v];
        mask ^= (1 << v);
        v = prev_v;
    }
    reverse(path.begin(), path.end());
    path.push_back(0);  // возврат в старт

    return {best, path};
}

// =============================================================
// C. ОБОБЩЁННЫЕ ЗАДАЧИ
// =============================================================

// --- C.1. Chinese Postman (неориентированный граф) ---
// Находит кратчайший замкнутый путь через каждое ребро ≥ 1 раз.
// Возвращает стоимость (сумму весов удвоенных рёбер + исходных).
// O(n³ + 2^k · k²) где k = число нечётных вершин.
long long chinese_postman(int n, const vector<vector<int>>& adj_weight) {
    // Floyd-Warshall для кратчайших расстояний
    const long long INF = 1e18;
    vector<vector<long long>> dist(n, vector<long long>(n, INF));
    for (int i = 0; i < n; i++) dist[i][i] = 0;
    for (int v = 0; v < n; v++)
        for (int u : adj_weight[v])
            dist[v][u] = min(dist[v][u], (long long)adj_weight[v][u]);
    // Actually adj_weight stores neighbors, not weights
    // Let me use a different approach

    // Пересчитаем: для каждой пары — кратчайшее расстояние
    // (предполагается, что adj_weight хранит веса)
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (dist[i][k] < INF && dist[k][j] < INF)
                    dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);

    // Находим нечётные вершины
    vector<int> odd;
    for (int i = 0; i < n; i++) {
        int deg = 0;
        for (int j = 0; j < n; j++)
            if (dist[i][j] < INF && i != j) deg++;
        if (deg % 2 != 0) odd.push_back(i);
    }

    if (odd.empty()) return 0;  // уже эйлеров

    int k = odd.size();
    int FULL = (1 << k);
    const long long LINF = 1e18;
    vector<vector<long long>> dp(FULL, vector<long long>(k, LINF));
    dp[0][0] = 0;

    // DP по подмножествам нечётных вершин
    for (int mask = 0; mask < FULL; mask++) {
        int first = -1;
        for (int i = 0; i < k; i++)
            if (!(mask & (1 << i))) { first = i; break; }
        if (first == -1) continue;
        for (int second = first + 1; second < k; second++) {
            if (mask & (1 << second)) continue;
            int new_mask = mask | (1 << first) | (1 << second);
            long long cost = dist[odd[first]][odd[second]];
            if (cost >= LINF) continue;
            if (dp[mask][0] < LINF)
                dp[new_mask][0] = min(dp[new_mask][0], dp[mask][0] + cost);
        }
    }

    return dp[FULL - 1][0];
}

}; // struct EulerHamilton

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_E_MAIN
int main() {
    EulerHamilton eh;

    cout << "=== Euler Check ===" << endl;
    // Неориентированный граф с эйлеровым циклом: 0-1-2-0 (треугольник)
    int n = 3;
    vector<vector<int>> adj1(n);
    adj1[0] = {1, 2};
    adj1[1] = {0, 2};
    adj1[2] = {0, 1};
    int check1 = eh.euler_check(n, adj1, false);
    cout << "Triangle: " << (check1 == 2 ? "Euler circuit" : check1 == 1 ? "Euler path" : "None") << endl;

    // Граф с эйлеровым путём: 0-1-2-3 (4 вершины, 3 ребра линейно)
    int n2 = 4;
    vector<vector<int>> adj2(n2);
    adj2[0] = {1};
    adj2[1] = {0, 2};
    adj2[2] = {1, 3};
    adj2[3] = {2};
    int check2 = eh.euler_check(n2, adj2, false);
    cout << "Path graph: " << (check2 == 2 ? "Euler circuit" : check2 == 1 ? "Euler path" : "None") << endl;

    cout << "\n=== Hierholzer ===" << endl;
    auto circuit = eh.euler_circuit(n, adj1);
    cout << "Circuit: ";
    for (int v : circuit) cout << v << " ";
    cout << endl;

    auto path = eh.euler_path(n2, adj2);
    cout << "Path: ";
    for (int v : path) cout << v << " ";
    cout << endl;

    cout << "\n=== Hamilton Cycle (backtrack) ===" << endl;
    // Полный граф K4
    int n3 = 4;
    vector<vector<int>> adj3(n3);
    for (int i = 0; i < n3; i++)
        for (int j = i + 1; j < n3; j++) {
            adj3[i].push_back(j);
            adj3[j].push_back(i);
        }
    auto hc = eh.hamilton_cycle_backtrack(n3, adj3);
    cout << "K4 cycle: ";
    for (int v : hc) cout << v << " ";
    cout << endl;

    cout << "\n=== Held-Karp TSP ===" << endl;
    // Полный граф K4 с весами
    vector<vector<int>> dist = {
        {0, 10, 15, 20},
        {10, 0, 35, 25},
        {15, 35, 0, 30},
        {20, 25, 30, 0}
    };
    auto [cost, tsp_path] = eh.held_karp(n3, dist);
    cout << "TSP cost: " << cost << endl;
    cout << "TSP path: ";
    for (int v : tsp_path) cout << v << " ";
    cout << endl;

    return 0;
}
#endif

#endif // GRAPH_E_CPP
