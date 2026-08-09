#ifndef DYNAMIC_J_CPP
#define DYNAMIC_J_CPP

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
#include <deque>
using namespace std;

// =============================================================
// J. СЛОЖНЫЕ ОПТИМИЗАЦИИ ДП
// =============================================================
// Структура md: A. Оптимизация памяти (J.1–J.4)
//               → B. Оптимизация времени (J.5–J.13)
//               → C. Параллельные и распределённые (J.14–J.17)
//
// OptimizationDP наследует SpecialDP (i.cpp), а через него GraphDP
// и т.д. Переиспользуются, а не дублируются: lcs (c.md C.12 — уже
// роллинг O(m)), edit_distance (c.md C.11), knapsack_01 (c.md C.19 —
// уже in-place), matrix_chain_min (d.md D.1 — диагональный порядок),
// merge_stones_min_cost (d.md D.9 — плотный O(n³) аналог для J.6),
// count_subsets_with_sum (i.md I.27 — in-place), min_subarray_len_ge_sum
// (i.md I.29). Новые реализации: только оптимизационные техники.
//
// Порядок методов строго соответствует порядку md (J.1 → J.13);
// J.3 (кроме edit_distance_rolling), J.4, J.13–J.17 — теоретические
// разделы без новых методов (указаны переиспользуемые). Файловые
// структуры: MonotoneCHT (J.8 — монотонный CHT, убывающие наклоны,
// возрастающие запросы), LiChaoTree (J.9 — динамический CHT).

#define SPECIALDP_MAIN
#include "../i/i.cpp"

// --- J.8. Монотонный CHT (минимум) ---
// Линии y = m·x + c добавляются с УБЫВАЮЩИМИ наклонами, запросы — с
// возрастающими x (указатель не возвращается). Линия 2 лишняя, если
// её интервал победы [x₁₂, x₂₃] пуст: (c₁−c₂)(m₃−m₂) ≥ (c₂−c₃)(m₂−m₁)
// (сравнение пересечений без деления, __int128).
struct MonotoneCHT {
    vector<long long> m, c;
    vector<int> id;
    int ptr = 0;

    void add(long long a, long long b, int idx = 0) {
        if (!m.empty() && m.back() == a) {
            if (c.back() <= b) return;
            m.pop_back();
            c.pop_back();
            id.pop_back();
        }
        while ((int)m.size() >= 2 &&
               (__int128)(c[m.size() - 2] - c.back()) * (a - m.back()) >=
                   (__int128)(c.back() - b) * (m.back() - m[m.size() - 2]))
            m.pop_back(), c.pop_back(), id.pop_back();
        m.push_back(a);
        c.push_back(b);
        id.push_back(idx);
    }

    long long query(long long x, int* arg = nullptr) {
        if (ptr >= (int)m.size()) ptr = (int)m.size() - 1;
        if (ptr < 0) return LLONG_MAX / 4;
        while (ptr + 1 < (int)m.size() &&
               (__int128)m[ptr + 1] * x + c[ptr + 1] <=
                   (__int128)m[ptr] * x + c[ptr])
            ptr++;
        if (arg) *arg = id[ptr];
        return m[ptr] * x + c[ptr];
    }
};

// --- J.9. Li Chao Tree (динамический CHT) ---
// Дерево отрезков по области x ∈ [L, R]; в узле — «лучшая» в середине
// линия; вытесненная уходит в половину, где она лучше (две прямые
// пересекаются один раз). O(log C) на добавление и запрос (минимум).
struct LiChaoTree {
    long long L, R;
    vector<long long> m, c, lc, rc;
    vector<char> has;

    LiChaoTree(long long L, long long R) : L(L), R(R) {}

    void add_line(long long a, long long b) {
        if (has.empty()) {
            m.push_back(a); c.push_back(b);
            lc.push_back(-1); rc.push_back(-1); has.push_back(1);
            return;
        }
        add_rec(0, L, R, a, b);
    }

    long long query(long long x) {
        if (m.empty()) return LLONG_MAX / 4;
        long long res = LLONG_MAX / 4;
        int v = 0;
        long long l = L, r = R;
        while (v != -1) {
            res = min(res, m[v] * x + c[v]);
            if (l == r) break;
            long long mid = l + (r - l) / 2;
            if (x <= mid) { v = lc[v]; r = mid; }
            else { v = rc[v]; l = mid + 1; }
        }
        return res;
    }

private:
    void add_rec(int v, long long l, long long r, long long a, long long b) {
        if (a == m[v] && b == c[v]) return;
        long long mid = l + (r - l) / 2;
        bool cur = (__int128)a * mid + b < (__int128)m[v] * mid + c[v];
        if (cur) { swap(a, m[v]); swap(b, c[v]); }
        if (l == r) return;
        bool better_left =
            (__int128)a * l + b < (__int128)m[v] * l + c[v];
        if (better_left) {
            if (lc[v] == -1) {
                lc[v] = (int)m.size();
                m.push_back(a); c.push_back(b);
                lc.push_back(-1); rc.push_back(-1); has.push_back(1);
                return;
            }
            add_rec(lc[v], l, mid, a, b);
        } else {
            if (rc[v] == -1) {
                rc[v] = (int)m.size();
                m.push_back(a); c.push_back(b);
                lc.push_back(-1); rc.push_back(-1); has.push_back(1);
                return;
            }
            add_rec(rc[v], mid + 1, r, a, b);
        }
    }
};

struct OptimizationDP : SpecialDP {

    // --- J.1. Rolling DP: минимальная стоимость пути в сетке ---
    // dp[i][j] = cost[i][j] + min(dp[i−1][j], dp[i][j−1]) — нужны
    // только предыдущая и текущая строки: два массива длины m.
    // O(n·m) времени, O(m) памяти.
    long long min_cost_grid_rolling(const vector<vector<long long>>& cost) {
        int n = (int)cost.size();
        if (n == 0) return 0;
        int m = (int)cost[0].size();
        vector<long long> prev(m), cur(m);
        prev[0] = cost[0][0];
        for (int j = 1; j < m; j++) prev[j] = prev[j - 1] + cost[0][j];
        for (int i = 1; i < n; i++) {
            cur[0] = prev[0] + cost[i][0];
            for (int j = 1; j < m; j++)
                cur[j] = cost[i][j] + min(prev[j], cur[j - 1]);
            prev.swap(cur);
        }
        return prev[m - 1];
    }

    // --- J.2. Bit DP: subset-sum через сдвиг-ИЛИ ---
    // Бит s вектора = достижима сумма s; добавление предмета — сдвиг
    // на x с переносом по 64-битным словам: 64 суммы за операцию.
    // O(n·S/64) времени, O(S/64) памяти.
    bool subset_sum_bitset(const vector<long long>& a, long long S) {
        if (S < 0) return false;
        int words = (int)(S / 64) + 1;
        vector<unsigned long long> bits(words, 0), nb(words, 0);
        bits[0] = 1;
        for (long long x : a) {
            if (x > S) continue;
            nb = bits;
            int ws = (int)(x / 64), bs = (int)(x % 64);
            for (int i = words - 1; i >= ws; i--) {
                unsigned long long v = nb[i - ws] << bs;
                if (bs && i - ws - 1 >= 0)
                    v |= nb[i - ws - 1] >> (64 - bs);
                bits[i] |= v;
            }
        }
        return (bits[S / 64] >> (S % 64)) & 1;
    }

    // --- J.3. Edit Distance: двухстрочная версия ---
    // Окно зависимости размера 2 по i — роллинг (J.1); полная таблица
    // и восстановление — c.md (C.11). O(n·m) времени, O(min(n, m)) памяти.
    long long edit_distance_rolling(const string& a, const string& b) {
        const string& A = a.size() < b.size() ? b : a;
        const string& B = a.size() < b.size() ? a : b;
        int n = (int)A.size(), m = (int)B.size();
        vector<long long> prev(m + 1), cur(m + 1);
        for (int j = 0; j <= m; j++) prev[j] = j;
        for (int i = 1; i <= n; i++) {
            cur[0] = i;
            for (int j = 1; j <= m; j++)
                cur[j] = min(prev[j] + 1,
                             min(cur[j - 1] + 1,
                                 prev[j - 1] + (A[i - 1] == B[j - 1] ? 0 : 1)));
            prev.swap(cur);
        }
        return prev[m];
    }

    // --- J.5. Divide & Conquer Optimization ---
    // dp[k][i] = min_{j<k..i} (dp[k−1][j] + cost(j+1, i)) при монотон-
    // ной точке оптимума (четырёхугольное неравенство на cost): слой
    // считается рекурсивно, перебор j — в коридоре [optL, optR].
    // O(K·n·log n) времени, O(n) памяти на слой.
    long long divide_conquer_min(int n, int K,
                                 const function<long long(int, int)>& cost) {
        if (K <= 0) return 0;
        if (K > n) return -1;
        const long long INF = LLONG_MAX / 4;
        vector<long long> prev(n + 1, INF), cur(n + 1, INF);
        for (int i = 1; i <= n; i++) prev[i] = cost(1, i);
        for (int k = 2; k <= K; k++) {
            fill(cur.begin(), cur.end(), INF);
            function<void(int, int, int, int)> compute =
                [&](int l, int r, int optL, int optR) {
                if (l > r) return;
                int mid = (l + r) / 2;
                long long best = INF;
                int bestJ = optL;
                for (int j = optL; j <= min(optR, mid - 1); j++) {
                    if (prev[j] >= INF / 2) continue;
                    long long val = prev[j] + cost(j + 1, mid);
                    if (val < best) { best = val; bestJ = j; }
                }
                cur[mid] = best;
                compute(l, mid - 1, optL, bestJ);
                compute(mid + 1, r, bestJ, optR);
            };
            compute(1, n, k - 1, n - 1);
            prev.swap(cur);
        }
        return prev[n] >= INF / 2 ? -1 : prev[n];
    }

    // --- J.6. Knuth Optimization: слияние соседних кучек (K = 2) ---
    // f[i][j] = minₖ (f[i][k] + f[k+1][j]) + Σa[i..j] с коридором
    // k ∈ [opt[i][j−1], opt[i+1][j]] (четырёхугольное неравенство);
    // плотный аналог для любого K — d.md (D.9). O(n²) времени.
    long long merge_stones_min_cost_knuth(const vector<long long>& a) {
        int n = (int)a.size();
        if (n <= 1) return 0;
        vector<long long> pref(n + 1, 0);
        for (int i = 0; i < n; i++) pref[i + 1] = pref[i] + a[i];
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        vector<vector<int>> opt(n, vector<int>(n, 0));
        for (int i = 0; i < n; i++) opt[i][i] = i;
        for (int len = 2; len <= n; len++) {
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                long long sum = pref[j + 1] - pref[i];
                long long best = LLONG_MAX / 4;
                int hi = min(opt[i + 1][j], j - 1);
                for (int k = opt[i][j - 1]; k <= hi; k++) {
                    long long val = f[i][k] + f[k + 1][j] + sum;
                    if (val < best) { best = val; opt[i][j] = k; }
                }
                f[i][j] = best;
            }
        }
        return f[0][n - 1];
    }

    // --- J.7. SMAWK: минимумы строк полностью монотонной матрицы ---
    // Редукция столбцов (доминируемый отбрасывается сравнением в
    // строке-кандидате), рекурсия на нечётных строках, добор чётных
    // по коридору между минимумами соседних. O(n + m) сравнений.
    vector<int> smawk_row_minima(int n, int m,
                                 const function<long long(int, int)>& get) {
        vector<int> allRows(n), allCols(m);
        for (int i = 0; i < n; i++) allRows[i] = i;
        for (int j = 0; j < m; j++) allCols[j] = j;
        function<vector<int>(const vector<int>&, const vector<int>&)> rec =
            [&](const vector<int>& rows, const vector<int>& cols)
                -> vector<int> {
            int R = (int)rows.size();
            vector<int> red;
            for (int c : cols) {
                while (!red.empty()) {
                    int r = rows[(int)red.size() - 1];
                    if (get(r, red.back()) > get(r, c)) red.pop_back();
                    else break;
                }
                if ((int)red.size() < R) red.push_back(c);
            }
            vector<int> res(R, -1);
            if (R == 1) {
                int best = red[0];
                for (int c : red)
                    if (get(rows[0], c) < get(rows[0], best)) best = c;
                res[0] = best;
                return res;
            }
            vector<int> oddRows;
            for (int i = 1; i < R; i += 2) oddRows.push_back(rows[i]);
            vector<int> oddRes = rec(oddRows, red);
            for (int i = 1; i < R; i += 2) res[i] = oddRes[i / 2];
            vector<int> pos((int)oddRes.size(), 0);
            int p = 0;
            for (int k = 0; k < (int)oddRes.size(); k++) {
                while (p + 1 < (int)red.size() && red[p] != oddRes[k]) p++;
                pos[k] = p;
            }
            for (int i = 0; i < R; i += 2) {
                int lo = i == 0 ? 0 : pos[(i - 1) / 2];
                int hi = i == R - 1 ? (int)red.size() - 1 : pos[(i + 1) / 2];
                long long bv = LLONG_MAX / 4;
                for (int t = lo; t <= hi; t++) {
                    long long v = get(rows[i], red[t]);
                    if (v < bv) { bv = v; res[i] = red[t]; }
                }
            }
            return res;
        };
        return rec(allRows, allCols);
    }

    // --- J.8. CHT: квадратичный переход ---
    // dp[i] = min_j (dp[j] + (x[i] − x[j])²) = x[i]² + min_j (m_j·x[i] + c_j),
    // m_j = −2x[j] (убывают), c_j = x[j]² + dp[j]; запросы x[i] растут —
    // монотонный CHT с указателем. O(n) времени.
    long long min_quadratic_transition_dp(const vector<long long>& x) {
        int n = (int)x.size();
        if (n == 0) return 0;
        vector<long long> dp(n, 0);
        MonotoneCHT cht;
        cht.add(-2 * x[0], x[0] * x[0]);
        for (int i = 1; i < n; i++) {
            dp[i] = x[i] * x[i] + cht.query(x[i]);
            cht.add(-2 * x[i], x[i] * x[i] + dp[i]);
        }
        return dp[n - 1];
    }

    // --- J.10. Задача CHT: Land Acquisition ---
    // Сортировка по w возр., доминируемые (w' ≥ w, h' ≥ h) отбрасыва-
    // ются — остаются h строго убывающие; группа j..i стоит w[i]·h[j]:
    // dp[i] = min_j (dp[j−1] + w[i]·h[j]) — линии (h[j], dp[j−1]),
    // запросы w[i] растут (J.8). O(n·log n) + O(n).
    long long max_profit_land_groups(
        const vector<pair<long long, long long>>& rects) {
        vector<pair<long long, long long>> r = rects;
        sort(r.begin(), r.end(), [](const auto& p, const auto& q) {
            if (p.first != q.first) return p.first < q.first;
            return p.second > q.second;
        });
        vector<pair<long long, long long>> f;
        long long maxh = LLONG_MIN;
        for (int i = (int)r.size() - 1; i >= 0; i--)
            if (r[i].second > maxh) { f.push_back(r[i]); maxh = r[i].second; }
        reverse(f.begin(), f.end());
        int n = (int)f.size();
        if (n == 0) return 0;
        vector<long long> dp(n, 0);
        MonotoneCHT cht;
        for (int i = 0; i < n; i++) {
            cht.add(f[i].second, i == 0 ? 0 : dp[i - 1]);
            dp[i] = cht.query(f[i].first);
        }
        return dp[n - 1];
    }

    // --- J.11. Aliens Trick: ровно K сегментов, штраф λ ---
    // solve(λ): dp[i] = min_j (dp[j] + (pref[i] − pref[j])²) + λ за
    // сегмент (монотонный CHT, J.8); cnt(λ) монотонно убывает —
    // бинарный поиск λ* = max с cnt ≥ K; ответ = cost(λ*) − λ*·K.
    // O(n·log C) времени, O(n) памяти.
    long long min_cost_k_segments_alien(const vector<long long>& a, int K) {
        int n = (int)a.size();
        if (K <= 0) return 0;
        if (K > n) return -1;
        vector<long long> pref(n + 1, 0);
        for (int i = 0; i < n; i++) pref[i + 1] = pref[i] + a[i];
        auto solve = [&](long long lambda) -> pair<long long, int> {
            MonotoneCHT cht;
            cht.add(0, 0, 0);
            vector<long long> vdp(n + 1, 0);
            vector<int> vcnt(n + 1, 0);
            for (int i = 1; i <= n; i++) {
                int arg = -1;
                long long best = cht.query(pref[i], &arg);
                vdp[i] = pref[i] * pref[i] + lambda + best;
                vcnt[i] = vcnt[arg] + 1;
                cht.add(-2 * pref[i], pref[i] * pref[i] + vdp[i], i);
            }
            return {vdp[n], vcnt[n]};
        };
        long long lo = 0, hi = LLONG_MAX / 8;
        while (lo < hi) {
            long long mid = lo + (hi - lo + 1) / 2;
            if (solve(mid).second >= K) lo = mid;
            else hi = mid - 1;
        }
        return solve(lo).first - lo * K;
    }

    // --- J.12. Очередь минимумов: подотрезок длины ≤ k ---
    // ans = max_i (pref[i] − min_{j∈[i−k, i−1]} pref[j]) — монотонная
    // очередь хранит префиксы с возрастающими значениями (голова —
    // минимум окна). O(n) времени, O(k) памяти.
    long long max_subarray_sum_limited(const vector<long long>& a, int k) {
        int n = (int)a.size();
        if (n == 0) return 0;
        vector<long long> pref(n + 1, 0);
        for (int i = 0; i < n; i++) pref[i + 1] = pref[i] + a[i];
        deque<int> dq;
        dq.push_back(0);
        long long ans = LLONG_MIN / 4;
        for (int i = 1; i <= n; i++) {
            while (!dq.empty() && dq.front() < i - k) dq.pop_front();
            ans = max(ans, pref[i] - pref[dq.front()]);
            while (!dq.empty() && pref[dq.back()] >= pref[i]) dq.pop_back();
            dq.push_back(i);
        }
        return ans;
    }
};

#ifndef OPTIMIZATIONDP_MAIN
signed main() {
    OptimizationDP dp;

    cout << "J.1: мин путь в сетке 3x3 = "
         << dp.min_cost_grid_rolling({{1, 3, 1}, {1, 5, 1}, {4, 2, 1}})
         << " (ожидаем 7)\n";
    cout << "J.2: subset-sum битсетом: S=10: "
         << dp.subset_sum_bitset({1, 2, 3, 4, 5}, 10)
         << " (ожидаем 1), S=16: "
         << dp.subset_sum_bitset({1, 2, 3, 4, 5}, 16) << " (ожидаем 0)\n";
    cout << "J.3: edit_distance_rolling(kitten, sitting) = "
         << dp.edit_distance_rolling("kitten", "sitting") << " (ожидаем 3)\n";
    vector<long long> seg = {1, 2, 3, 4};
    vector<long long> sp(5, 0);
    for (int i = 0; i < 4; i++) sp[i + 1] = sp[i] + seg[i];
    cout << "J.5: D&C, K=2, cost=(сумма)^2: "
         << dp.divide_conquer_min(4, 2, [&](int l, int r) {
                return (sp[r] - sp[l - 1]) * (sp[r] - sp[l - 1]);
            })
         << " (ожидаем 52)\n";
    cout << "J.6: Knuth merge {4,1,3,2}: "
         << dp.merge_stones_min_cost_knuth({4, 1, 3, 2}) << " (ожидаем 20)\n";
    auto sm = dp.smawk_row_minima(4, 4, [](int i, int j) {
        return (long long)(i - j) * (i - j);
    });
    cout << "J.7: SMAWK (i-j)^2: {" << sm[0] << ", " << sm[1] << ", "
         << sm[2] << ", " << sm[3] << "} (ожидаем {0, 1, 2, 3})\n";
    cout << "J.8: квадратичный переход {0,1,3,6}: "
         << dp.min_quadratic_transition_dp({0, 1, 3, 6}) << " (ожидаем 14)\n";
    LiChaoTree lct(0, 5);
    lct.add_line(2, 1);
    lct.add_line(-1, 3);
    lct.add_line(3, -10);
    cout << "J.9: Li Chao запросы {0,1,3,5}: {" << lct.query(0) << ", "
         << lct.query(1) << ", " << lct.query(3) << ", " << lct.query(5)
         << "} (ожидаем {-10, -7, -1, -2})\n";
    cout << "J.10: Land Acquisition: "
         << dp.max_profit_land_groups({{4, 3}, {6, 2}, {3, 5}})
         << " (ожидаем 30)\n";
    cout << "J.11: Aliens, K=2, {1,3,2}: "
         << dp.min_cost_k_segments_alien({1, 3, 2}, 2) << " (ожидаем 20)\n";
    cout << "J.12: подотрезок длины <= 2: "
         << dp.max_subarray_sum_limited({1, 2, 3, -2, 1, 4, 3}, 2)
         << " (ожидаем 7)\n";
}
#endif // OPTIMIZATIONDP_MAIN
#endif // DYNAMIC_J_CPP
