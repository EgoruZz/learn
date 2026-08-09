#ifndef DYNAMIC_I_CPP
#define DYNAMIC_I_CPP

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
// I. СПЕЦИАЛЬНЫЕ ВИДЫ И ТЕХНИКИ ДП
// =============================================================
// Структура md: A. Цифровая динамика (I.1–I.9)
//               → B. Вероятностная динамика (I.10–I.17)
//               → C. Комбинаторные игры (I.18–I.25)
//               → D. Параметрическая динамика (I.26–I.30)
//               → E. Временная динамика (I.31–I.34)
//               → F. Ленивая динамика (I.35–I.37)
//
// SpecialDP наследует GraphDP (h.cpp), а через него TreeDP и т.д.
// Переиспользуются, а не дублируются: тип OpCost (b.md B.5 — I.36),
// can_win_dag (h.md H.12 — связь I.19), топологический порядок
// (h.md H.1 — каркас I.18). Приватный topo_sort класса GraphDP не
// виден наследникам — там, где нужен порядок обхода, используется
// DFS-мемоизация (эквивалентна по значениям, вычисляет только
// достижимые состояния — тема F).
//
// Порядок методов строго соответствует порядку md (I.1 → I.37);
// I.2 — каркас f(R) − f(L−1) внутри методов A; I.10, I.22 — теория
// без отдельных методов. Приватные помощники: to_digits_base, mex,
// solve_linear_system.

#define GRAPHDP_MAIN
#include "../h/h.cpp"

struct SpecialDP : GraphDP {

    // --- I.1/I.2. Подсчёт чисел с цифрой d ---
    // dp(pos, tight, lz, seen): seen — цифра d уже встречена на
    // значащей позиции; диапазон — через f(R) − f(L−1) (I.2).
    // O(n·b) времени, O(n) памяти.
    long long count_containing_digit(long long L, long long R, int d) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            vector<vector<vector<vector<long long>>>> memo(
                n, vector<vector<vector<long long>>>(2,
                vector<vector<long long>>(2, vector<long long>(2, -1))));
            function<long long(int, int, int, int)> dfs =
                [&](int pos, int tight, int lz, int seen) -> long long {
                if (pos == n) return seen;
                long long& res = memo[pos][tight][lz][seen];
                if (res != -1) return res;
                res = 0;
                int limit = tight ? digits[pos] : 9;
                for (int dig = 0; dig <= limit; dig++) {
                    int ntight = tight && dig == digits[pos];
                    int nlz = lz && dig == 0;
                    int nseen = seen || (!nlz && dig == d);
                    res += dfs(pos + 1, ntight, nlz, nseen);
                }
                return res;
            };
            return dfs(0, 1, 1, 0);
        };
        return f(R) - f(L - 1);
    }

    // --- I.3. Сумма чисел, содержащих цифру d ---
    // Пара (cnt, sum): cnt — число завершений, sum — сумма их зна-
    // чений; вклад цифры dig на позиции pos — cnt·dig·b^(n−1−pos).
    // O(n·b) времени, O(n) памяти.
    long long sum_containing_digit(long long L, long long R, int d) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            vector<long long> pow10(n + 1, 1);
            for (int i = 1; i <= n; i++) pow10[i] = pow10[i - 1] * 10;
            vector<vector<vector<vector<pair<long long, long long>>>>> memo(
                n, vector<vector<vector<pair<long long, long long>>>>(2,
                vector<vector<pair<long long, long long>>>(2,
                vector<pair<long long, long long>>(2, {-1, 0}))));
            function<pair<long long, long long>(int, int, int, int)> dfs =
                [&](int pos, int tight, int lz, int seen)
                    -> pair<long long, long long> {
                if (pos == n) return {seen ? 1 : 0, 0};
                auto& res = memo[pos][tight][lz][seen];
                if (res.first != -1) return res;
                res = {0, 0};
                int limit = tight ? digits[pos] : 9;
                for (int dig = 0; dig <= limit; dig++) {
                    int ntight = tight && dig == digits[pos];
                    int nlz = lz && dig == 0;
                    int nseen = seen || (!nlz && dig == d);
                    auto ch = dfs(pos + 1, ntight, nlz, nseen);
                    res.first += ch.first;
                    res.second += ch.second + ch.first * dig * pow10[n - 1 - pos];
                }
                return res;
            };
            return dfs(0, 1, 1, 0).second;
        };
        return f(R) - f(L - 1);
    }

    // --- I.4. Сумма цифр всех чисел диапазона ---
    // Пара (cnt, sum): вклад цифры без веса разряда (вес = 1).
    // O(n·b) времени, O(n) памяти.
    long long sum_of_digits_range(long long L, long long R) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            vector<vector<vector<vector<pair<long long, long long>>>>> memo(
                n, vector<vector<vector<pair<long long, long long>>>>(2,
                vector<vector<pair<long long, long long>>>(2,
                vector<pair<long long, long long>>(2, {-1, 0}))));
            function<pair<long long, long long>(int, int, int)> dfs =
                [&](int pos, int tight, int lz)
                    -> pair<long long, long long> {
                if (pos == n) return {1, 0};
                auto& res = memo[pos][tight][lz][0];
                if (res.first != -1) return res;
                res = {0, 0};
                int limit = tight ? digits[pos] : 9;
                for (int dig = 0; dig <= limit; dig++) {
                    int ntight = tight && dig == digits[pos];
                    int nlz = lz && dig == 0;
                    auto ch = dfs(pos + 1, ntight, nlz);
                    res.first += ch.first;
                    res.second += ch.second + ch.first * dig;
                }
                return res;
            };
            return dfs(0, 1, 1).second;
        };
        return f(R) - f(L - 1);
    }

    // --- I.5. Числа без повторяющихся цифр ---
    // Маска использованных цифр в состоянии; лидирующие нули в маску
    // не входят (lz). O(n·b·2^b) времени, O(n·2^b) памяти.
    long long count_distinct_digits(long long L, long long R) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            vector<vector<vector<vector<long long>>>> memo(
                n, vector<vector<vector<long long>>>(2,
                vector<vector<long long>>(2, vector<long long>(1 << 10, -1))));
            function<long long(int, int, int, int)> dfs =
                [&](int pos, int tight, int lz, int mask) -> long long {
                if (pos == n) return lz ? 0 : 1;
                long long& res = memo[pos][tight][lz][mask];
                if (res != -1) return res;
                res = 0;
                int limit = tight ? digits[pos] : 9;
                for (int dig = 0; dig <= limit; dig++) {
                    int ntight = tight && dig == digits[pos];
                    if (lz && dig == 0) {
                        res += dfs(pos + 1, ntight, 1, mask);
                        continue;
                    }
                    if (mask & (1 << dig)) continue;
                    res += dfs(pos + 1, ntight, 0, mask | (1 << dig));
                }
                return res;
            };
            return dfs(0, 1, 1, 0);
        };
        return f(R) - f(L - 1);
    }

    // --- I.6. Числа, делящиеся на K ---
    // Остаток как состояние: mod' = (mod·b + dig) mod K; лист —
    // mod == 0. O(n·b·K) времени, O(n·K) памяти (большие K — I.7).
    long long count_divisible_digits(long long L, long long R, int K) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            vector<vector<vector<vector<long long>>>> memo(
                n, vector<vector<vector<long long>>>(2,
                vector<vector<long long>>(2, vector<long long>(K, -1))));
            function<long long(int, int, int, int)> dfs =
                [&](int pos, int tight, int lz, int mod) -> long long {
                if (pos == n) return lz ? 0 : (mod == 0);
                long long& res = memo[pos][tight][lz][mod];
                if (res != -1) return res;
                res = 0;
                int limit = tight ? digits[pos] : 9;
                for (int dig = 0; dig <= limit; dig++) {
                    int ntight = tight && dig == digits[pos];
                    int nlz = lz && dig == 0;
                    res += dfs(pos + 1, ntight, nlz, (mod * 10 + dig) % K);
                }
                return res;
            };
            return dfs(0, 1, 1, 0);
        };
        return f(R) - f(L - 1);
    }

    // --- I.7. Сложные состояния: (sum, mod) через map ---
    // Кортеж (pos, tight, lz, sum, mod) кодируется в long long;
    // unordered_map хранит только посещённые состояния — работает
    // при больших K (до ~10^9). O(n·b·|посещённых|) времени.
    long long count_digit_sum_and_divisible(long long L, long long R,
                                            int S, long long K) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            unordered_map<long long, long long> memo;
            function<long long(int, int, int, int, long long)> dfs =
                [&](int pos, int tight, int lz, int sum, long long mod)
                    -> long long {
                if (pos == n) return lz ? 0 : (mod == 0);
                long long key = ((((long long)pos * (S + 1) + sum) * K + mod)
                                 * 2 + tight) * 2 + lz;
                auto it = memo.find(key);
                if (it != memo.end()) return it->second;
                long long res = 0;
                int limit = tight ? digits[pos] : 9;
                for (int dig = 0; dig <= limit; dig++) {
                    if (sum + dig > S) continue;
                    int ntight = tight && dig == digits[pos];
                    int nlz = lz && dig == 0;
                    res += dfs(pos + 1, ntight, nlz, sum + dig,
                               (mod * 10 + dig) % K);
                }
                memo[key] = res;
                return res;
            };
            return dfs(0, 1, 1, 0, 0);
        };
        return f(R) - f(L - 1);
    }

    // --- I.8. Сумма цифр ≤ S ---
    // dp(pos, tight, lz, sum): сумма как измерение; отсечение
    // sum + dig ≤ S на переходах. O(n·b·S) времени, O(n·S) памяти.
    long long count_digit_sum_limit(long long L, long long R, int S) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            vector<vector<vector<vector<long long>>>> memo(
                n, vector<vector<vector<long long>>>(2,
                vector<vector<long long>>(2, vector<long long>(S + 1, -1))));
            function<long long(int, int, int, int)> dfs =
                [&](int pos, int tight, int lz, int sum) -> long long {
                if (pos == n) return lz ? 0 : 1;
                long long& res = memo[pos][tight][lz][sum];
                if (res != -1) return res;
                res = 0;
                int limit = tight ? digits[pos] : 9;
                for (int dig = 0; dig <= limit; dig++) {
                    if (sum + dig > S) continue;
                    int ntight = tight && dig == digits[pos];
                    int nlz = lz && dig == 0;
                    res += dfs(pos + 1, ntight, nlz, sum + dig);
                }
                return res;
            };
            return dfs(0, 1, 1, 0);
        };
        return f(R) - f(L - 1);
    }

    // --- I.8. Ровно k единиц в двоичной записи ---
    // Основание b = 2: dp(pos, tight, lz, ones); лист — ones == k.
    // O(n·2·k) времени, O(n·k) памяти (n ≤ 60).
    long long count_numbers_with_k_bits(long long L, long long R, int k) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 2);
            int n = (int)digits.size();
            vector<vector<vector<vector<long long>>>> memo(
                n, vector<vector<vector<long long>>>(2,
                vector<vector<long long>>(2, vector<long long>(n + 1, -1))));
            function<long long(int, int, int, int)> dfs =
                [&](int pos, int tight, int lz, int ones) -> long long {
                if (pos == n) return ones == k;
                long long& res = memo[pos][tight][lz][ones];
                if (res != -1) return res;
                res = 0;
                int limit = tight ? digits[pos] : 1;
                for (int dig = 0; dig <= limit; dig++) {
                    int ntight = tight && dig == digits[pos];
                    int nlz = lz && dig == 0;
                    res += dfs(pos + 1, ntight, nlz, ones + dig);
                }
                return res;
            };
            return dfs(0, 1, 1, 0);
        };
        return f(R) - f(L - 1);
    }

    // --- I.9. Палиндромные числа в диапазоне ---
    // Палиндром ширины w задаётся половинкой h длины ceil(w/2);
    // для w < n — все половинки, для w = n — с проверкой зеркаль-
    // ного числа ≤ x (монотонность по h — одна корректировка).
    // O(n²) времени на число, O(n) памяти.
    long long count_palindromic_range(long long L, long long R) {
        auto f = [&](long long x) -> long long {
            if (x <= 0) return 0;
            vector<int> digits = to_digits_base(x, 10);
            int n = (int)digits.size();
            long long total = 0;
            for (int w = 1; w < n; w++) {
                int halfLen = (w + 1) / 2;
                long long cnt = 9;
                for (int i = 1; i < halfLen; i++) cnt *= 10;
                total += cnt;
            }
            int halfLen = (n + 1) / 2;
            long long upperHalf = 0;
            for (int i = 0; i < halfLen; i++)
                upperHalf = upperHalf * 10 + digits[i];
            long long minHalf = 1;
            for (int i = 1; i < halfLen; i++) minHalf *= 10;
            long long cnt = upperHalf - minHalf + 1;
            long long half = upperHalf, mirror = upperHalf;
            long long rest = (n % 2 == 0) ? half : half / 10;
            while (rest > 0) {
                mirror = mirror * 10 + rest % 10;
                rest /= 10;
            }
            if (mirror > x) cnt--;
            return total + cnt;
        };
        return f(R) - f(L - 1);
    }

    // --- I.11. Ожидание числа бросков кости ---
    // dp[i] = 1 + Σ_{f=1..d} dp[i+f]/d, i < S; dp[i] = 0, i ≥ S.
    // Состояния растут — обратный проход. O(S·d) времени, O(S+d)
    // памяти.
    double expected_dice_rolls_to_sum(int S, int d) {
        vector<double> dp(S + d, 0.0);
        for (int i = S - 1; i >= 0; i--) {
            double sum = 0;
            for (int f = 1; f <= d; f++) sum += dp[i + f];
            dp[i] = 1.0 + sum / d;
        }
        return dp[0];
    }

    // --- I.12. Ожидание шагов до поглощения в 1D-блуждании ---
    // E[i] = 1 + (E[i−1] + E[i+1])/2, E[0] = E[n] = 0 — замкнутое
    // решение E[i] = i·(n−i) (подстановка в уравнение — тождество).
    // O(n) времени и памяти.
    vector<double> expected_absorbing_steps_1d(int n) {
        vector<double> e(n + 1, 0.0);
        for (int i = 0; i <= n; i++) e[i] = (double)i * (n - i);
        return e;
    }

    // --- I.13. Стационарное распределение цепи Маркова ---
    // π·P = π, Σπ = 1: строки 0..n−2 — (Pᵀ − I)π = 0, последняя —
    // Σπ = 1; Гаусс с частичным выбором ведущего; вырожденная
    // система (несколько классов) — пустой вектор. O(n³).
    vector<double> stationary_distribution(const vector<vector<double>>& P) {
        int n = (int)P.size();
        if (n == 1) return {1.0};
        vector<vector<double>> A(n, vector<double>(n, 0.0));
        vector<double> b(n, 0.0);
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n; j++)
                A[i][j] = P[j][i] - (i == j ? 1.0 : 0.0);
        for (int j = 0; j < n; j++) A[n - 1][j] = 1.0;
        b[n - 1] = 1.0;
        return solve_linear_system(A, b);
    }

    // --- I.14. Алгоритм Витерби ---
    // dp[t][s] = max_{s'} dp[t−1][s']·trans[s'][s]·emit[s][obs_t];
    // численно устойчиво в логарифмической шкале; prev[t][s] —
    // аргумент максимума; восстановление — обратный проход.
    // O(T·|S|²) времени, O(T·|S|) памяти.
    pair<vector<int>, double> viterbi(const vector<int>& obs,
                                      const vector<vector<double>>& trans,
                                      const vector<vector<double>>& emit,
                                      const vector<double>& init) {
        int T = (int)obs.size();
        int S = (int)trans.size();
        const double NEG = -1e300;
        vector<vector<double>> dp(T, vector<double>(S, NEG));
        vector<vector<int>> prev(T, vector<int>(S, -1));
        for (int s = 0; s < S; s++) {
            double p = emit[s][obs[0]];
            if (p > 0 && init[s] > 0)
                dp[0][s] = log(init[s]) + log(p);
        }
        for (int t = 1; t < T; t++) {
            for (int s = 0; s < S; s++) {
                double em = emit[s][obs[t]];
                if (em <= 0) continue;
                double le = log(em);
                for (int sp = 0; sp < S; sp++) {
                    if (dp[t - 1][sp] <= NEG / 2 || trans[sp][s] <= 0)
                        continue;
                    double cand = dp[t - 1][sp] + log(trans[sp][s]) + le;
                    if (cand > dp[t][s]) {
                        dp[t][s] = cand;
                        prev[t][s] = sp;
                    }
                }
            }
        }
        int best = 0;
        for (int s = 1; s < S; s++)
            if (dp[T - 1][s] > dp[T - 1][best]) best = s;
        vector<int> path(T);
        double prob = 0.0;
        if (dp[T - 1][best] > NEG / 2) {
            prob = exp(dp[T - 1][best]);
            for (int t = T - 1; t >= 0; t--) {
                path[t] = best;
                best = prev[t][best];
            }
        }
        return {path, prob};
    }

    // --- I.15. Ожидание шагов до поглощения (система) ---
    // E[i] = 1 + Σ_j P[i][j]·E[j] (не-поглощающие), E[i] = 0
    // (поглощающие, P[i][i] = 1); система (I − P)E = 1 — Гаусс.
    // O(n³) времени, O(n²) памяти.
    double expected_steps_absorbing(const vector<vector<double>>& P,
                                    int start) {
        int n = (int)P.size();
        vector<vector<double>> A(n, vector<double>(n, 0.0));
        vector<double> b(n, 0.0);
        for (int i = 0; i < n; i++) {
            if (P[i][i] > 1 - 1e-9) {
                A[i][i] = 1.0;
                b[i] = 0.0;
            } else {
                for (int j = 0; j < n; j++)
                    A[i][j] = (i == j ? 1.0 : 0.0) - P[i][j];
                b[i] = 1.0;
            }
        }
        vector<double> e = solve_linear_system(A, b);
        return e.empty() ? 0.0 : e[start];
    }

    // --- I.16. Вероятность достижения цели на сетке ---
    // dp[i][j] = p·dp[i][j+1] + (1−p)·dp[i+1][j]; цель — (n−1, m−1),
    // за границей 0. O(n·m) времени, O(n·m) памяти.
    double grid_reach_probability(int n, int m, double pRight) {
        vector<vector<double>> dp(n + 1, vector<double>(m + 1, 0.0));
        dp[n - 1][m - 1] = 1.0;
        for (int i = n - 1; i >= 0; i--)
            for (int j = m - 1; j >= 0; j--) {
                if (i == n - 1 && j == m - 1) continue;
                dp[i][j] = pRight * dp[i][j + 1] +
                           (1.0 - pRight) * dp[i + 1][j];
            }
        return dp[0][0];
    }

    // --- I.17. Разорение игрока (Gambler's Ruin) ---
    // Вероятность достичь 0 из капитала capital при цели target и
    // вероятности выигрыша раунда p: p = 0.5 → (target−capital)/target;
    // иначе r = q/p: (r^capital − r^target) / (1 − r^target).
    // O(1) — формула (рекурсия второго порядка с граничными условиями).
    double gambler_ruin_probability(long long capital, long long target,
                                    double p) {
        if (fabs(p - 0.5) < 1e-12)
            return (double)(target - capital) / target;
        double r = (1.0 - p) / p;
        return (pow(r, (double)capital) - pow(r, (double)target)) /
               (1.0 - pow(r, (double)target));
    }

    // --- I.18. Числа Гранди на DAG ---
    // g(v) = mex{ g(to) } — DFS-мемоизация (эквивалент обратного
    // топологического порядка h.md H.1). O(V + E) времени и памяти.
    vector<int> grundy_dag(const vector<vector<int>>& adj) {
        int n = (int)adj.size();
        vector<int> g(n, -1);
        function<int(int)> dfs = [&](int v) -> int {
            if (g[v] != -1) return g[v];
            vector<int> vals;
            for (int to : adj[v]) vals.push_back(dfs(to));
            return g[v] = mex_value(vals);
        };
        for (int v = 0; v < n; v++) dfs(v);
        return g;
    }

    // --- I.19. P- и N-позиции ---
    // win(v) ⇔ g(v) ≠ 0 (I.18); одиночная позиция — h.md H.12
    // (`can_win_dag`). O(V + E).
    vector<bool> win_lose_positions(const vector<vector<int>>& adj) {
        vector<int> g = grundy_dag(adj);
        vector<bool> win(g.size());
        for (int v = 0; v < (int)g.size(); v++) win[v] = g[v] != 0;
        return win;
    }

    // --- I.20. Ним ---
    // xor всех размеров кучек ≠ 0 ⇔ первый выигрывает (каждая
    // кучка — компонента, I.21). O(n).
    bool nim_winner(const vector<long long>& piles) {
        long long x = 0;
        for (long long p : piles) x ^= p;
        return x != 0;
    }

    // --- I.20. Kayles: ряд кеглей, сбить 1 или 2 соседние ---
    // g(n) = mex{ g(a) ⊕ g(n−1−a), g(a) ⊕ g(n−2−a) } — расщепление
    // ряда на два независимых (XOR, I.21). O(n²) времени.
    vector<int> kayles_grundy(int n) {
        vector<int> g(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            vector<int> vals;
            for (int a = 0; a < i; a++)
                vals.push_back(g[a] ^ g[i - 1 - a]);
            for (int a = 0; a + 1 < i; a++)
                vals.push_back(g[a] ^ g[i - 2 - a]);
            g[i] = mex_value(vals);
        }
        return g;
    }

    // --- I.20. Dawson's Kayles: сбить ровно 2 соседние кегли ---
    // g(n) = mex{ g(a) ⊕ g(n−2−a) }; периодичность нимберов —
    // I.24. O(n²) времени.
    vector<int> dawson_kayles_grundy(int n) {
        vector<int> g(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            vector<int> vals;
            for (int a = 0; a + 1 < i; a++)
                vals.push_back(g[a] ^ g[i - 2 - a]);
            g[i] = mex_value(vals);
        }
        return g;
    }

    // --- I.20. Wythoff: две кучки ---
    // P-позиции (⌊k·φ⌋, ⌊k·φ²⌋), φ = (1+√5)/2: при a ≤ b проверка
    // a == ⌊(b−a)·φ⌋. O(1).
    bool wythoff_winner(long long a, long long b) {
        if (a > b) swap(a, b);
        long long k = b - a;
        long double phi = (1.0L + sqrtl(5.0L)) / 2.0L;
        bool isP = a == (long long)floorl((long double)k * phi);
        return !isP;
    }

    // --- I.21. Теорема Шпрага-Гранди ---
    // Сумма независимых компонент — XOR нимберов; первый выигрывает
    // ⇔ XOR ≠ 0. O(n).
    bool sprague_grundy_winner(const vector<int>& grundies) {
        int x = 0;
        for (int g : grundies) x ^= g;
        return x != 0;
    }

    // --- I.23. Выигрышный ход на DAG ---
    // Из выигрышной позиции есть ход в позицию с g = 0 (I.19);
    // возвращает вершину-цель такого хода или −1. O(V + E).
    int winning_move_dag(const vector<vector<int>>& adj, int s) {
        vector<int> g = grundy_dag(adj);
        for (int to : adj[s])
            if (g[to] == 0) return to;
        return -1;
    }

    // --- I.24. Subtraction Games ---
    // g(i) = mex{ g(i−m) : m ≤ i, m ∈ S }; последовательность g
    // периодична (конечное max S — принцип Дирихле). O(n·|S|).
    vector<int> subtraction_grundy(int n, const vector<int>& moves) {
        vector<int> g(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            vector<int> vals;
            for (int m : moves)
                if (m <= i) vals.push_back(g[i - m]);
            g[i] = mex_value(vals);
        }
        return g;
    }

    // --- I.25. Green Hackenbush ---
    // Дерево с корнем в земле: g(v) = (v == root ? 0 : 1) + XOR
    // g(детей) — «бамбук длины L = ним L»; колонизация: ветка =
    // бамбук длины g(ветки). O(V) времени и памяти.
    long long green_hackenbush_value(const vector<vector<int>>& children,
                                     int root) {
        int n = (int)children.size();
        vector<long long> g(n, 0);
        vector<int> order;
        function<void(int)> dfs = [&](int v) {
            order.push_back(v);
            for (int c : children[v]) dfs(c);
        };
        dfs(root);
        reverse(order.begin(), order.end());
        for (int v : order) {
            long long x = 0;
            for (int c : children[v]) x ^= g[c];
            g[v] = (v == root ? 0 : 1) + x;
        }
        return g[root];
    }

    // --- I.26. Выбор k элементов без соседних ---
    // dp[i][k] = dp[i−1][k] + dp[i−2][k−1]; скроллинг по i (два
    // слоя); замкнутая форма C(n−k+1, k). O(n·k) времени, O(k)
    // памяти.
    long long count_select_k_non_adjacent(int n, int k) {
        if (k == 0) return 1;
        if (k > (n + 1) / 2) return 0;
        vector<long long> prev2(k + 1, 0), prev1(k + 1, 0);
        prev2[0] = 1;                       // i = 0
        prev1[0] = 1;                       // i = 1
        if (k >= 1) prev1[1] = 1;
        for (int i = 2; i <= n; i++) {
            vector<long long> cur(k + 1, 0);
            cur[0] = 1;
            for (int j = 1; j <= k && j <= i; j++)
                cur[j] = prev1[j] + prev2[j - 1];
            prev2.swap(prev1);
            prev1.swap(cur);
        }
        return prev1[k];
    }

    // --- I.27. Подсчёт подмножеств с суммой S ---
    // dp[j] += dp[j−w] по предметам, обратный порядок по j (0/1 —
    // каждый предмет не более раза). O(n·S) времени, O(S) памяти.
    long long count_subsets_with_sum(int S, const vector<int>& a) {
        vector<long long> dp(S + 1, 0);
        dp[0] = 1;
        for (int w : a)
            for (int j = S; j >= w; j--)
                dp[j] += dp[j - w];
        return dp[S];
    }

    // --- I.28. Разбиения числа на ровно k слагаемых ---
    // p(n, k) = p(n−1, k−1) + p(n−k, k) (содержит 1 / нет единиц);
    // базы p(0, 0) = 1, p(n, 0) = 0. O(n·k) времени и памяти.
    long long count_partitions_exactly_k(int n, int k) {
        vector<vector<long long>> dp(n + 1, vector<long long>(k + 1, 0));
        dp[0][0] = 1;
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= k && j <= i; j++) {
                dp[i][j] = dp[i - 1][j - 1];
                if (i >= j) dp[i][j] += dp[i - j][j];
            }
        return dp[n][k];
    }

    // --- I.29. Минимальная длина подотрезка с суммой ≥ S ---
    // Два указателя (неотрицательные элементы): r растёт, l подтя-
    // гивается пока сумма ≥ S. O(n) времени, O(1) памяти.
    long long min_subarray_len_ge_sum(long long S, const vector<long long>& a) {
        if (S <= 0) return 0;
        long long best = LLONG_MAX, sum = 0;
        int l = 0;
        for (int r = 0; r < (int)a.size(); r++) {
            sum += a[r];
            while (sum >= S && l <= r) {
                best = min(best, (long long)(r - l + 1));
                sum -= a[l];
                l++;
            }
        }
        return best == LLONG_MAX ? -1 : best;
    }

    // --- I.30. Строки без k единиц подряд ---
    // dp[run] — число строк, оканчивающихся серией из run единиц:
    // dp[0] = Σ dp, dp[run] = dp[run−1] (run < k). O(n·k) времени,
    // O(k) памяти; большой n — матричное возведение (h.md H.8).
    long long count_binary_strings_without_k_ones(int n, int k) {
        if (k <= 0) return 0;
        vector<long long> dp(k + 1, 0);
        dp[0] = 1;
        if (k >= 1) dp[1] = 1;
        for (int i = 2; i <= n; i++) {
            vector<long long> cur(k + 1, 0);
            for (int run = 0; run < k; run++) cur[0] += dp[run];
            for (int run = 1; run < k; run++) cur[run] = dp[run - 1];
            dp.swap(cur);
        }
        long long total = 0;
        for (int run = 0; run < k; run++) total += dp[run];
        return total;
    }

    // --- I.31. Взвешенные интервальные работы ---
    // Сортировка по end (end[i] монотонны); dp[i] = max(dp[i−1],
    // profit + dp[p(i)]), p(i) — последняя работа с end ≤ start[i]
    // (бинарный поиск по монотонной оси end). O(m·log m), память O(m).
    long long max_profit_weighted_intervals(
        const vector<tuple<int, int, long long>>& jobs) {
        int m = (int)jobs.size();
        vector<tuple<int, int, long long>> jb = jobs;
        sort(jb.begin(), jb.end(),
             [](const auto& a, const auto& b) { return get<1>(a) < get<1>(b); });
        vector<long long> dp(m, 0);
        for (int i = 0; i < m; i++) {
            auto [s, e, p] = jb[i];
            int lo = 0, hi = i - 1, best = -1;
            while (lo <= hi) {
                int mid = (lo + hi) / 2;
                if (get<1>(jb[mid]) <= s) { best = mid; lo = mid + 1; }
                else hi = mid - 1;
            }
            dp[i] = max(i ? dp[i - 1] : 0, p + (best == -1 ? 0 : dp[best]));
        }
        return m ? dp[m - 1] : 0;
    }

    // --- I.32. Стоимости, зависящие от времени ---
    // dp[t][v] = min(dp[t−1][v], min_{(u,v)} dp[t−1][u] + c(u,v,t−1));
    // edges — (u, v, vector стоимости на каждый шаг 0..T−1).
    // O(T·E) времени, O(T·V) памяти (скроллинг — J).
    vector<long long> time_dependent_shortest_path(
        int n, const vector<tuple<int, int, vector<long long>>>& edges,
        int T, int s) {
        const long long INF = LLONG_MAX / 4;
        vector<vector<long long>> dp(T + 1, vector<long long>(n, INF));
        dp[0][s] = 0;
        for (int t = 1; t <= T; t++) {
            dp[t] = dp[t - 1];
            for (auto [u, v, cost] : edges)
                if (dp[t - 1][u] < INF &&
                        dp[t - 1][u] + cost[t - 1] < dp[t][v])
                    dp[t][v] = dp[t - 1][u] + cost[t - 1];
        }
        return dp[T];
    }

    // --- I.33. Работы с дедлайнами (единичная длительность) ---
    // dp[t] — максимум прибыли к моменту t: по работам (d, p) в
    // порядке d: for t = d..1: dp[t] = max(dp[t], dp[t−1] + p);
    // обновление сверху вниз — работа используется не более раза.
    // O(m·D) времени, O(D) памяти.
    long long max_profit_deadline_jobs(
        const vector<pair<int, long long>>& jobs) {
        vector<pair<int, long long>> jb = jobs;
        sort(jb.begin(), jb.end());
        int D = 0;
        for (auto [d, p] : jb) D = max(D, d);
        vector<long long> dp(D + 1, 0);
        for (auto [d, p] : jb)
            for (int t = d; t >= 1; t--)
                dp[t] = max(dp[t], dp[t - 1] + p);
        long long best = 0;
        for (long long x : dp) best = max(best, x);
        return best;
    }

    // --- I.34. Минимальное число интервалов для покрытия [L, R] ---
    // Сжатая ось — только конечные точки: сортировка по началу,
    // жадный шаг — интервал с l ≤ cur и максимальным r (доминирует
    // любой другой, не продвигающий дальше). O(m·log m), память O(m).
    long long min_intervals_to_cover(long long L, long long R,
                                     const vector<pair<long long, long long>>& intervals) {
        vector<pair<long long, long long>> iv = intervals;
        sort(iv.begin(), iv.end());
        long long cur = L;
        long long cnt = 0;
        int i = 0, m = (int)iv.size();
        while (cur < R) {
            long long best = cur;
            while (i < m && iv[i].first <= cur) {
                best = max(best, iv[i].second);
                i++;
            }
            if (best <= cur) return -1;
            cur = best;
            cnt++;
        }
        return cnt;
    }

    // --- I.35. Оптимальная игра «возьми с конца» ---
    // dp[l][r] = max(a[l] − dp[l+1][r], a[r] − dp[l][r−1]) — раз-
    // ность счёта первого и второго; мемоизация по требованию —
    // вычисляются только состояния, достижимые из запроса.
    // O(n²) состояний (лениво — меньше).
    long long optimal_pick_game(const vector<long long>& a) {
        int n = (int)a.size();
        vector<vector<long long>> memo(n, vector<long long>(n, LLONG_MIN));
        function<long long(int, int)> dfs = [&](int l, int r) -> long long {
            if (l > r) return 0;
            if (l == r) return a[l];
            long long& res = memo[l][r];
            if (res != LLONG_MIN) return res;
            return res = max(a[l] - dfs(l + 1, r), a[r] - dfs(l, r - 1));
        };
        return dfs(0, n - 1);
    }

    // --- I.36. Ленивая динамика: минимальные операции до 1 ---
    // Разрежённая мемоизация (unordered_map): посещаются только
    // n, n/2, n/3, ... — глубина O(log n); тип операций — OpCost
    // (b.md B.5, плотный O(n) аналог — `min_ops_to_one`).
    long long min_ops_to_one_lazy(long long n, const vector<OpCost>& ops) {
        const long long INF = LLONG_MAX / 4;
        unordered_map<long long, long long> memo;
        function<long long(long long)> dfs = [&](long long x) -> long long {
            if (x == 1) return 0;
            auto it = memo.find(x);
            if (it != memo.end()) return it->second;
            long long best = INF;
            for (const OpCost& op : ops) {
                if (!op.div) {
                    if (op.v < x)
                        best = min(best, op.cost + dfs(x - op.v));
                } else {
                    if (x % op.v == 0)
                        best = min(best, op.cost + dfs(x / op.v));
                }
            }
            memo[x] = best;
            return best;
        };
        return dfs(n);
    }

    // --- I.37. Ленивый длиннейший путь на DAG ---
    // Мемоизированный DFS от запрошенной вершины — вычисляются
    // только достижимые состояния (H.2 — полный проход по всем).
    // O(V + E) по достижимой части.
    vector<long long> dag_longest_path_lazy(
        const vector<vector<pair<int, int>>>& adj, int s) {
        int n = (int)adj.size();
        const long long NEG = LLONG_MIN / 4;
        vector<long long> dp(n, NEG);
        vector<char> done(n, 0);
        function<long long(int)> dfs = [&](int v) -> long long {
            if (done[v]) return dp[v];
            done[v] = 1;
            long long best = 0;
            for (auto [to, w] : adj[v])
                best = max(best, (long long)w + dfs(to));
            return dp[v] = best;
        };
        dfs(s);
        return dp;
    }

private:
    // Цифры числа x в системе b от старшего разряда; x = 0 → {0}.
    vector<int> to_digits_base(long long x, int b) {
        if (x == 0) return {0};
        vector<int> d;
        while (x > 0) {
            d.push_back((int)(x % b));
            x /= b;
        }
        reverse(d.begin(), d.end());
        return d;
    }

    // mex — наименьшее неотрицательное, отсутствующее в наборе.
    int mex_value(const vector<int>& vals) {
        vector<char> seen(vals.size() + 1, 0);
        for (int v : vals)
            if (0 <= v && v <= (int)vals.size()) seen[v] = 1;
        for (int v = 0;; v++)
            if (!seen[v]) return v;
    }

    // Гаусс с частичным выбором ведущего (Gauss-Jordan); пустой
    // вектор — нет единственного решения.
    vector<double> solve_linear_system(vector<vector<double>> a,
                                       vector<double> b) {
        int n = (int)a.size();
        for (int col = 0; col < n; col++) {
            int piv = col;
            for (int r = col + 1; r < n; r++)
                if (fabs(a[r][col]) > fabs(a[piv][col])) piv = r;
            if (fabs(a[piv][col]) < 1e-12) return {};
            swap(a[col], a[piv]);
            swap(b[col], b[piv]);
            double d = a[col][col];
            for (int c = col; c < n; c++) a[col][c] /= d;
            b[col] /= d;
            for (int r = 0; r < n; r++) {
                if (r == col) continue;
                double f = a[r][col];
                if (fabs(f) < 1e-12) continue;
                for (int c = col; c < n; c++) a[r][c] -= f * a[col][c];
                b[r] -= f * b[col];
            }
        }
        return b;
    }
};

#ifndef SPECIALDP_MAIN
signed main() {
    SpecialDP dp;

    cout << "I.1/I.2: с цифрой 7: [1,100] = " << dp.count_containing_digit(1, 100, 7)
         << " (ожидаем 19), [20,40] = " << dp.count_containing_digit(20, 40, 7)
         << " (ожидаем 2), [1,20] = " << dp.count_containing_digit(1, 20, 7)
         << " (ожидаем 2)\n";
    cout << "I.3: сумма чисел с цифрой 7: [1,20] = "
         << dp.sum_containing_digit(1, 20, 7) << " (ожидаем 24)\n";
    cout << "I.4: сумма цифр [1,10] = " << dp.sum_of_digits_range(1, 10)
         << " (ожидаем 46)\n";
    cout << "I.5: без повторов цифр: [1,10] = " << dp.count_distinct_digits(1, 10)
         << " (ожидаем 10), [1,100] = " << dp.count_distinct_digits(1, 100)
         << " (ожидаем 90)\n";
    cout << "I.6: делятся на 3: [1,100] = " << dp.count_divisible_digits(1, 100, 3)
         << " (ожидаем 33)\n";
    cout << "I.7: сумма цифр <= 9 и делятся на 5: [1,100] = "
         << dp.count_digit_sum_and_divisible(1, 100, 9, 5) << " (ожидаем 15)\n";
    cout << "I.8: сумма цифр <= 5: [1,100] = " << dp.count_digit_sum_limit(1, 100, 5)
         << " (ожидаем 21), ровно 2 единицы в двоичной: [1,20] = "
         << dp.count_numbers_with_k_bits(1, 20, 2) << " (ожидаем 9)\n";
    cout << "I.9: палиндромы: [1,1000] = " << dp.count_palindromic_range(1, 1000)
         << " (ожидаем 108), [1,10] = " << dp.count_palindromic_range(1, 10)
         << " (ожидаем 9)\n";

    cout << "I.11: броски до суммы 2 (кость 6): " << dp.expected_dice_rolls_to_sum(2, 6)
         << " (ожидаем 1.16667)\n";
    auto rw = dp.expected_absorbing_steps_1d(3);
    cout << "I.12: 1D-блуждание n=3: {" << rw[0] << ", " << rw[1] << ", "
         << rw[2] << ", " << rw[3] << "} (ожидаем {0, 2, 2, 0})\n";
    auto st = dp.stationary_distribution({{0.9, 0.1}, {0.4, 0.6}});
    cout << "I.13: стационарное распределение = {" << st[0] << ", " << st[1]
         << "} (ожидаем {0.8, 0.2})\n";
    auto vit = dp.viterbi({0, 0, 1}, {{0.5, 0.5}, {0.5, 0.5}},
                          {{1, 0}, {0, 1}}, {0.5, 0.5});
    cout << "I.14: Витерби: путь {";
    for (size_t i = 0; i < vit.first.size(); i++)
        cout << (i ? ", " : "") << vit.first[i];
    cout << "} (ожидаем {0, 0, 1}), вероятность " << vit.second << "\n";
    cout << "I.15: шаги до поглощения 0->1->2: "
         << dp.expected_steps_absorbing({{0, 1, 0}, {0, 0, 1}, {0, 0, 1}}, 0)
         << " (ожидаем 2)\n";
    cout << "I.16: сетка 2x2, p=0.5: " << dp.grid_reach_probability(2, 2, 0.5)
         << " (ожидаем 0.5)\n";
    cout << "I.17: разорение: p=0.5: " << dp.gambler_ruin_probability(1, 3, 0.5)
         << " (ожидаем 0.666667), p=0.6: " << dp.gambler_ruin_probability(1, 3, 0.6)
         << " (ожидаем 0.526316)\n";

    auto g = dp.grundy_dag({{1, 2}, {3}, {3}, {}});
    cout << "I.18: Гранди = {" << g[0] << ", " << g[1] << ", " << g[2]
         << ", " << g[3] << "} (ожидаем {0, 1, 1, 0})\n";
    auto wl = dp.win_lose_positions({{1, 2}, {3}, {3}, {}});
    cout << "I.19: win = {" << wl[0] << ", " << wl[1] << ", " << wl[2]
         << ", " << wl[3] << "} (ожидаем {0, 1, 1, 0})\n";
    cout << "I.20: Ним {1,2,3}: " << dp.nim_winner({1, 2, 3})
         << " (ожидаем 0), {1,2,4}: " << dp.nim_winner({1, 2, 4})
         << " (ожидаем 1)\n";
    auto kg = dp.kayles_grundy(10);
    cout << "I.20: Kayles g(4) = " << kg[4] << " (ожидаем 1), g(7) = " << kg[7]
         << " (ожидаем 2)\n";
    auto dg = dp.dawson_kayles_grundy(10);
    cout << "I.20: Dawson g(4) = " << dg[4] << " (ожидаем 2), g(5) = " << dg[5]
         << " (ожидаем 0)\n";
    cout << "I.20: Wythoff (1,2): " << dp.wythoff_winner(1, 2)
         << " (ожидаем 0), (2,3): " << dp.wythoff_winner(2, 3)
         << " (ожидаем 1), (3,5): " << dp.wythoff_winner(3, 5)
         << " (ожидаем 0)\n";
    cout << "I.21: Шпраг-Гранди {1,2,3}: " << dp.sprague_grundy_winner({1, 2, 3})
         << " (ожидаем 0), {2,2,1}: " << dp.sprague_grundy_winner({2, 2, 1})
         << " (ожидаем 1)\n";
    cout << "I.23: выигрышный ход: " << dp.winning_move_dag({{1, 2}, {3}, {3}, {}}, 0)
         << " (ожидаем -1), с ребром 0->3: "
         << dp.winning_move_dag({{1, 2, 3}, {3}, {3}, {}}, 0) << " (ожидаем 3)\n";
    auto sg = dp.subtraction_grundy(10, {1, 3, 4});
    cout << "I.24: вычитания {1,3,4}: g(7) = " << sg[7] << " (ожидаем 0), g(10) = "
         << sg[10] << " (ожидаем 1)\n";
    cout << "I.25: Hackenbush: бамбук 3 = "
         << dp.green_hackenbush_value({{1}, {2}, {3}, {}}, 0)
         << " (ожидаем 3), дерево = "
         << dp.green_hackenbush_value({{1, 2}, {3}, {}, {}}, 0)
         << " (ожидаем 3)\n";

    cout << "I.26: выбрать 2 из 5 без соседних: "
         << dp.count_select_k_non_adjacent(5, 2) << " (ожидаем 6)\n";
    cout << "I.27: суммы 5 из {1,2,3}: " << dp.count_subsets_with_sum(5, {1, 2, 3})
         << " (ожидаем 1), из {1,2,3,4}: "
         << dp.count_subsets_with_sum(5, {1, 2, 3, 4}) << " (ожидаем 2)\n";
    cout << "I.28: p(5,2) = " << dp.count_partitions_exactly_k(5, 2)
         << " (ожидаем 2), p(6,3) = " << dp.count_partitions_exactly_k(6, 3)
         << " (ожидаем 3)\n";
    cout << "I.29: минимум длины с суммой >= 7: "
         << dp.min_subarray_len_ge_sum(7, {2, 3, 1, 2, 4, 3}) << " (ожидаем 2)\n";
    cout << "I.30: строки без 11 (n=3): "
         << dp.count_binary_strings_without_k_ones(3, 2) << " (ожидаем 5)\n";

    cout << "I.31: взвешенные интервалы: "
         << dp.max_profit_weighted_intervals(
                {{1, 3, 5}, {2, 5, 6}, {4, 6, 5}, {6, 7, 4}})
         << " (ожидаем 14)\n";
    auto td = dp.time_dependent_shortest_path(3,
        {{0, 1, {1, 10}}, {1, 2, {10, 1}}, {0, 2, {5, 5}}}, 2, 0);
    cout << "I.32: время-зависимые стоимости: dp[2][2] = " << td[2]
         << " (ожидаем 2)\n";
    cout << "I.33: дедлайны: "
         << dp.max_profit_deadline_jobs({{2, 100}, {1, 50}, {2, 10}})
         << " (ожидаем 150)\n";
    cout << "I.34: покрытие [0,10]: "
         << dp.min_intervals_to_cover(0, 10, {{0, 3}, {3, 5}, {5, 10}})
         << " (ожидаем 3), без {3,5}: "
         << dp.min_intervals_to_cover(0, 10, {{0, 3}, {5, 10}})
         << " (ожидаем -1)\n";

    cout << "I.35: игра с концами {1,2,3,4}: "
         << dp.optimal_pick_game({1, 2, 3, 4}) << " (ожидаем 2)\n";
    cout << "I.36: лениво до 1: n=10, ops {−1, /2, /3}: "
         << dp.min_ops_to_one_lazy(10, {{1, 1, false}, {2, 1, true}, {3, 1, true}})
         << " (ожидаем 3)\n";
    vector<vector<pair<int, int>>> lazydag = {{{1, 4}, {2, 1}}, {{3, 2}},
                                              {{3, 4}}, {}};
    auto ld = dp.dag_longest_path_lazy(lazydag, 0);
    cout << "I.37: ленивый путь: dp[0] = " << ld[0] << " (ожидаем 6), dp[3] = "
         << ld[3] << " (ожидаем 0)\n";
}
#endif // SPECIALDP_MAIN
#endif // DYNAMIC_I_CPP
