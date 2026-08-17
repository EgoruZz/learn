#ifndef GRAPH_B_CPP
#define GRAPH_B_CPP

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <functional>
#include <algorithm>
#include <climits>
using namespace std;

// =============================================================
// B. СВЯЗНОСТЬ В ГРАФАХ
// =============================================================
// Структура md: A. Компоненты связности
//               → B. Мосты и точки сочленения
//               → C. k-связность и минимальные разрезы
//               → D. Двудольные графы и раскраски
//               → E. Динамическая связность
//               → F. 2-SAT и конденсация графа импликаций
//
// ConnectivityAlgorithms — наследует GraphBasics (a.cpp).
// Вводит: Union-Find (упрощённый), SCC (Kosaraju + Tarjan),
// мосты и точки сочленения (Тарьян), biconnected components,
// Stoer-Wagner min-cut, 2-SAT через SCC.

#ifndef INSIDE_GRAPH_B
#define INSIDE_GRAPH_B
#include "../a/a.cpp"
#undef INSIDE_GRAPH_B
#endif

struct ConnectivityAlgorithms : GraphBasics {

// =============================================================
// A. КОМПОНЕНТЫ СВЯЗНОСТИ (через Union-Find)
// =============================================================

// --- A.1. Union-Find (DSU) — упрощённая версия ---
// find(v) — поиск корня с path compression.
// union(u, v) — объединение по рангу.
// O(α(n)) амортизированно на операцию.
struct DSU {
    vector<int> parent, rank_val;
    int components;

    DSU(int n = 0) : parent(n), rank_val(n, 0), components(n) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    int find(int v) {
        if (parent[v] == v) return v;
        return parent[v] = find(parent[v]);  // path compression
    }

    bool unite(int u, int v) {
        u = find(u); v = find(v);
        if (u == v) return false;
        if (rank_val[u] < rank_val[v]) swap(u, v);
        parent[v] = u;
        if (rank_val[u] == rank_val[v]) rank_val[u]++;
        components--;
        return true;
    }

    bool connected(int u, int v) { return find(u) == find(v); }
};

// --- A.2. Компоненты через DSU ---
// O(m · α(n)) суммарно.
int count_components_dsu(int n, const vector<pair<int,int>>& edges) {
    DSU dsu(n);
    for (auto& [u, v] : edges) dsu.unite(u, v);
    return dsu.components;
}

// =============================================================
// B. СИЛЬНЫЕ КОМПОНЕНТЫ СВЯЗНОСТИ (SCC)
// =============================================================

// --- B.1. Kosaraju's Algorithm (два DFS) ---
// Возвращает: scc_id[v] = номер SCC (0..scc_count-1).
// O(n + m) время, O(n + m) память.
pair<vector<int>, int> kosaraju_scc(int n, const vector<vector<int>>& adj) {
    // Шаг 1: DFS для определения порядка выхода
    vector<bool> visited(n, false);
    vector<int> order;
    function<void(int)> dfs1 = [&](int v) {
        visited[v] = true;
        for (int u : adj[v])
            if (!visited[u]) dfs1(u);
        order.push_back(v);
    };
    for (int i = 0; i < n; i++)
        if (!visited[i]) dfs1(i);

    // Шаг 2: построение обратного графа
    vector<vector<int>> radj(n);
    for (int v = 0; v < n; v++)
        for (int u : adj[v])
            radj[u].push_back(v);

    // Шаг 3: DFS по обратному графу в порядке убывания order
    fill(visited.begin(), visited.end(), false);
    vector<int> scc_id(n, -1);
    int scc_count = 0;
    function<void(int)> dfs2 = [&](int v) {
        visited[v] = true;
        scc_id[v] = scc_count;
        for (int u : radj[v])
            if (!visited[u]) dfs2(u);
    };
    for (int i = n - 1; i >= 0; i--) {
        int v = order[i];
        if (!visited[v]) {
            dfs2(v);
            scc_count++;
        }
    }

    return {scc_id, scc_count};
}

// --- B.2. Tarjan's Algorithm for SCC (один DFS) ---
// Возвращает: scc_id[v] = номер SCC (в обратном топологическом порядке).
// O(n + m) время.
pair<vector<int>, int> tarjan_scc(int n, const vector<vector<int>>& adj) {
    vector<int> tin(n, -1), low(n, -1);
    vector<int> st;           // стек для SCC
    vector<bool> on_st(n, false);
    vector<int> scc_id(n, -1);
    int timer = 0, scc_count = 0;

    function<void(int)> dfs = [&](int v) {
        tin[v] = low[v] = timer++;
        st.push_back(v);
        on_st[v] = true;

        for (int u : adj[v]) {
            if (tin[u] == -1) {
                dfs(u);
                low[v] = min(low[v], low[u]);
            } else if (on_st[u]) {
                low[v] = min(low[v], tin[u]);
            }
        }

        if (low[v] == tin[v]) {
            while (true) {
                int u = st.back(); st.pop_back();
                on_st[u] = false;
                scc_id[u] = scc_count;
                if (u == v) break;
            }
            scc_count++;
        }
    };

    for (int i = 0; i < n; i++)
        if (tin[i] == -1) dfs(i);

    return {scc_id, scc_count};
}

// --- B.3. Конденсация графа ---
// Возвращает: dag_adj (конденсация) + scc_id.
pair<vector<vector<int>>, vector<int>> condensation(
    int n, const vector<vector<int>>& adj) {
    auto [scc_id, scc_count] = tarjan_scc(n, adj);
    vector<vector<int>> dag_adj(scc_count);
    for (int v = 0; v < n; v++) {
        for (int u : adj[v]) {
            if (scc_id[v] != scc_id[u])
                dag_adj[scc_id[v]].push_back(scc_id[u]);
        }
    }
    // Удаление дубликатов рёбер
    for (int i = 0; i < scc_count; i++) {
        sort(dag_adj[i].begin(), dag_adj[i].end());
        dag_adj[i].erase(unique(dag_adj[i].begin(), dag_adj[i].end()),
                         dag_adj[i].end());
    }
    return {dag_adj, scc_id};
}

// =============================================================
// B. МОСТЫ И ТОЧКИ СОЧЛЕНИЯ
// =============================================================

// --- B.4. Мосты (bridges) через Тарьяна ---
// Возвращает список мостов как пар (u, v), u < v.
// O(n + m) время.
vector<pair<int,int>> find_bridges(int n, const vector<vector<int>>& adj) {
    vector<int> tin(n, -1), low(n, -1);
    int timer = 0;
    vector<pair<int,int>> bridges;

    function<void(int, int)> dfs = [&](int v, int p) {
        tin[v] = low[v] = timer++;
        for (int u : adj[v]) {
            if (u == p) continue;
            if (tin[u] == -1) {
                dfs(u, v);
                low[v] = min(low[v], low[u]);
                if (low[u] > tin[v]) {
                    bridges.push_back({min(v, u), max(v, u)});
                }
            } else {
                low[v] = min(low[v], tin[u]);
            }
        }
    };

    for (int i = 0; i < n; i++)
        if (tin[i] == -1) dfs(i, -1);

    return bridges;
}

// --- B.5. Точки сочленения (articulation points) ---
// Возвращает список точек сочленения.
// O(n + m) время.
vector<int> find_articulation_points(int n, const vector<vector<int>>& adj) {
    vector<int> tin(n, -1), low(n, -1);
    int timer = 0;
    vector<bool> is_ap(n, false);

    function<void(int, int)> dfs = [&](int v, int p) {
        tin[v] = low[v] = timer++;
        int children = 0;
        for (int u : adj[v]) {
            if (u == p) continue;
            if (tin[u] == -1) {
                children++;
                dfs(u, v);
                low[v] = min(low[v], low[u]);
                // Корень DFS-дерева
                if (p == -1 && children >= 2)
                    is_ap[v] = true;
                // Не корень
                if (p != -1 && low[u] >= tin[v])
                    is_ap[v] = true;
            } else {
                low[v] = min(low[v], tin[u]);
            }
        }
    };

    for (int i = 0; i < n; i++)
        if (tin[i] == -1) dfs(i, -1);

    vector<int> result;
    for (int i = 0; i < n; i++)
        if (is_ap[i]) result.push_back(i);
    return result;
}

// --- B.6. Biconnected Components ---
// Возвращает компоненты как список списков рёбер.
// O(n + m) время.
vector<vector<pair<int,int>>> biconnected_components(
    int n, const vector<vector<int>>& adj) {
    vector<int> tin(n, -1), low(n, -1);
    int timer = 0;
    vector<pair<int,int>> st;  // стек рёбер
    vector<vector<pair<int,int>>> components;

    function<void(int, int)> dfs = [&](int v, int p) {
        tin[v] = low[v] = timer++;
        int children = 0;
        for (int u : adj[v]) {
            if (u == p) continue;
            if (tin[u] == -1) {
                children++;
                st.push_back({v, u});
                dfs(u, v);
                low[v] = min(low[v], low[u]);
                if ((p == -1 && children >= 2) ||
                    (p != -1 && low[u] >= tin[v])) {
                    vector<pair<int,int>> comp;
                    while (true) {
                        auto e = st.back(); st.pop_back();
                        comp.push_back(e);
                        if (e == make_pair(v, u)) break;
                    }
                    components.push_back(comp);
                }
            } else if (tin[u] < tin[v] && u != p) {
                low[v] = min(low[v], tin[u]);
                st.push_back({v, u});
            }
        }
    };

    for (int i = 0; i < n; i++)
        if (tin[i] == -1) dfs(i, -1);

    // Оставшиеся рёбра на стеке — последняя компонента
    if (!st.empty()) {
        vector<pair<int,int>> comp;
        while (!st.empty()) { comp.push_back(st.back()); st.pop_back(); }
        components.push_back(comp);
    }

    return components;
}

// =============================================================
// C. MIN-CUT (Stoer-Wagner)
// =============================================================

// --- C.1. Stoer-Wagner: глобальный min-cut ---
// Возвращает стоимость min-cut и пару вершин (s, t).
// O(n³) время, O(n²) память.
pair<int, pair<int,int>> stoer_wagner(int n,
    const vector<vector<int>>& weight) {
    // weight[i][j] = вес ребра (i,j); 0 если ребра нет
    vector<vector<int>> w = weight;
    vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = i;

    int best_cut = INT_MAX;
    pair<int,int> best_pair = {0, 0};
    int remaining = n;

    while (remaining > 1) {
        // Phase: находим most tightly connected pair
        vector<bool> added(remaining, false);
        vector<int> prev(remaining, -1);
        vector<int> degree(remaining, 0);  // суммарные веса к добавленным

        int last = -1;
        for (int i = 0; i < remaining; i++) {
            // Выбираем вершину с макс. degree среди не добавленных
            int sel = -1;
            for (int j = 0; j < remaining; j++)
                if (!added[j] && (sel == -1 || degree[j] > degree[sel]))
                    sel = j;
            added[sel] = true;
            if (i == remaining - 1) {
                // Последняя добавленная = t; предпоследняя = s
                int s = prev[sel], t = sel;
                if (degree[sel] < best_cut) {
                    best_cut = degree[sel];
                    best_pair = {v[s], v[t]};
                }
                // Стягиваем s и t: s поглощает t
                for (int j = 0; j < remaining; j++) {
                    w[v[s]][v[j]] += w[v[t]][v[j]];
                    w[v[j]][v[s]] = w[v[s]][v[j]];
                }
                // Удаляем t из массива вершин
                v[t] = v[remaining - 1];
                remaining--;
                break;
            }
            last = sel;
            // Обновляем degree
            for (int j = 0; j < remaining; j++)
                if (!added[j])
                    degree[j] += w[v[sel]][v[j]];
            prev[sel] = last;
        }
    }

    return {best_cut, best_pair};
}

// =============================================================
// F. 2-SAT
// =============================================================

// --- F.1. 2-SAT через SCC ---
// n — число переменных; clauses — список пар (a, b), где
// a = (var_a, is_neg_a), b = (var_b, is_neg_b).
// (x ∨ y) где x = var_a (true) или ¬var_a (false).
// Возвращает: vector<bool> (оценка) или пустой вектор если UNSAT.
vector<bool> two_sat(int n, const vector<pair<pair<int,bool>,pair<int,bool>>>& clauses) {
    int nodes = 2 * n;  // xᵢ → индекс 2*i; ¬xᵢ → индекс 2*i+1
    vector<vector<int>> adj(nodes);

    auto lit_to_node = [](int var, bool is_neg, int n) -> int {
        // var: 0..n-1; is_neg: true если ¬x
        return 2 * var + (is_neg ? 1 : 0);
    };
    auto negate_node = [nodes](int node) -> int {
        return node ^ 1;  // xor с 1: 2i ↔ 2i+1
    };

    // Построение графа импликаций
    for (auto& [a, b] : clauses) {
        int na = lit_to_node(a.first, a.second, n);
        int nb = lit_to_node(b.first, b.second, n);
        // (a ∨ b) → (¬a → b) и (¬b → a)
        adj[negate_node(na)].push_back(nb);
        adj[negate_node(nb)].push_back(na);
    }

    // SCC через Tarjan
    auto [scc_id, scc_count] = tarjan_scc(nodes, adj);

    // Проверка: xᵢ и ¬xᵢ в одной компоненте → UNSAT
    vector<bool> result(n);
    for (int i = 0; i < n; i++) {
        if (scc_id[2 * i] == scc_id[2 * i + 1])
            return {};  // UNSAT
        // xᵢ = true если scc_id[xᵢ] < scc_id[¬xᵢ]
        result[i] = scc_id[2 * i] < scc_id[2 * i + 1];
    }
    return result;
}

}; // struct ConnectivityAlgorithms

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_B_MAIN
int main() {
    ConnectivityAlgorithms ca;

    cout << "=== SCC (Kosaraju) ===" << endl;
    int n = 8;
    vector<vector<int>> adj(n);
    // Пример: 0→1→2→0, 3→4→5→3, 2→3, 6→7
    vector<pair<int,int>> directed_edges = {
        {0,1},{1,2},{2,0},{3,4},{4,5},{5,3},{2,3},{6,7}
    };
    for (auto& [u, v] : directed_edges) adj[u].push_back(v);

    auto [scc_id, scc_count] = ca.kosaraju_scc(n, adj);
    cout << "SCC count: " << scc_count << endl;
    for (int i = 0; i < n; i++)
        cout << "v" << i << " -> SCC " << scc_id[i] << endl;

    cout << "\n=== SCC (Tarjan) ===" << endl;
    auto [scc_id2, scc_count2] = ca.tarjan_scc(n, adj);
    cout << "SCC count: " << scc_count2 << endl;

    cout << "\n=== Condensation ===" << endl;
    auto [dag_adj, dag_scc] = ca.condensation(n, adj);
    cout << "DAG vertices: " << dag_adj.size() << endl;
    for (int i = 0; i < (int)dag_adj.size(); i++) {
        cout << "SCC " << i << ": ";
        for (int u : dag_adj[i]) cout << u << " ";
        cout << endl;
    }

    cout << "\n=== Bridges ===" << endl;
    int n2 = 7;
    vector<vector<int>> adj2(n2);
    vector<pair<int,int>> und_edges = {{0,1},{1,2},{0,3},{3,4},{4,5},{2,5},{4,6}};
    for (auto& [u, v] : und_edges) { adj2[u].push_back(v); adj2[v].push_back(u); }
    auto bridges = ca.find_bridges(n2, adj2);
    for (auto& [u, v] : bridges)
        cout << u << " - " << v << endl;

    cout << "\n=== Articulation Points ===" << endl;
    auto aps = ca.find_articulation_points(n2, adj2);
    for (int v : aps) cout << v << " ";
    cout << endl;

    cout << "\n=== Biconnected Components ===" << endl;
    auto bcc = ca.biconnected_components(n2, adj2);
    for (int i = 0; i < (int)bcc.size(); i++) {
        cout << "BCC " << i << ": ";
        for (auto& [u, v] : bcc[i])
            cout << "(" << u << "," << v << ") ";
        cout << endl;
    }

    cout << "\n=== Stoer-Wagner Min-Cut ===" << endl;
    int n3 = 4;
    vector<vector<int>> w(n3, vector<int>(n3, 0));
    w[0][1] = 1; w[1][0] = 1;
    w[1][2] = 2; w[2][1] = 2;
    w[2][3] = 3; w[3][2] = 3;
    w[0][3] = 4; w[3][0] = 4;
    auto [cut_val, cut_pair] = ca.stoer_wagner(n3, w);
    cout << "Min-cut value: " << cut_val << endl;
    cout << "Cut: " << cut_pair.first << " - " << cut_pair.second << endl;

    cout << "\n=== 2-SAT ===" << endl;
    // (x0 ∨ x1) ∧ (¬x0 ∨ x1) ∧ (x0 ∨ ¬x1)
    // Ожидаем: x0 = true, x1 = true
    vector<pair<pair<int,bool>,pair<int,bool>>> clauses = {
        {{0, false}, {1, false}},  // x0 ∨ x1
        {{0, true},  {1, false}},  // ¬x0 ∨ x1
        {{0, false}, {1, true}}    // x0 ∨ ¬x1
    };
    auto sat_result = ca.two_sat(2, clauses);
    if (sat_result.empty()) {
        cout << "UNSAT" << endl;
    } else {
        for (int i = 0; i < (int)sat_result.size(); i++)
            cout << "x" << i << " = " << (sat_result[i] ? "true" : "false") << endl;
    }

    return 0;
}
#endif

#endif // GRAPH_B_CPP
