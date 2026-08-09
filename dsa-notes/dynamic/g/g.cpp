#ifndef DYNAMIC_G_CPP
#define DYNAMIC_G_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <climits>
#include <utility>
#include <cmath>
#include <functional>
#include <map>
#include <set>
#include <queue>
#include <tuple>
using namespace std;

// =============================================================
// G. ДИНАМИКА НА ДЕРЕВЬЯХ
// =============================================================
// Структура md: A. Классификация (G.1–G.2)
//               → B. Типовые состояния (G.3–G.7)
//               → C. Примеры задач (G.8–G.17)
//               → D. Специальные техники (G.18–G.20)
//
// TreeDP наследует ProfileDP (f.cpp), а через него SubsetDP,
// SegmentDP, MatrixDP, LinearDP, DPBasics. Переиспользуются, а не
// дублируются: независимое множество по маскам (e.md E.11 — G.4:
// на дереве полином), 0/1-рюкзак (c.md C.20 — G.17), рекурсия
// «разделяй и властвуй» (f.md F.7 — G.19), битовые операции
// (e.md E.1 — G.16).
//
// Порядок методов строго соответствует порядку md (G.4 → G.20);
// G.1–G.3 (post-order, rerooting, флаги 0/1), G.10–G.12 (частные
// случаи G.4/G.5/G.7) и G.18 (общая схема down/up) отдельного
// метода не требуют — их реализуют перечисленные ниже методы.
// Ограничения: рекурсивный DFS — высота дерева в пределах стека;
// бинарный подъём (G.16) — n ≤ 10⁶ (2²⁰ прыжков).

#define PROFILEDP_MAIN
#include "../f/f.cpp"

struct TreeDP : ProfileDP {

    struct MISResult {
        long long size;
        long long ways;
        vector<int> taken;
    };

    // --- G.4. Максимум независимого множества (MIS) ---
    // dp[v][1] = 1 + Σ dp[c][0] (v взята — дети запрещены);
    // dp[v][0] = Σ max(dp[c][0], dp[c][1]) (v свободна — дети
    // решаются независимо); ответ max(dp[0][0], dp[0][1]). Счёт
    // способов — перемножение по детям, равенство dp0 = dp1 даёт
    // сумму способов. Восстановление — жадный спуск по dp.
    // O(n) времени и памяти.
    MISResult max_independent_set_tree(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<vector<long long>> dp(n, vector<long long>(2, 0));
        vector<vector<long long>> ways(n, vector<long long>(2, 1));
        function<void(int, int)> dfs = [&](int v, int p) {
            dp[v][1] = 1;
            for (int to : adj[v]) {
                if (to == p) continue;
                dfs(to, v);
                dp[v][1] += dp[to][0];
                ways[v][1] *= ways[to][0];
                if (dp[to][0] > dp[to][1]) {
                    dp[v][0] += dp[to][0];
                    ways[v][0] *= ways[to][0];
                } else if (dp[to][1] > dp[to][0]) {
                    dp[v][0] += dp[to][1];
                    ways[v][0] *= ways[to][1];
                } else {
                    dp[v][0] += dp[to][0];
                    ways[v][0] *= ways[to][0] + ways[to][1];
                }
            }
        };
        dfs(0, -1);
        MISResult res;
        res.size = max(dp[0][0], dp[0][1]);
        if (dp[0][0] == dp[0][1]) res.ways = ways[0][0] + ways[0][1];
        else res.ways = (dp[0][0] > dp[0][1]) ? ways[0][0] : ways[0][1];
        function<void(int, int, bool)> build = [&](int v, int p, bool in) {
            if (in) res.taken.push_back(v);
            for (int to : adj[v]) {
                if (to == p) continue;
                build(to, v, in ? false : (dp[to][1] >= dp[to][0]));
            }
        };
        build(0, -1, dp[0][1] >= dp[0][0]);
        return res;
    }

    // --- G.5. Минимальное вершинное покрытие (MVC) ---
    // dp[v][1] = 1 + Σ min(dp[c][0], dp[c][1]) (v взята — рёбра
    // v—c покрыты); dp[v][0] = Σ dp[c][1] (v не взята — все дети
    // обязаны). Сверка (теорема Кёнига для деревьев): |MVC| =
    // n − |MIS| (дополнение независимого множества, G.4).
    // O(n) времени и памяти.
    long long min_vertex_cover_tree(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<vector<long long>> dp(n, vector<long long>(2, 0));
        function<void(int, int)> dfs = [&](int v, int p) {
            dp[v][1] = 1;
            for (int to : adj[v]) {
                if (to == p) continue;
                dfs(to, v);
                dp[v][1] += min(dp[to][0], dp[to][1]);
                dp[v][0] += dp[to][1];
            }
        };
        dfs(0, -1);
        return min(dp[0][0], dp[0][1]);
    }

    // --- G.6. Минимальное доминирующее множество (DMS) ---
    // Три состояния: dp[v][0] = 1 + Σ min(dp[c][0], dp[c][1],
    // dp[c][2]) (v выбрана); dp[v][1] = Σ min(dp[c][0], dp[c][1]) +
    // min_c (dp[c][0] − min(dp[c][0], dp[c][1])) (v не выбрана,
    // доминируется ребёнком — поправка «хотя бы один ребёнок вы-
    // бран»); dp[v][2] = Σ dp[c][1] (v ждёт доминирования от ро-
    // дителя — дети не выбраны). Лист: dp = (1, ∞, 0); корень не
    // может быть в состоянии 2. O(n) времени и памяти.
    long long min_dominating_set_tree(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        const long long INF = LLONG_MAX / 4;
        vector<vector<long long>> dp(n, vector<long long>(3, 0));
        function<void(int, int)> dfs = [&](int v, int p) {
            dp[v][0] = 1;
            dp[v][2] = 0;
            long long base = 0, corr = INF;
            bool hasChild = false;
            for (int to : adj[v]) {
                if (to == p) continue;
                hasChild = true;
                dfs(to, v);
                dp[v][0] += min(dp[to][0], min(dp[to][1], dp[to][2]));
                long long mn = min(dp[to][0], dp[to][1]);
                base += mn;
                corr = min(corr, dp[to][0] - mn);
                dp[v][2] += dp[to][1];
            }
            dp[v][1] = hasChild ? base + corr : INF;
        };
        dfs(0, -1);
        return min(dp[0][0], dp[0][1]);
    }

    // --- G.7. Максимальное паросочетание (Maximum Matching) ---
    // dp[v][0] = Σ max(dp[c][0], dp[c][1]) + max(0, max_c (dp[c][1]
    // + 1 − max(dp[c][0], dp[c][1]))) (ребро к родителю не исполь-
    // зовано — v может соединиться с лучшим ребёнком); dp[v][1] =
    // Σ max(dp[c][0], dp[c][1]) (ребро к родителю использовано).
    // Ответ dp[0][0]. O(n) времени и памяти.
    long long max_matching_tree(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<vector<long long>> dp(n, vector<long long>(2, 0));
        function<void(int, int)> dfs = [&](int v, int p) {
            long long sum = 0, best = 0;
            for (int to : adj[v]) {
                if (to == p) continue;
                dfs(to, v);
                long long mx = max(dp[to][0], dp[to][1]);
                sum += mx;
                best = max(best, dp[to][1] + 1 - mx);
            }
            dp[v][1] = sum;
            dp[v][0] = sum + max(0LL, best);
        };
        dfs(0, -1);
        return dp[0][0];
    }

    // --- G.8. Binary Tree Path Sum ---
    // (а) сумма значений на путях корень–лист: dfs с накоплением
    // cur, лист добавляет cur в ответ; (б) down[v] = val[v] +
    // max(0, down лучшего ребёнка); максимум пути — val[v] + лучший
    // + второй лучший down детей (каркас «лучший + второй лучший»
    // — b.md B.1). O(n) времени, O(высота) памяти.
    long long sum_root_to_leaf_paths(const vector<int>& values,
                                     const vector<vector<int>>& children) {
        long long total = 0;
        function<void(int, long long)> dfs = [&](int v, long long cur) {
            cur += values[v];
            if (children[v].empty()) {
                total += cur;
                return;
            }
            for (int to : children[v]) dfs(to, cur);
        };
        dfs(0, 0);
        return total;
    }

    long long max_path_sum_any_two(const vector<int>& values,
                                   const vector<vector<int>>& children) {
        int n = (int)values.size();
        vector<long long> down(n, 0);
        long long best = LLONG_MIN;
        function<void(int)> dfs = [&](int v) {
            long long b1 = 0, b2 = 0;
            for (int to : children[v]) {
                dfs(to);
                long long d = down[to];
                if (d > b1) {
                    b2 = b1;
                    b1 = d;
                } else if (d > b2) {
                    b2 = d;
                }
            }
            down[v] = values[v] + max(0LL, b1);
            best = max(best, values[v] + b1 + b2);
        };
        dfs(0);
        return best;
    }

    // --- G.9. Maximum Sum BST (наибольшее по сумме BST-поддерево) ---
    // Снизу вверх для v возвращаем (valid, sum, min, max): если оба
    // ребёнка валидны и val[v] > max(left) и val[v] < min(right) —
    // узел валиден, сумма склеивается; невалидный узел «ломает» все
    // поддеревья выше (индукция по высоте). Ответ — максимум сумм
    // валидных поддеревьев (пустое поддерево — 0). O(n) времени и
    // памяти.
    long long max_sum_bst_subtree(const vector<int>& values,
                                  const vector<int>& left,
                                  const vector<int>& right) {
        int n = (int)values.size();
        long long best = 0;
        function<tuple<bool, long long, int, int>(int)> dfs =
            [&](int v) -> tuple<bool, long long, int, int> {
            if (v < 0) return make_tuple(true, 0LL, INT_MAX, INT_MIN);
            auto [lv, ls, lmn, lmx] = dfs(left[v]);
            auto [rv, rs, rmn, rmx] = dfs(right[v]);
            if (lv && rv && values[v] > lmx && values[v] < rmn) {
                long long sum = (long long)values[v] + ls + rs;
                best = max(best, sum);
                return make_tuple(true, sum, min(values[v], lmn),
                                  max(values[v], rmx));
            }
            return make_tuple(false, 0LL, INT_MIN, INT_MAX);
        };
        dfs(0);
        return best;
    }

    // --- G.13. Tree Diameter ---
    // Метод (а): два прохода — самая дальняя от произвольной вер-
    // шины f, затем самая дальняя от f; расстояние (f, g) — диа-
    // метр. Метод (б, однопроходный) — в G.8: максимум путей с
    // единичными значениями = диаметр + 1. O(n) времени и памяти.
    long long tree_diameter(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        auto far = [&](int s) {
            vector<int> dist(n, -1);
            queue<int> q;
            dist[s] = 0;
            q.push(s);
            int best = s;
            while (!q.empty()) {
                int v = q.front();
                q.pop();
                if (dist[v] > dist[best]) best = v;
                for (int to : adj[v])
                    if (dist[to] < 0) {
                        dist[to] = dist[v] + 1;
                        q.push(to);
                    }
            }
            return make_pair(best, dist[best]);
        };
        auto f = far(0);
        return far(f.first).second;
    }

    // Взвешенный вариант: та же схема двух обходов с весами рёбер.
    long long tree_diameter_weighted(const vector<vector<pair<int, int>>>& adj) {
        int n = (int)adj.size();
        auto far = [&](int s) {
            vector<long long> dist(n, -1);
            dist[s] = 0;
            vector<int> st;
            st.push_back(s);
            pair<int, long long> best = {s, 0};
            while (!st.empty()) {
                int v = st.back();
                st.pop_back();
                if (dist[v] > best.second) best = {v, dist[v]};
                for (auto [to, w] : adj[v])
                    if (dist[to] < 0) {
                        dist[to] = dist[v] + w;
                        st.push_back(to);
                    }
            }
            return best;
        };
        auto f = far(0);
        return far(f.first).second;
    }

    // --- G.14. Подсчёт поддеревьев заданного размера ---
    // dp[v][s] — связные подмножества размера s в поддереве v,
    // содержащие v; база dp[v][1] = 1; ребёнок c: не берём (мно-
    // житель dp[v][s]) или берём (рюкзак dp[v][s]·dp[c][t] — 0/1-
    // рюкзак c.md C.20). Ответ — Σ_v dp[v][k]: у каждого подмно-
    // жества единственная верхняя вершина (деления на орбиты нет).
    // O(n²) суммарно (рюкзак по размерам поддеревьев).
    long long count_subtrees_of_size(const vector<vector<int>>& adj, int k) {
        if (k < 1) return 0;
        int n = (int)adj.size();
        vector<vector<long long>> dp(n, vector<long long>(k + 1, 0));
        long long ans = 0;
        function<void(int, int)> dfs = [&](int v, int p) {
            dp[v][1] = 1;
            for (int to : adj[v]) {
                if (to == p) continue;
                dfs(to, v);
                vector<long long> ndp(k + 1, 0);
                for (int s = 1; s <= k; s++) {
                    if (!dp[v][s]) continue;
                    ndp[s] += dp[v][s];
                    for (int t = 1; s + t <= k; t++)
                        ndp[s + t] += dp[v][s] * dp[to][t];
                }
                dp[v].swap(ndp);
            }
            ans += dp[v][k];
        };
        dfs(0, -1);
        return ans;
    }

    // --- G.15. Сумма расстояний до всех вершин ---
    // Rerooting (G.2): первый проход — sz[v], sumDown[v] (сумма
    // расстояний внутри поддерева); перенос через ребро v—to:
    // ans[to] = ans[v] + n − 2·sz[to] (вершины поддерева to при-
    // ближаются на 1, остальные отдаляются на 1). O(n) времени и
    // памяти.
    vector<long long> sum_distances_all_vertices(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<int> sz(n, 0);
        vector<long long> sumDown(n, 0);
        function<void(int, int)> dfs1 = [&](int v, int p) {
            sz[v] = 1;
            for (int to : adj[v])
                if (to != p) {
                    dfs1(to, v);
                    sz[v] += sz[to];
                    sumDown[v] += sumDown[to] + sz[to];
                }
        };
        dfs1(0, -1);
        vector<long long> ans(n, 0);
        ans[0] = sumDown[0];
        function<void(int, int)> dfs2 = [&](int v, int p) {
            for (int to : adj[v])
                if (to != p) {
                    ans[to] = ans[v] + n - 2LL * sz[to];
                    dfs2(to, v);
                }
        };
        dfs2(0, -1);
        return ans;
    }

    // --- G.16. Kth Ancestor / бинарный подъём ---
    // up[j][v] = 2^j-й предок, up[j][v] = up[j−1][up[j−1][v]];
    // запрос — прыжки по битам k. Задел для ДП на деревьях (не
    // ДП сам по себе). Предподсчёт O(n·log n), запрос O(log n).
    int kth_ancestor(const vector<vector<int>>& adj, int v, int k) {
        int n = (int)adj.size();
        int LOG = 1;
        while ((1 << LOG) <= n) LOG++;
        vector<vector<int>> up(LOG, vector<int>(n, -1));
        vector<int> depth(n, 0);
        vector<int> q;
        q.reserve(n);
        q.push_back(0);
        for (int i = 0; i < (int)q.size(); i++) {
            int x = q[i];
            for (int to : adj[x])
                if (to != up[0][x]) {
                    up[0][to] = x;
                    depth[to] = depth[x] + 1;
                    q.push_back(to);
                }
        }
        for (int j = 1; j < LOG; j++)
            for (int x = 0; x < n; x++)
                if (up[j - 1][x] != -1)
                    up[j][x] = up[j - 1][up[j - 1][x]];
        if (k > depth[v]) return -1;
        for (int j = 0; j < LOG; j++)
            if ((k >> j) & 1) v = up[j][v];
        return v;
    }

    // --- G.17. Tree Knapsack (рюкзак на дереве) ---
    // dp[v][s] — максимум ценности выборки веса s из поддерева v,
    // содержащей v; база dp[v][w[v]] = val[v]; слияние ребёнка —
    // 0/1-рюкзак (c.md C.20): не брать (ndp = dp[v]) или брать
    // (dp[v][s] + dp[to][t]); ответ — max_s dp[root][s] по s ≤ cap.
    // O(n·cap) суммарно.
    long long tree_knapsack(const vector<vector<int>>& children,
                            const vector<int>& weights,
                            const vector<int>& values, int cap) {
        int n = (int)children.size();
        const long long NEG = LLONG_MIN / 4;
        vector<vector<long long>> dp(n, vector<long long>(cap + 1, NEG));
        function<void(int)> dfs = [&](int v) {
            if (weights[v] <= cap) dp[v][weights[v]] = values[v];
            for (int to : children[v]) {
                dfs(to);
                vector<long long> ndp = dp[v];
                for (int s = weights[v]; s <= cap; s++) {
                    if (dp[v][s] == NEG) continue;
                    for (int t = weights[to]; s + t <= cap; t++)
                        if (dp[to][t] != NEG)
                            ndp[s + t] = max(ndp[s + t], dp[v][s] + dp[to][t]);
                }
                dp[v].swap(ndp);
            }
        };
        dfs(0);
        long long ans = 0;
        for (int s = 0; s <= cap; s++) ans = max(ans, dp[0][s]);
        return ans;
    }

    // --- G.19. DP на центроидном разложении ---
    // decompose: центроид c компоненты; все пары с dist_c(u) +
    // dist_c(v) ≤ k (пути через c) — сортировка + два указателя;
    // вычитание пар внутри одного подкомпонента (пересчёт с по-
    // правкой на ребро); удаление c и рекурсия. Любой путь имеет
    // единственный первый центроид, через который проходит (вклю-
    // чения-исключения по уровням). O(n·log n) времени.
    long long count_paths_length_at_most(const vector<vector<int>>& adj, int k) {
        int n = (int)adj.size();
        vector<int> removed(n, 0), sz(n, 0);
        long long ans = 0;
        function<long long(vector<long long>&)> count_pairs =
            [&](vector<long long>& d) {
            sort(d.begin(), d.end());
            long long cnt = 0;
            int j = (int)d.size() - 1;
            for (int i = 0; i < (int)d.size(); i++) {
                while (j > i && d[i] + d[j] > k) j--;
                if (j > i) cnt += j - i;
            }
            return cnt;
        };
        function<void(int, int)> get_sz = [&](int v, int p) {
            sz[v] = 1;
            for (int to : adj[v])
                if (to != p && !removed[to]) {
                    get_sz(to, v);
                    sz[v] += sz[to];
                }
        };
        function<int(int, int, int)> find_centroid =
            [&](int v, int p, int total) {
            for (int to : adj[v])
                if (to != p && !removed[to] && sz[to] > total / 2)
                    return find_centroid(to, v, total);
            return v;
        };
        function<void(int, int, long long, vector<long long>&)> collect =
            [&](int v, int p, long long d, vector<long long>& dists) {
            dists.push_back(d);
            for (int to : adj[v])
                if (to != p && !removed[to])
                    collect(to, v, d + 1, dists);
        };
        function<void(int)> decompose = [&](int start) {
            get_sz(start, -1);
            int c = find_centroid(start, -1, sz[start]);
            vector<long long> all;
            collect(c, -1, 0, all);
            ans += count_pairs(all);
            for (int to : adj[c]) {
                if (removed[to]) continue;
                vector<long long> sub;
                collect(to, c, 1, sub);
                ans -= count_pairs(sub);
            }
            removed[c] = 1;
            for (int to : adj[c])
                if (!removed[to]) decompose(to);
        };
        decompose(0);
        return ans;
    }

    // --- G.20. Ограничения на размер компонент ---
    // dp[v][s] — число разрезаний поддерева v (компоненты ≤ k),
    // где компонента v имеет размер s; база dp[v][1] = 1; ребёнок:
    // ребро режется (множитель Σ_t dp[c][t]) или нет (рюкзак
    // new[s + t] += dp[v][s]·dp[c][t]); ответ Σ_s dp[root][s].
    // O(n²) суммарно (рюкзак по размерам, G.17).
    long long count_partitions_max_size(const vector<vector<int>>& adj, int k) {
        int n = (int)adj.size();
        vector<vector<long long>> dp(n, vector<long long>(k + 1, 0));
        vector<long long> total(n, 0);
        function<void(int, int)> dfs = [&](int v, int p) {
            dp[v][1] = 1;
            for (int to : adj[v]) {
                if (to == p) continue;
                dfs(to, v);
                vector<long long> ndp(k + 1, 0);
                for (int s = 1; s <= k; s++) {
                    if (!dp[v][s]) continue;
                    ndp[s] += dp[v][s] * total[to];
                    for (int t = 1; s + t <= k; t++)
                        ndp[s + t] += dp[v][s] * dp[to][t];
                }
                dp[v].swap(ndp);
            }
            for (int s = 1; s <= k; s++) total[v] += dp[v][s];
        };
        dfs(0, -1);
        long long ans = 0;
        for (int s = 1; s <= k; s++) ans += dp[0][s];
        return ans;
    }
};

#ifndef TREEDP_MAIN
signed main() {
    TreeDP dp;

    vector<vector<int>> chain4 = {{1}, {0, 2}, {1, 3}, {2}};
    auto mis4 = dp.max_independent_set_tree(chain4);
    cout << "G.4: MIS цепочки 0-1-2-3: размер = " << mis4.size
         << ", способов = " << mis4.ways << " (ожидаем 2, 3), одно множество: {";
    for (size_t i = 0; i < mis4.taken.size(); i++)
        cout << (i ? ", " : "") << mis4.taken[i];
    cout << "}\n";

    vector<vector<int>> tree6 = {{1}, {0, 2, 3}, {1, 4, 5}, {1}, {2}, {2}};
    auto mis6 = dp.max_independent_set_tree(tree6);
    cout << "G.5: MVC дерева 6 = " << dp.min_vertex_cover_tree(tree6)
         << " (ожидаем 2; сверка: 6 - MIS = " << (6 - mis6.size) << ")\n";

    cout << "G.6: DMS пути из 4 = " << dp.min_dominating_set_tree(chain4)
         << " (ожидаем 2), дерева 6 = " << dp.min_dominating_set_tree(tree6)
         << " (ожидаем 2)\n";

    cout << "G.7: matching дерева 6 = " << dp.max_matching_tree(tree6)
         << " (ожидаем 2)\n";

    vector<vector<int>> ch6 = {{1}, {2, 3}, {4, 5}, {}, {}, {}};
    vector<int> vals6 = {1, 2, 3, 4, 5, 6};
    cout << "G.8: суммы путей корень-лист = "
         << dp.sum_root_to_leaf_paths(vals6, ch6) << " (ожидаем 30), максимум пути = "
         << dp.max_path_sum_any_two(vals6, ch6) << " (ожидаем 15)\n";

    vector<int> bv = {10, 5, 15, 2, 8, 12, 20};
    vector<int> bl = {1, 3, 5, -1, -1, -1, -1};
    vector<int> br = {2, 4, 6, -1, -1, -1, -1};
    cout << "G.9: max_sum_bst_subtree = " << dp.max_sum_bst_subtree(bv, bl, br)
         << " (ожидаем 72)\n";
    vector<int> bv2 = {10, 5, 15, 9, 8, 12, 20};
    cout << "G.9: левый ребёнок 5 = 9: " << dp.max_sum_bst_subtree(bv2, bl, br)
         << " (ожидаем 47)\n";

    cout << "G.13: диаметр дерева 6 = " << dp.tree_diameter(tree6)
         << " (ожидаем 3; сверка: max_path_sum(единичные) = "
         << dp.max_path_sum_any_two(vector<int>(6, 1), ch6) << " = диаметр + 1)\n";
    vector<vector<pair<int, int>>> wchain = {{{1, 5}}, {{0, 5}, {2, 7}},
                                             {{1, 7}, {3, 2}}, {{2, 2}}};
    cout << "G.13: взвешенная цепочка = " << dp.tree_diameter_weighted(wchain)
         << " (ожидаем 14)\n";

    cout << "G.14: поддеревья размера 1/2/3 цепочки из 4: "
         << dp.count_subtrees_of_size(chain4, 1) << ", "
         << dp.count_subtrees_of_size(chain4, 2) << ", "
         << dp.count_subtrees_of_size(chain4, 3)
         << " (ожидаем 4, 3, 2)\n";

    vector<long long> sd4 = dp.sum_distances_all_vertices(chain4);
    cout << "G.15: цепочка: ans = {";
    for (size_t i = 0; i < sd4.size(); i++) cout << (i ? ", " : "") << sd4[i];
    cout << "} (ожидаем {6, 4, 4, 6})\n";
    vector<long long> sd6 = dp.sum_distances_all_vertices(tree6);
    cout << "G.15: дерево 6: ans = {";
    for (size_t i = 0; i < sd6.size(); i++) cout << (i ? ", " : "") << sd6[i];
    cout << "} (ожидаем {11, 7, 7, 11, 11, 11}, сумма 58 = 2·29 парных)\n";

    vector<vector<int>> chain5 = {{1}, {0, 2}, {1, 3}, {2, 4}, {3}};
    cout << "G.16: kth(4, 2) = " << dp.kth_ancestor(chain5, 4, 2)
         << " (ожидаем 2), kth(4, 10) = " << dp.kth_ancestor(chain5, 4, 10)
         << " (ожидаем -1)\n";

    vector<vector<int>> courses = {{1, 2}, {3}, {4}, {}, {}};
    vector<int> cw = {1, 1, 1, 1, 1}, cv = {5, 4, 3, 2, 1};
    cout << "G.17: cap 3 = " << dp.tree_knapsack(courses, cw, cv, 3)
         << " (ожидаем 12), cap 4 = " << dp.tree_knapsack(courses, cw, cv, 4)
         << " (ожидаем 14), cap 5 = " << dp.tree_knapsack(courses, cw, cv, 5)
         << " (ожидаем 15)\n";

    cout << "G.19: пары с расстоянием <= 1: "
         << dp.count_paths_length_at_most(chain4, 1) << " (ожидаем 3), <= 2: "
         << dp.count_paths_length_at_most(chain4, 2) << " (ожидаем 5), <= 3: "
         << dp.count_paths_length_at_most(chain4, 3) << " (ожидаем 6)\n";

    vector<vector<int>> chain3 = {{1}, {0, 2}, {1}};
    cout << "G.20: k = 1: " << dp.count_partitions_max_size(chain3, 1)
         << " (ожидаем 1), k = 2: " << dp.count_partitions_max_size(chain3, 2)
         << " (ожидаем 3), k = 3: " << dp.count_partitions_max_size(chain3, 3)
         << " (ожидаем 4)\n";
}
#endif // TREEDP_MAIN
#endif // DYNAMIC_G_CPP
