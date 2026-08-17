#ifndef GRAPH_J_CPP
#define GRAPH_J_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <queue>
using namespace std;

// =============================================================
// J. СПЕЦИАЛЬНЫЕ КЛАССЫ ГРАФОВ И АЛГОРИТМЫ
// =============================================================
// Структура md: A. Графы с ограниченными параметрами
//               → B. Гиперграфы и мультиграфы
//
// SpecialGraphs — наследует PlanarGraphs (i.cpp).
// Реализует: интервальные графы, графы перестановок,
// tree decomposition basics, гиперграфы (bipartite repr).

#ifndef INSIDE_GRAPH_J
#define INSIDE_GRAPH_J
#include "../i/i.cpp"
#undef INSIDE_GRAPH_J
#endif

struct SpecialGraphs : PlanarGraphs {

// =============================================================
// A. ИНТЕРВАЛЬНЫЕ ГРАФЫ
// =============================================================

// --- A.1. Maximum clique в интервальном графе ---
// intervals[i] = (left, right) — интервалы.
// Стратегия: sweep line — подсчёт максимального числа активных интервалов.
// O(n log n) время.
struct Interval { int l, r; };

int max_clique_interval(vector<Interval> intervals) {
    vector<pair<int,int>> events;  // (point, +1/-1)
    for (auto& iv : intervals) {
        events.push_back({iv.l, 1});
        events.push_back({iv.r, -1});
    }
    sort(events.begin(), events.end());
    int cur = 0, max_clique = 0;
    for (auto& [pt, delta] : events) {
        cur += delta;
        max_clique = max(max_clique, cur);
    }
    return max_clique;
}

// --- A.2. Maximum independent set в интервальном графе ---
// Жадный: сортировка по правому концу,贪心 выбор непересекающихся.
// O(n log n) время.
int max_independent_set_interval(vector<Interval> intervals) {
    sort(intervals.begin(), intervals.end(),
         [](const Interval& a, const Interval& b) {
             return a.r < b.r;
         });
    int count = 0, last_r = -1;
    for (auto& iv : intervals) {
        if (iv.l > last_r) {
            count++;
            last_r = iv.r;
        }
    }
    return count;
}

// --- A.3. Minimum coloring интервального графа ---
// = maximum clique (perfect graph property).
// O(n log n) через sweep line + greedy assignment.
int min_coloring_interval(vector<Interval> intervals) {
    return max_clique_interval(intervals);
}

// =============================================================
// B. ГРАФЫ ПЕРЕСТАНОВОК
// =============================================================

// --- B.1. Подсчёт инверсий через merge sort ---
// O(n log n) время, O(n) дополнительная память.
long long count_inversions(vector<int> arr) {
    long long count = 0;
    function<void(int, int)> merge_sort = [&](int l, int r) {
        if (r - l <= 1) return;
        int mid = (l + r) / 2;
        merge_sort(l, mid);
        merge_sort(mid, r);
        vector<int> merged;
        int i = l, j = mid;
        while (i < mid && j < r) {
            if (arr[i] <= arr[j]) {
                merged.push_back(arr[i++]);
            } else {
                merged.push_back(arr[j++]);
                count += mid - i;
            }
        }
        while (i < mid) merged.push_back(arr[i++]);
        while (j < r) merged.push_back(arr[j++]);
        for (int k = 0; k < (int)merged.size(); k++)
            arr[l + k] = merged[k];
    };
    merge_sort(0, arr.size());
    return count;
}

// --- B.2. Подсчёт инверсий через Fenwick tree ---
// O(n log n) время.
long long count_inversions_fenwick(vector<int> arr) {
    int n = arr.size();
    // Координатное сжатие
    vector<int> sorted_arr = arr;
    sort(sorted_arr.begin(), sorted_arr.end());
    sorted_arr.erase(unique(sorted_arr.begin(), sorted_arr.end()), sorted_arr.end());
    for (int& x : arr)
        x = lower_bound(sorted_arr.begin(), sorted_arr.end(), x) - sorted_arr.begin() + 1;

    int sz = sorted_arr.size() + 2;
    vector<int> bit(sz, 0);

    auto update = [&](int i, int delta) {
        for (; i < sz; i += i & (-i)) bit[i] += delta;
    };
    auto query = [&](int i) -> int {
        int sum = 0;
        for (; i > 0; i -= i & (-i)) sum += bit[i];
        return sum;
    };

    long long inversions = 0;
    for (int i = n - 1; i >= 0; i--) {
        inversions += query(arr[i] - 1);
        update(arr[i], 1);
    }
    return inversions;
}

// =============================================================
// C. TREE DECOMPOSITION (упрощённый)
// =============================================================

// --- C.1. Ширина дерева: lower bound через treewidth ---
// Упрощённая оценка: tw ≥ max clique size − 1 (для хордальных графов tw = max clique − 1).
// Для общих графов: tw ≥ max independent set в complement?.
// Здесь: вычисляем max clique через brute force для малых графов.
int treewidth_lower_bound(int n, const vector<vector<int>>& adj) {
    // Max clique через перебор всех подмножеств (O(2^n) — только для малых n)
    int max_clique = 0;
    for (int mask = 1; mask < (1 << n); mask++) {
        bool is_clique = true;
        vector<int> vertices;
        for (int i = 0; i < n; i++)
            if (mask & (1 << i)) vertices.push_back(i);
        for (int i = 0; i < (int)vertices.size() && is_clique; i++)
            for (int j = i + 1; j < (int)vertices.size() && is_clique; j++)
                if (find(adj[vertices[i]].begin(), adj[vertices[i]].end(),
                         vertices[j]) == adj[vertices[i]].end())
                    is_clique = false;
        if (is_clique)
            max_clique = max(max_clique, (int)vertices.size());
    }
    return max_clique - 1;
}

// --- C.2. Tree decomposition для path graph ---
// Path graph имеет treewidth = 1.
// Каждое ребро — bag размера 2; дерево — линейная цепочка.
pair<int, vector<vector<int>>> path_decomposition(int n) {
    vector<vector<int>> bags;
    for (int i = 0; i < n - 1; i++)
        bags.push_back({i, i + 1});
    return {1, bags};
}

// --- C.3. Tree decomposition для tree ---
// treewidth = 1; bags = рёбра дерева.
pair<int, vector<vector<int>>> tree_decomposition(
    int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> bags;
    for (auto& [u, v] : edges)
        bags.push_back({u, v});
    return {1, bags};
}

// =============================================================
// D. ГИПЕРГРАФЫ
// =============================================================

// --- D.1. Bipartite representation гиперграфа ---
// Гиперграф H = (V, E) → bipartite graph B = (V ∪ E, edges)
// где ребро (v, e) ∈ B если v ∈ e в H.
// Возвращает: число вершин и рёбер bipartite representation.
pair<int,int> hypergraph_bipartite(int n, const vector<vector<int>>& hyperedges) {
    int v_count = n;
    int e_count = hyperedges.size();
    int b_edges = 0;
    for (auto& he : hyperedges)
        b_edges += he.size();
    return {v_count + e_count, b_edges};
}

// --- D.2. Hitting Set (brute force) ---
// Минимальное подмножество вершин, пересекающее каждое гиперребро.
// O(2^n · m) — только для малых n.
int hitting_set_brute(int n, const vector<vector<int>>& hyperedges) {
    int best = n;
    for (int mask = 1; mask < (1 << n); mask++) {
        bool covers_all = true;
        for (auto& he : hyperedges) {
            bool hit = false;
            for (int v : he)
                if (mask & (1 << v)) { hit = true; break; }
            if (!hit) { covers_all = false; break; }
        }
        if (covers_all)
            best = min(best, __builtin_popcount(mask));
    }
    return best;
}

}; // struct SpecialGraphs

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_J_MAIN
int main() {
    SpecialGraphs sg;

    cout << "=== Interval Graph ===" << endl;
    vector<SpecialGraphs::Interval> intervals = {{1,4},{2,5},{6,8},{7,9},{3,7}};
    cout << "Max clique: " << sg.max_clique_interval(intervals) << endl;
    cout << "Max independent set: " << sg.max_independent_set_interval(intervals) << endl;
    cout << "Min coloring: " << sg.min_coloring_interval(intervals) << endl;

    cout << "\n=== Inversions ===" << endl;
    vector<int> perm = {2, 4, 1, 5, 3};
    cout << "Inversions (merge sort): " << sg.count_inversions(perm) << endl;
    cout << "Inversions (Fenwick): " << sg.count_inversions_fenwick(perm) << endl;

    cout << "\n=== Treewidth ===" << endl;
    // K4: tw = 3
    vector<vector<int>> adj_k4(4);
    for (int i = 0; i < 4; i++)
        for (int j = i+1; j < 4; j++) {
            adj_k4[i].push_back(j);
            adj_k4[j].push_back(i);
        }
    cout << "K4 treewidth lower bound: " << sg.treewidth_lower_bound(4, adj_k4) << endl;

    // Path graph 0-1-2-3: tw = 1
    vector<vector<int>> adj_path(4);
    adj_path[0] = {1}; adj_path[1] = {0,2};
    adj_path[2] = {1,3}; adj_path[3] = {2};
    cout << "Path treewidth lower bound: " << sg.treewidth_lower_bound(4, adj_path) << endl;

    auto [tw, bags] = sg.path_decomposition(4);
    cout << "Path decomposition bags: ";
    for (auto& bag : bags) {
        cout << "{";
        for (int i = 0; i < (int)bag.size(); i++)
            cout << bag[i] << (i+1 < (int)bag.size() ? "," : "");
        cout << "} ";
    }
    cout << endl;

    cout << "\n=== Hypergraph ===" << endl;
    vector<vector<int>> he = {{0,1,2},{1,2,3},{0,3}};
    auto [bv, be] = sg.hypergraph_bipartite(4, he);
    cout << "Bipartite repr: V=" << bv << ", E=" << be << endl;
    cout << "Hitting set size: " << sg.hitting_set_brute(4, he) << endl;

    return 0;
}
#endif

#endif // GRAPH_J_CPP
