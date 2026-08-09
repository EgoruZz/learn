#ifndef DYNAMIC_D_CPP
#define DYNAMIC_D_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <climits>
#include <utility>
using namespace std;

// =============================================================
// D. ДИНАМИКА ПО ОТРЕЗКАМ И ПОДСТРОКАМ
// =============================================================
// Структура md: A. Оптимизация разбиений — Interval DP (D.1–D.10)
//               → B. Динамика на подстроках (D.11–D.22)
//
// SegmentDP наследует MatrixDP (c.cpp), а через него LinearDP (b.cpp)
// и DPBasics (a.cpp). Переиспользуются, а не дублируются: 1D-ядро
// max_subarray_sum_no_more_than_k (C.6 → формализация D.19),
// неограниченный рюкзак knapsack_unbounded (C.20 → D.5), счёт
// подмножеств subset_sum_count (C.23 → сверка D.22).
//
// Порядок методов строго соответствует порядку md (D.1 → D.22).
// Ограничения int64: счётные методы (D.11, D.17, D.18, D.22) растут
// быстро — демо на малых значениях; модулярные версии — раздел I.

#define MATRIXDP_MAIN
#include "../c/c.cpp"

struct SegmentDP : MatrixDP {

    // --- D.1. Matrix Chain Multiplication ---
    // dp[i][j] = minₖ (dp[i][k] + dp[k+1][j] + p[i]·p[k+1]·p[j+1]) —
    // последнее умножение делит блок матриц i..j на левый (i..k) и
    // правый (k+1..j); dp[i][i] = 0 (a.md B.2 — длина внешний цикл).
    // O(n³) времени, O(n²) памяти.
    long long matrix_chain_min(const vector<long long>& p) {
        int n = (int)p.size() - 1;
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                f[i][j] = LLONG_MAX / 4;
                for (int k = i; k < j; k++)
                    f[i][j] = min(f[i][j], f[i][k] + f[k + 1][j]
                                        + p[i] * p[k + 1] * p[j + 1]);
            }
        return f[0][n - 1];
    }

    // --- D.2. Matrix Chain Order: восстановление скобок ---
    // Тот же пересчёт плюс opt[i][j] = argmin k — точка последнего
    // умножения; скобки восстанавливаются рекурсивным обходом блоков
    // (a.md B.4 — обратный проход по таблице решений). O(n³) времени.
    string matrix_chain_order(const vector<long long>& p) {
        int n = (int)p.size() - 1;
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        vector<vector<int>> opt(n, vector<int>(n, -1));
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                f[i][j] = LLONG_MAX / 4;
                for (int k = i; k < j; k++) {
                    long long cur = f[i][k] + f[k + 1][j]
                                  + p[i] * p[k + 1] * p[j + 1];
                    if (cur < f[i][j]) { f[i][j] = cur; opt[i][j] = k; }
                }
            }
        function<string(int, int)> build = [&](int i, int j) {
            if (i == j) return "A" + to_string(i + 1);
            int k = opt[i][j];
            return "(" + build(i, k) + build(k + 1, j) + ")";
        };
        return build(0, n - 1);
    }

    // --- D.2. Matrix Chain: разные цены скобок ---
    // merge(l, r, c) — стоимость последнего умножения блока (l×r)·(r×c);
    // произведение p[i]·p[k+1]·p[j+1] — частный случай merge
    // (поглощение констант). O(n³) времени, O(n²) памяти.
    long long matrix_chain_general(
            const vector<long long>& p,
            const function<long long(long long, long long, long long)>& merge) {
        int n = (int)p.size() - 1;
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                f[i][j] = LLONG_MAX / 4;
                for (int k = i; k < j; k++)
                    f[i][j] = min(f[i][j], f[i][k] + f[k + 1][j]
                                        + merge(p[i], p[k + 1], p[j + 1]));
            }
        return f[0][n - 1];
    }

    // --- D.3. Palindrome Partitioning ---
    // pal[i][j] — палиндромность s[i..j] (предподсчёт по длинам);
    // f[i] = 1 + min{f[j] : pal[j][i−1]} — последняя палиндромная часть
    // s[j..i). Каркас «1 + min по предыдущим» — b.md B.1/B.6.
    // O(n²) времени, O(n²) памяти.
    long long min_palindrome_partitions(const string& s) {
        int n = (int)s.size();
        vector<vector<bool>> pal(n, vector<bool>(n, false));
        for (int i = 0; i < n; i++) pal[i][i] = true;
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                if (s[i] == s[j] && (len == 2 || pal[i + 1][j - 1]))
                    pal[i][j] = true;
            }
        vector<long long> f(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            f[i] = LLONG_MAX / 4;
            for (int j = 0; j < i; j++)
                if (pal[j][i - 1]) f[i] = min(f[i], f[j] + 1);
        }
        return f[n];
    }

    // --- D.4. Optimal Binary Search Tree ---
    // f[i][j] = minₖ (f[i][k−1] + f[k+1][j]) + Σp[i..j] — корень k делит
    // блок ключей; добавка Σp — все ключи блока опускаются на уровень
    // вниз, не зависит от выбора k (префиксные суммы за O(1)).
    // O(n³) времени, O(n²) памяти; Knuth-оптимизация — раздел J (J.6).
    long long optimal_bst_cost(const vector<long long>& freq) {
        int n = (int)freq.size();
        vector<long long> pref(n + 1, 0);
        for (int i = 0; i < n; i++) pref[i + 1] = pref[i] + freq[i];
        vector<vector<long long>> f(n + 2, vector<long long>(n + 2, 0));
        for (int len = 1; len <= n; len++)
            for (int i = 1; i + len - 1 <= n; i++) {
                int j = i + len - 1;
                f[i][j] = LLONG_MAX / 4;
                for (int k = i; k <= j; k++)
                    f[i][j] = min(f[i][j], f[i][k - 1] + f[k + 1][j]
                                        + pref[j] - pref[i - 1]);
            }
        return f[1][n];
    }

    // --- D.5. Rod Cutting ---
    // f[len] = maxᵢ (f[len − i] + p[i]) — первый кусок длины i; это ровно
    // неограниченный рюкзак (C.20) с «предметами» (вес i, ценность p[i])
    // и вместимостью len — переиспользуем knapsack_unbounded.
    // O(n²) времени, O(n) памяти.
    long long rod_cutting(const vector<long long>& p, long long n) {
        vector<long long> w(n), v(n);
        for (long long i = 0; i < n; i++) {
            w[i] = i + 1;
            v[i] = (i < (long long)p.size()) ? p[i] : 0;
        }
        return knapsack_unbounded(w, v, n);
    }

    // --- D.6. Burst Balloons (заполнение снаружи внутрь) ---
    // f[i][j] — выручка за лопание шаров (i, j); k — ПОСЛЕДНИЙ лопнувший
    // шар отрезка: к его моменту соседи — фиксированные i и j, поэтому
    // f[i][j] = maxₖ (f[i][k] + f[k][j] + a[i]·a[k]·a[j]); фиктивные
    // единицы по краям (поглощение констант). O(n³) времени, O(n²) памяти.
    long long max_burst_balloons(const vector<long long>& a) {
        int n = (int)a.size();
        vector<long long> b(n + 2, 1);
        for (int i = 0; i < n; i++) b[i + 1] = a[i];
        vector<vector<long long>> f(n + 2, vector<long long>(n + 2, 0));
        for (int len = 2; len <= n + 1; len++)
            for (int i = 0; i + len <= n + 1; i++) {
                int j = i + len;
                f[i][j] = 0;
                for (int k = i + 1; k < j; k++)
                    f[i][j] = max(f[i][j], f[i][k] + f[k][j]
                                        + b[i] * b[k] * b[j]);
            }
        return f[0][n + 1];
    }

    // --- D.7. Scramble String ---
    // f[i][j][len] — можно ли получить b[j..j+len) из a[i..i+len);
    // без обмена: f[i][j][k] ∧ f[i+k][j+k][len−k]; с обменом:
    // f[i][j+len−k][k] ∧ f[i+k][j][len−k]; база f[i][j][1] = (a[i]=b[j]).
    // O(n⁴) времени, O(n³) памяти.
    bool is_scramble(const string& a, const string& b) {
        int n = (int)a.size();
        vector<vector<vector<bool>>> f(n, vector<vector<bool>>(
                n, vector<bool>(n + 1, false)));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                f[i][j][1] = (a[i] == b[j]);
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++)
                for (int j = 0; j + len <= n; j++)
                    for (int k = 1; k < len; k++) {
                        if ((f[i][j][k] && f[i + k][j + k][len - k]) ||
                            (f[i][j + len - k][k] && f[i + k][j][len - k])) {
                            f[i][j][len] = true;
                            break;
                        }
                    }
        return f[0][0][n];
    }

    // --- D.8. Partition Array for Maximum Sum ---
    // f[i] = max_{l=1..min(k,i)} (f[i − l] + l·max(a[i−l..i−1])) — последний
    // кусок длины l; максимум пересчитывается по мере роста l.
    // O(n·k) времени, O(n) памяти; монотонная очередь — раздел J (J.12).
    long long partition_array_max_sum(const vector<long long>& a, int k) {
        int n = (int)a.size();
        vector<long long> f(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            long long mx = 0;
            f[i] = -(LLONG_MAX / 4);
            for (int l = 1; l <= k && l <= i; l++) {
                mx = max(mx, a[i - l]);
                f[i] = max(f[i], f[i - l] + l * mx);
            }
        }
        return f[n];
    }

    // --- D.9. Minimum Cost to Merge Stones ---
    // f[i][j] — цена сведения a[i..j] к одной кучке (определена при
    // (j−i) % (K−1) == 0); последнее слияние K кучек — разбиение по
    // k = i, i+K−1, ...: f[i][j] = minₖ (f[i][k] + f[k+1][j]) + Σa[i..j]
    // (сумма добавляется ровно за последний ход). Если (n−1) % (K−1) ≠ 0,
    // сведение невозможно — −1. O(n³/K) времени, O(n²) памяти.
    long long merge_stones_min_cost(const vector<long long>& a, int K) {
        int n = (int)a.size();
        if ((n - 1) % (K - 1) != 0) return -1;
        vector<long long> pref(n + 1, 0);
        for (int i = 0; i < n; i++) pref[i + 1] = pref[i] + a[i];
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                f[i][j] = LLONG_MAX / 4;
                for (int k = i; k < j; k += K - 1)
                    f[i][j] = min(f[i][j], f[i][k] + f[k + 1][j]);
                if ((j - i) % (K - 1) == 0)
                    f[i][j] += pref[j + 1] - pref[i];
            }
        return f[0][n - 1];
    }

    // --- D.10. Minimum Score Triangulation ---
    // f[i][j] = minₖ (f[i][k] + f[k][j] + a[i]·a[k]·a[j]) по i < k < j —
    // треугольник (i, k, j) с фиксированным ребром (i, j) делит
    // многоугольник на два; f[i][i+1] = 0 (вырожденный отрезок);
    // обобщение Catalan-структур (C_iter — math/combinatorics).
    // O(n³) времени, O(n²) памяти.
    long long min_score_triangulation(const vector<long long>& a) {
        int n = (int)a.size();
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        for (int len = 2; len < n; len++)
            for (int i = 0; i + len < n; i++) {
                int j = i + len;
                f[i][j] = LLONG_MAX / 4;
                for (int k = i + 1; k < j; k++)
                    f[i][j] = min(f[i][j], f[i][k] + f[k][j]
                                        + a[i] * a[k] * a[j]);
            }
        return f[0][n - 1];
    }

    // --- D.11. Longest Palindromic Subsequence (LPS) ---
    // f[i][j] = f[i+1][j−1] + 2 при s[i] = s[j] (крайние входят), иначе
    // f[i][j] = max(f[i+1][j], f[i][j−1]) (крайний не используется) —
    // полное разбиение случая. O(n²) времени, O(n²) памяти.
    long long lps(const string& s) {
        int n = (int)s.size();
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        for (int i = 0; i < n; i++) f[i][i] = 1;
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                if (s[i] == s[j]) f[i][j] = f[i + 1][j - 1] + 2;
                else f[i][j] = max(f[i + 1][j], f[i][j - 1]);
            }
        return f[0][n - 1];
    }

    // --- D.11. Число палиндромных подпоследовательностей (по позициям) ---
    // при s[i] ≠ s[j]: f[i][j] = f[i+1][j] + f[i][j−1] − f[i+1][j−1]
    // (пересечение вычитается); при s[i] = s[j]: + 1 — пара (i, j) вокруг
    // каждой внутренней подпоследовательности. O(n²) времени, O(n²) памяти.
    long long count_palindromic_subsequences(const string& s) {
        int n = (int)s.size();
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        for (int i = 0; i < n; i++) f[i][i] = 1;
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                long long inner = (len > 2) ? f[i + 1][j - 1] : 0;
                if (s[i] == s[j]) f[i][j] = f[i + 1][j] + f[i][j - 1] + 1;
                else f[i][j] = f[i + 1][j] + f[i][j - 1] - inner;
            }
        return f[0][n - 1];
    }

    // --- D.12. Longest Palindromic Substring: расширение от центров ---
    // n нечётных + n−1 чётных центров; расширяем, пока символы равны.
    // O(n²) времени, O(1) памяти; Manacher — ниже (O(n)).
    string longest_palindromic_substring(const string& s) {
        int n = (int)s.size(), bestL = 0, bestLen = 1;
        for (int c = 0; c < n; c++) {
            for (int l = c, r = c; l >= 0 && r < n && s[l] == s[r]; l--, r++)
                if (r - l + 1 > bestLen) { bestLen = r - l + 1; bestL = l; }
            for (int l = c, r = c + 1; l >= 0 && r < n && s[l] == s[r]; l--, r++)
                if (r - l + 1 > bestLen) { bestLen = r - l + 1; bestL = l; }
        }
        return s.substr(bestL, bestLen);
    }

    // --- D.12. Manacher: радиусы палиндромов за O(n) ---
    // d1[i] — нечётные палиндромы с центром i, d2[i] — чётные (центр
    // между i−1 и i); зеркальный радиус по самому правому палиндрому
    // (l, r): k = min(d[l + r − i], r − i + 1) — правая граница r только
    // растёт, суммарное расширение O(n).
    pair<vector<int>, vector<int>> manacher(const string& s) {
        int n = (int)s.size();
        vector<int> d1(n, 0), d2(n, 0);
        for (int i = 0, l = 0, r = -1; i < n; i++) {
            int k = (i > r) ? 1 : min(d1[l + r - i], r - i + 1);
            while (i - k >= 0 && i + k < n && s[i - k] == s[i + k]) k++;
            d1[i] = k--;
            if (i + k > r) { l = i - k; r = i + k; }
        }
        for (int i = 0, l = 0, r = -1; i < n; i++) {
            int k = (i > r) ? 0 : min(d2[l + r - i + 1], r - i + 1);
            while (i - k - 1 >= 0 && i + k < n && s[i - k - 1] == s[i + k]) k++;
            d2[i] = k--;
            if (i + k > r) { l = i - k - 1; r = i + k; }
        }
        return {d1, d2};
    }

    // --- D.12. Longest Palindromic Substring: Manacher ---
    // Максимум по 2·d1[i]−1 и 2·d2[i]; восстановление подстроки по центру
    // и радиусу. O(n) времени, O(n) памяти.
    string longest_palindromic_substring_manacher(const string& s) {
        auto [d1, d2] = manacher(s);
        int n = (int)s.size(), bestL = 0, bestLen = 1;
        for (int i = 0; i < n; i++)
            if (2 * d1[i] - 1 > bestLen) {
                bestLen = 2 * d1[i] - 1;
                bestL = i - d1[i] + 1;
            }
        for (int i = 0; i < n; i++)
            if (2 * d2[i] > bestLen) { bestLen = 2 * d2[i]; bestL = i - d2[i]; }
        return s.substr(bestL, bestLen);
    }

    // --- D.13. Minimum Insertions to Make Palindrome ---
    // Сведение к LPS: уже палиндромные символы образуют LPS, остальные
    // дублируются вставкой — ответ = n − lps(s). O(n²) времени, O(n²) памяти.
    long long min_insertions_palindrome(const string& s) {
        return (long long)s.size() - lps(s);
    }

    // --- D.13. Путь к палиндрому: удаления и замены с весами ---
    // Обобщение Edit Distance (C.11) на цель-палиндром: удаляем крайние
    // символы (del) или сравниваем пару (i, j) с ценой замены rep:
    // f[i][j] = min(f[i+1][j] + del, f[i][j−1] + del, f[i+1][j−1] + (s[i]=s[j]
    // ? 0 : rep)). O(n²) времени, O(n²) памяти.
    long long min_palindrome_edit(const string& s, long long del, long long rep) {
        int n = (int)s.size();
        vector<vector<long long>> f(n, vector<long long>(n, 0));
        for (int len = 2; len <= n; len++)
            for (int i = 0; i + len <= n; i++) {
                int j = i + len - 1;
                long long best = min(f[i + 1][j] + del, f[i][j - 1] + del);
                long long keep = (len > 2 ? f[i + 1][j - 1] : 0)
                               + (s[i] == s[j] ? 0 : rep);
                f[i][j] = min(best, keep);
            }
        return f[0][n - 1];
    }

    // --- D.14. LIS: наивная O(n²) ---
    // f[i] = 1 + max{f[j] : j < i, a[j] < a[i]} — последний элемент
    // подпоследовательности a[i]. O(n²) времени, O(n) памяти.
    long long lis_naive(const vector<long long>& a) {
        int n = (int)a.size();
        vector<long long> f(n, 1);
        long long best = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++)
                if (a[j] < a[i]) f[i] = max(f[i], f[j] + 1);
            best = max(best, f[i]);
        }
        return best;
    }

    // --- D.14. LIS: пасьянс O(n·log n) ---
    // tails[l] — минимальный хвост подпоследовательности длины l+1
    // (tails монотонно возрастает); lower_bound — строгий вариант,
    // upper_bound — нестрогий (разрешаем равные элементы).
    long long lis_length(const vector<long long>& a, bool strict) {
        vector<long long> tails;
        for (long long x : a) {
            auto it = strict ? lower_bound(tails.begin(), tails.end(), x)
                             : upper_bound(tails.begin(), tails.end(), x);
            if (it == tails.end()) tails.push_back(x);
            else *it = x;
        }
        return (long long)tails.size();
    }

    // --- D.14. LIS: восстановление через prev (cp-алгоритм) ---
    // prev[i] — индекс предшественника в момент вставки a[i] в пасьянс;
    // цепочка от последнего хвоста восстанавливает подпоследовательность
    // (значения вдоль prev строго возрастают). O(n·log n) времени.
    vector<long long> lis_with_path(const vector<long long>& a) {
        vector<int> tails;
        vector<int> prev(a.size(), -1);
        for (int i = 0; i < (int)a.size(); i++) {
            auto it = lower_bound(tails.begin(), tails.end(), i,
                                  [&](int idx, int val) { return a[idx] < a[val]; });
            int pos = (int)(it - tails.begin());
            if (pos > 0) prev[i] = tails[pos - 1];
            if (it == tails.end()) tails.push_back(i);
            else *it = i;
        }
        vector<long long> res;
        for (int i = tails.back(); i != -1; i = prev[i]) res.push_back(a[i]);
        reverse(res.begin(), res.end());
        return res;
    }

    // --- D.14. Число LIS ---
    // cnt[i] — число LIS, оканчивающихся a[i]: cnt[i] = Σ cnt[j] для
    // j < i с a[j] < a[i] и f[j] + 1 = f[i]; ответ — Σ cnt[i] для
    // максимального f. O(n²) времени, O(n) памяти.
    long long lis_count(const vector<long long>& a) {
        int n = (int)a.size();
        vector<long long> f(n, 1), cnt(n, 1);
        long long best = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++)
                if (a[j] < a[i] && f[j] + 1 > f[i]) {
                    f[i] = f[j] + 1;
                    cnt[i] = cnt[j];
                } else if (a[j] < a[i] && f[j] + 1 == f[i]) {
                    cnt[i] += cnt[j];
                }
            best = max(best, f[i]);
        }
        long long res = 0;
        for (int i = 0; i < n; i++)
            if (f[i] == best) res += cnt[i];
        return res;
    }

    // --- D.15. Max Product Subarray: пара минимум/максимум ---
    // Отрицательные a меняют местами роль минимума и максимума (знак
    // произведения меняется): mx = max(a, mx·a, mn·a), mn = min(...) —
    // расширение состояния a.md A.3 (Кадане — b.md B.4). O(n) времени.
    long long max_product_subarray(const vector<long long>& a) {
        long long mx = a[0], mn = a[0], best = a[0];
        for (int i = 1; i < (int)a.size(); i++) {
            long long pmx = mx, pmn = mn;
            mx = max({a[i], pmx * a[i], pmn * a[i]});
            mn = min({a[i], pmx * a[i], pmn * a[i]});
            best = max(best, mx);
        }
        return best;
    }

    // --- D.16. Longest Repeating Subsequence ---
    // LCS строки с самой собой (C.12) с запретом одной и той же позиции:
    // переход по диагонали только при i ≠ j. O(n²) времени, O(n) памяти.
    long long longest_repeating_subsequence(const string& s) {
        int n = (int)s.size();
        vector<long long> dp(n + 1, 0), ndp(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (s[i - 1] == s[j - 1] && i != j) ndp[j] = dp[j - 1] + 1;
                else ndp[j] = max(dp[j], ndp[j - 1]);
            }
            swap(dp, ndp);
        }
        return dp[n];
    }

    // --- D.17. Count Palindromic Substrings ---
    // Расширение от центров с подсчётом: каждый шаг расширения — новая
    // палиндромная подстрока; всего Σ(d1[i] + d2[i]) в терминах D.12.
    // O(n²) времени, O(1) памяти; Manacher даёт O(n).
    long long count_palindromic_substrings(const string& s) {
        int n = (int)s.size();
        long long cnt = 0;
        for (int c = 0; c < n; c++) {
            for (int l = c, r = c; l >= 0 && r < n && s[l] == s[r]; l--, r++) cnt++;
            for (int l = c, r = c + 1; l >= 0 && r < n && s[l] == s[r]; l--, r++) cnt++;
        }
        return cnt;
    }

    // --- D.18. Count Distinct Subsequences ---
    // f[i] = 2·f[i−1] − f[last[c]−1] — все подпоследовательности префикса
    // удваиваются добавлением c в конец; дубли — подпоследовательности,
    // уже оканчивающиеся c (от последнего вхождения); last — позиция
    // последнего вхождения (алфавит произвольного размера — unordered_map).
    // O(n) времени, O(n) памяти.
    long long count_distinct_subsequences(const string& s) {
        unordered_map<char, int> last;
        vector<long long> f(s.size() + 1, 0);
        f[0] = 1;
        for (int i = 1; i <= (int)s.size(); i++) {
            char c = s[i - 1];
            f[i] = 2 * f[i - 1] - (last.count(c) ? f[last[c] - 1] : 0);
            last[c] = i;
        }
        return f[s.size()];
    }

    // --- D.19. Подотрезок с суммой ≤ K (формализация 1D-ядра C.6) ---
    // Реализация — max_subarray_sum_no_more_than_k в c.cpp (C.6):
    // префиксные суммы + упорядоченное множество (lower_bound) за
    // O(m·log m); здесь только переиспользование, без дублирования.

    // --- D.20. Longest Bitonic Subsequence ---
    // d1[i] — LIS, оканчивающийся в i; d2[i] — LDS, начинающийся в i
    // (обратный проход); вершина — общий элемент: ответ = maxᵢ (d1[i] +
    // d2[i] − 1). O(n²) времени, O(n) памяти; O(n log n) — пасьянс (D.14).
    long long longest_bitonic_subsequence(const vector<long long>& a) {
        int n = (int)a.size();
        vector<long long> d1(n, 1), d2(n, 1);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < i; j++)
                if (a[j] < a[i]) d1[i] = max(d1[i], d1[j] + 1);
        for (int i = n - 1; i >= 0; i--)
            for (int j = n - 1; j > i; j--)
                if (a[j] < a[i]) d2[i] = max(d2[i], d2[j] + 1);
        long long best = 0;
        for (int i = 0; i < n; i++) best = max(best, d1[i] + d2[i] - 1);
        return best;
    }

    // --- D.21. Maximum Sum Increasing Subsequence ---
    // Та же структура, что D.14, с суммой вместо длины:
    // f[i] = a[i] + max{f[j] : j < i, a[j] < a[i]}. O(n²) времени,
    // O(n) памяти; дерево отрезков по значениям — раздел J.
    long long max_sum_increasing_subsequence(const vector<long long>& a) {
        int n = (int)a.size();
        vector<long long> f(n, 0);
        long long best = 0;
        for (int i = 0; i < n; i++) {
            f[i] = a[i];
            for (int j = 0; j < i; j++)
                if (a[j] < a[i]) f[i] = max(f[i], f[j] + a[i]);
            best = max(best, f[i]);
        }
        return best;
    }

    // --- D.22. Count Subsequences with Sum S (по позиции и сумме) ---
    // f[i][s] = f[i−1][s] + f[i−1][s − aᵢ] — последний элемент i не входит
    // или входит; позиция в состоянии исключает перестановки и дубли.
    // Одномерная 0-1-свёртка той же рекуррентности — subset_sum_count
    // (C.23, сверка в демо). O(n·S) времени, O(n·S) памяти.
    long long count_subsequences_sum(const vector<long long>& a, long long S) {
        int n = (int)a.size();
        vector<vector<long long>> f(n + 1, vector<long long>(S + 1, 0));
        f[0][0] = 1;
        for (int i = 1; i <= n; i++)
            for (long long s = 0; s <= S; s++) {
                f[i][s] = f[i - 1][s];
                if (s >= a[i - 1]) f[i][s] += f[i - 1][s - a[i - 1]];
            }
        return f[n][S];
    }
};

#ifndef SEGMENTDP_MAIN
signed main() {
    SegmentDP dp;

    cout << "D.1: matrix_chain_min({40,20,30,10,30}) = "
         << dp.matrix_chain_min({40, 20, 30, 10, 30}) << " (ожидаем 26000)\n";
    cout << "D.1: matrix_chain_min({10,20,30,40}) = "
         << dp.matrix_chain_min({10, 20, 30, 40}) << " (ожидаем 18000)\n";

    cout << "D.2: matrix_chain_order({40,20,30,10,30}) = "
         << dp.matrix_chain_order({40, 20, 30, 10, 30})
         << " (ожидаем ((A1(A2A3))A4))\n";
    cout << "D.2: matrix_chain_general(произведение, сверка с D.1) = "
         << dp.matrix_chain_general({40, 20, 30, 10, 30},
             [](long long x, long long y, long long z) { return x * y * z; })
         << " (ожидаем 26000)\n";

    cout << "D.3: min_palindrome_partitions(aab) = "
         << dp.min_palindrome_partitions("aab") << " (ожидаем 2)\n";
    cout << "D.3: min_palindrome_partitions(abba) = "
         << dp.min_palindrome_partitions("abba") << " (ожидаем 1)\n";

    cout << "D.4: optimal_bst_cost({4,2,6,3}) = "
         << dp.optimal_bst_cost({4, 2, 6, 3}) << " (ожидаем 26)\n";

    cout << "D.5: rod_cutting(цены {1..20}, 8) = "
         << dp.rod_cutting({1, 5, 8, 9, 10, 17, 17, 20}, 8) << " (ожидаем 22)\n";
    cout << "D.5: сверка — knapsack_unbounded(W=8) = "
         << dp.knapsack_unbounded({1, 2, 3, 4, 5, 6, 7, 8},
                                  {1, 5, 8, 9, 10, 17, 17, 20}, 8)
         << " (ожидаем 22, C.20)\n";

    cout << "D.6: max_burst_balloons({3,1,5,8}) = "
         << dp.max_burst_balloons({3, 1, 5, 8}) << " (ожидаем 167)\n";

    cout << "D.7: is_scramble(great, rgeat) = "
         << dp.is_scramble("great", "rgeat") << " (ожидаем 1)\n";
    cout << "D.7: is_scramble(abcde, caebd) = "
         << dp.is_scramble("abcde", "caebd") << " (ожидаем 0)\n";

    cout << "D.8: partition_array_max_sum({1,15,7,9,2,5,10}, k=3) = "
         << dp.partition_array_max_sum({1, 15, 7, 9, 2, 5, 10}, 3)
         << " (ожидаем 84)\n";

    cout << "D.9: merge_stones_min_cost({3,2,4,1}, K=2) = "
         << dp.merge_stones_min_cost({3, 2, 4, 1}, 2) << " (ожидаем 20)\n";
    cout << "D.9: merge_stones_min_cost({3,2,4,1}, K=3) = "
         << dp.merge_stones_min_cost({3, 2, 4, 1}, 3) << " (ожидаем -1)\n";

    cout << "D.10: min_score_triangulation({1,2,3}) = "
         << dp.min_score_triangulation({1, 2, 3}) << " (ожидаем 6)\n";
    cout << "D.10: min_score_triangulation({3,7,4,5}) = "
         << dp.min_score_triangulation({3, 7, 4, 5}) << " (ожидаем 144)\n";

    cout << "D.11: lps(bbbab) = " << dp.lps("bbbab") << " (ожидаем 4)\n";
    cout << "D.11: lps(cbbd) = " << dp.lps("cbbd") << " (ожидаем 2)\n";
    cout << "D.11: count_palindromic_subsequences(abab) = "
         << dp.count_palindromic_subsequences("abab") << " (ожидаем 8)\n";

    cout << "D.12: longest_palindromic_substring(babad) = "
         << dp.longest_palindromic_substring("babad") << " (ожидаем bab)\n";
    auto mc = dp.manacher("babad");
    cout << "D.12: manacher(babad): d1 =";
    for (int x : mc.first) cout << " " << x;
    cout << " (ожидаем 1 2 2 1 1), d2 =";
    for (int x : mc.second) cout << " " << x;
    cout << " (ожидаем 0 0 0 0 0)\n";
    cout << "D.12: longest_palindromic_substring_manacher(cbbd) = "
         << dp.longest_palindromic_substring_manacher("cbbd")
         << " (ожидаем bb)\n";

    cout << "D.13: min_insertions_palindrome(mbadm) = "
         << dp.min_insertions_palindrome("mbadm") << " (ожидаем 2)\n";
    cout << "D.13: min_palindrome_edit(abc, del=1, rep=2) = "
         << dp.min_palindrome_edit("abc", 1, 2) << " (ожидаем 2)\n";

    cout << "D.14: lis_naive({10,9,2,5,3,7,101,18}) = "
         << dp.lis_naive({10, 9, 2, 5, 3, 7, 101, 18}) << " (ожидаем 4)\n";
    cout << "D.14: lis_length({2,2,2,2}, строго) = "
         << dp.lis_length({2, 2, 2, 2}, true) << " (ожидаем 1)\n";
    cout << "D.14: lis_length({2,2,2,2}, нестрого) = "
         << dp.lis_length({2, 2, 2, 2}, false) << " (ожидаем 4)\n";
    auto lisP = dp.lis_with_path({10, 9, 2, 5, 3, 7, 101, 18});
    cout << "D.14: lis_with_path =";
    for (long long x : lisP) cout << " " << x;
    cout << " (ожидаем 2 3 7 18)\n";
    cout << "D.14: lis_count({1,3,5,4,7}) = "
         << dp.lis_count({1, 3, 5, 4, 7}) << " (ожидаем 2)\n";

    cout << "D.15: max_product_subarray({2,3,-2,4}) = "
         << dp.max_product_subarray({2, 3, -2, 4}) << " (ожидаем 6)\n";
    cout << "D.15: max_product_subarray({-2,0,-1}) = "
         << dp.max_product_subarray({-2, 0, -1}) << " (ожидаем 0)\n";

    cout << "D.16: longest_repeating_subsequence(aabebcdd) = "
         << dp.longest_repeating_subsequence("aabebcdd") << " (ожидаем 3)\n";
    cout << "D.16: longest_repeating_subsequence(abc) = "
         << dp.longest_repeating_subsequence("abc") << " (ожидаем 0)\n";

    cout << "D.17: count_palindromic_substrings(aaa) = "
         << dp.count_palindromic_substrings("aaa") << " (ожидаем 6)\n";
    cout << "D.17: count_palindromic_substrings(abc) = "
         << dp.count_palindromic_substrings("abc") << " (ожидаем 3)\n";

    cout << "D.18: count_distinct_subsequences(abc) = "
         << dp.count_distinct_subsequences("abc") << " (ожидаем 8)\n";
    cout << "D.18: count_distinct_subsequences(aaa) = "
         << dp.count_distinct_subsequences("aaa") << " (ожидаем 4)\n";
    cout << "D.18: count_distinct_subsequences(aba) = "
         << dp.count_distinct_subsequences("aba") << " (ожидаем 7)\n";

    cout << "D.19: max_subarray_sum_no_more_than_k({1,2,3,-4,5}, 5) = "
         << dp.max_subarray_sum_no_more_than_k({1, 2, 3, -4, 5}, 5)
         << " (ожидаем 5, 1D-ядро C.6)\n";
    cout << "D.19: max_subarray_sum_no_more_than_k({1,0,1}, 2) = "
         << dp.max_subarray_sum_no_more_than_k({1, 0, 1}, 2)
         << " (ожидаем 2)\n";

    cout << "D.20: longest_bitonic_subsequence({1,11,2,10,4,5,2,1}) = "
         << dp.longest_bitonic_subsequence({1, 11, 2, 10, 4, 5, 2, 1})
         << " (ожидаем 6)\n";

    cout << "D.21: max_sum_increasing_subsequence({1,101,2,3,100,4,5}) = "
         << dp.max_sum_increasing_subsequence({1, 101, 2, 3, 100, 4, 5})
         << " (ожидаем 106)\n";

    cout << "D.22: count_subsequences_sum({1,2,3,4}, 5) = "
         << dp.count_subsequences_sum({1, 2, 3, 4}, 5) << " (ожидаем 2)\n";
    cout << "D.22: сверка — subset_sum_count({1,2,3,4}, 5) = "
         << dp.subset_sum_count({1, 2, 3, 4}, 5) << " (ожидаем 2, C.23)\n";
}
#endif // SEGMENTDP_MAIN
#endif // DYNAMIC_D_CPP
