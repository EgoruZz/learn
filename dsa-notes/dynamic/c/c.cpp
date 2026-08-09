#ifndef DYNAMIC_C_CPP
#define DYNAMIC_C_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <deque>
#include <queue>
#include <set>
#include <cctype>
#include <climits>
#include <utility>
using namespace std;

// =============================================================
// C. ДВУМЕРНАЯ И МНОГОМЕРНАЯ ДИНАМИКА
// =============================================================
// Структура md: A. Динамика на матрицах и путях (C.1–C.10)
//               → B. Сравнительная динамика — выравнивание строк (C.11–C.18)
//               → C. Рюкзачные задачи (C.19–C.28)
//
// MatrixDP наследует LinearDP (b.cpp), а через него DPBasics (a.cpp).
// Переиспользуются, а не дублируются: Кадане max_subarray_sum (B.4),
// размен coin_change_min/coin_change_ways (B.6, B.10), пути в сетке
// grid_paths (B.11); бином C_iter — math/combinatorics (a/a.cpp).
// Точки переиспользования: C.2 (grid_paths), C.6 (max_subarray_sum),
// C.26 (coin_change_min, coin_change_ways).
//
// Порядок методов строго соответствует порядку md (C.1 → C.28).
// Ограничения int64 (демонстрационные версии, без модуля): подсчёты
// (C.2, C.9, C.10, C.23, C.24, C.28) растут быстро — демо на малых
// значениях; модулярные версии появятся с задачами в следующих разделах.

#define LINEARDP_MAIN
#include "../b/b.cpp"

struct MatrixDP : LinearDP {

    // --- C.1. Minimum Cost Path: произвольный набор ходов ---
    // f(i, j) = w(i, j) + min_{(Δi,Δj) ∈ moves} f(i−Δi, j−Δj);
    // клетка без входящих ходов: f(i, j) = w(i, j). Ходы обязаны быть
    // «вперёд»: Δi > 0, либо Δi = 0 и Δj > 0 — тогда входящие ходы
    // идут из клеток, обработанных раньше (табуляция по строкам).
    // O(n·m·|moves|) времени, O(n·m) памяти.
    long long min_cost_path(const vector<vector<long long>>& w,
                            const vector<pair<int, int>>& moves) {
        int n = (int)w.size(), m = (int)w[0].size();
        vector<vector<long long>> f(n, vector<long long>(m, 0));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++) {
                long long best = 0;
                bool has = false;
                for (auto [di, dj] : moves) {
                    int pi = i - di, pj = j - dj;
                    if (pi >= 0 && pj >= 0 && pi < n && pj < m &&
                        (!has || f[pi][pj] < best)) {
                        best = f[pi][pj];
                        has = true;
                    }
                }
                f[i][j] = w[i][j] + (has ? best : 0);
            }
        return f[n - 1][m - 1];
    }

    // --- C.1. Minimum Cost Path: классика, ходы {вниз, вправо} ---
    // Частный случай min_cost_path с фиксированным набором ходов.
    long long min_cost_path_2(const vector<vector<long long>>& w) {
        return min_cost_path(w, {{1, 0}, {0, 1}});
    }

    // --- C.2. Unique Paths: клетки с препятствиями ---
    // f(i, j) = (f(i−1, j) + f(i, j−1))·[клетка свободна], f(0,0) = 1 —
    // последний ход либо сверху, либо слева; запрещённая клетка даёт
    // 0 способов. Без препятствий ответ — C(n+m−2, n−1), переиспользуем
    // grid_paths (b.md B.11). O(n·m) времени, O(m) памяти.
    long long unique_paths_obstacles(const vector<vector<int>>& blocked) {
        int n = (int)blocked.size(), m = (int)blocked[0].size();
        vector<long long> dp(m, 0);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++) {
                if (blocked[i][j]) { dp[j] = 0; continue; }
                if (i == 0 && j == 0) dp[j] = 1;
                else dp[j] = dp[j] + (j > 0 ? dp[j - 1] : 0);
            }
        return dp[m - 1];
    }

    // --- C.2. Unique Paths: не более k поворотов ---
    // Состояние расширяется (a.md A.3): f(i, j, t, d) — число путей в
    // (i, j) ровно с t поворотами, последний ход — в направлении d ∈
    // {вниз, вправо}; ход в том же направлении не меняет t,
    // перпендикулярный — увеличивает на 1. Ответ — Σ_{t ≤ k} Σ_d f.
    // O(n·m·k) времени, O(m·k) памяти.
    long long paths_with_turn_limit(int n, int m, int k) {
        // dp[j][t][d]: d = 0 — вниз, d = 1 — вправо
        vector<vector<vector<long long>>> dp(
            m, vector<vector<long long>>(k + 1, vector<long long>(2, 0)));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++) {
                if (i == 0 && j == 0) {
                    dp[j][0][0] = 1;   // виртуальный «ход» из старта
                    dp[j][0][1] = 1;
                    continue;
                }
                vector<vector<long long>> cur(
                    k + 1, vector<long long>(2, 0));
                if (i > 0)  // вход сверху: последний ход — вниз
                    for (int t = 0; t <= k; t++) {
                        cur[t][0] += dp[j][t][0];                // шли вниз
                        if (t > 0) cur[t][0] += dp[j][t - 1][1]; // поворот
                    }
                if (j > 0)  // вход слева: последний ход — вправо
                    for (int t = 0; t <= k; t++) {
                        cur[t][1] += dp[j - 1][t][1];                // шли вправо
                        if (t > 0) cur[t][1] += dp[j - 1][t - 1][0]; // поворот
                    }
                dp[j] = cur;
            }
        long long ans = 0;
        for (int t = 0; t <= k; t++)
            for (int d = 0; d < 2; d++) ans += dp[m - 1][t][d];
        return ans;
    }

    // --- C.3. Maximum Path Sum: треугольник ---
    // Снизу вверх: f(i, j) = a(i, j) + max(f(i+1, j), f(i+1, j+1));
    // ответ f(0, 0). O(n²) времени, O(n) памяти (один массив нижнего
    // уровня пересчитывается по строкам).
    long long max_path_sum_triangle(const vector<vector<long long>>& a) {
        vector<long long> dp = a.back();
        for (int i = (int)a.size() - 2; i >= 0; i--)
            for (int j = 0; j <= i; j++)
                dp[j] = a[i][j] + max(dp[j], dp[j + 1]);
        return dp[0];
    }

    // --- C.3. Maximum Path Sum: матрица ---
    // Старт — любая клетка первого столбца, финиш — любого последнего;
    // f(i, 0) = a(i, 0), f(i, j) = a(i, j) + max(f(i−1, j), f(i, j−1));
    // ответ — максимум по последнему столбцу. O(n·m) времени и памяти.
    long long max_path_sum_matrix(const vector<vector<long long>>& a) {
        int n = (int)a.size(), m = (int)a[0].size();
        vector<vector<long long>> f(n, vector<long long>(m, 0));
        for (int i = 0; i < n; i++) f[i][0] = a[i][0];
        for (int j = 1; j < m; j++)
            for (int i = 0; i < n; i++) {
                long long best = f[i][j - 1];
                if (i > 0) best = max(best, f[i - 1][j]);
                f[i][j] = a[i][j] + best;
            }
        long long ans = -(LLONG_MAX / 4);
        for (int i = 0; i < n; i++) ans = max(ans, f[i][m - 1]);
        return ans;
    }

    // --- C.4. Trapped Water (2D): заливка от границы ---
    // Приоритетная очередь — «фронт заливки»: клетка с минимальной
    // высотой на границе открытой области определяет уровень воды для
    // клеток за ней. Уровень level = max(level, h) при извлечении;
    // сосед заливается (вода level − h(сосед)), если уровень уже выше
    // его высоты. O(n·m·log(n·m)) времени, O(n·m) памяти.
    long long trapped_water_2d(const vector<vector<long long>>& h) {
        int n = (int)h.size(), m = (int)h[0].size();
        if (n < 3 || m < 3) return 0;
        typedef pair<long long, pair<int, int>> Cell;
        priority_queue<Cell, vector<Cell>, greater<Cell>> pq;
        vector<vector<bool>> vis(n, vector<bool>(m, false));
        auto add = [&](int i, int j) {
            if (!vis[i][j]) {
                vis[i][j] = true;
                pq.push({h[i][j], {i, j}});
            }
        };
        for (int i = 0; i < n; i++) { add(i, 0); add(i, m - 1); }
        for (int j = 0; j < m; j++) { add(0, j); add(n - 1, j); }
        int di[4] = {1, -1, 0, 0}, dj[4] = {0, 0, 1, -1};
        long long level = 0, ans = 0;
        while (!pq.empty()) {
            auto [hgt, pos] = pq.top();
            pq.pop();
            level = max(level, hgt);
            for (int d = 0; d < 4; d++) {
                int ni = pos.first + di[d], nj = pos.second + dj[d];
                if (ni < 0 || nj < 0 || ni >= n || nj >= m || vis[ni][nj]) continue;
                vis[ni][nj] = true;
                if (level > h[ni][nj]) ans += level - h[ni][nj];
                pq.push({h[ni][nj], {ni, nj}});
            }
        }
        return ans;
    }

    // --- C.5. Maximal Square: наибольший квадрат из единиц ---
    // f(i, j) = 1 + min(f(i−1, j), f(i, j−1), f(i−1, j−1)) при a(i, j) =
    // 1 — сторона наибольшего квадрата с правым нижним углом в (i, j);
    // ответ — максимум f². O(n·m) времени, O(m) памяти.
    long long maximal_square(const vector<vector<int>>& a) {
        int n = (int)a.size(), m = (int)a[0].size();
        vector<long long> dp(m, 0);
        long long side = 0;
        for (int i = 0; i < n; i++) {
            long long diag = 0;   // f(i−1, j−1)
            for (int j = 0; j < m; j++) {
                long long up = dp[j];   // f(i−1, j)
                long long left = (j > 0 ? dp[j - 1] : 0);   // f(i, j−1)
                dp[j] = (a[i][j] == 1 ? 1 + min(min(up, left), diag) : 0);
                diag = up;
                side = max(side, dp[j]);
            }
        }
        return side * side;
    }

    // --- C.5. Largest Rectangle in Histogram: монотонный стек ---
    // Для каждого столбца j наибольший прямоугольник «упирается» в
    // h(j): границы — ближайшие меньшие высоты слева и справа,
    // площадь = h(j)·(right − left − 1). O(m) времени, O(m) памяти.
    long long largest_rect_histogram(const vector<long long>& h) {
        int m = (int)h.size();
        long long best = 0;
        vector<int> st;
        for (int j = 0; j <= m; j++) {
            long long hj = (j < m ? h[j] : 0);
            while (!st.empty() && h[st.back()] > hj) {
                long long height = h[st.back()];
                st.pop_back();
                int left = st.empty() ? -1 : st.back();
                best = max(best, height * (j - left - 1));
            }
            st.push_back(j);
        }
        return best;
    }

    // --- C.5. Maximal Rectangle: гистограммы по строкам ---
    // h(j) = число единиц подряд в столбце j, считая строку i последней;
    // наибольший прямоугольник в каждой гистограмме — largest_rect_histogram.
    // O(n·m) времени, O(m) памяти.
    long long maximal_rectangle(const vector<vector<int>>& a) {
        int n = (int)a.size(), m = (int)a[0].size();
        vector<long long> h(m, 0);
        long long best = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++)
                h[j] = (a[i][j] == 1 ? h[j] + 1 : 0);
            best = max(best, largest_rect_histogram(h));
        }
        return best;
    }

    // --- C.6. Maximum Sum Submatrix: пары строк + Кадане ---
    // Фиксируем пару строк (top, bottom): c(j) = Σ_{i=top..bottom} a(i, j);
    // максимум между этими строками — максимальный подотрезок c, Кадане —
    // переиспользуем max_subarray_sum (b.md B.4). Перебор всех пар строк:
    // O(n²·m) времени, O(m) памяти.
    long long max_sum_submatrix(const vector<vector<long long>>& a) {
        int n = (int)a.size(), m = (int)a[0].size();
        long long best = -(LLONG_MAX / 4);
        for (int top = 0; top < n; top++) {
            vector<long long> c(m, 0);
            for (int bottom = top; bottom < n; bottom++) {
                for (int j = 0; j < m; j++) c[j] += a[bottom][j];
                best = max(best, max_subarray_sum(c));
            }
        }
        return best;
    }

    // --- C.6. 1D-ядро: подотрезок с суммой ≤ K (формализовано в D.19) ---
    // Префиксные суммы: подотрезок (l, r] имеет сумму pref[r] − pref[l];
    // для каждого pref[r] нужен наибольший l с pref[l] ≥ pref[r] − K —
    // ищем lower_bound(pref[r] − K) во множестве встреченных префиксов.
    // O(m·log m) времени, O(m) памяти.
    long long max_subarray_sum_no_more_than_k(
            const vector<long long>& a, long long K) {
        long long best = -(LLONG_MAX / 4);
        long long pref = 0;
        set<long long> seen;
        seen.insert(0);
        for (long long x : a) {
            pref += x;
            auto it = seen.lower_bound(pref - K);
            if (it != seen.end()) best = max(best, pref - *it);
            seen.insert(pref);
        }
        return best;
    }

    // --- C.6. Maximum Sum Submatrix ≤ K ---
    // Тот же перебор пар строк; максимум между строками — 1D-ядро
    // max_subarray_sum_no_more_than_k (переиспользуем). O(n²·m·log m).
    long long max_sum_submatrix_no_more_than_k(
            const vector<vector<long long>>& a, long long K) {
        int n = (int)a.size(), m = (int)a[0].size();
        long long best = -(LLONG_MAX / 4);
        for (int top = 0; top < n; top++) {
            vector<long long> c(m, 0);
            for (int bottom = top; bottom < n; bottom++) {
                for (int j = 0; j < m; j++) c[j] += a[bottom][j];
                best = max(best, max_subarray_sum_no_more_than_k(c, K));
            }
        }
        return best;
    }

    // --- C.7. Dungeon Game: потребности от конца к началу ---
    // need(i, j) = max(1, min(need(i+1, j), need(i, j+1)) − a(i, j)) —
    // минимальное здоровье, с которым можно выйти из (i, j) и выжить;
    // за пределами матрицы потребность 1 (у принцессы) и бесконечность
    // (переход туда невозможен). Обратная динамика исключает параметр
    // «текущее здоровье» из состояния (a.md A.3). O(n·m) времени.
    long long dungeon_game(const vector<vector<long long>>& a) {
        int n = (int)a.size(), m = (int)a[0].size();
        const long long INF = LLONG_MAX / 4;
        vector<vector<long long>> need(n + 1, vector<long long>(m + 1, INF));
        need[n - 1][m] = 1;   // справа от принцессы
        need[n][m - 1] = 1;   // снизу от принцессы
        for (int i = n - 1; i >= 0; i--)
            for (int j = m - 1; j >= 0; j--)
                need[i][j] = max(1LL, min(need[i + 1][j], need[i][j + 1]) - a[i][j]);
        return need[0][0];
    }

    // --- C.8. Cherry Pickup: две «машины» ---
    // Обратный путь эквивалентен второму пути из (0, 0); обе машины
    // делают по t шагов: i₁ + j₁ = i₂ + j₂ = t — состояние (t, i₁, i₂);
    // вишня в общей клетке считается один раз. O(n³) времени, O(n²) памяти.
    long long cherry_pickup(const vector<vector<int>>& g) {
        int n = (int)g.size();
        const long long NEG = -(LLONG_MAX / 4);
        if (g[0][0] == -1 || g[n - 1][n - 1] == -1) return 0;
        vector<vector<long long>> dp(n, vector<long long>(n, NEG));
        vector<vector<long long>> nxt(n, vector<long long>(n, NEG));
        dp[0][0] = (g[0][0] == 1 ? 1 : 0);
        for (int t = 1; t <= 2 * n - 2; t++) {
            for (auto& row : nxt) fill(row.begin(), row.end(), NEG);
            int lo = max(0, t - (n - 1)), hi = min(n - 1, t);
            for (int i1 = lo; i1 <= hi; i1++)
                for (int i2 = lo; i2 <= hi; i2++) {
                    int j1 = t - i1, j2 = t - i2;
                    if (g[i1][j1] == -1 || g[i2][j2] == -1) continue;
                    long long best = NEG;
                    for (int p1 = i1 - 1; p1 <= i1; p1++)
                        for (int p2 = i2 - 1; p2 <= i2; p2++)
                            if (p1 >= 0 && p2 >= 0 && p1 < n && p2 < n)
                                best = max(best, dp[p1][p2]);
                    if (best == NEG) continue;
                    long long val = best + (g[i1][j1] == 1 ? 1 : 0);
                    if (i1 != i2) val += (g[i2][j2] == 1 ? 1 : 0);
                    nxt[i1][i2] = val;
                }
            swap(dp, nxt);
        }
        return max(0LL, dp[n - 1][n - 1]);
    }

    // --- C.9. Count Square Submatrices ---
    // f(i, j) — сторона наибольшего квадрата с углом в (i, j) (C.5),
    // число квадратов, «заканчивающихся» в (i, j), равно f(i, j);
    // каждый квадрат матрицы имеет единственный правый нижний угол —
    // суммирование без двойного счёта. O(n·m) времени, O(m) памяти.
    long long count_square_submatrices(const vector<vector<int>>& a) {
        int n = (int)a.size(), m = (int)a[0].size();
        vector<long long> dp(m, 0);
        long long ans = 0;
        for (int i = 0; i < n; i++) {
            long long diag = 0;
            for (int j = 0; j < m; j++) {
                long long up = dp[j];
                long long left = (j > 0 ? dp[j - 1] : 0);
                dp[j] = (a[i][j] == 1 ? 1 + min(min(up, left), diag) : 0);
                diag = up;
                ans += dp[j];
            }
        }
        return ans;
    }

    // --- C.10. Ладьи на доске Феррера (строки по возрастанию длин) ---
    // f(i, j) = f(i−1, j) + f(i−1, j−1)·(h(i) − j + 1): либо не берём
    // строку i, либо ставим ладью — j−1 уже поставленных занимают
    // столбцы ≤ h(i) (строки короче), свободных клеток h(i) − (j−1).
    // Сортировка по возрастанию обязательна. O(k²) времени, O(k) памяти.
    vector<long long> ferrers_rook_dp(const vector<long long>& h) {
        int k = (int)h.size();
        vector<long long> f(k + 1, 0);
        f[0] = 1;
        for (int i = 0; i < k; i++)
            for (int j = min(i + 1, k); j >= 1; j--)
                f[j] += f[j - 1] * (h[i] - j + 1);
        return f;
    }

    // --- C.10. Не бьющие слоны на доске n×n ---
    // Цвета независимы: ответ — свёртка f_чёрных·f_белых; диагонали
    // одного цвета — доска Феррера с длинами len(s) = min(s, 2n−2−s)+1
    // (s — номер диагонали, s = 0..2n−2), слон = «ладья» на ней.
    // O(n²) времени, O(n) памяти.
    long long non_attacking_bishops(int n, int k) {
        auto lens = [&](int par) {
            vector<long long> v;
            for (int s = par; s <= 2 * n - 2; s += 2)
                v.push_back(min(s, 2 * n - 2 - s) + 1);
            sort(v.begin(), v.end());
            return v;
        };
        vector<long long> black = lens(0), white = lens(1);
        vector<long long> fb = ferrers_rook_dp(black);
        vector<long long> fw = ferrers_rook_dp(white);
        long long ans = 0;
        for (int j = max(0, k - (int)white.size());
             j <= min(k, (int)black.size()); j++)
            ans += fb[j] * fw[k - j];
        return ans;
    }

    // --- C.11. Edit Distance (Левенштейн) с весами ---
    // f(i, j) = min( f(i−1, j) + β, f(i, j−1) + α,
    //                f(i−1, j−1) + γ·[a(i−1) ≠ b(j−1)] ),
    // базы f(i, 0) = i·β, f(0, j) = j·α; α — вставка, β — удаление,
    // γ — замена. O(n·m) времени, O(n·m) памяти.
    long long edit_distance_general(const string& a, const string& b,
                                    long long alpha, long long beta,
                                    long long gamma) {
        int n = (int)a.size(), m = (int)b.size();
        vector<vector<long long>> f(n + 1, vector<long long>(m + 1, 0));
        for (int i = 0; i <= n; i++) f[i][0] = i * beta;
        for (int j = 0; j <= m; j++) f[0][j] = j * alpha;
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++)
                f[i][j] = min(f[i - 1][j] + beta,
                              min(f[i][j - 1] + alpha,
                                  f[i - 1][j - 1] +
                                      (a[i - 1] == b[j - 1] ? 0 : gamma)));
        return f[n][m];
    }

    // --- C.11. Edit Distance: классика (α = β = γ = 1) ---
    long long edit_distance(const string& a, const string& b) {
        return edit_distance_general(a, b, 1, 1, 1);
    }

    // --- C.11. Edit Distance: восстановление выравнивания ---
    // Обратный проход по таблице (a.md B.4): совпадение — диагональ,
    // иначе ветка, давшая минимум (удаление — гэп в b, вставка — в a).
    // Возвращает (стоимость, выровненные строки a', b').
    pair<long long, pair<string, string>> edit_distance_alignment(
            const string& a, const string& b,
            long long alpha, long long beta, long long gamma) {
        int n = (int)a.size(), m = (int)b.size();
        vector<vector<long long>> f(n + 1, vector<long long>(m + 1, 0));
        for (int i = 0; i <= n; i++) f[i][0] = i * beta;
        for (int j = 0; j <= m; j++) f[0][j] = j * alpha;
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++)
                f[i][j] = min(f[i - 1][j] + beta,
                              min(f[i][j - 1] + alpha,
                                  f[i - 1][j - 1] +
                                      (a[i - 1] == b[j - 1] ? 0 : gamma)));
        string ra, rb;
        int i = n, j = m;
        while (i > 0 || j > 0) {
            if (i > 0 && j > 0 &&
                f[i][j] == f[i - 1][j - 1] +
                               (a[i - 1] == b[j - 1] ? 0 : gamma)) {
                ra += a[i - 1];
                rb += b[j - 1];
                i--; j--;
            } else if (i > 0 && f[i][j] == f[i - 1][j] + beta) {
                ra += a[i - 1];
                rb += '-';
                i--;
            } else {
                ra += '-';
                rb += b[j - 1];
                j--;
            }
        }
        reverse(ra.begin(), ra.end());
        reverse(rb.begin(), rb.end());
        return {f[n][m], {ra, rb}};
    }

    // --- C.12. Longest Common Subsequence: длина ---
    // f(i, j) = f(i−1, j−1) + 1 при a(i−1) = b(j−1), иначе
    // f(i, j) = max(f(i−1, j), f(i, j−1)). O(n·m) времени, O(m) памяти.
    long long lcs(const string& a, const string& b) {
        int n = (int)a.size(), m = (int)b.size();
        vector<long long> dp(m + 1, 0), ndp(m + 1, 0);
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= m; j++) {
                if (a[i - 1] == b[j - 1]) ndp[j] = dp[j - 1] + 1;
                else ndp[j] = max(dp[j], ndp[j - 1]);
            }
            swap(dp, ndp);
        }
        return dp[m];
    }

    // --- C.12. Longest Common Subsequence: восстановление ---
    // Обратный проход: при равенстве — диагональ, иначе — в большего
    // из соседей (при равенстве вверх, a.md B.4). O(n·m) памяти.
    string lcs_with_path(const string& a, const string& b) {
        int n = (int)a.size(), m = (int)b.size();
        vector<vector<long long>> f(n + 1, vector<long long>(m + 1, 0));
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++) {
                if (a[i - 1] == b[j - 1]) f[i][j] = f[i - 1][j - 1] + 1;
                else f[i][j] = max(f[i - 1][j], f[i][j - 1]);
            }
        string res;
        int i = n, j = m;
        while (i > 0 && j > 0) {
            if (a[i - 1] == b[j - 1]) {
                res += a[i - 1];
                i--; j--;
            } else if (f[i - 1][j] >= f[i][j - 1]) i--;
            else j--;
        }
        reverse(res.begin(), res.end());
        return res;
    }

    // --- C.12. LCS перестановок через LIS: O(n log n) ---
    // pos(x) — позиция x в a; LCS(a, b) = LIS(pos(b₁), ..., pos(bₘ)) —
    // возрастающая подпоследовательность индексов; «пасьянс» с
    // lower_bound (детальный разбор LIS — раздел D, D.14).
    long long lcs_perm(const vector<long long>& a, const vector<long long>& b) {
        vector<long long> pos(a.size() + 1);
        for (int i = 0; i < (int)a.size(); i++) pos[a[i]] = i;
        vector<long long> tails;
        for (long long x : b) {
            auto it = lower_bound(tails.begin(), tails.end(), pos[x]);
            if (it == tails.end()) tails.push_back(pos[x]);
            else *it = pos[x];
        }
        return (long long)tails.size();
    }

    // --- C.13. Longest Common Substring ---
    // f(i, j) = f(i−1, j−1) + 1 при совпадении, иначе 0 (обрыв) — длина
    // общего суффикса префиксов; ответ — максимум по всей таблице.
    // O(n·m) времени, O(m) памяти.
    long long longest_common_substring(const string& a, const string& b) {
        int n = (int)a.size(), m = (int)b.size();
        vector<long long> dp(m + 1, 0);
        long long best = 0;
        for (int i = 1; i <= n; i++) {
            long long diag = 0;
            for (int j = 1; j <= m; j++) {
                long long up = dp[j];
                dp[j] = (a[i - 1] == b[j - 1] ? diag + 1 : 0);
                diag = up;
                best = max(best, dp[j]);
            }
        }
        return best;
    }

    // --- C.14. Wildcard Matching ---
    // '?' — один любой символ, '*' — любая (возможно пустая) строка:
    // f(i, j) = f(i, j−1) ИЛИ f(i−1, j) для '*', иначе обычное
    // сопоставление. O(n·m) времени, O(m) памяти.
    bool wildcard_match(const string& s, const string& p) {
        int n = (int)s.size(), m = (int)p.size();
        vector<char> dp(m + 1, 0);
        dp[0] = 1;
        for (int j = 1; j <= m; j++)
            if (p[j - 1] == '*') dp[j] = dp[j - 1];   // пустая строка vs '*...'
        for (int i = 1; i <= n; i++) {
            vector<char> cur(m + 1, 0);
            for (int j = 1; j <= m; j++) {
                if (p[j - 1] == '*')
                    cur[j] = cur[j - 1] || dp[j];   // '*' пуста | '*' съела s(i−1)
                else
                    cur[j] = (p[j - 1] == '?' || p[j - 1] == s[i - 1]) &&
                             dp[j - 1];
            }
            dp = cur;
        }
        return dp[m];
    }

    // --- C.15. Regex Match ---
    // '.' — один любой символ; «x*» — ноль и более повторений
    // предыдущего символа: f(i, j) = f(i, j−2) ИЛИ (x совпал И f(i−1, j)).
    // O(n·m) времени, O(n·m) памяти.
    bool regex_match(const string& s, const string& p) {
        int n = (int)s.size(), m = (int)p.size();
        vector<vector<char>> f(n + 1, vector<char>(m + 1, 0));
        f[0][0] = 1;
        for (int j = 1; j <= m; j++)
            if (p[j - 1] == '*') f[0][j] = f[0][j - 2];   // «x*» — ноль раз
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++) {
                if (p[j - 1] == '*') {
                    f[i][j] = f[i][j - 2];   // ноль повторений
                    if (j >= 2 && (p[j - 2] == '.' || p[j - 2] == s[i - 1]) &&
                        f[i - 1][j])
                        f[i][j] = 1;         // съели s(i−1), пара остаётся
                } else if (p[j - 1] == '.' || p[j - 1] == s[i - 1]) {
                    f[i][j] = f[i - 1][j - 1];
                }
            }
        return f[n][m];
    }

    // --- C.16. Abbreviation ---
    // Строчную букву s можно удалить или оставить как заглавную
    // (совпадение регистронезависимое); заглавную удалить нельзя —
    // она обязана совпасть с t. Полное разбиение по последнему символу.
    // O(n·m) времени, O(n·m) памяти.
    bool can_abbreviate(const string& s, const string& t) {
        int n = (int)s.size(), m = (int)t.size();
        vector<vector<char>> f(n + 1, vector<char>(m + 1, 0));
        f[0][0] = 1;
        for (int i = 1; i <= n; i++)
            f[i][0] = f[i - 1][0] && islower((unsigned char)s[i - 1]);
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++) {
                char c = s[i - 1];
                if (islower((unsigned char)c)) {
                    f[i][j] = f[i - 1][j];   // удалить
                    if (toupper((unsigned char)c) == (unsigned char)t[j - 1] &&
                        f[i - 1][j - 1])
                        f[i][j] = 1;         // оставить как заглавную
                } else if (c == t[j - 1]) {
                    f[i][j] = f[i - 1][j - 1];
                }
            }
        return f[n][m];
    }

    // --- C.17. Smith-Waterman: локальное выравнивание ---
    // f(i, j) = max(0, f(i−1, j−1) + S(aᵢ, bⱼ), f(i−1, j) − g,
    //               f(i, j−1) − g); обнуление — выравнивание может
    // начаться в любой клетке; ответ — максимум по ВСЕЙ матрице.
    // O(n·m) времени, O(m) памяти.
    long long smith_waterman(const string& a, const string& b,
                             long long match, long long mismatch, long long g) {
        int n = (int)a.size(), m = (int)b.size();
        vector<long long> dp(m + 1, 0);
        long long best = 0;
        for (int i = 1; i <= n; i++) {
            long long diag = 0;   // f(i−1, j−1)
            for (int j = 1; j <= m; j++) {
                long long up = dp[j];   // f(i−1, j)
                long long sc = (a[i - 1] == b[j - 1]) ? match : mismatch;
                long long val = max(0LL, max(diag + sc, max(up - g, dp[j - 1] - g)));
                dp[j] = val;
                diag = up;
                best = max(best, val);
            }
        }
        return best;
    }

    // --- C.18. Needleman-Wunsch: глобальное выравнивание ---
    // Та же схема без обнуления: f(i, 0) = −i·g, f(0, j) = −j·g,
    // f(i, j) = max(совпадение/замена, гэп в a, гэп в b); ответ f(n, m).
    // Единая параметризация (S, g): C.11 — min-версия, C.12 — S = 1/0,
    // g = 0, C.17 — локальная. O(n·m) времени и памяти.
    long long needleman_wunsch(const string& a, const string& b,
                               long long match, long long mismatch, long long g) {
        int n = (int)a.size(), m = (int)b.size();
        vector<vector<long long>> f(n + 1, vector<long long>(m + 1, 0));
        for (int i = 0; i <= n; i++) f[i][0] = -i * g;
        for (int j = 0; j <= m; j++) f[0][j] = -j * g;
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++) {
                long long sc = (a[i - 1] == b[j - 1]) ? match : mismatch;
                f[i][j] = max(f[i - 1][j - 1] + sc,
                              max(f[i - 1][j] - g, f[i][j - 1] - g));
            }
        return f[n][m];
    }

    // --- C.19. 0-1 Knapsack ---
    // f(w) = max(f(w), f(w − cᵢ) + vᵢ) — по весам СВЕРХУ ВНИЗ: предмет
    // используется не более одного раза (f(w − cᵢ) ещё не обновлён
    // предметом i). O(n·W) времени, O(W) памяти.
    long long knapsack_01(const vector<long long>& c,
                          const vector<long long>& v, long long W) {
        vector<long long> f(W + 1, 0);
        for (int i = 0; i < (int)c.size(); i++)
            for (long long w = W; w >= c[i]; w--)
                f[w] = max(f[w], f[w - c[i]] + v[i]);
        return f[W];
    }

    // --- C.19. 0-1 Knapsack: восстановление набора ---
    // Двумерная таблица + обратный проход (a.md B.4): предмет i входит,
    // если f(i, w) ≠ f(i−1, w). Возвращает (ценность, индексы предметов).
    pair<long long, vector<int>> knapsack_01_path(
            const vector<long long>& c, const vector<long long>& v, long long W) {
        int n = (int)c.size();
        vector<vector<long long>> f(n + 1, vector<long long>(W + 1, 0));
        for (int i = 1; i <= n; i++)
            for (long long w = 0; w <= W; w++) {
                f[i][w] = f[i - 1][w];
                if (w >= c[i - 1])
                    f[i][w] = max(f[i][w], f[i - 1][w - c[i - 1]] + v[i - 1]);
            }
        vector<int> take;
        long long w = W;
        for (int i = n; i >= 1; i--)
            if (f[i][w] != f[i - 1][w]) {
                take.push_back(i - 1);
                w -= c[i - 1];
            }
        return {f[n][W], take};
    }

    // --- C.19. 0-1 Knapsack: ДП по ценности (огромные W) ---
    // g(v) — минимальный вес для ценности ровно v; g(v) = min(g(v),
    // g(v − vᵢ) + cᵢ) по v сверху вниз; ответ — max v с g(v) ≤ W.
    // O(n·V) времени, V = Σvᵢ; O(V) памяти.
    long long knapsack_by_value(const vector<long long>& c,
                                const vector<long long>& v, long long W) {
        long long V = 0;
        for (long long x : v) V += x;
        const long long INF = LLONG_MAX / 4;
        vector<long long> g(V + 1, INF);
        g[0] = 0;
        for (int i = 0; i < (int)c.size(); i++)
            for (long long val = V; val >= v[i]; val--)
                if (g[val - v[i]] != INF)
                    g[val] = min(g[val], g[val - v[i]] + c[i]);
        for (long long val = V; val >= 0; val--)
            if (g[val] <= W) return val;
        return 0;
    }

    // --- C.20. Неограниченный рюкзак ---
    // По весам СНИЗУ ВВЕРХ: f(w) = maxᵢ (f(w − cᵢ) + vᵢ) — проход вверх
    // позволяет многократное использование предмета (это единственное
    // отличие от C.19). O(n·W) времени, O(W) памяти.
    long long knapsack_unbounded(const vector<long long>& c,
                                 const vector<long long>& v, long long W) {
        vector<long long> f(W + 1, 0);
        for (long long w = 1; w <= W; w++)
            for (int i = 0; i < (int)c.size(); i++)
                if (w >= c[i]) f[w] = max(f[w], f[w - c[i]] + v[i]);
        return f[W];
    }

    // --- C.21. Рюкзак с повторениями: двоичное разбиение ---
    // kᵢ экземпляров → группы 1, 2, 4, ..., 2^t, остаток: любое
    // количество до kᵢ набирается суммой подмножества групп (двоичная
    // запись); каждая группа — «предмет» (cᵢ·g, vᵢ·g) → переиспользуем
    // knapsack_01. O(W·Σ log kᵢ) времени, O(W) памяти.
    long long knapsack_bounded(const vector<long long>& c,
                               const vector<long long>& v,
                               const vector<long long>& k, long long W) {
        vector<long long> C, V;
        for (int i = 0; i < (int)c.size(); i++) {
            long long cnt = k[i];
            for (long long g = 1; cnt > 0; g <<= 1) {
                long long take = min(g, cnt);
                cnt -= take;
                C.push_back(c[i] * take);
                V.push_back(v[i] * take);
            }
        }
        return knapsack_01(C, V, W);
    }

    // --- C.21. Рюкзак с повторениями: монотонная очередь ---
    // Полный O(n·W): для предмета и остатка r = w mod cᵢ задача
    // «максимум в окне» по слою q = (w − r)/cᵢ:
    // f(r + q·c) = max_{t ∈ [q − kᵢ, q]} (f_стар(r + t·c) − t·v) + q·v.
    // Монотонная очередь по q (задел — b.md B.22, раздел J).
    long long knapsack_bounded_monotone(const vector<long long>& c,
                                        const vector<long long>& v,
                                        const vector<long long>& k,
                                        long long W) {
        vector<long long> f(W + 1, 0);
        for (int i = 0; i < (int)c.size(); i++) {
            long long ci = c[i], vi = v[i], ki = k[i];
            vector<long long> nf = f;
            for (long long r = 0; r < ci; r++) {
                deque<pair<long long, long long>> dq;   // (f − t·v, t)
                long long qmax = (W - r) / ci;
                for (long long q = 0; q <= qmax; q++) {
                    long long cand = f[r + q * ci] - q * vi;
                    while (!dq.empty() && dq.back().first <= cand) dq.pop_back();
                    dq.push_back({cand, q});
                    while (!dq.empty() && dq.front().second < q - ki)
                        dq.pop_front();
                    nf[r + q * ci] =
                        max(nf[r + q * ci], dq.front().first + q * vi);
                }
            }
            f = nf;
        }
        return f[W];
    }

    // --- C.22. Minimum Partition ---
    // Достижимость сумм по s сверху вниз (0-1); меньшая часть —
    // подмножество с суммой ≤ S/2, максимум такой суммы минимизирует
    // разность S − 2·s. O(n·S/2) времени, O(S) памяти.
    long long min_partition_difference(const vector<long long>& a) {
        long long S = 0;
        for (long long x : a) S += x;
        vector<char> f(S / 2 + 1, 0);
        f[0] = 1;
        for (long long x : a)
            for (long long s = S / 2; s >= x; s--)
                if (f[s - x]) f[s] = 1;
        long long best = 0;
        for (long long s = S / 2; s >= 0; s--)
            if (f[s]) { best = s; break; }
        return S - 2 * best;
    }

    // --- C.23. Subset Sum: достижимость ---
    // f(s) |= f(s − aᵢ) по s сверху вниз — 0-1 семантика.
    // O(n·S) времени, O(S) памяти.
    bool subset_sum_reachable(const vector<long long>& a, long long S) {
        vector<char> f(S + 1, 0);
        f[0] = 1;
        for (long long x : a)
            for (long long s = S; s >= x; s--)
                if (f[s - x]) f[s] = 1;
        return f[S];
    }

    // --- C.23. Subset Sum: число подмножеств ---
    // g(s) += g(s − aᵢ) по s сверху вниз, g(0) = 1 — последний добав-
    // ленный элемент различает подмножества; порядок суммирования (0-1)
    // исключает перестановки. O(n·S) времени, O(S) памяти.
    long long subset_sum_count(const vector<long long>& a, long long S) {
        vector<long long> f(S + 1, 0);
        f[0] = 1;
        for (long long x : a)
            for (long long s = S; s >= x; s--)
                f[s] += f[s - x];
        return f[S];
    }

    // --- C.24. Target Sum: сведение к подмножеству ---
    // ΣP − ΣN = target, ΣP + ΣN = S → ΣP = (S + target)/2 — счёт
    // подмножеств с этой суммой, переиспользуем subset_sum_count (C.23);
    // разрешимо при чётном S + target и 0 ≤ (S + target)/2 ≤ S.
    // O(n·S) времени, O(S) памяти.
    long long target_sum(const vector<long long>& a, long long target) {
        long long S = 0;
        for (long long x : a) S += x;
        long long sumP = S + target;
        if (sumP < 0 || sumP % 2 != 0) return 0;
        sumP /= 2;
        if (sumP > S) return 0;
        return subset_sum_count(a, sumP);
    }

    // --- C.24. Target Sum: прямая динамика (сверка) ---
    // Рюкзак по всем достижимым суммам: диапазон [−S, S] со сдвигом
    // на S; счёт по слоям. O(n·S) времени, O(S) памяти.
    long long target_sum_direct(const vector<long long>& a, long long target) {
        long long S = 0;
        for (long long x : a) S += x;
        vector<long long> f(2 * S + 1, 0), nf(2 * S + 1, 0);
        f[S] = 1;
        for (long long x : a) {
            fill(nf.begin(), nf.end(), 0);
            for (long long s = -S; s <= S; s++)
                if (f[s + S]) {
                    nf[s + x + S] += f[s + S];
                    nf[s - x + S] += f[s + S];
                }
            swap(f, nf);
        }
        return (target >= -S && target <= S) ? f[target + S] : 0;
    }

    // --- C.25. Largest Divisible Subset ---
    // После сортировки свойство «каждая пара делит» эквивалентно цепочке
    // делимости соседей (транзитивность): f(i) = 1 + max{f(j): j < i,
    // a[i] % a[j] == 0} — лучшая цепочка, оканчивающаяся a[i]; prev[i]
    // для восстановления. O(n²) времени, O(n) памяти.
    vector<long long> largest_divisible_subset(const vector<long long>& a) {
        vector<long long> x = a;
        sort(x.begin(), x.end());
        int n = (int)x.size();
        vector<int> f(n, 1), prev(n, -1);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < i; j++)
                if (x[i] % x[j] == 0 && f[j] + 1 > f[i]) {
                    f[i] = f[j] + 1;
                    prev[i] = j;
                }
        int bi = 0;
        for (int i = 1; i < n; i++)
            if (f[i] > f[bi]) bi = i;
        vector<long long> res;
        for (int i = bi; i != -1; i = prev[i]) res.push_back(x[i]);
        reverse(res.begin(), res.end());
        return res;
    }

    // --- C.26. Coin Change: минимум стоимости (монеты с весами) ---
    // Монеты cᵢ со стоимостью costᵢ: g(s) = minᵢ (g(s − cᵢ) + costᵢ) —
    // тот же рюкзак, что C.20, но минимум стоимости вместо максимума
    // ценности (неограниченный запас). Без весов — переиспользуем
    // coin_change_min/coin_change_ways из базы (b.md B.6, B.10).
    // O(s·k) времени, O(s) памяти; −1, если сумма недостижима.
    long long coin_change_min_weighted(const vector<long long>& coins,
                                       const vector<long long>& cost,
                                       long long s) {
        const long long INF = LLONG_MAX / 4;
        vector<long long> g(s + 1, INF);
        g[0] = 0;
        for (long long w = 1; w <= s; w++)
            for (int i = 0; i < (int)coins.size(); i++)
                if (w >= coins[i] && g[w - coins[i]] != INF)
                    g[w] = min(g[w], g[w - coins[i]] + cost[i]);
        return g[s] == INF ? -1 : g[s];
    }

    // --- C.27. 2D-рюкзак (объём + вес) ---
    // f(w, c) = max(f(w, c), f(w − cᵢ, c − pᵢ) + vᵢ) — два измерения
    // состояния (a.md A.3), циклы по обоим сверху вниз (0-1).
    // O(n·W·C) времени, O(W·C) памяти.
    long long knapsack_2d(const vector<long long>& c,
                          const vector<long long>& p,
                          const vector<long long>& v,
                          long long W, long long C) {
        vector<vector<long long>> f(W + 1, vector<long long>(C + 1, 0));
        for (int i = 0; i < (int)c.size(); i++)
            for (long long w = W; w >= c[i]; w--)
                for (long long cap = C; cap >= p[i]; cap--)
                    f[w][cap] =
                        max(f[w][cap], f[w - c[i]][cap - p[i]] + v[i]);
        return f[W][C];
    }

    // --- C.28. Подсчёт подмножеств с суммой S по модулю M ---
    // Та же рекуррентность, что subset_sum_count (C.23), с редукцией
    // по модулю на каждом шаге — олимпиадный стандарт для больших
    // ответов. O(n·S) времени, O(S) памяти.
    long long count_subsets_sum_mod(const vector<long long>& a,
                                    long long S, long long M) {
        vector<long long> f(S + 1, 0);
        f[0] = 1;
        for (long long x : a)
            for (long long s = S; s >= x; s--) {
                f[s] += f[s - x];
                if (f[s] >= M) f[s] -= M;
            }
        return f[S];
    }
};

// =============================================================
// Демонстрация (сверка с ожидаемыми значениями)
// =============================================================
#ifndef MATRIXDP_MAIN
signed main() {
    MatrixDP dp;

    cout << "C.1: min_cost_path(3x3, {вниз,вправо}) = "
         << dp.min_cost_path({{1, 3, 1}, {1, 5, 1}, {4, 2, 1}}, {{1, 0}, {0, 1}})
         << " (ожидаем 7)\n";
    cout << "C.1: min_cost_path(3x3, +диагональ) = "
         << dp.min_cost_path({{1, 3, 1}, {1, 5, 1}, {4, 2, 1}},
                             {{1, 0}, {0, 1}, {1, 1}})
         << " (ожидаем 5: путь (0,0) → (1,0) → (2,1) → (2,2))\n";
    cout << "C.1: min_cost_path_2(3x3) = "
         << dp.min_cost_path_2({{1, 3, 1}, {1, 5, 1}, {4, 2, 1}})
         << " (ожидаем 7)\n";

    cout << "C.2: unique_paths_obstacles(3x3, запрет (1,1)) = "
         << dp.unique_paths_obstacles({{0, 0, 0}, {0, 1, 0}, {0, 0, 0}})
         << " (ожидаем 2)\n";
    cout << "C.2: unique_paths_obstacles(10x10 без запретов) = "
         << dp.unique_paths_obstacles(vector<vector<int>>(10, vector<int>(10, 0)))
         << " == grid_paths(10, 10) = " << dp.grid_paths(10, 10)
         << " (ожидаем 48620, сверка с биномом B.11)\n";
    cout << "C.2: paths_with_turn_limit(2, 2, 0) = "
         << dp.paths_with_turn_limit(2, 2, 0) << " (ожидаем 0)\n";
    cout << "C.2: paths_with_turn_limit(2, 2, 1) = "
         << dp.paths_with_turn_limit(2, 2, 1) << " (ожидаем 2)\n";
    cout << "C.2: paths_with_turn_limit(3, 3, 1) = "
         << dp.paths_with_turn_limit(3, 3, 1) << " (ожидаем 2)\n";

    cout << "C.3: max_path_sum_triangle({{5},{1,6},{4,3,9},{2,7,1,6}}) = "
         << dp.max_path_sum_triangle({{5}, {1, 6}, {4, 3, 9}, {2, 7, 1, 6}})
         << " (ожидаем 26)\n";
    cout << "C.3: max_path_sum_matrix({{1..9}}) = "
         << dp.max_path_sum_matrix({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}})
         << " (ожидаем 26)\n";

    cout << "C.4: trapped_water_2d(3x6) = "
         << dp.trapped_water_2d({{1, 4, 3, 1, 3, 2},
                                 {3, 2, 1, 3, 2, 4},
                                 {2, 3, 3, 2, 3, 1}})
         << " (ожидаем 4)\n";

    cout << "C.5: maximal_square(4x5) = "
         << dp.maximal_square({{1, 0, 1, 0, 0},
                               {1, 0, 1, 1, 1},
                               {1, 1, 1, 1, 1},
                               {1, 0, 0, 1, 0}})
         << " (ожидаем 4)\n";
    cout << "C.5: largest_rect_histogram({2,1,5,6,2,3}) = "
         << dp.largest_rect_histogram({2, 1, 5, 6, 2, 3}) << " (ожидаем 10)\n";
    cout << "C.5: maximal_rectangle(4x5) = "
         << dp.maximal_rectangle({{1, 0, 1, 0, 0},
                                  {1, 0, 1, 1, 1},
                                  {1, 1, 1, 1, 1},
                                  {1, 0, 0, 1, 0}})
         << " (ожидаем 6)\n";

    cout << "C.6: max_sum_submatrix(2x3) = "
         << dp.max_sum_submatrix({{1, 0, 1}, {0, -2, 3}}) << " (ожидаем 4)\n";
    cout << "C.6: max_sum_submatrix_no_more_than_k(2x3, K=2) = "
         << dp.max_sum_submatrix_no_more_than_k({{1, 0, 1}, {0, -2, 3}}, 2)
         << " (ожидаем 2)\n";

    cout << "C.7: dungeon_game(3x3) = "
         << dp.dungeon_game({{-2, -3, 3}, {-5, -10, 1}, {10, 30, -5}})
         << " (ожидаем 7)\n";

    cout << "C.8: cherry_pickup(3x3) = "
         << dp.cherry_pickup({{0, 1, -1}, {1, 0, -1}, {1, 1, 1}})
         << " (ожидаем 5)\n";

    cout << "C.9: count_square_submatrices(3x4) = "
         << dp.count_square_submatrices({{0, 1, 1, 1},
                                         {1, 1, 1, 1},
                                         {0, 1, 1, 1}})
         << " (ожидаем 15)\n";

    auto fr = dp.ferrers_rook_dp({1, 1, 3, 3});
    cout << "C.10: ferrers_rook_dp({1,1,3,3}) =";
    for (long long x : fr) cout << " " << x;
    cout << " (ожидаем 1 8 14 4 0)\n";
    cout << "C.10: non_attacking_bishops(4, 2) = "
         << dp.non_attacking_bishops(4, 2) << " (ожидаем 92)\n";

    cout << "C.11: edit_distance(horse, ros) = "
         << dp.edit_distance("horse", "ros") << " (ожидаем 3)\n";
    cout << "C.11: edit_distance_general(horse, ros, 1, 1, 1) = "
         << dp.edit_distance_general("horse", "ros", 1, 1, 1) << " (ожидаем 3)\n";
    auto al = dp.edit_distance_alignment("kitten", "sitting", 1, 1, 1);
    cout << "C.11: edit_distance_alignment(kitten, sitting) = "
         << al.first << " (ожидаем 3), выравнивание: " << al.second.first
         << " / " << al.second.second << "\n";

    cout << "C.12: lcs(abcde, ace) = " << dp.lcs("abcde", "ace")
         << " (ожидаем 3)\n";
    cout << "C.12: lcs_with_path(abcde, ace) = "
         << dp.lcs_with_path("abcde", "ace") << " (ожидаем ace)\n";
    cout << "C.12: lcs_perm({1..5}, {3,1,4,2,5}) = "
         << dp.lcs_perm({1, 2, 3, 4, 5}, {3, 1, 4, 2, 5}) << " (ожидаем 3)\n";

    cout << "C.13: longest_common_substring(abcxyz, xyzabc) = "
         << dp.longest_common_substring("abcxyz", "xyzabc") << " (ожидаем 3)\n";

    cout << "C.14: wildcard_match(adceb, *a*b) = "
         << dp.wildcard_match("adceb", "*a*b") << " (ожидаем 1)\n";
    cout << "C.14: wildcard_match(acdcb, a*c?b) = "
         << dp.wildcard_match("acdcb", "a*c?b") << " (ожидаем 0)\n";

    cout << "C.15: regex_match(aab, c*a*b) = "
         << dp.regex_match("aab", "c*a*b") << " (ожидаем 1)\n";
    cout << "C.15: regex_match(mississippi, mis*is*p*.) = "
         << dp.regex_match("mississippi", "mis*is*p*.") << " (ожидаем 0)\n";

    cout << "C.16: can_abbreviate(AbcDE, ABDE) = "
         << dp.can_abbreviate("AbcDE", "ABDE") << " (ожидаем 1)\n";
    cout << "C.16: can_abbreviate(daBcd, ABC) = "
         << dp.can_abbreviate("daBcd", "ABC") << " (ожидаем 1)\n";
    cout << "C.16: can_abbreviate(daBcd, ACD) = "
         << dp.can_abbreviate("daBcd", "ACD") << " (ожидаем 0)\n";

    cout << "C.17: smith_waterman(abc, abc, 2, -1, 2) = "
         << dp.smith_waterman("abc", "abc", 2, -1, 2) << " (ожидаем 6)\n";
    cout << "C.17: smith_waterman(abc, axc, 2, -1, 2) = "
         << dp.smith_waterman("abc", "axc", 2, -1, 2) << " (ожидаем 3)\n";

    cout << "C.18: needleman_wunsch(abc, abc, 2, -1, 1) = "
         << dp.needleman_wunsch("abc", "abc", 2, -1, 1) << " (ожидаем 6)\n";
    cout << "C.18: needleman_wunsch(abc, axc, 2, -1, 1) = "
         << dp.needleman_wunsch("abc", "axc", 2, -1, 1) << " (ожидаем 3)\n";

    cout << "C.19: knapsack_01(W=5) = "
         << dp.knapsack_01({2, 3, 4, 5}, {3, 4, 5, 6}, 5) << " (ожидаем 7)\n";
    auto ks = dp.knapsack_01_path({2, 3, 4, 5}, {3, 4, 5, 6}, 5);
    cout << "C.19: knapsack_01_path(W=5) = " << ks.first << " (ожидаем 7), набор:";
    for (int idx : ks.second) cout << " " << idx;
    cout << " (ожидаем 1 0 — порядок обратного прохода)\n";
    cout << "C.19: knapsack_by_value(W=5) = "
         << dp.knapsack_by_value({2, 3, 4, 5}, {3, 4, 5, 6}, 5) << " (ожидаем 7)\n";

    cout << "C.20: knapsack_unbounded(W=7) = "
         << dp.knapsack_unbounded({1, 3, 4}, {1, 4, 5}, 7) << " (ожидаем 9)\n";

    cout << "C.21: knapsack_bounded(W=10) = "
         << dp.knapsack_bounded({2, 3, 4}, {3, 4, 5}, {2, 1, 3}, 10)
         << " (ожидаем 13)\n";
    cout << "C.21: knapsack_bounded_monotone(W=10) = "
         << dp.knapsack_bounded_monotone({2, 3, 4}, {3, 4, 5}, {2, 1, 3}, 10)
         << " (ожидаем 13, сверка с двоичным разбиением)\n";

    cout << "C.22: min_partition_difference({1,6,11,5}) = "
         << dp.min_partition_difference({1, 6, 11, 5}) << " (ожидаем 1)\n";

    cout << "C.23: subset_sum_reachable(S=9) = "
         << dp.subset_sum_reachable({3, 34, 4, 12, 5, 2}, 9) << " (ожидаем 1)\n";
    cout << "C.23: subset_sum_count(S=5) = "
         << dp.subset_sum_count({1, 2, 3, 4}, 5) << " (ожидаем 2)\n";

    cout << "C.24: target_sum(target=3) = "
         << dp.target_sum({1, 1, 1, 1, 1}, 3) << " (ожидаем 5)\n";
    cout << "C.24: target_sum_direct(target=3) = "
         << dp.target_sum_direct({1, 1, 1, 1, 1}, 3) << " (ожидаем 5)\n";

    auto divs = dp.largest_divisible_subset({1, 2, 4, 8});
    cout << "C.25: largest_divisible_subset({1,2,4,8}) = {";
    for (size_t q = 0; q < divs.size(); q++) {
        if (q) cout << ",";
        cout << divs[q];
    }
    cout << "} (размер 4); ({1,2,3}) размер "
         << dp.largest_divisible_subset({1, 2, 3}).size() << " (ожидаем 2)\n";

    cout << "C.26: coin_change_min_weighted({1,3,4}, {1,2,3}, 6) = "
         << dp.coin_change_min_weighted({1, 3, 4}, {1, 2, 3}, 6) << " (ожидаем 4)\n";
    cout << "C.26: из базы coin_change_min({1,3,4}, 6) = "
         << dp.coin_change_min({1, 3, 4}, 6).first << " (ожидаем 2, B.6)\n";
    cout << "C.26: из базы coin_change_ways({1,2,3}, 4) = "
         << dp.coin_change_ways({1, 2, 3}, 4) << " (ожидаем 4, B.10)\n";

    cout << "C.27: knapsack_2d(W=6, C=6) = "
         << dp.knapsack_2d({2, 3, 4}, {2, 3, 4}, {3, 4, 5}, 6, 6)
         << " (ожидаем 8: (2,2,3) + (4,4,5): вес 6, объём 6)\n";

    cout << "C.28: count_subsets_sum_mod(S=5, mod 1e9+7) = "
         << dp.count_subsets_sum_mod({1, 2, 3, 4}, 5, 1000000007LL)
         << " (ожидаем 2)\n";
}
#endif // MATRIXDP_MAIN
#endif // DYNAMIC_C_CPP
