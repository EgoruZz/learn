#ifndef GRAPH_L_CPP
#define GRAPH_L_CPP

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
using namespace std;

// =============================================================
// L. ДИНАМИЧЕСКИЕ И ОНЛАЙН АЛГОРИТМЫ
// =============================================================
// Структура md: A. Динамические графовые алгоритмы
//               → B. Онлайн алгоритмы
//
// DynamicGraphAlgorithms — наследует ProbabilisticGraph (k.cpp).
// Реализует: incremental/decremental connectivity (DSU, rollback DSU),
// dynamic MST, online matching (greedy).

#ifndef INSIDE_GRAPH_L
#define INSIDE_GRAPH_L
#include "../k/k.cpp"
#undef INSIDE_GRAPH_L
#endif

struct DynamicGraphAlgorithms : ProbabilisticGraph {

// =============================================================
// A. ДИНАМИЧЕСКАЯ СВЯЗНОСТЬ
// =============================================================

// --- A.1. Incremental Connectivity (DSU) ---
struct IncrementalConnectivity {
    vector<int> parent, rank_val;
    int components;

    IncrementalConnectivity(int n = 0) : parent(n), rank_val(n, 0), components(n) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    int find(int v) {
        if (parent[v] == v) return v;
        return parent[v] = find(parent[v]);
    }

    bool add_edge(int u, int v) {
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

// --- A.2. Rollback DSU (для offline decremental) ---
struct RollbackDSU {
    vector<int> parent, rank_val;
    int components;
    stack<tuple<int,int,int>> history;  // (child, parent, rank_change)

    RollbackDSU(int n = 0) : parent(n), rank_val(n, 0), components(n) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    int find(int v) {
        while (parent[v] != v) v = parent[v];
        return v;
    }

    bool unite(int u, int v) {
        u = find(u); v = find(v);
        if (u == v) { history.push({-1, -1, -1}); return false; }
        if (rank_val[u] < rank_val[v]) swap(u, v);
        history.push({v, parent[v], rank_val[u]});
        parent[v] = u;
        if (rank_val[u] == rank_val[v]) rank_val[u]++;
        components--;
        return true;
    }

    int snapshot() { return history.size(); }

    void rollback(int snap) {
        while ((int)history.size() > snap) {
            auto [child, par, rank_ch] = history.top();
            history.pop();
            if (child == -1) continue;
            parent[child] = par;
            rank_val[par] = rank_ch;
            components++;
        }
    }
};

// --- A.3. Offline Decremental Connectivity ---
// edges_to_remove — список рёбер, которые будут удалены по порядку.
// Возвращает: список размеров компонент после каждого удаления.
vector<int> offline_decremental(int n, vector<pair<int,int>> edges,
    const vector<pair<int,int>>& queries) {
    // Начинаем со всеми рёбрами, обрабатываем удаления в обратном порядке
    RollbackDSU dsu(n);
    set<pair<int,int>> active;
    for (auto& [u, v] : edges) {
        dsu.unite(min(u,v), max(u,v));
        active.insert({min(u,v), max(u,v)});
    }

    // Снимки перед каждым удалением
    vector<int> snapshots;
    for (auto& [u, v] : queries) {
        snapshots.push_back(dsu.snapshot());
        active.erase({min(u,v), max(u,v)});
    }

    // Откатываем удаления в обратном порядке
    vector<int> result;
    for (int i = (int)queries.size() - 1; i >= 0; i--) {
        auto [u, v] = queries[i];
        dsu.rollback(snapshots[i]);
        // После отката — ребро (u,v) снова активно
        dsu.unite(min(u,v), max(u,v));
    }

    // Вычисляем финальные размеры компонент
    vector<int> comp_size(n, 0);
    for (int i = 0; i < n; i++)
        comp_size[dsu.find(i)]++;
    for (int i = 0; i < n; i++)
        if (dsu.find(i) == i) result.push_back(comp_size[i]);

    return result;
}

// =============================================================
// B. ДИНАМИЧЕСКОЕ MST
// =============================================================

// --- B.1. Dynamic MST (упрощённый) ---
// Инкрементальное MST: при добавлении ребра — проверяем, улучшает ли оно MST.
// Для удаления — полный пересчёт через Kruskal.
// Здесь: упрощённая обёртка, хранящая все рёбра и пересчитывающая MST.
struct DynamicMST {
    int n;
    SpanningTrees solver;
    vector<tuple<int,int,int>> all_edges;

    DynamicMST(int n) : n(n), solver() {}

    // Добавление ребра + пересчёт MST.
    pair<vector<tuple<int,int,int>>, long long> add_edge(
        int u, int v, int w) {
        all_edges.push_back({u, v, w});
        auto result = solver.kruskal(n, all_edges);
        return {result.mst_edges, result.total_weight};
    }

    // Удаление ребра + пересчёт MST.
    pair<vector<tuple<int,int,int>>, long long> remove_edge(
        int u, int v) {
        auto it = find_if(all_edges.begin(), all_edges.end(),
            [&](const auto& e) {
                return (get<0>(e) == u && get<1>(e) == v) ||
                       (get<0>(e) == v && get<1>(e) == u);
            });
        if (it != all_edges.end()) all_edges.erase(it);
        auto result = solver.kruskal(n, all_edges);
        return {result.mst_edges, result.total_weight};
    }
};

// =============================================================
// C. ОНЛАЙН ПАРОСОЧЕТАНИЯ
// =============================================================

// --- C.1. Online Matching (Greedy) ---
// Вершины из L появляются по одной; при появлении — связываем
// с первым доступным соседом из R.
// Возвращает: размер паросочетания.
// Competitive ratio: 1/2.
struct OnlineMatching {
    int n, m;
    vector<int> match_r;  // match_r[j] = вершина из L
    int size;

    OnlineMatching(int n, int m) : n(n), m(m), match_r(m, -1), size(0) {}

    // Вершина u ∈ L появилась с набором соседей adj[u].
    // Жадно связываем с первым свободным соседом.
    bool add_vertex(int u, const vector<int>& neighbors) {
        for (int v : neighbors) {
            if (match_r[v] == -1) {
                match_r[v] = u;
                size++;
                return true;
            }
        }
        return false;
    }

    int get_size() const { return size; }
};

// =============================================================
// D. COMPETITIVE ANALYSIS
// =============================================================

// Competitive ratio для greedy online matching на произвольных графах.
// Гарантия: |M_online| ≥ (1/2) · |M_optimal|.
// Доказательство: каждое ребро в offline-optimal покрывает ≥ 1 вершину,
// которая могла бы быть сопоставлена в online.
double greedy_competitive_ratio() { return 0.5; }

}; // struct DynamicGraphAlgorithms

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_L_MAIN
int main() {
    DynamicGraphAlgorithms dla;

    cout << "=== Incremental Connectivity ===" << endl;
    DynamicGraphAlgorithms::IncrementalConnectivity ic(6);
    ic.add_edge(0, 1); ic.add_edge(1, 2); ic.add_edge(3, 4);
    cout << "0-2 connected: " << ic.connected(0, 2) << endl;
    cout << "0-3 connected: " << ic.connected(0, 3) << endl;
    ic.add_edge(2, 5); ic.add_edge(4, 5);
    cout << "0-5 connected: " << ic.connected(0, 5) << endl;
    cout << "Components: " << ic.components << endl;

    cout << "\n=== Rollback DSU ===" << endl;
    DynamicGraphAlgorithms::RollbackDSU rdsu(5);
    rdsu.unite(0, 1); rdsu.unite(2, 3);
    int snap = rdsu.snapshot();
    rdsu.unite(1, 4);
    cout << "Before rollback: components=" << rdsu.components << endl;
    rdsu.rollback(snap);
    cout << "After rollback: components=" << rdsu.components << endl;

    cout << "\n=== Online Matching ===" << endl;
    DynamicGraphAlgorithms::OnlineMatching om(3, 3);
    om.add_vertex(0, {0, 1});
    om.add_vertex(1, {1, 2});
    om.add_vertex(2, {0});
    cout << "Online matching size: " << om.get_size() << endl;

    cout << "\n=== Competitive Ratio ===" << endl;
    cout << "Greedy ratio: " << dla.greedy_competitive_ratio() << endl;

    return 0;
}
#endif

#endif // GRAPH_L_CPP
