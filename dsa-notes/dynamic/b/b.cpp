#ifndef DYNAMIC_B_CPP
#define DYNAMIC_B_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <deque>
#include <utility>
#include <climits>
using namespace std;

// =============================================================
// B. ОДНОМЕРНАЯ ДИНАМИКА
// =============================================================
// Структура md: A. Классические задачи (B.1–B.8)
//               → B. Комбинаторная динамика (B.9–B.13)
//               → C. Дополнительные олимпиадные задачи (B.14–B.22)
//
// LinearDP наследует DPBasics (a.cpp): мемоизация, табуляция,
// восстановление ответа и разминка (fib_memo, fib_dp, catalan_dp,
// min_steps_to_one_path) переиспользуются, а не дублируются.
// Биномиальные коэффициенты — свободная функция C_iter из
// math/combinatorics (a/a.cpp) — нужна для путей в сетке (B.11),
// Каталана по формуле (B.12) и частичных беспорядков (B.13).
//
// Порядок методов строго соответствует порядку md (B.1 → B.22).
// Ограничения int64 (демонстрационные версии, без модуля): подсчёты
// (B.9, B.10, B.11, B.12) растут быстро — демо на малых значениях;
// модулярные версии появятся с задачами в следующих разделах.

#define DP_BASICS_MAIN
#include "../a/a.cpp"

#define COMBINATORICS_MAIN
#include "../../math/combinatorics/a/a.cpp"

// Операция для B.5: вычитание v или деление на v с ценой cost
struct OpCost {
    long long v;        // вычитаемое / делитель
    long long cost;     // цена применения
    bool div;           // true — деление, false — вычитание
};

struct LinearDP : DPBasics {

    // --- B.1. Climbing Stairs: шаги длины 1..k ---
    // f(n) = Σ_{i=1..k, i ≤ n} f(n−i), f(0) = 1 (стоим у подножия);
    // последний шаг длины i различает множества способов.
    // O(n·k) времени, O(n) памяти (достаточно последних k значений).
    long long climbing_stairs(int n, int k) {
        vector<long long> dp(n + 1, 0);
        dp[0] = 1;
        for (int i = 1; i <= n; i++)
            for (int d = 1; d <= k && d <= i; d++)
                dp[i] += dp[i - d];
        return dp[n];
    }

    // --- B.1. Climbing Stairs: не более t шагов длины m подряд ---
    // Состояние расширяется длиной серии (a.md A.3): dp[i][j] — способы
    // на ступени i с серией шагов длины m длиной j (j = 0 — последний шаг
    // другой длины); шаг длины m ведёт в j+1 (пока j+1 ≤ t), другой длины —
    // в j = 0; ответ — Σⱼ dp[n][j]. O(n·t·k) времени, O(n·t) памяти.
    long long climbing_stairs_constrained(int n, int k, int m, int t) {
        vector<vector<long long>> dp(n + 1, vector<long long>(t + 1, 0));
        dp[0][0] = 1;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= t; j++) {
                if (dp[i][j] == 0) continue;
                for (int d = 1; d <= k && i + d <= n; d++) {
                    if (d == m) {
                        if (j + 1 <= t) dp[i + d][j + 1] += dp[i][j];
                    } else {
                        dp[i + d][0] += dp[i][j];
                    }
                }
            }
        }
        long long ans = 0;
        for (int j = 0; j <= t; j++) ans += dp[n][j];
        return ans;
    }

    // --- B.2. Линейная рекуррента порядка k ---
    // a[n] = Σ_{i=1..k} c[i−1]·a[n−i]; init — база a[0..k−1]. Значения
    // хранятся по кольцу размера k: a[i mod k] = a[i]. Трибоначчи
    // (T(0)=0, T(1)=0, T(2)=1): linear_recurrence(n, {1,1,1}, {0,0,1}).
    // O(n·k) времени, O(k) памяти; матрица перехода k×k (O(k³·log n)) —
    // раздел C (MatrixDP).
    long long linear_recurrence(long long n, const vector<long long>& coeff,
                                const vector<long long>& init) {
        int k = (int)coeff.size();
        vector<long long> a(k, 0);
        for (int i = 0; i < k; i++) a[i] = init[i];
        if (n < k) return a[(int)n];
        for (long long i = k; i <= n; i++) {
            long long v = 0;
            for (int j = 0; j < k; j++) v += coeff[j] * a[(i - 1 - j) % k];
            a[i % k] = v;
        }
        return a[n % k];
    }

    // --- B.3. Max Non Adjacent Sum ---
    // dp[i] = max(dp[i−1], dp[i−2] + a[i]): пропустить i-й или взять его;
    // храним два последних значения. O(n) времени, O(1) памяти.
    // Значения неотрицательные (классика House Robber); пустой выбор — 0.
    long long max_non_adjacent_sum(const vector<long long>& a) {
        long long prev2 = 0, prev1 = 0;
        for (long long x : a) {
            long long cur = max(prev1, prev2 + x);
            prev2 = prev1;
            prev1 = cur;
        }
        return prev1;
    }

    // --- B.3. Кольцевой вариант: первый и последний элементы соседние ---
    // Максимум двух проходов: по a[0..n−2] (без последнего) и по a[1..n−1]
    // (без первого) — любой допустимый выбор не содержит оба конца сразу.
    long long max_non_adjacent_sum_circular(const vector<long long>& a) {
        int n = (int)a.size();
        if (n == 1) return a[0];
        long long best = 0;
        long long prev2 = 0, prev1 = 0;
        for (int i = 1; i < n; i++) {
            long long cur = max(prev1, prev2 + a[i]);
            prev2 = prev1;
            prev1 = cur;
        }
        best = max(best, prev1);
        prev2 = prev1 = 0;
        for (int i = 0; i < n - 1; i++) {
            long long cur = max(prev1, prev2 + a[i]);
            prev2 = prev1;
            prev1 = cur;
        }
        best = max(best, prev1);
        return best;
    }

    // --- B.4. Max Subarray Sum (алгоритм Кадане) ---
    // best[i] = max(best[i−1] + a[i], a[i]) — лучший подотрезок,
    // оканчивающийся в i; ответ — максимум по всем i. O(n) времени, O(1) памяти.
    long long max_subarray_sum(const vector<long long>& a) {
        if (a.empty()) return 0;
        long long cur = a[0], best = a[0];
        for (int i = 1; i < (int)a.size(); i++) {
            cur = max(cur + a[i], a[i]);
            best = max(best, cur);
        }
        return best;
    }

    // --- B.4. Минимальный подотрезок — та же рекуррентность с min ---
    long long min_subarray_sum(const vector<long long>& a) {
        if (a.empty()) return 0;
        long long cur = a[0], best = a[0];
        for (int i = 1; i < (int)a.size(); i++) {
            cur = min(cur + a[i], a[i]);
            best = min(best, cur);
        }
        return best;
    }

    // --- B.5. Minimum Steps To One с ценами операций ---
    // f(1) = 0; f(n) = min по операциям (cost + f(результат)): вычитание —
    // пока n − v ≥ 1, деление — только при n % p == 0. Классика
    // «1 + min(f(n−1), f(n/2), f(n/3))» — частный случай единичных цен.
    // O(n·|ops|) времени, O(n) памяти. Восстановление пути — a.md B.4.
    long long min_ops_to_one(int n, const vector<OpCost>& ops) {
        const long long INF = LLONG_MAX / 4;
        vector<long long> dp(n + 1, INF);
        dp[1] = 0;
        for (int i = 2; i <= n; i++) {
            for (const OpCost& op : ops) {
                if (!op.div) {
                    if (op.v < i && dp[i - op.v] + op.cost < dp[i])
                        dp[i] = dp[i - op.v] + op.cost;
                } else {
                    if (i % op.v == 0 && dp[i / op.v] + op.cost < dp[i])
                        dp[i] = dp[i / op.v] + op.cost;
                }
            }
        }
        return dp[n];
    }

    // --- B.6. Minimum Coin Change: минимум монет + набор ---
    // dp[s] = min_{c ≤ s} (dp[s − c]) + 1; INF — сумма недостижима;
    // prev[s] — монета оптимального перехода, обратный проход собирает
    // набор (a.md B.4). O(s·k) времени, O(s) памяти.
    pair<long long, vector<long long>> coin_change_min(const vector<long long>& coins, int s) {
        const long long INF = LLONG_MAX / 4;
        vector<long long> dp(s + 1, INF), prev(s + 1, -1);
        dp[0] = 0;
        for (int x = 1; x <= s; x++)
            for (long long c : coins)
                if (c <= x && dp[x - c] + 1 < dp[x]) {
                    dp[x] = dp[x - c] + 1;
                    prev[x] = c;
                }
        vector<long long> used;
        if (dp[s] < INF) {
            for (int x = s; x > 0; x -= (int)prev[x]) used.push_back(prev[x]);
        }
        return {dp[s], used};
    }

    // --- B.7. Minimum Tickets Cost ---
    // dp[d] = dp[d−1] для дней без поездки; для дня с поездкой
    // dp[d] = minⱼ (dp[max(0, d − dⱼ)] + pⱼ) — билет j, купленный в день
    // d − dⱼ + 1, покрывает d. O(D·|билеты|) времени, O(D) памяти.
    long long min_tickets_cost(const vector<int>& days, const vector<int>& dur,
                               const vector<long long>& price) {
        int D = days.back();
        vector<char> visit(D + 1, 0);
        for (int d : days) visit[d] = 1;
        vector<long long> dp(D + 1, 0);
        for (int d = 1; d <= D; d++) {
            if (!visit[d]) { dp[d] = dp[d - 1]; continue; }
            long long best = LLONG_MAX / 4;
            for (int j = 0; j < (int)dur.size(); j++)
                best = min(best, dp[max(0, d - dur[j])] + price[j]);
            dp[d] = best;
        }
        return dp[D];
    }

    // --- B.8. Задача Иосифа: J(n,k) = (J(n−1,k) + k) mod n, J(1,k) = 0 ---
    // 0-индексация; после первого выбывания круг размера n−1 со сдвигом k.
    // O(n) времени, O(1) памяти.
    long long josephus(long long n, long long k) {
        long long x = 0;
        for (long long m = 2; m <= n; m++) x = (x + k) % m;
        return x;
    }

    // --- B.8. Индексация с 1: ответ = J(n, k) + 1 ---
    long long josephus_1based(long long n, long long k) { return josephus(n, k) + 1; }

    // --- B.8. Быстрый вариант O(k·log n): прыжки по блокам ---
    // x = J(m, k); t шагов подряд не «заворачиваются», пока x + t·k ≤ m + t,
    // т.е. t ≤ (m − x)/(k − 1); при t = 0 — обычный шаг; после прыжка при
    // x ≥ m — x %= m. Для k = 1 ответ — n − 1.
    long long josephus_fast(long long n, long long k) {
        if (k == 1) return n - 1;
        long long m = 1, x = 0;
        while (m < n) {
            long long t = (m - x) / (k - 1);
            if (t == 0) {
                m++;
                x = (x + k) % m;
                continue;
            }
            if (m + t > n) t = n - m;
            x += t * k;
            m += t;
            if (x >= m) x %= m;
        }
        return x;
    }

    // --- B.8. Последние m выживших ---
    // Рекуррентность сохраняется для набора: при росте размера круга sz
    // от m до n каждый выживший сдвигается sᵢ = (sᵢ + k) mod sz (набор
    // {0..m−1} при sz = m). O((n − m)·m) времени, O(m) памяти.
    vector<long long> josephus_last_m(long long n, long long k, int m) {
        vector<long long> s(m);
        for (int i = 0; i < m; i++) s[i] = i;
        for (long long sz = m + 1; sz <= n; sz++)
            for (int i = 0; i < m; i++) s[i] = (s[i] + k) % sz;
        sort(s.begin(), s.end());
        return s;
    }

    // --- B.9. Combination Sum IV: порядок важен ---
    // dp[0] = 1; dp[s] = Σ_{cᵢ ≤ s} dp[s − cᵢ] — внешний цикл по суммам,
    // монеты — внутренний (последний элемент последовательности — монета).
    // O(s·k) времени, O(s) памяти; подсчёты растут быстро — демо на малых s.
    long long combination_sum4(const vector<long long>& coins, int s) {
        vector<long long> dp(s + 1, 0);
        dp[0] = 1;
        for (int x = 1; x <= s; x++)
            for (long long c : coins)
                if (c <= x) dp[x] += dp[x - c];
        return dp[s];
    }

    // --- B.10. Размен монет: количество способов, порядок не важен ---
    // Внешний цикл по монетам: dp[s] += dp[s − cᵢ] — наборы только из уже
    // введённых монет; перестановки не считаются. O(s·k) времени, O(s) памяти.
    long long coin_change_ways(const vector<long long>& coins, int s) {
        vector<long long> dp(s + 1, 0);
        dp[0] = 1;
        for (long long c : coins)
            for (int x = c; x <= s; x++)
                dp[x] += dp[x - c];
        return dp[s];
    }

    // --- B.11. Пути в сетке: C(n₁ + n₂, n₁) ---
    // Из (0,0) в (n₁, n₂) шагами вниз/вправо — перестановка n₁ + n₂ шагов,
    // позиции шагов одного типа выбираются биномом. O(n₁) времени (C_iter).
    // С запрещёнными клетками — раздел C (MatrixDP).
    long long grid_paths(int rows, int cols) {
        return C_iter(rows + cols - 2, rows - 1);
    }

    // --- B.12. Числа Каталана по формуле: C(2n, n)/(n+1) ---
    // Сверяется с catalan_dp (база, a.md 5.3); при n ≤ 30 в int64:
    // C(60,30)/31 = C_30 = 3814986502092304.
    long long catalan_by_binom(int n) {
        return C_iter(2LL * n, n) / (n + 1);
    }

    // --- B.13. Беспорядки: D(n) = (n−1)·(D(n−1) + D(n−2)) ---
    // D(1) = 0, D(2) = 1 (D(0) = 1 — пустая перестановка); O(n) времени,
    // O(1) памяти. Асимптотика: ⌊n!/e + 1/2⌋.
    long long derangements(int n) {
        if (n == 0) return 1;
        long long d0 = 0, d1 = 1;
        if (n == 1) return d0;
        if (n == 2) return d1;
        for (int i = 3; i <= n; i++) {
            long long d2 = (long long)(i - 1) * (d0 + d1);
            d0 = d1;
            d1 = d2;
        }
        return d1;
    }

    // --- B.13. Через включения-исключения (сверка с рекуррентностью) ---
    // D(n) = Σ_{i=0..n} (−1)ⁱ·C(n, i)·(n−i)! — факториалы в int64 до n = 20.
    long long derangements_ie(int n) {
        vector<long long> fact(n + 1, 1);
        for (int i = 1; i <= n; i++) fact[i] = fact[i - 1] * i;
        long long res = 0;
        for (int i = 0; i <= n; i++) {
            long long term = C_iter(n, i) * fact[n - i];
            res += (i % 2 ? -term : term);
        }
        return res;
    }

    // --- B.13. Частичные беспорядки: ровно k неподвижных точек ---
    // C(n, k)·D(n − k) — выбрать позиции неподвижных + беспорядок остальных.
    long long derangements_partial(int n, int k) {
        return C_iter(n, k) * derangements(n - k);
    }

    // --- B.14. Decode Ways: алфавит из m кодов ---
    // Коды — одна цифра 1..m или две цифры 10..m: dp[i] =
    // dp[i−1]·[1 ≤ aᵢ ≤ m] + dp[i−2]·[10 ≤ aᵢ₋₁aᵢ ≤ m]; невалидные коды
    // дают 0 (индикатор). O(n) времени, O(n) памяти (достаточно двух).
    long long decode_ways(const string& s, int m) {
        int n = (int)s.size();
        vector<long long> dp(n + 1, 0);
        dp[0] = 1;
        for (int i = 1; i <= n; i++) {
            int a = s[i - 1] - '0';
            if (1 <= a && a <= m) dp[i] += dp[i - 1];
            if (i >= 2) {
                int b = (s[i - 2] - '0') * 10 + a;
                if (10 <= b && b <= m) dp[i] += dp[i - 2];
            }
        }
        return dp[n];
    }

    // --- B.15. Одна транзакция: profit = max(p[i] − min_{j<i} p[j]) ---
    // Покупаем в минимуме до i, продаём в i. O(n) времени, O(1) памяти.
    long long max_profit_one(const vector<long long>& p) {
        long long mn = p[0], best = 0;
        for (int i = 1; i < (int)p.size(); i++) {
            best = max(best, p[i] - mn);
            mn = min(mn, p[i]);
        }
        return best;
    }

    // --- B.15. k транзакций ---
    // dp[t][d] = max(dp[t][d−1], p[d] + max_{d'<d} (dp[t−1][d'] − p[d']));
    // максимум по d' поддерживается в ходе прохода. O(k·n) времени, O(n) памяти.
    long long max_profit_k(int k, const vector<long long>& p) {
        int n = (int)p.size();
        vector<long long> prev(n, 0), cur(n, 0);
        for (int t = 1; t <= k; t++) {
            long long best = prev[0] - p[0];
            cur[0] = 0;
            for (int d = 1; d < n; d++) {
                cur[d] = max(cur[d - 1], p[d] + best);
                best = max(best, prev[d] - p[d]);
            }
            prev.swap(cur);
        }
        return prev[n - 1];
    }

    // --- B.15. k транзакций с комиссией fee за транзакцию ---
    // Из цены продажи вычитается fee: p[d] − fee + max_{d'<d}(dp[t−1][d'] − p[d']).
    long long max_profit_k_fee(int k, const vector<long long>& p, long long fee) {
        int n = (int)p.size();
        vector<long long> prev(n, 0), cur(n, 0);
        for (int t = 1; t <= k; t++) {
            long long best = prev[0] - p[0];
            cur[0] = 0;
            for (int d = 1; d < n; d++) {
                cur[d] = max(cur[d - 1], p[d] - fee + best);
                best = max(best, prev[d] - p[d]);
            }
            prev.swap(cur);
        }
        return prev[n - 1];
    }

    // --- B.15. Cooldown: покупка не раньше, чем через день после продажи ---
    // Состояние — наличие акций (a.md A.3): dp0[d] = max(dp0[d−1], dp1[d−1] + p[d]);
    // dp1[d] = max(dp1[d−1], dp0[d−2] − p[d]) — покупка требует двух дней
    // без акций. O(n) времени, O(1) памяти.
    long long max_profit_cooldown(const vector<long long>& p) {
        long long d0_prev2 = 0, d0_prev1 = 0;
        long long d1_prev1 = -(LLONG_MAX / 4);
        for (long long x : p) {
            long long d0 = max(d0_prev1, d1_prev1 + x);
            long long d1 = max(d1_prev1, d0_prev2 - x);
            d0_prev2 = d0_prev1;
            d0_prev1 = d0;
            d1_prev1 = d1;
        }
        return d0_prev1;
    }

    // --- B.16. Jump Game: достижимость конца ---
    // Множество достижимых позиций — префикс [0..farthest]: если i > farthest,
    // позиция i недостижима. O(n) времени, O(1) памяти.
    bool can_reach(const vector<int>& a) {
        long long farthest = 0;
        for (int i = 0; i < (int)a.size(); i++) {
            if (i > farthest) return false;
            farthest = max(farthest, (long long)i + a[i]);
        }
        return true;
    }

    // --- B.16. Минимальное число прыжков (жадность слоями) ---
    // Прыжок засчитывается при достижении границы текущего слоя cur;
    // farthest — граница следующего слоя. O(n) времени, O(1) памяти.
    int min_jumps(const vector<int>& a) {
        int n = (int)a.size();
        if (n <= 1) return 0;
        int cur = 0, farthest = 0, jumps = 0;
        for (int i = 0; i < n - 1; i++) {
            farthest = max(farthest, i + a[i]);
            if (i == cur) {
                jumps++;
                cur = farthest;
            }
        }
        return jumps;
    }

    // --- B.17. Ugly Numbers: слияние k монотонных последовательностей ---
    // Каждое следующее число — минимум кандидатов res[idx[j]]·pⱼ; указатели
    // idx[j] двигаются у всех j, давших минимум (без дубликатов).
    // O(n·k) времени, O(n) памяти.
    long long ugly_numbers(int n, const vector<long long>& primes) {
        vector<long long> res(n, 1);
        vector<int> idx(primes.size(), 0);
        for (int i = 1; i < n; i++) {
            long long nxt = LLONG_MAX / 4;
            for (int j = 0; j < (int)primes.size(); j++)
                nxt = min(nxt, res[idx[j]] * primes[j]);
            res[i] = nxt;
            for (int j = 0; j < (int)primes.size(); j++)
                if (res[idx[j]] * primes[j] == nxt) idx[j]++;
        }
        return res[n - 1];
    }

    // --- B.18. Минимальное число слагаемых из множества parts ---
    // dp[n] = min_{p ∈ parts, p ≤ n} dp[n − p] + 1; Perfect Squares —
    // частный случай (parts = квадраты); та же рекуррентность, что B.6,
    // но без восстановления. O(n·|parts|) времени, O(n) памяти.
    long long min_summands(int n, const vector<int>& parts) {
        const long long INF = LLONG_MAX / 4;
        vector<long long> dp(n + 1, INF);
        dp[0] = 0;
        for (int x = 1; x <= n; x++)
            for (int p : parts)
                if (p <= x && dp[x - p] + 1 < dp[x]) dp[x] = dp[x - p] + 1;
        return dp[n];
    }

    // --- B.19. Paint Fence: не более t одинаковых цветов подряд ---
    // Состояние — длина текущей серии (a.md A.3): dp[i][1] = (k−1)·Σⱼ dp[i−1][j]
    // (серию не продолжаем), dp[i][j] = dp[i−1][j−1] для j = 2..t (продолжаем).
    // Классика (t = 2): n = 3, k = 2 → 6. O(n·t) времени, O(t) памяти.
    long long paint_fence(int n, int k, int t) {
        vector<long long> dp(t + 1, 0), ndp(t + 1, 0);
        dp[1] = k;
        for (int i = 2; i <= n; i++) {
            fill(ndp.begin(), ndp.end(), 0);
            long long total = 0;
            for (int j = 1; j <= t; j++) total += dp[j];
            ndp[1] = total * (k - 1);
            for (int j = 2; j <= t; j++) ndp[j] = dp[j - 1];
            dp.swap(ndp);
        }
        long long ans = 0;
        for (int j = 1; j <= t; j++) ans += dp[j];
        return ans;
    }

    // --- B.20. Delete and Earn: сжатие по значениям → House Robber (B.3) ---
    // sum[v] = v·count(v); конфликтуют только соседние значения — берём
    // max_non_adjacent_sum по массиву сумм. O(max + n) времени, O(max) памяти.
    long long delete_and_earn(const vector<int>& a) {
        if (a.empty()) return 0;
        int mx = *max_element(a.begin(), a.end());
        vector<long long> sum(mx + 1, 0);
        for (int x : a) sum[x] += x;
        return max_non_adjacent_sum(sum);
    }

    // --- B.21. Максимальная цепочка пар (Activity DP) ---
    // Сортировка по второму элементу; dp[i] = 1 + max_{j: bⱼ < aᵢ} dp[j] —
    // лучшая цепочка, оканчивающаяся парой i. O(n²) времени, O(n) памяти;
    // жадность по правому концу даёт O(n log n) (см. md).
    long long max_pair_chain(vector<pair<long long, long long>> pairs) {
        int n = (int)pairs.size();
        sort(pairs.begin(), pairs.end(),
             [](const pair<long long, long long>& x, const pair<long long, long long>& y) {
                 return x.second < y.second;
             });
        vector<long long> dp(n, 1);
        long long ans = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++)
                if (pairs[j].second < pairs[i].first) dp[i] = max(dp[i], dp[j] + 1);
            ans = max(ans, dp[i]);
        }
        return ans;
    }

    // --- B.22. Максимальный подотрезок длины ≤ k (обобщение B.4) ---
    // Сумма [l, r] = pref[r] − pref[l−1]; для каждого r нужен минимум pref
    // в окне l−1 ∈ [r−k, r−1] — монотонная очередь (deque, задел на J):
    // индексы с возрастающими pref; из конца выбрасываем всё ≥ pref[r−1],
    // из начала — вышедшие из окна (< r−k); минимум — в голове.
    // O(n) времени, O(k) памяти.
    long long max_subarray_sum_len_k(const vector<long long>& a, int k) {
        int n = (int)a.size();
        if (n == 0) return 0;
        vector<long long> pref(n + 1, 0);
        for (int i = 1; i <= n; i++) pref[i] = pref[i - 1] + a[i - 1];
        deque<int> dq;
        long long best = -(LLONG_MAX / 4);
        for (int r = 1; r <= n; r++) {
            while (!dq.empty() && dq.front() < r - k) dq.pop_front();
            while (!dq.empty() && pref[dq.back()] >= pref[r - 1]) dq.pop_back();
            dq.push_back(r - 1);
            best = max(best, pref[r] - pref[dq.front()]);
        }
        return best;
    }
};

// =============================================================
// Демонстрация (сверка с ожидаемыми значениями)
// =============================================================
#ifndef LINEARDP_MAIN
signed main() {
    LinearDP dp;

    cout << "B.1: climbing_stairs(10, 2) = " << dp.climbing_stairs(10, 2) << " (ожидаем 89)\n";
    cout << "B.1: climbing_stairs(10, 3) = " << dp.climbing_stairs(10, 3) << " (ожидаем 274)\n";
    cout << "B.1: climbing_stairs_constrained(5, 2, 1, 1) = "
         << dp.climbing_stairs_constrained(5, 2, 1, 1) << " (ожидаем 3)\n";

    cout << "B.2: linear_recurrence(25, {1,1,1}, {0,0,1}) [трибоначчи] = "
         << dp.linear_recurrence(25, {1, 1, 1}, {0, 0, 1}) << " (ожидаем 755476)\n";
    cout << "B.2: linear_recurrence(90, {1,1}, {0,1}) [фибоначчи] = "
         << dp.linear_recurrence(90, {1, 1}, {0, 1}) << " (ожидаем 2880067194370816120)\n";

    cout << "B.3: max_non_adjacent_sum({2,7,9,3,1}) = "
         << dp.max_non_adjacent_sum({2, 7, 9, 3, 1}) << " (ожидаем 12)\n";
    cout << "B.3: max_non_adjacent_sum_circular({2,3,2}) = "
         << dp.max_non_adjacent_sum_circular({2, 3, 2}) << " (ожидаем 3)\n";

    cout << "B.4: max_subarray_sum({-2,1,-3,4,-1,2,1,-5,4}) = "
         << dp.max_subarray_sum({-2, 1, -3, 4, -1, 2, 1, -5, 4}) << " (ожидаем 6)\n";
    cout << "B.4: min_subarray_sum({-2,1,-3,4,-1,2,1,-5,4}) = "
         << dp.min_subarray_sum({-2, 1, -3, 4, -1, 2, 1, -5, 4}) << " (ожидаем -5)\n";

    cout << "B.5: min_ops_to_one(10, {-1:1, /3:3}) = "
         << dp.min_ops_to_one(10, {{1, 1, false}, {3, 3, true}}) << " (ожидаем 6)\n";
    cout << "B.5: min_ops_to_one(10, {-1:1, /2:1, /3:1}) = "
         << dp.min_ops_to_one(10, {{1, 1, false}, {2, 1, true}, {3, 1, true}}) << " (ожидаем 3)\n";

    auto coins = dp.coin_change_min({1, 3, 4}, 6);
    cout << "B.6: coin_change_min({1,3,4}, 6) = " << coins.first << " (ожидаем 2), набор:";
    for (long long c : coins.second) cout << " " << c;
    cout << "\n";

    cout << "B.7: min_tickets_cost({1,4,6,7,8,20}, {1,7,30}, {2,7,15}) = "
         << dp.min_tickets_cost({1, 4, 6, 7, 8, 20}, {1, 7, 30}, {2, 7, 15}) << " (ожидаем 11)\n";

    cout << "B.8: josephus(10, 3) = " << dp.josephus(10, 3) << " (ожидаем 3)\n";
    cout << "B.8: josephus_1based(10, 3) = " << dp.josephus_1based(10, 3) << " (ожидаем 4)\n";
    cout << "B.8: josephus_fast(10^18, 2) = " << dp.josephus_fast(1000000000000000000LL, 2)
         << " (ожидаем 847078495393153024)\n";
    cout << "B.8: josephus_fast(10, 3) = " << dp.josephus_fast(10, 3) << " (ожидаем 3)\n";
    auto surv = dp.josephus_last_m(10, 3, 3);
    cout << "B.8: josephus_last_m(10, 3, 3) =";
    for (long long x : surv) cout << " " << x;
    cout << " (ожидаем 3 4 9)\n";

    cout << "B.9: combination_sum4({1,2,3}, 4) = "
         << dp.combination_sum4({1, 2, 3}, 4) << " (ожидаем 7)\n";
    cout << "B.10: coin_change_ways({1,2,3}, 4) = "
         << dp.coin_change_ways({1, 2, 3}, 4) << " (ожидаем 4)\n";
    cout << "B.11: grid_paths(10, 10) = " << dp.grid_paths(10, 10) << " (ожидаем 48620)\n";

    cout << "B.12: catalan_by_binom(30) = " << dp.catalan_by_binom(30)
         << " (ожидаем 3814986502092304, совпадает с catalan_dp(30) = "
         << dp.catalan_dp(30) << ")\n";

    cout << "B.13: derangements(10) = " << dp.derangements(10) << " (ожидаем 1334961)\n";
    cout << "B.13: derangements_ie(10) = " << dp.derangements_ie(10) << " (ожидаем 1334961)\n";
    cout << "B.13: derangements_partial(5, 2) = " << dp.derangements_partial(5, 2)
         << " (ожидаем 20)\n";

    cout << "B.14: decode_ways(\"226\", 26) = " << dp.decode_ways("226", 26) << " (ожидаем 3)\n";
    cout << "B.14: decode_ways(\"226\", 20) = " << dp.decode_ways("226", 20) << " (ожидаем 1)\n";

    cout << "B.15: max_profit_one({7,1,5,3,6,4}) = "
         << dp.max_profit_one({7, 1, 5, 3, 6, 4}) << " (ожидаем 5)\n";
    cout << "B.15: max_profit_k(2, {3,2,6,5,0,3}) = "
         << dp.max_profit_k(2, {3, 2, 6, 5, 0, 3}) << " (ожидаем 7)\n";
    cout << "B.15: max_profit_k_fee(2, {1,3,2,8,4,9}, 2) = "
         << dp.max_profit_k_fee(2, {1, 3, 2, 8, 4, 9}, 2) << " (ожидаем 8)\n";
    cout << "B.15: max_profit_cooldown({1,2,3,0,2}) = "
         << dp.max_profit_cooldown({1, 2, 3, 0, 2}) << " (ожидаем 3)\n";

    cout << "B.16: can_reach({2,3,1,1,4}) = " << dp.can_reach({2, 3, 1, 1, 4})
         << " (ожидаем 1); can_reach({3,2,1,0,4}) = " << dp.can_reach({3, 2, 1, 0, 4})
         << " (ожидаем 0)\n";
    cout << "B.16: min_jumps({2,3,1,1,4}) = " << dp.min_jumps({2, 3, 1, 1, 4})
         << " (ожидаем 2)\n";

    cout << "B.17: ugly_numbers(10, {2,3,5}) = " << dp.ugly_numbers(10, {2, 3, 5})
         << " (ожидаем 12)\n";

    cout << "B.18: min_summands(12, {1,4,9}) [квадраты] = "
         << dp.min_summands(12, {1, 4, 9}) << " (ожидаем 3)\n";
    cout << "B.18: min_summands(7, {2,5}) = " << dp.min_summands(7, {2, 5}) << " (ожидаем 2)\n";
    long long r18 = dp.min_summands(3, {2, 5});
    cout << "B.18: min_summands(3, {2,5}) = "
         << (r18 > 1e18 ? "INF" : to_string(r18)) << " (ожидаем INF)\n";

    cout << "B.19: paint_fence(3, 2, 2) = " << dp.paint_fence(3, 2, 2) << " (ожидаем 6)\n";
    cout << "B.19: paint_fence(4, 2, 2) = " << dp.paint_fence(4, 2, 2) << " (ожидаем 10)\n";
    cout << "B.19: paint_fence(3, 2, 3) = " << dp.paint_fence(3, 2, 3) << " (ожидаем 8)\n";

    cout << "B.20: delete_and_earn({3,4,2}) = " << dp.delete_and_earn({3, 4, 2})
         << " (ожидаем 6)\n";
    cout << "B.20: delete_and_earn({2,2,3,3,3,4}) = " << dp.delete_and_earn({2, 2, 3, 3, 3, 4})
         << " (ожидаем 9)\n";

    cout << "B.21: max_pair_chain({{1,2},{2,3},{3,4}}) = "
         << dp.max_pair_chain({{1, 2}, {2, 3}, {3, 4}}) << " (ожидаем 2)\n";
    cout << "B.21: max_pair_chain({{1,2},{7,8},{4,5}}) = "
         << dp.max_pair_chain({{1, 2}, {7, 8}, {4, 5}}) << " (ожидаем 3)\n";

    cout << "B.22: max_subarray_sum_len_k({1,2,3,-4,5}, 2) = "
         << dp.max_subarray_sum_len_k({1, 2, 3, -4, 5}, 2) << " (ожидаем 5)\n";
}
#endif // LINEARDP_MAIN
#endif // DYNAMIC_B_CPP
