#ifndef DYNAMIC_E_CPP
#define DYNAMIC_E_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <climits>
#include <utility>
#include <cmath>
using namespace std;

// =============================================================
// E. ДИНАМИКА ПО ПОДМНОЖЕСТВАМ (БИТОВЫЕ МАСКИ)
// =============================================================
// Структура md: A. Основные техники (E.1–E.4)
//               → B. Классические задачи (E.5–E.13)
//               → C. Оптимизации (E.14–E.19)
//
// SubsetDP наследует SegmentDP (d.cpp), а через него MatrixDP,
// LinearDP, DPBasics. Переиспользуются, а не дублируются: беспорядки
// derangements (b.md B.13 — сверка E.13), достижимость подмножественных
// сумм subset_sum_reachable (C.23 — сверка E.14), ладьи на доске
// Феррера ferrers_rook_dp (C.10 — частный случай E.13).
//
// Порядок методов строго соответствует порядку md (E.1 → E.19).
// Ограничения: n ≤ 20–25 (память 2ⁿ·n); счётные методы (E.10, E.16,
// E.18) растут быстро — демо на малых значениях.

#define SEGMENTDP_MAIN
#include "../d/d.cpp"

struct SubsetDP : SegmentDP {

    // --- E.1. Bitmask: базовые операции ---
    // Подмножество {0..n−1} — число: бит i = 1 ⇔ i ∈ X; popcount —
    // размер множества, lowbit — младший установленный бит.
    long long popcount(long long mask) { return __builtin_popcountll(mask); }

    bool test_bit(long long mask, int i) { return (mask >> i) & 1; }

    long long lowbit(long long mask) { return mask & (-mask); }

    // --- E.2. Перебор подмасок ---
    // for (sub = mask; sub; sub = (sub − 1) & mask) + подмаска 0:
    // вычитание 1 с сохранением разрешающей маски перебирает подмаски
    // в убывающем порядке; суммарно по всем маскам — O(3ⁿ).
    long long count_submasks(long long mask) {
        long long cnt = 0;
        for (long long sub = mask;; sub = (sub - 1) & mask) {
            cnt++;
            if (!sub) break;
        }
        return cnt;
    }

    // Демонстрация суммарной сложности: Σ_mask 2^popcount(mask) = 3ⁿ.
    long long sum_submasks_counts(int n) {
        long long total = 0;
        for (long long mask = 0; mask < (1LL << n); mask++)
            total += count_submasks(mask);
        return total;
    }

    // --- E.3. Генерация подмножеств: в порядке размера ---
    // Подмаски в корзины по popcount, внутри корзины — по возрастанию.
    // O(2ⁿ) времени, O(2ⁿ) памяти.
    vector<long long> subsets_sorted_by_size(long long mask) {
        vector<vector<long long>> buckets(popcount(mask) + 1);
        for (long long sub = mask;; sub = (sub - 1) & mask) {
            buckets[popcount(sub)].push_back(sub);
            if (!sub) break;
        }
        for (auto& b : buckets) sort(b.begin(), b.end());
        vector<long long> res;
        for (auto& b : buckets)
            res.insert(res.end(), b.begin(), b.end());
        return res;
    }

    // --- E.4. All Construct: сборка множества из частей ---
    // dp[mask] = Σ dp[mask \ part] по part ⊆ mask — последняя положенная
    // часть part; порядок частей важен. O(2ⁿ·|parts|) времени.
    long long count_constructs(int n, const vector<long long>& parts) {
        long long full = (1LL << n) - 1;
        vector<long long> dp(1LL << n, 0);
        dp[0] = 1;
        for (long long mask = 1; mask <= full; mask++)
            for (long long part : parts)
                if ((part & mask) == part) dp[mask] += dp[mask ^ part];
        return dp[full];
    }

    // --- E.5. TSP: путь из старта 0 без возврата ---
    // dp[mask][v] = min_{u ∈ mask \ {v}} (dp[mask \ {v}][u] + w[u][v]) —
    // последний шаг u → v; база dp[{0}][0] = 0; ответ min_v dp[full][v].
    // O(2ⁿ·n²) времени, O(2ⁿ·n) памяти.
    long long tsp_path(const vector<vector<long long>>& w) {
        int n = (int)w.size();
        vector<vector<long long>> dp(1LL << n, vector<long long>(n, LLONG_MAX / 4));
        dp[1][0] = 0;
        for (long long mask = 1; mask < (1LL << n); mask++)
            if (mask & 1)
                for (int v = 1; v < n; v++)
                    if (mask >> v & 1)
                        for (int u = 0; u < n; u++)
                            if ((mask >> u & 1) && u != v)
                                dp[mask][v] = min(dp[mask][v],
                                    dp[mask ^ (1LL << v)][u] + w[u][v]);
        long long best = LLONG_MAX / 4;
        for (int v = 0; v < n; v++)
            best = min(best, dp[(1LL << n) - 1][v]);
        return best;
    }

    // --- E.6. TSP: замкнутый цикл с возвратом в старт ---
    // Та же таблица dp; ответ min_v (dp[full][v] + w[v][0]) — ребро
    // возврата из конца пути. O(2ⁿ·n²) времени, O(2ⁿ·n) памяти.
    long long tsp_cycle(const vector<vector<long long>>& w) {
        int n = (int)w.size();
        vector<vector<long long>> dp(1LL << n, vector<long long>(n, LLONG_MAX / 4));
        dp[1][0] = 0;
        for (long long mask = 1; mask < (1LL << n); mask++)
            if (mask & 1)
                for (int v = 1; v < n; v++)
                    if (mask >> v & 1)
                        for (int u = 0; u < n; u++)
                            if ((mask >> u & 1) && u != v)
                                dp[mask][v] = min(dp[mask][v],
                                    dp[mask ^ (1LL << v)][u] + w[u][v]);
        long long best = LLONG_MAX / 4;
        for (int v = 0; v < n; v++)
            best = min(best, dp[(1LL << n) - 1][v] + w[v][0]);
        return best;
    }

    // --- E.8. Bitonic TSP ---
    // Точки отсортированы по x; dp[i][j] (i > j) — два пути из 0:
    // один оканчивается в i, другой в j, покрыты точки 0..i; при
    // j < i−1 точка i продолжает путь из i−1; при j = i−1 — точка i
    // стартует второй путь из k < i−1. Ответ — соединение в n−1.
    // O(n²) времени, O(n²) памяти. Расстояние — параметр d(i, j).
    double bitonic_tsp(const vector<pair<long long, long long>>& points) {
        int n = (int)points.size();
        vector<vector<double>> d(n, vector<double>(n, 0));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                long long dx = points[i].first - points[j].first;
                long long dy = points[i].second - points[j].second;
                d[i][j] = sqrt((double)(dx * dx + dy * dy));
            }
        vector<vector<double>> dp(n, vector<double>(n, 0));
        dp[1][0] = d[0][1];
        for (int i = 2; i < n; i++) {
            for (int j = 0; j < i - 1; j++)
                dp[i][j] = dp[i - 1][j] + d[i - 1][i];
            dp[i][i - 1] = 1e300;
            for (int k = 0; k < i - 1; k++)
                dp[i][i - 1] = min(dp[i][i - 1], dp[i - 1][k] + d[k][i]);
        }
        double best = 1e300;
        for (int j = 0; j < n - 1; j++)
            best = min(best, dp[n - 1][j] + d[j][n - 1]);
        return best;
    }

    // --- E.9. Assignment Problem ---
    // dp[mask] = min_{i ∈ mask} (dp[mask \ {i}] + cost[r][i]), где
    // r = popcount(mask) − 1 — последний назначаемый рабочий; работы —
    // биты маски. O(2ⁿ·n) времени, O(2ⁿ) памяти.
    long long assignment_min(const vector<vector<long long>>& cost) {
        int n = (int)cost.size();
        vector<long long> dp(1LL << n, LLONG_MAX / 4);
        dp[0] = 0;
        for (long long mask = 1; mask < (1LL << n); mask++) {
            int r = (int)popcount(mask) - 1;
            for (int i = 0; i < n; i++)
                if (mask >> i & 1)
                    dp[mask] = min(dp[mask],
                        dp[mask ^ (1LL << i)] + cost[r][i]);
        }
        return dp[(1LL << n) - 1];
    }

    // --- E.10. Hamiltonian Path: счёт ---
    // dp[mask][v] = Σ dp[mask \ {v}][u] по рёбрам u → v — последняя
    // вершина пути; путь: Σ_v dp[full][v]; существование — счёт > 0.
    // O(2ⁿ·n²) времени, O(2ⁿ·n) памяти.
    long long hamiltonian_path_count(int n, const vector<long long>& adj) {
        vector<vector<long long>> dp(1LL << n, vector<long long>(n, 0));
        for (int v = 0; v < n; v++) dp[1LL << v][v] = 1;
        for (long long mask = 1; mask < (1LL << n); mask++)
            for (int v = 0; v < n; v++)
                if (dp[mask][v])
                    for (int u = 0; u < n; u++)
                        if (!(mask >> u & 1) && (adj[v] >> u & 1))
                            dp[mask | (1LL << u)][u] += dp[mask][v];
        long long res = 0;
        for (int v = 0; v < n; v++) res += dp[(1LL << n) - 1][v];
        return res;
    }

    // --- E.10. Hamiltonian Cycle: счёт с фиксированным стартом 0 ---
    // Цикл = путь 0 → ... → v + ребро v → 0; старт фиксирован — база
    // dp[{0}][0] = 1, каждый цикл учитывается ровно один раз.
    long long hamiltonian_cycle_count(int n, const vector<long long>& adj) {
        vector<vector<long long>> dp(1LL << n, vector<long long>(n, 0));
        dp[1][0] = 1;
        for (long long mask = 1; mask < (1LL << n); mask++)
            if (mask & 1)
                for (int v = 0; v < n; v++)
                    if (dp[mask][v])
                        for (int u = 0; u < n; u++)
                            if (!(mask >> u & 1) && (adj[v] >> u & 1))
                                dp[mask | (1LL << u)][u] += dp[mask][v];
        long long res = 0;
        for (int v = 1; v < n; v++)
            if (adj[v] >> 0 & 1) res += dp[(1LL << n) - 1][v];
        return res;
    }

    // --- E.11. Maximum Clique: перебор масок ---
    // Маска — клика, если для всех i ∈ mask: adj[i] ∩ mask = mask \ {i}
    // (все соседи внутри). O(2ⁿ·n) времени, O(n) памяти.
    long long max_clique(int n, const vector<long long>& adj) {
        long long full = (1LL << n) - 1, best = 0;
        for (long long mask = 1; mask <= full; mask++) {
            bool ok = true;
            for (int i = 0; i < n && ok; i++)
                if (mask >> i & 1)
                    if ((adj[i] & mask) != (mask ^ (1LL << i))) ok = false;
            if (ok) best = max(best, popcount(mask));
        }
        return best;
    }

    // --- E.11. Maximum Independent Set: клика в дополнении ---
    // Дополнение графа (поглощение констант: n фиксировано) + max_clique.
    long long max_independent_set(int n, const vector<long long>& adj) {
        vector<long long> comp(n);
        for (int i = 0; i < n; i++)
            comp[i] = ((1LL << n) - 1) ^ adj[i] ^ (1LL << i);
        return max_clique(n, comp);
    }

    // --- E.12. Set Cover: минимум ---
    // dp[mask | s] = min(dp[mask | s], dp[mask] + 1) — добавляем множество
    // s к уже покрытому mask; база dp[0] = 0. O(2ⁿ·|sets|) времени.
    long long min_set_cover(int n, const vector<long long>& sets) {
        long long full = (1LL << n) - 1;
        vector<long long> dp(full + 1, LLONG_MAX / 4);
        dp[0] = 0;
        for (long long mask = 0; mask <= full; mask++)
            if (dp[mask] != LLONG_MAX / 4)
                for (long long s : sets)
                    dp[mask | s] = min(dp[mask | s], dp[mask] + 1);
        return dp[full];
    }

    // --- E.12. Set Cover: счёт неупорядоченных способов ---
    // 0-1 по множествам: dp[mask | s] += dp[mask] (каждое множество —
    // не более раза; порядок не важен). O(2ⁿ·|sets|) времени.
    long long count_set_covers(int n, const vector<long long>& sets) {
        long long full = (1LL << n) - 1;
        vector<long long> dp(full + 1, 0);
        dp[0] = 1;
        for (long long s : sets)
            for (long long mask = full; mask >= 0; mask--)
                if (dp[mask]) dp[mask | s] += dp[mask];
        return dp[full];
    }

    // --- E.13. Ладьи на запрещённых клетках: R_r по маскам ---
    // dp[mask] — число расстановок по обработанным строкам (столбцы —
    // биты маски); строка: пропуск или ладья в запрещённой клетке c ∉ mask
    // (не бьющие ладьи = попарно разные столбцы). R_r = Σ dp[mask] по
    // popcount(mask) = r. O(n·2ⁿ) времени, O(2ⁿ) памяти.
    vector<long long> rook_placement_counts(int n,
                                            const vector<long long>& forbidden) {
        vector<long long> dp(1LL << n, 0), ndp(1LL << n, 0);
        dp[0] = 1;
        for (int i = 0; i < n; i++) {
            ndp = dp;
            for (long long mask = 0; mask < (1LL << n); mask++)
                if (dp[mask])
                    for (int c = 0; c < n; c++)
                        if ((forbidden[i] >> c & 1) && !(mask >> c & 1))
                            ndp[mask | (1LL << c)] += dp[mask];
            dp = ndp;
        }
        vector<long long> R(n + 1, 0);
        for (long long mask = 0; mask < (1LL << n); mask++)
            R[popcount(mask)] += dp[mask];
        return R;
    }

    // --- E.13. Перестановки без запрещённых позиций ---
    // Включения-исключения: Σ_{r=0}^{n} (−1)^r · R_r · (n−r)!; частный
    // случай Sᵢ = {i} — беспорядки (b.md B.13, сверка derangements).
    long long permutations_avoiding(int n,
                                    const vector<long long>& forbidden) {
        vector<long long> R = rook_placement_counts(n, forbidden);
        vector<long long> fact(n + 1, 1);
        for (int i = 1; i <= n; i++) fact[i] = fact[i - 1] * i;
        long long ans = 0;
        for (int r = 0; r <= n; r++)
            ans += (r % 2 == 0 ? 1 : -1) * R[r] * fact[n - r];
        return ans;
    }

    // --- E.14. Meet-in-the-middle: максимальная сумма ≤ T ---
    // Половины n/2: все 2^(n/2) сумм каждой; сортировка правой, для
    // каждой левой x — upper_bound(T − x). O(2^(n/2)·log) времени.
    long long max_subset_sum_mitm(const vector<long long>& a, long long T) {
        int n = (int)a.size(), h = n / 2;
        vector<long long> left, right;
        for (long long mask = 0; mask < (1LL << h); mask++) {
            long long s = 0;
            for (int i = 0; i < h; i++)
                if (mask >> i & 1) s += a[i];
            left.push_back(s);
        }
        for (long long mask = 0; mask < (1LL << (n - h)); mask++) {
            long long s = 0;
            for (int i = 0; i < n - h; i++)
                if (mask >> i & 1) s += a[h + i];
            right.push_back(s);
        }
        sort(right.begin(), right.end());
        long long best = 0;
        for (long long x : left) {
            auto it = upper_bound(right.begin(), right.end(), T - x);
            if (it != right.begin()) best = max(best, x + *(it - 1));
        }
        return best;
    }

    // --- E.15. SOS DP: сумма по подмножествам ---
    // Инвариант по битам: после бита i в f[mask] — сумма по подмноже-
    // ствам, различающимся только битами 0..i; переход f[mask] +=
    // f[mask ^ (1 << i)] для установленного i. O(n·2ⁿ) времени.
    vector<long long> sos_sum(const vector<long long>& a) {
        int n = (int)a.size();
        int bits = 0;
        while ((1 << bits) < n) bits++;
        vector<long long> f = a;
        for (int i = 0; i < bits; i++)
            for (int mask = 0; mask < n; mask++)
                if (mask >> i & 1) f[mask] += f[mask ^ (1 << i)];
        return f;
    }

    // --- E.15. SOS DP: сумма по супермножествам (обратное) ---
    // Для mask без бита i: f[mask] += f[mask | (1 << i)] — те же переходы
    // в обратную сторону. O(n·2ⁿ) времени.
    vector<long long> sos_superset_sum(const vector<long long>& a) {
        int n = (int)a.size();
        int bits = 0;
        while ((1 << bits) < n) bits++;
        vector<long long> f = a;
        for (int i = 0; i < bits; i++)
            for (int mask = 0; mask < n; mask++)
                if (!(mask >> i & 1)) f[mask] += f[mask | (1 << i)];
        return f;
    }

    // --- E.16. Number of Ways to Wear Different Hats ---
    // По шляпам h = 1..M: ndp[mask | (1 << i)] += dp[mask] для человека i,
    // умеющего надеть h (маска — одетые люди; бит — не более одной шляпы).
    // O(M·2ⁿ·n) времени, O(2ⁿ) памяти.
    long long count_hat_assignments(const vector<vector<int>>& hats) {
        int n = (int)hats.size(), M = 0;
        for (auto& v : hats)
            for (int h : v) M = max(M, h);
        vector<vector<int>> byHat(M + 1);
        for (int i = 0; i < n; i++)
            for (int h : hats[i]) byHat[h].push_back(i);
        vector<long long> dp(1LL << n, 0);
        dp[0] = 1;
        for (int h = 1; h <= M; h++) {
            vector<long long> ndp = dp;
            for (long long mask = 0; mask < (1LL << n); mask++)
                if (dp[mask])
                    for (int i : byHat[h])
                        if (!(mask >> i & 1)) ndp[mask | (1LL << i)] += dp[mask];
            dp = ndp;
        }
        return dp[(1LL << n) - 1];
    }

    // --- E.17. Partition to K Equal Sum Subsets ---
    // dp[mask] = сумма текущего блока по модулю target (−1 — недостижимо);
    // переход добавляет a[i] в блок при dp[mask] + a[i] ≤ target; ответ
    // dp[full] = 0. O(2ⁿ·n) времени, O(2ⁿ) памяти.
    bool can_partition_k_subsets(const vector<long long>& a, int k) {
        long long sum = 0;
        for (long long x : a) sum += x;
        if (sum % k != 0) return false;
        long long target = sum / k;
        int n = (int)a.size();
        vector<long long> dp(1LL << n, -1);
        dp[0] = 0;
        for (long long mask = 0; mask < (1LL << n); mask++) {
            if (dp[mask] == -1) continue;
            for (int i = 0; i < n; i++)
                if (!(mask >> i & 1) && dp[mask] + a[i] <= target)
                    dp[mask | (1LL << i)] = (dp[mask] + a[i]) % target;
        }
        return dp[(1LL << n) - 1] == 0;
    }

    // --- E.18. Matching: число совершенных паросочетаний ---
    // dp[mask] = Σ dp[mask \ {i, j}] по рёбрам (i, j) ⊆ mask, i — младший
    // бит маски (фиксация i исключает дубли пар). O(2ⁿ·n) времени.
    long long count_perfect_matchings(int n, const vector<long long>& adj) {
        vector<long long> dp(1LL << n, 0);
        dp[0] = 1;
        for (long long mask = 0; mask < (1LL << n); mask++) {
            if (!dp[mask]) continue;
            int i = 0;
            while (i < n && (mask >> i & 1)) i++;
            if (i >= n) continue;
            for (int j = i + 1; j < n; j++)
                if (!(mask >> j & 1) && (adj[i] >> j & 1))
                    dp[mask | (1LL << i) | (1LL << j)] += dp[mask];
        }
        return dp[(1LL << n) - 1];
    }

    // --- E.19. TSP: восстановление + порядок состояний по popcount ---
    // Внешний цикл по popcount гарантирует: зависимости dp[mask \ {v}]
    // (меньший popcount) обработаны раньше; prev[mask][v] — предшествен-
    // ник v; маршрут — обратный проход от argmin_v dp[full][v] (a.md B.4).
    // O(2ⁿ·n²) времени, O(2ⁿ·n) памяти.
    pair<long long, vector<int>> tsp_path_with_path(
            const vector<vector<long long>>& w) {
        int n = (int)w.size();
        vector<vector<long long>> dp(1LL << n, vector<long long>(n, LLONG_MAX / 4));
        vector<vector<int>> prev(1LL << n, vector<int>(n, -1));
        dp[1][0] = 0;
        for (int len = 2; len <= n; len++)
            for (long long mask = 0; mask < (1LL << n); mask++)
                if (mask & 1 && popcount(mask) == len)
                    for (int v = 1; v < n; v++)
                        if (mask >> v & 1)
                            for (int u = 0; u < n; u++)
                                if ((mask >> u & 1) && u != v) {
                                    long long cand = dp[mask ^ (1LL << v)][u] + w[u][v];
                                    if (cand < dp[mask][v]) {
                                        dp[mask][v] = cand;
                                        prev[mask][v] = u;
                                    }
                                }
        long long full = (1LL << n) - 1, best = LLONG_MAX / 4;
        int end = -1;
        for (int v = 0; v < n; v++)
            if (dp[full][v] < best) { best = dp[full][v]; end = v; }
        vector<int> route;
        for (long long mask = full, v = end; v != -1;) {
            route.push_back(v);
            int u = prev[mask][v];
            mask ^= 1LL << v;
            v = u;
        }
        reverse(route.begin(), route.end());
        return {best, route};
    }
};

#ifndef SUBSETDP_MAIN
signed main() {
    SubsetDP dp;

    cout << "E.1: popcount(11) = " << dp.popcount(11) << " (ожидаем 3)\n";
    cout << "E.1: test_bit(11, 3) = " << dp.test_bit(11, 3)
         << ", test_bit(11, 2) = " << dp.test_bit(11, 2)
         << " (ожидаем 1 0)\n";
    cout << "E.1: lowbit(12) = " << dp.lowbit(12) << " (ожидаем 4)\n";

    cout << "E.2: count_submasks(15) = " << dp.count_submasks(15)
         << " (ожидаем 16 = 2^4)\n";
    cout << "E.2: sum_submasks_counts(4) = " << dp.sum_submasks_counts(4)
         << " (ожидаем 81 = 3^4)\n";

    auto subs = dp.subsets_sorted_by_size(11);
    cout << "E.3: subsets_sorted_by_size(1011b) =";
    for (long long s : subs) cout << " " << s;
    cout << " (ожидаем 0 1 2 8 3 9 10 11)\n";

    cout << "E.4: count_constructs(3, {1,2,4}) = "
         << dp.count_constructs(3, {1, 2, 4}) << " (ожидаем 6 = 3!)\n";

    vector<vector<long long>> w4 = {{0, 10, 15, 20},
                                    {10, 0, 35, 25},
                                    {15, 35, 0, 30},
                                    {20, 25, 30, 0}};
    cout << "E.5: tsp_path(4 города) = " << dp.tsp_path(w4)
         << " (ожидаем 65: 0->1->3->2)\n";
    cout << "E.6: tsp_cycle(4 города) = " << dp.tsp_cycle(w4)
         << " (ожидаем 80: 0->2->3->1->0)\n";

    cout << "E.8: bitonic_tsp(коллинеарные 4 точки) = "
         << dp.bitonic_tsp({{0, 0}, {1, 0}, {2, 0}, {3, 0}})
         << " (ожидаем 6)\n";
    cout << "E.8: bitonic_tsp(треугольник) = "
         << dp.bitonic_tsp({{0, 0}, {1, 1}, {2, 0}})
         << " (ожидаем 4.82843 = 2 + 2*sqrt(2))\n";

    cout << "E.9: assignment_min(3x3) = "
         << dp.assignment_min({{3, 2, 7}, {5, 1, 3}, {2, 3, 1}})
         << " (ожидаем 5)\n";

    vector<long long> k4 = {(1LL << 4) - 1 - (1LL << 0),
                            (1LL << 4) - 1 - (1LL << 1),
                            (1LL << 4) - 1 - (1LL << 2),
                            (1LL << 4) - 1 - (1LL << 3)};
    cout << "E.10: hamiltonian_path_count(K4 ориентир.) = "
         << dp.hamiltonian_path_count(4, k4) << " (ожидаем 24 = 4!)\n";
    cout << "E.10: hamiltonian_cycle_count(K4 ориентир.) = "
         << dp.hamiltonian_cycle_count(4, k4) << " (ожидаем 6 = 3!)\n";
    cout << "E.10: hamiltonian_path_count(цепь 0->1->2->3) = "
         << dp.hamiltonian_path_count(4, {1LL << 1, 1LL << 2, 1LL << 3, 0})
         << " (ожидаем 1)\n";

    vector<long long> c4 = {(1LL << 1) | (1LL << 3), (1LL << 0) | (1LL << 2),
                            (1LL << 1) | (1LL << 3), (1LL << 0) | (1LL << 2)};
    cout << "E.11: max_clique(C4) = " << dp.max_clique(4, c4)
         << " (ожидаем 2), max_independent_set(C4) = "
         << dp.max_independent_set(4, c4) << " (ожидаем 2)\n";
    cout << "E.11: max_clique(K4) = " << dp.max_clique(4, k4)
         << " (ожидаем 4)\n";

    cout << "E.12: min_set_cover(3, {3,6,7}) = "
         << dp.min_set_cover(3, {3, 6, 7}) << " (ожидаем 1)\n";
    cout << "E.12: count_set_covers(3, {3,6,7}) = "
         << dp.count_set_covers(3, {3, 6, 7}) << " (ожидаем 5)\n";

    vector<long long> diag = {1, 2, 4};
    auto R = dp.rook_placement_counts(3, diag);
    cout << "E.13: rook_placement_counts(диагональ) =";
    for (long long x : R) cout << " " << x;
    cout << " (ожидаем 1 3 3 1)\n";
    cout << "E.13: permutations_avoiding(диагональ) = "
         << dp.permutations_avoiding(3, diag)
         << " == derangements(3) = " << dp.derangements(3)
         << " (ожидаем 2, сверка с B.13)\n";
    cout << "E.13: permutations_avoiding(n=2, запрет (0,0)) = "
         << dp.permutations_avoiding(2, {1, 0}) << " (ожидаем 1)\n";

    cout << "E.14: max_subset_sum_mitm({1,2,4,8,16,32}, 30) = "
         << dp.max_subset_sum_mitm({1, 2, 4, 8, 16, 32}, 30)
         << " (ожидаем 30)\n";
    cout << "E.14: max_subset_sum_mitm({1,3,5}, 7) = "
         << dp.max_subset_sum_mitm({1, 3, 5}, 7) << " (ожидаем 6)\n";

    vector<long long> arr = {1, 2, 3, 4, 5, 6, 7, 8};
    auto fsum = dp.sos_sum(arr);
    cout << "E.15: sos_sum: f[7] = " << fsum[7] << " (ожидаем 36), f[5] = "
         << fsum[5] << " (ожидаем 14)\n";
    auto gsum = dp.sos_superset_sum(arr);
    cout << "E.15: sos_superset_sum: g[1] = " << gsum[1] << " (ожидаем 20)\n";

    cout << "E.16: count_hat_assignments({{1,2},{2,3}}) = "
         << dp.count_hat_assignments({{1, 2}, {2, 3}}) << " (ожидаем 3)\n";

    cout << "E.17: can_partition_k_subsets({4,3,2,3,5,2,1}, 4) = "
         << dp.can_partition_k_subsets({4, 3, 2, 3, 5, 2, 1}, 4)
         << " (ожидаем 1)\n";
    cout << "E.17: can_partition_k_subsets({1,2,3,4}, 3) = "
         << dp.can_partition_k_subsets({1, 2, 3, 4}, 3) << " (ожидаем 0)\n";

    cout << "E.18: count_perfect_matchings(цикл 4) = "
         << dp.count_perfect_matchings(4, c4) << " (ожидаем 2)\n";

    auto tspP = dp.tsp_path_with_path(w4);
    cout << "E.19: tsp_path_with_path = " << tspP.first
         << " (ожидаем 65), маршрут:";
    for (int v : tspP.second) cout << " " << v;
    cout << " (ожидаем 0 1 3 2)\n";
}
#endif // SUBSETDP_MAIN
#endif // DYNAMIC_E_CPP
