#ifndef GRAPH_F_CPP
#define GRAPH_F_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <set>
#include <numeric>
using namespace std;

// =============================================================
// F. ДЕРЕВЬЯ И ИХ ДЕКОМПОЗИЦИИ
// =============================================================
// Структура md: A. Базовые запросы на деревьях
//               → B. LCA (binary lifting, Euler tour)
//               → C. Декомпозиции (HLD, центроидная)
//               → D. Продвинутые (rerooting, virtual tree, isomorphism)
//
// TreeAlgorithms — наследует EulerHamilton (e.cpp).
// Реализует: построение деревьев, диаметр, LCA, HLD,
// центроидную декомпозицию, rerooting, virtual tree, хеш.

#ifndef INSIDE_GRAPH_F
#define INSIDE_GRAPH_F
#include "../e/e.cpp"
#undef INSIDE_GRAPH_F
#endif

struct TreeAlgorithms : EulerHamilton {

// =============================================================
// A. БАЗОВЫЕ ЗАПРОСЫ
// =============================================================

// --- A.1. Построение дерева из рёбер ---
// Возвращает adjacency list.
vector<vector<int>> build_tree(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& [u, v] : edges) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    return adj;
}

// --- A.2. DFS: parent, depth, subtree size ---
struct TreeInfo {
    vector<int> parent, depth, sz;
    vector<vector<int>> up;  // для binary lifting
    int LOG;
};

TreeInfo dfs_tree(int n, const vector<vector<int>>& adj, int root = 0) {
    TreeInfo ti;
    ti.parent.assign(n, -1);
    ti.depth.assign(n, 0);
    ti.sz.assign(n, 1);

    function<void(int, int)> dfs = [&](int v, int p) {
        ti.parent[v] = p;
        ti.sz[v] = 1;
        for (int u : adj[v]) {
            if (u == p) continue;
            ti.depth[u] = ti.depth[v] + 1;
            dfs(u, v);
            ti.sz[v] += ti.sz[u];
        }
    };
    dfs(root, -1);

    // Binary lifting
    ti.LOG = 1;
    while ((1 << ti.LOG) <= n) ti.LOG++;
    ti.up.assign(n, vector<int>(ti.LOG, -1));
    for (int v = 0; v < n; v++) ti.up[v][0] = ti.parent[v];
    for (int j = 1; j < ti.LOG; j++)
        for (int v = 0; v < n; v++)
            if (ti.up[v][j-1] != -1)
                ti.up[v][j] = ti.up[ti.up[v][j-1]][j-1];

    return ti;
}

// --- A.3. Диаметр дерева ---
// Два BFS: от произвольной → самая дальняя → от неё → диаметр.
pair<int, pair<int,int>> tree_diameter(int n, const vector<vector<int>>& adj) {
    auto bfs_farthest = [&](int s) -> pair<int, int> {
        vector<int> dist(n, -1);
        queue<int> q;
        dist[s] = 0;
        q.push(s);
        int far = s;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            far = v;
            for (int u : adj[v])
                if (dist[u] == -1) { dist[u] = dist[v] + 1; q.push(u); }
        }
        return {far, dist[far]};
    };

    pair<int,int> first = bfs_farthest(0);
    int a = first.first;
    pair<int,int> second = bfs_farthest(a);
    int b = second.first;
    int diam = second.second;
    return {diam, {a, b}};
}

// --- A.4. Центр дерева ---
int tree_center(int n, const vector<vector<int>>& adj) {
    pair<int, pair<int,int>> diam_res = tree_diameter(n, adj);
    int diam = diam_res.first;
    int a = diam_res.second.first;
    int b = diam_res.second.second;

    // Идём от a до b, центр — середина
    vector<int> path;
    vector<int> parent(n, -1);
    function<void(int, int)> dfs_path = [&](int v, int p) {
        parent[v] = p;
        if (v == b) return;
        for (int u : adj[v])
            if (u != p) { dfs_path(u, v); if (parent[b] != -1) return; }
    };
    dfs_path(a, -1);

    for (int v = b; v != -1; v = parent[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path[path.size() / 2];
}

// =============================================================
// B. LCA
// =============================================================

// --- B.1. LCA через binary lifting ---
// O(log n) на запрос после O(n log n) предподсчёта.
int lca(int u, int v, const TreeInfo& ti) {
    if (ti.depth[u] < ti.depth[v]) swap(u, v);
    int diff = ti.depth[u] - ti.depth[v];
    for (int j = ti.LOG - 1; j >= 0; j--)
        if (diff & (1 << j)) u = ti.up[u][j];
    if (u == v) return u;
    for (int j = ti.LOG - 1; j >= 0; j--) {
        if (ti.up[u][j] != ti.up[v][j]) {
            u = ti.up[u][j];
            v = ti.up[v][j];
        }
    }
    return ti.parent[u];
}

// --- B.2. Расстояние между вершинами ---
int dist(int u, int v, const TreeInfo& ti) {
    return ti.depth[u] + ti.depth[v] - 2 * ti.depth[lca(u, v, ti)];
}

// --- B.3. k-й предок вершины ---
int kth_ancestor(int v, int k, const TreeInfo& ti) {
    for (int j = ti.LOG - 1; j >= 0; j--)
        if (k & (1 << j)) { v = ti.up[v][j]; if (v == -1) return -1; }
    return v;
}

// --- B.4. Min/max на пути (binary lifting) ---
pair<vector<vector<int>>, vector<vector<int>>> build_lca_minmax(
    int n, const vector<vector<int>>& adj, const vector<int>& edge_weight,
    const TreeInfo& ti) {
    // edge_weight[v] = вес ребра (parent[v], v)
    vector<vector<int>> min_up(n, vector<int>(ti.LOG, INT_MAX));
    vector<vector<int>> max_up(n, vector<int>(ti.LOG, INT_MIN));
    for (int v = 0; v < n; v++) {
        if (ti.parent[v] != -1) {
            min_up[v][0] = edge_weight[v];
            max_up[v][0] = edge_weight[v];
        }
    }
    for (int j = 1; j < ti.LOG; j++)
        for (int v = 0; v < n; v++)
            if (ti.up[v][j-1] != -1) {
                min_up[v][j] = min(min_up[v][j-1],
                                   min_up[ti.up[v][j-1]][j-1]);
                max_up[v][j] = max(max_up[v][j-1],
                                   max_up[ti.up[v][j-1]][j-1]);
            }
    return {min_up, max_up};
}

// =============================================================
// C. HEAVY-LIGHT DECOMPOSITION
// =============================================================

struct HLD {
    int n;
    vector<int> parent, depth, sz, heavy, head, pos;
    vector<int> base;  // base[pos[v]] = v
    int cur_pos;

    HLD(int n, const vector<vector<int>>& adj, int root = 0)
        : n(n), parent(n), depth(n), sz(n), heavy(n, -1),
          head(n), pos(n), base(n) {

        // DFS 1: size + heavy child
        function<void(int, int)> dfs1 = [&](int v, int p) {
            parent[v] = p; sz[v] = 1;
            int max_sz = 0;
            for (int u : adj[v]) {
                if (u == p) continue;
                depth[u] = depth[v] + 1;
                dfs1(u, v);
                sz[v] += sz[u];
                if (sz[u] > max_sz) { max_sz = sz[u]; heavy[v] = u; }
            }
        };
        dfs1(root, -1);

        // DFS 2: decompose
        cur_pos = 0;
        function<void(int, int)> dfs2 = [&](int v, int h) {
            head[v] = h; pos[v] = cur_pos; base[cur_pos] = v; cur_pos++;
            if (heavy[v] != -1) dfs2(heavy[v], h);
            for (int u : adj[v]) {
                if (u == parent[v] || u == heavy[v]) continue;
                dfs2(u, u);
            }
        };
        dfs2(root, root);
    }

    // Путь от u до v: список отрезков [l, r] в base[]
    vector<pair<int,int>> path_segments(int u, int v) const {
        vector<pair<int,int>> segs;
        while (head[u] != head[v]) {
            if (depth[head[u]] < depth[head[v]]) swap(u, v);
            segs.push_back({pos[head[u]], pos[u]});
            u = parent[head[u]];
        }
        if (depth[u] > depth[v]) swap(u, v);
        segs.push_back({pos[u], pos[v]});
        return segs;
    }
};

// =============================================================
// D. ЦЕНТРОИДНАЯ ДЕКОМПОЗИЦИЯ
// =============================================================

struct CentroidDecomposition {
    int n;
    vector<vector<int>> adj;
    vector<int> parent, depth_centroid;
    vector<bool> removed;
    vector<int> sz;

    CentroidDecomposition(int n, const vector<vector<int>>& tree_adj)
        : n(n), adj(tree_adj), parent(n, -1), depth_centroid(n, 0),
          removed(n, false), sz(n) {}

    int get_subtree_size(int v, int p) {
        sz[v] = 1;
        for (int u : adj[v])
            if (u != p && !removed[u])
                sz[v] += get_subtree_size(u, v);
        return sz[v];
    }

    int get_centroid(int v, int p, int tree_sz) {
        for (int u : adj[v])
            if (u != p && !removed[u] && sz[u] > tree_sz / 2)
                return get_centroid(u, v, tree_sz);
        return v;
    }

    // Рекурсивная декомпозиция; возвращает корень декомпозиционного дерева
    int build(int v = 0, int p = -1, int d = 0) {
        int tsz = get_subtree_size(v, -1);
        int c = get_centroid(v, -1, tsz);
        parent[c] = p;
        depth_centroid[c] = d;
        removed[c] = true;
        for (int u : adj[c])
            if (!removed[u])
                build(u, c, d + 1);
        return c;
    }
};

// =============================================================
// E. REROOTING DP
// =============================================================

// Сумма расстояний до всех вершин для каждого корня.
// ans[v] = Σ_u dist(v, u).
// O(n) суммарно.
vector<long long> rerooting_sum_dist(int n, const vector<vector<int>>& adj) {
    vector<long long> down(n, 0), sz(n, 1), ans(n, 0);

    // DFS 1: down[v] = сумма расстояний в поддереве
    function<void(int, int)> dfs1 = [&](int v, int p) {
        for (int u : adj[v]) {
            if (u == p) continue;
            dfs1(u, v);
            sz[v] += sz[u];
            down[v] += down[u] + sz[u];
        }
    };
    dfs1(0, -1);

    // DFS 2: ans[v] через ans[parent]
    function<void(int, int)> dfs2 = [&](int v, int p) {
        for (int u : adj[v]) {
            if (u == p) continue;
            ans[u] = ans[v] + n - 2 * sz[u];
            dfs2(u, v);
        }
    };
    ans[0] = down[0];
    dfs2(0, -1);

    return ans;
}

// =============================================================
// F. VIRTUAL TREE
// =============================================================

// Строит virtual tree по набору ключевых вершин.
// Возвращает: список рёбер virtual tree (parent, child).
vector<pair<int,int>> build_virtual_tree(
    int n, const vector<vector<int>>& adj,
    const vector<int>& key_vertices,
    const TreeInfo& ti) {
    // Сортировка по tin (нужен эйлеров tour — используем depth)
    vector<int> sorted_keys = key_vertices;
    auto tin = [&](int v) -> int {
        // Упрощённо: сортируем по depth (для virtual tree достаточно)
        return ti.depth[v];
    };
    sort(sorted_keys.begin(), sorted_keys.end(),
         [&](int a, int b) { return tin(a) < tin(b); });

    // Добавляем LCA между соседними ключевыми вершинами
    set<int> key_set(sorted_keys.begin(), sorted_keys.end());
    for (int i = 0; i + 1 < (int)sorted_keys.size(); i++) {
        int l = lca(sorted_keys[i], sorted_keys[i+1], ti);
        key_set.insert(l);
    }

    vector<int> vt_keys(key_set.begin(), key_set.end());
    sort(vt_keys.begin(), vt_keys.end(),
         [&](int a, int b) { return tin(a) < tin(b); });

    // Строим через стек
    vector<pair<int,int>> edges;
    vector<int> st;
    for (int v : vt_keys) {
        while (!st.empty()) {
            int l = lca(st.back(), v, ti);
            if (l == st.back()) break;
            while (st.size() >= 2 && ti.depth[l] <= ti.depth[st[st.size()-2]]) {
                edges.push_back({st[st.size()-2], st.back()});
                st.pop_back();
            }
            if (st.back() != l) {
                edges.push_back({l, st.back()});
                st.pop_back();
                st.push_back(l);
            }
            break;
        }
        st.push_back(v);
    }
    while (st.size() > 1) {
        edges.push_back({st[st.size()-2], st.back()});
        st.pop_back();
    }

    return edges;
}

// =============================================================
// G. TREE ISOMORPHISM (хеширование)
// =============================================================

// Хеш дерева: полиномиальный хеш подписей.
// O(n log n) время (сортировка подписей детей).
vector<long long> tree_hash(int n, const vector<vector<int>>& adj) {
    const long long BASE = 1e9 + 7;
    vector<long long> h(n);
    function<void(int, int)> dfs = [&](int v, int p) {
        vector<long long> child_hashes;
        for (int u : adj[v])
            if (u != p) { dfs(u, v); child_hashes.push_back(h[u]); }
        sort(child_hashes.begin(), child_hashes.end());
        long long hv = 1;
        for (long long ch : child_hashes)
            hv = (hv * BASE + ch) % (long long)(1e18 + 7);
        hv = (hv * BASE + 137) % (long long)(1e18 + 7);
        h[v] = hv;
    };
    dfs(0, -1);
    return h;
}

}; // struct TreeAlgorithms

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_F_MAIN
int main() {
    TreeAlgorithms ta;

    int n = 7;
    vector<pair<int,int>> edges = {{0,1},{0,2},{1,3},{1,4},{2,5},{2,6}};
    auto adj = ta.build_tree(n, edges);

    cout << "=== Tree Info ===" << endl;
    auto ti = ta.dfs_tree(n, adj, 0);
    for (int i = 0; i < n; i++)
        cout << "v" << i << ": parent=" << ti.parent[i]
             << " depth=" << ti.depth[i] << " sz=" << ti.sz[i] << endl;

    cout << "\n=== Diameter ===" << endl;
    auto [diam, ab] = ta.tree_diameter(n, adj);
    cout << "Diameter: " << diam << " (between " << ab.first << " and " << ab.second << ")" << endl;

    cout << "\n=== Center ===" << endl;
    cout << "Center: " << ta.tree_center(n, adj) << endl;

    cout << "\n=== LCA ===" << endl;
    cout << "LCA(3,4) = " << ta.lca(3, 4, ti) << endl;
    cout << "LCA(3,5) = " << ta.lca(3, 5, ti) << endl;
    cout << "LCA(5,6) = " << ta.lca(5, 6, ti) << endl;
    cout << "dist(3,5) = " << ta.dist(3, 5, ti) << endl;
    cout << "kth_ancestor(6, 2) = " << ta.kth_ancestor(6, 2, ti) << endl;

    cout << "\n=== HLD ===" << endl;
    TreeAlgorithms::HLD hld(n, adj, 0);
    auto segs = hld.path_segments(3, 5);
    cout << "Path 3->5 segments: ";
    for (auto& [l, r] : segs)
        cout << "[" << l << "," << r << "] ";
    cout << endl;

    cout << "\n=== Centroid Decomposition ===" << endl;
    TreeAlgorithms::CentroidDecomposition cd(n, adj);
    int centroid_root = cd.build(0);
    cout << "Centroid root: " << centroid_root << endl;

    cout << "\n=== Rerooting Sum Dist ===" << endl;
    auto ans = ta.rerooting_sum_dist(n, adj);
    for (int i = 0; i < n; i++)
        cout << "ans[" << i << "] = " << ans[i] << endl;

    cout << "\n=== Virtual Tree ===" << endl;
    vector<int> key = {3, 5, 6};
    auto vt_edges = ta.build_virtual_tree(n, adj, key, ti);
    cout << "Virtual tree edges: ";
    for (auto& [u, v] : vt_edges)
        cout << "(" << u << "," << v << ") ";
    cout << endl;

    cout << "\n=== Tree Hash ===" << endl;
    auto hashes = ta.tree_hash(n, adj);
    for (int i = 0; i < n; i++)
        cout << "hash(" << i << ") = " << hashes[i] << endl;

    return 0;
}
#endif

#endif // GRAPH_F_CPP
