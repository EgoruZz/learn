#ifndef DYNAMIC_H_CPP
#define DYNAMIC_H_CPP

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
#include <unordered_set>
#include <unordered_map>
using namespace std;

// =============================================================
// H. ДИНАМИКА НА ГРАФАХ (DAG И НЕ ТОЛЬКО)
// =============================================================
// Структура md: A. На DAG (H.1–H.4)
//               → B. Алгоритмы на графах как ДП (H.5–H.8)
//               → C. Специальные задачи (H.9–H.14)
//
// GraphDP наследует TreeDP (g.cpp), а через него ProfileDP,
// SubsetDP, SegmentDP, MatrixDP, LinearDP, DPBasics. Переисполь-
// зуются, а не дублируются: House Robber на цикле
// max_non_adjacent_sum_circular (b.md B.3 — H.11), паросочетание
// на дереве (g.md G.7 — сверка H.10).
//
// Порядок методов строго соответствует порядку md (H.1 → H.14);
// H.11 (цикл) отдельного метода не требует — переиспользуется
// b.md B.3. Топологическая сортировка — приватный помощник.
// Ограничения: Флойд и степени матриц — малые V (O(V³));

#define TREEDP_MAIN
#include "../g/g.cpp"

struct GraphDP : TreeDP {

    // --- H.1. Количество путей ---
    // Топологический порядок (Kahn): dp[v] = Σ_{(u,v)} dp[u] —
    // все предшественники u обработаны раньше v; база dp[s] = 1.
    // O(V + E) времени и памяти.
    vector<long long> count_dag_paths(const vector<vector<int>>& adj, int s) {
        int n = (int)adj.size();
        vector<int> ord = topo_sort(adj);
        vector<long long> dp(n, 0);
        dp[s] = 1;
        for (int v : ord)
            for (int to : adj[v]) dp[to] += dp[v];
        return dp;
    }

    // --- H.2. Самый длинный/короткий путь ---
    // dp[v] = max_{(u,v)} (dp[u] + w(u,v)) в топологическом по-
    // рядке; prev[v] — предшественник; восстановление — обратный
    // проход по prev. Минимум — замена max на min (тот же каркас).
    // O(V + E) времени и памяти.
    pair<long long, vector<int>> dag_longest_path(
        const vector<vector<pair<int, int>>>& adj, int s, int t) {
        int n = (int)adj.size();
        vector<vector<int>> plain(n);
        for (int u = 0; u < n; u++)
            for (auto [v, w] : adj[u]) plain[u].push_back(v);
        vector<int> ord = topo_sort(plain);
        const long long NEG = LLONG_MIN / 4;
        vector<long long> dp(n, NEG);
        vector<int> prev(n, -1);
        dp[s] = 0;
        for (int v : ord) {
            if (dp[v] == NEG) continue;
            for (auto [to, w] : adj[v])
                if (dp[to] < dp[v] + w) {
                    dp[to] = dp[v] + w;
                    prev[to] = v;
                }
        }
        if (dp[t] == NEG) return {NEG, {}};
        vector<int> path;
        for (int v = t; v != -1; v = prev[v]) path.push_back(v);
        reverse(path.begin(), path.end());
        return {dp[t], path};
    }

    // --- H.3. Word Break ---
    // dp[i] = ∃j: dp[j] и s[j..i) ∈ dict — последнее слово;
    // база dp[0] = true. Подсчёт способов — суммы вместо ∃.
    // O(n·|dict|·len) времени, O(n) памяти.
    bool word_break(const string& s, const vector<string>& dict) {
        int n = (int)s.size();
        vector<bool> dp(n + 1, false);
        dp[0] = true;
        for (int i = 1; i <= n; i++)
            for (const string& w : dict)
                if (i >= (int)w.size() && dp[i - w.size()] &&
                        s.substr(i - w.size(), w.size()) == w) {
                    dp[i] = true;
                    break;
                }
        return dp[n];
    }

    long long count_word_breaks(const string& s, const vector<string>& dict) {
        int n = (int)s.size();
        vector<long long> dp(n + 1, 0);
        dp[0] = 1;
        for (int i = 1; i <= n; i++)
            for (const string& w : dict)
                if (i >= (int)w.size() && dp[i - w.size()] &&
                        s.substr(i - w.size(), w.size()) == w)
                    dp[i] += dp[i - w.size()];
        return dp[n];
    }

    // --- H.4. Word Ladder ---
    // BFS от start по графу слов: соседи — слова словаря, отли-
    // чающиеся одной буквой; dist — длина цепочки (слои BFS —
    // кратчайшие пути в невзвешенном графе). O(|dict|·L·σ), где
    // σ — размер алфавита (здесь строчные латинские буквы, σ = 26).
    int word_ladder(const string& start, const string& end,
                    const vector<string>& dict) {
        if (start == end) return 1;
        unordered_set<string> words(dict.begin(), dict.end());
        if (!words.count(end)) return -1;
        unordered_map<string, int> dist;
        queue<string> q;
        dist[start] = 1;
        q.push(start);
        while (!q.empty()) {
            string cur = q.front();
            q.pop();
            int d = dist[cur];
            for (int i = 0; i < (int)cur.size(); i++) {
                string nxt = cur;
                for (char c = 'a'; c <= 'z'; c++) {
                    nxt[i] = c;
                    if (nxt == cur) continue;
                    if (words.count(nxt) && !dist.count(nxt)) {
                        dist[nxt] = d + 1;
                        if (nxt == end) return d + 1;
                        q.push(nxt);
                    }
                }
            }
        }
        return -1;
    }

    // --- H.5. Floyd-Warshall ---
    // ДП по промежуточным вершинам {0..k−1}: d[i][j] = min(d[i][j],
    // d[i][k] + d[k][j]) — путь либо не использует k, либо прохо-
    // дит через k. Транзитивное замыкание — булев вариант (∨/∧).
    // O(V³) времени, O(V²) памяти.
    vector<vector<long long>> floyd_warshall(const vector<vector<long long>>& w) {
        int n = (int)w.size();
        const long long INF = LLONG_MAX / 4;
        vector<vector<long long>> d = w;
        for (int k = 0; k < n; k++)
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    if (d[i][k] < INF && d[k][j] < INF)
                        d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
        return d;
    }

    vector<vector<int>> transitive_closure(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<vector<int>> r(n, vector<int>(n, 0));
        for (int i = 0; i < n; i++) {
            r[i][i] = 1;
            for (int to : adj[i]) r[i][to] = 1;
        }
        for (int k = 0; k < n; k++)
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    r[i][j] = r[i][j] || (r[i][k] && r[k][j]);
        return r;
    }

    // --- H.6. Bellman-Ford ---
    // Релаксация рёбер как ДП по числу шагов: после n−1 итераций —
    // кратчайшие пути (простой путь ≤ n−1 ребро); n-я итерация
    // выявляет отрицательный цикл (ребро продолжает релаксировать).
    // O(V·E) времени, O(V) памяти.
    pair<vector<long long>, bool> bellman_ford(
        int n, const vector<tuple<int, int, int>>& edges, int s) {
        const long long INF = LLONG_MAX / 4;
        vector<long long> dist(n, INF);
        dist[s] = 0;
        for (int it = 0; it < n - 1; it++)
            for (auto [u, v, w] : edges)
                if (dist[u] < INF && dist[u] + w < dist[v])
                    dist[v] = dist[u] + w;
        for (auto [u, v, w] : edges)
            if (dist[u] < INF && dist[u] + w < dist[v])
                return {dist, true};
        return {dist, false};
    }

    // --- H.7. Min Distance Up Bottom ---
    // Итеративный вариант Беллмана-Форда снизу вверх: dp[k][v] —
    // минимум по путям из ≤ k рёбер; слой k−1 → k — один допол-
    // нительный переход (ndp = dp). O(k·E) времени, O(V) памяти.
    vector<long long> min_distance_up_bottom(
        const vector<vector<pair<int, int>>>& adj, int s, int maxSteps) {
        int n = (int)adj.size();
        const long long INF = LLONG_MAX / 4;
        vector<long long> dp(n, INF);
        dp[s] = 0;
        for (int step = 0; step < maxSteps; step++) {
            vector<long long> ndp = dp;
            for (int u = 0; u < n; u++)
                if (dp[u] < INF)
                    for (auto [v, w] : adj[u])
                        ndp[v] = min(ndp[v], dp[u] + w);
            dp.swap(ndp);
        }
        return dp;
    }

    // --- H.8. Число путей длины k (степень матрицы смежности) ---
    // A^k[i][j] — маршруты длины k: маршрут длины a+b склеивается
    // в середине (перебор средней вершины); бинарное возведение в
    // степень, A^0 = I. O(V³·log k) времени, O(V²) памяти.
    vector<vector<long long>> count_walks_length_k(const vector<vector<int>>& adj,
                                                   int k) {
        int n = (int)adj.size();
        auto mul = [&](const vector<vector<long long>>& a,
                       const vector<vector<long long>>& b) {
            vector<vector<long long>> c(n, vector<long long>(n, 0));
            for (int i = 0; i < n; i++)
                for (int t = 0; t < n; t++)
                    if (a[i][t])
                        for (int j = 0; j < n; j++)
                            c[i][j] += a[i][t] * b[t][j];
            return c;
        };
        vector<vector<long long>> A(n, vector<long long>(n, 0));
        vector<vector<long long>> R(n, vector<long long>(n, 0));
        for (int i = 0; i < n; i++) {
            R[i][i] = 1;
            for (int j : adj[i]) A[i][j] = 1;
        }
        while (k > 0) {
            if (k & 1) R = mul(R, A);
            A = mul(A, A);
            k >>= 1;
        }
        return R;
    }

    // --- H.9. Maximum Clique в интервальном графе ---
    // Sweep line: события (начало +1, конец −1) по позиции; мак-
    // симум активных интервалов — ответ (свойство Хелли: попарное
    // пересечение ⇔ общая точка). O(n·log n) времени, O(n) памяти.
    long long max_clique_interval_graph(const vector<pair<int, int>>& intervals) {
        vector<pair<int, int>> events;
        for (auto [l, r] : intervals) {
            events.push_back({l, 1});
            events.push_back({r, -1});
        }
        sort(events.begin(), events.end());
        long long cur = 0, best = 0;
        for (auto [pos, d] : events) {
            cur += d;
            best = max(best, cur);
        }
        return best;
    }

    // --- H.10. MIS в двудольном графе (теорема Кёнига) ---
    // Алгоритм Куна: для каждой левой вершины DFS ищет увеличива-
    // ющий путь (свободное, занятое, свободное, ...) с переметкой
    // занятых рёбер; критерий Бержа — отсутствие увеличивающих
    // путей ⇔ максимальность. MIS = n − matching (Кёниг: MVC =
    // matching, MIS = n − MVC). O(V·E) времени.
    int max_bipartite_matching(const vector<vector<int>>& leftAdj,
                               int nRight) {
        int nLeft = (int)leftAdj.size();
        vector<int> matchR(nRight, -1);
        int res = 0;
        function<bool(int, vector<int>&)> tryKuhn =
            [&](int u, vector<int>& used) {
            for (int v : leftAdj[u]) {
                if (used[v]) continue;
                used[v] = 1;
                if (matchR[v] == -1 || tryKuhn(matchR[v], used)) {
                    matchR[v] = u;
                    return true;
                }
            }
            return false;
        };
        for (int u = 0; u < nLeft; u++) {
            vector<int> used(nRight, 0);
            if (tryKuhn(u, used)) res++;
        }
        return res;
    }

    long long mis_bipartite(const vector<vector<int>>& leftAdj, int nRight) {
        long long n = (long long)leftAdj.size() + nRight;
        return n - max_bipartite_matching(leftAdj, nRight);
    }

    // --- H.12. Игры на DAG ---
    // win(v) = ∃(v,u): ¬win(u); мемоизация DFS от листьев (DAG —
    // рекурсия завершается). P/N-позиции — взаимная индукция
    // (раздел I, I.19). O(V + E) времени и памяти.
    bool can_win_dag(const vector<vector<int>>& adj, int s) {
        int n = (int)adj.size();
        vector<int> state(n, 0);  // 0 — неизвестно, 1 — выигрыш, 2 — проигрыш
        function<bool(int)> dfs = [&](int v) {
            if (state[v]) return state[v] == 1;
            if (adj[v].empty()) {
                state[v] = 2;
                return false;
            }
            for (int to : adj[v])
                if (!dfs(to)) {
                    state[v] = 1;
                    return true;
                }
            state[v] = 2;
            return false;
        };
        return dfs(s);
    }

    // --- H.13. Minimum Path Cover в DAG ---
    // Расщепление: v → v_out (левая доля), v_in (правая); ребро
    // u→v → u_out→v_in; покрытие = n − max паросочетание (Кёниг
    // для DAG): каждая склейка «конец → начало» — ребро паросоче-
    // тания, непокрытые вершины — начала путей. O(V·E) времени.
    long long min_path_cover_dag(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<vector<int>> leftAdj(n);
        for (int u = 0; u < n; u++)
            for (int v : adj[u]) leftAdj[u].push_back(v);
        return (long long)n - max_bipartite_matching(leftAdj, n);
    }

    // --- H.14. Counting путей с ограничениями ---
    // dp[v] = Σ_{(u,v): ограничение выполнено} dp[u] — H.1 с филь-
    // тром перехода (здесь — чередование цветов вершин). O(V + E)
    // времени и памяти.
    long long count_alternating_paths(const vector<vector<int>>& adj,
                                      const vector<int>& colors,
                                      int s, int t) {
        int n = (int)adj.size();
        vector<int> ord = topo_sort(adj);
        vector<long long> dp(n, 0);
        dp[s] = 1;
        for (int v : ord) {
            if (!dp[v]) continue;
            for (int to : adj[v])
                if (colors[to] != colors[v]) dp[to] += dp[v];
        }
        return dp[t];
    }

private:
    // Топологическая сортировка (Kahn) — каркас H.1, H.2, H.14.
    // O(V + E). DAG предполагается корректным.
    vector<int> topo_sort(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<int> indeg(n, 0);
        for (int u = 0; u < n; u++)
            for (int v : adj[u]) indeg[v]++;
        queue<int> q;
        for (int v = 0; v < n; v++)
            if (indeg[v] == 0) q.push(v);
        vector<int> ord;
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            ord.push_back(v);
            for (int to : adj[v])
                if (--indeg[to] == 0) q.push(to);
        }
        return ord;
    }
};

#ifndef GRAPHDP_MAIN
signed main() {
    GraphDP dp;
    const long long INF = LLONG_MAX / 4;

    vector<vector<int>> dag = {{1, 2}, {3}, {3, 4}, {5}, {5}, {}};
    auto cnt = dp.count_dag_paths(dag, 0);
    cout << "H.1: путей 0->5: " << cnt[5] << " (ожидаем 3), 0->3: " << cnt[3]
         << " (ожидаем 2)\n";
    auto cnt2 = dp.count_dag_paths(dag, 2);
    cout << "H.1: путей 2->5: " << cnt2[5] << " (ожидаем 2)\n";

    vector<vector<pair<int, int>>> dagw = {{{1, 4}, {2, 1}}, {{3, 2}},
                                           {{3, 4}, {4, 2}}, {{5, 5}},
                                           {{5, 1}}, {}};
    auto lp = dp.dag_longest_path(dagw, 0, 5);
    cout << "H.2: длина = " << lp.first << " (ожидаем 11), путь: {";
    for (size_t i = 0; i < lp.second.size(); i++)
        cout << (i ? ", " : "") << lp.second[i];
    cout << "}\n";

    cout << "H.3: word_break(\"leetcode\") = "
         << dp.word_break("leetcode", {"leet", "code"}) << " (ожидаем 1), "
         << "catsandog = " << dp.word_break("catsandog",
             {"cats", "dog", "sand", "and", "cat"}) << " (ожидаем 0)\n";
    cout << "H.3: count_word_breaks(\"aaaa\", {a, aa}) = "
         << dp.count_word_breaks("aaaa", {"a", "aa"}) << " (ожидаем 5)\n";

    cout << "H.4: hit->cog = "
         << dp.word_ladder("hit", "cog", {"hot", "dot", "dog", "lot", "log", "cog"})
         << " (ожидаем 5), hot->dog = "
         << dp.word_ladder("hot", "dog", {"hot", "dog", "dot"})
         << " (ожидаем 3), без cog = "
         << dp.word_ladder("hit", "cog", {"hot", "dot", "dog", "lot", "log"})
         << " (ожидаем -1)\n";

    vector<vector<long long>> w4 = {{0, 3, INF, 7}, {8, 0, 2, INF},
                                    {5, INF, 0, 1}, {2, INF, INF, 0}};
    auto fw = dp.floyd_warshall(w4);
    cout << "H.5: dist[0][2] = " << fw[0][2] << " (ожидаем 5), dist[0][3] = "
         << fw[0][3] << " (ожидаем 6), dist[3][2] = " << fw[3][2]
         << " (ожидаем 7)\n";
    auto tc = dp.transitive_closure({{1}, {2}, {}, {0}});
    cout << "H.5: замыкание: 3->2 = " << tc[3][2] << " (ожидаем 1), 2->3 = "
         << tc[2][3] << " (ожидаем 0)\n";

    auto bf0 = dp.bellman_ford(4, {{0, 1, 3}, {0, 3, 7}, {1, 0, 8}, {1, 2, 2},
                                   {2, 0, 5}, {2, 3, 1}, {3, 0, 2}}, 0);
    cout << "H.6: dist = {" << bf0.first[0] << ", " << bf0.first[1] << ", "
         << bf0.first[2] << ", " << bf0.first[3] << "} (ожидаем {0, 3, 5, 6}), "
         << "цикл: " << bf0.second << " (ожидаем 0)\n";
    auto bf1 = dp.bellman_ford(3, {{0, 1, 1}, {1, 2, -1}, {2, 0, -1}}, 0);
    cout << "H.6: отрицательный цикл: " << bf1.second << " (ожидаем 1)\n";

    vector<vector<pair<int, int>>> spg = {{{1, 3}, {3, 7}}, {{2, 2}},
                                          {{3, 1}}, {}};
    auto l1 = dp.min_distance_up_bottom(spg, 0, 1);
    auto l2 = dp.min_distance_up_bottom(spg, 0, 2);
    auto l3 = dp.min_distance_up_bottom(spg, 0, 3);
    cout << "H.7: <= 1 ребра dist[3] = " << l1[3] << " (ожидаем 7), "
         << "<= 2 рёбер dist[2] = " << l2[2] << " (ожидаем 5), dist[3] = "
         << l2[3] << " (ожидаем 7), <= 3 рёбер dist[3] = " << l3[3]
         << " (ожидаем 6)\n";

    vector<vector<int>> c3 = {{1, 2}, {0, 2}, {0, 1}};
    auto w2 = dp.count_walks_length_k(c3, 2);
    auto w4m = dp.count_walks_length_k(c3, 4);
    cout << "H.8: цикл C3: A^2[0][0] = " << w2[0][0] << " (ожидаем 2), "
         << "A^2[0][1] = " << w2[0][1] << " (ожидаем 1), A^4[0][0] = "
         << w4m[0][0] << " (ожидаем 6)\n";
    vector<vector<int>> chain = {{1}, {2}, {3}, {}};
    auto c2m = dp.count_walks_length_k(chain, 2);
    auto c3m = dp.count_walks_length_k(chain, 3);
    cout << "H.8: цепочка: A^2[0][2] = " << c2m[0][2] << " (ожидаем 1), "
         << "A^3[0][3] = " << c3m[0][3] << " (ожидаем 1), A^4[0][3] = "
         << dp.count_walks_length_k(chain, 4)[0][3] << " (ожидаем 0)\n";

    cout << "H.9: max clique {(1,4),(2,5),(3,6)} = "
         << dp.max_clique_interval_graph({{1, 4}, {2, 5}, {3, 6}})
         << " (ожидаем 3), {(1,4),(2,3),(3,5),(4,7)} = "
         << dp.max_clique_interval_graph({{1, 4}, {2, 3}, {3, 5}, {4, 7}})
         << " (ожидаем 2)\n";

    vector<vector<int>> bi = {{0, 1}, {0}, {2}};
    cout << "H.10: паросочетание = " << dp.max_bipartite_matching(bi, 3)
         << " (ожидаем 3), MIS = " << dp.mis_bipartite(bi, 3)
         << " (ожидаем 3)\n";
    vector<vector<int>> bi2 = {{0, 1}, {0}, {}};
    cout << "H.10: без ребра 2-5: паросочетание = "
         << dp.max_bipartite_matching(bi2, 3) << " (ожидаем 2), MIS = "
         << dp.mis_bipartite(bi2, 3) << " (ожидаем 4)\n";

    cout << "H.11: цикл {2,3,2} = "
         << dp.max_non_adjacent_sum_circular({2, 3, 2})
         << " (ожидаем 3), {1,2,3,1} = "
         << dp.max_non_adjacent_sum_circular({1, 2, 3, 1})
         << " (ожидаем 4) — переиспользование b.md B.3\n";

    vector<vector<int>> game = {{1, 2, 5}, {3}, {3}, {4}, {5}, {}};
    cout << "H.12: ветвящаяся игра: win(0) = "
         << dp.can_win_dag(game, 0) << " (ожидаем 1)\n";
    vector<vector<int>> game2 = {{1}, {2}, {}};
    cout << "H.12: цепочка 0->1->2: win(0) = "
         << dp.can_win_dag(game2, 0) << " (ожидаем 0)\n";

    vector<vector<int>> dag2 = {{1, 2}, {3}, {3}, {}};
    cout << "H.13: min path cover = " << dp.min_path_cover_dag(dag2)
         << " (ожидаем 2), цепочка из 4 = "
         << dp.min_path_cover_dag(chain) << " (ожидаем 1)\n";

    vector<vector<int>> dag3 = {{1, 2}, {3}, {3}, {4}, {}};
    cout << "H.14: чередующиеся пути 0->4 = "
         << dp.count_alternating_paths(dag3, {0, 1, 1, 0, 1}, 0, 4)
         << " (ожидаем 2)\n";
}
#endif // GRAPHDP_MAIN
#endif // DYNAMIC_H_CPP
