#ifndef DYNAMIC_F_CPP
#define DYNAMIC_F_CPP

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
using namespace std;

// =============================================================
// F. ДИНАМИКА ПО ПРОФИЛЮ
// =============================================================
// Структура md: A. Основные понятия (F.1–F.3)
//               → B. Задачи на замощение (F.4–F.11)
//               → C. Другие задачи (F.12–F.14)
//
// ProfileDP наследует SubsetDP (e.cpp), а через него SegmentDP,
// MatrixDP, LinearDP, DPBasics. Переиспользуются, а не дублируются:
// битовые операции popcount, test_bit (e.md E.1 — F.13), числа
// Фибоначчи как каркас замощений 2×N (b.md B.9 — F.4).
//
// Порядок методов строго соответствует порядку md (F.2 → F.14);
// F.3 (Plug DP «клетка за клеткой») и F.11 (мосты) отдельного метода
// не требуют — их реализуют count_hamiltonian_cycles (F.13) и
// count_domino_tilings (F.5) соответственно.
// Ограничения: m ≤ 20–25 (память 2ᵐ); перебор замощений (F.9) и
// Plug DP с метками (F.13) — только малые доски.

#define SUBSETDP_MAIN
#include "../e/e.cpp"

struct ProfileDP : SubsetDP {

    // --- F.2. Переходы между профилями ---
    // trans[prev] — список масок next, совместимых с prev: DFS по
    // клеткам строки (F.2): занятая клетка (бит prev) — пропуск;
    // свободная — горизонтальное домино (c, c+1) или вертикальное
    // (бит c в next — вертикаль, «входящая» в следующую строку).
    // O(3ᵐ) времени, O(2ᵐ) памяти.
    vector<vector<long long>> row_transitions(int m) {
        vector<vector<long long>> trans(1LL << m);
        for (long long prev = 0; prev < (1LL << m); prev++) {
            function<void(int, long long)> dfs = [&](int c, long long next) {
                if (c == m) {
                    trans[prev].push_back(next);
                    return;
                }
                if ((prev >> c) & 1) {
                    dfs(c + 1, next);
                    return;
                }
                if (c + 1 < m && !((prev >> (c + 1)) & 1))
                    dfs(c + 2, next);
                dfs(c + 1, next | (1LL << c));
            };
            dfs(0, 0);
        }
        return trans;
    }

    // --- F.4. Домино 2×N ---
    // F(n) = F(n−1) + F(n−2), F(0) = F(1) = 1 — последний столбец:
    // вертикальное домино (остаток 2×(n−1)) или два горизонтальных
    // (остаток 2×(n−2)) — числа Фибоначчи (b.md B.9). O(n) времени,
    // O(1) памяти.
    long long domino_tiling_2n(int n) {
        long long a = 1, b = 1;
        for (int i = 2; i <= n; i++) {
            long long c = a + b;
            a = b;
            b = c;
        }
        return b;
    }

    // --- F.5. Домино M×N ---
    // dp по строкам: dp[next] += dp[prev] для next ∈ trans[prev]
    // (F.2); после n строк ответ dp[0] — ничего не выступает за
    // доску (вертикали последней строки дают next ≠ 0 и не входят
    // в ответ). O(n·2ᵐ·|trans|) времени, O(2ᵐ) памяти.
    long long count_domino_tilings(int m, int n) {
        auto trans = row_transitions(m);
        vector<long long> dp(1LL << m, 0), ndp;
        dp[0] = 1;
        for (int i = 0; i < n; i++) {
            ndp.assign(1LL << m, 0);
            for (long long prev = 0; prev < (1LL << m); prev++)
                if (dp[prev])
                    for (long long next : trans[prev])
                        ndp[next] += dp[prev];
            dp.swap(ndp);
        }
        return dp[0];
    }

    // --- F.6. Замощение с дефектами (дырками) ---
    // Тот же DFS, но клетка-дырка считается занятой: вертикальное
    // домино не опирается на дырку (проверка blocked[i+1]), горизон-
    // тальное не покрывает дырку (проверка blocked[i]).
    // O(n·2ᵐ·|trans|) времени, O(2ᵐ) памяти.
    long long count_domino_tilings_blocked(int m, int n,
                                           const vector<long long>& blocked) {
        vector<long long> dp(1LL << m, 0), ndp;
        dp[0] = 1;
        for (int i = 0; i < n; i++) {
            ndp.assign(1LL << m, 0);
            long long cur = blocked[i], nxt = (i + 1 < n) ? blocked[i + 1] : 0;
            for (long long prev = 0; prev < (1LL << m); prev++) {
                if (!dp[prev]) continue;
                function<void(int, long long)> dfs = [&](int c, long long next) {
                    if (c == m) {
                        ndp[next] += dp[prev];
                        return;
                    }
                    if (((prev >> c) & 1) || ((cur >> c) & 1)) {
                        dfs(c + 1, next);
                        return;
                    }
                    if (c + 1 < m && !((prev >> (c + 1)) & 1)
                            && !((cur >> (c + 1)) & 1))
                        dfs(c + 2, next);
                    if (!((nxt >> c) & 1))
                        dfs(c + 1, next | (1LL << c));
                };
                dfs(0, 0);
            }
            dp.swap(ndp);
        }
        return dp[0];
    }

    // --- F.7. Тримино (L-образные): счёт ---
    // 4 ориентации L в окне 2×2 (текущая + следующая строки), каждая
    // привязана к своей «верхней» клетке: {c, c+1}×{next c},
    // {c, c+1}×{next c+1}, {c}×{next c, next c+1} — с якорем c,
    // {c}×{next c−1, next c} — с якорем c (фигура задевает ровно
    // одну клетку текущей строки). O(n·2ᵐ·4) времени.
    long long count_tromino_tilings(int m, int n) {
        vector<long long> dp(1LL << m, 0), ndp;
        dp[0] = 1;
        for (int i = 0; i < n; i++) {
            ndp.assign(1LL << m, 0);
            for (long long prev = 0; prev < (1LL << m); prev++) {
                if (!dp[prev]) continue;
                function<void(int, long long)> dfs = [&](int c, long long next) {
                    if (c == m) {
                        ndp[next] += dp[prev];
                        return;
                    }
                    if ((prev >> c) & 1) {
                        dfs(c + 1, next);
                        return;
                    }
                    if (c + 1 < m) {
                        // {c, c+1} × {next c}
                        if (!((prev >> (c + 1)) & 1) && !((next >> c) & 1))
                            dfs(c + 2, next | (1LL << c));
                        // {c, c+1} × {next c+1}
                        if (!((prev >> (c + 1)) & 1) && !((next >> (c + 1)) & 1))
                            dfs(c + 2, next | (1LL << (c + 1)));
                        // {c} × {next c, next c+1}
                        if (!((next >> c) & 1) && !((next >> (c + 1)) & 1))
                            dfs(c + 1, next | (1LL << c) | (1LL << (c + 1)));
                    }
                    // {c} × {next c−1, next c}
                    if (c >= 1 && !((next >> (c - 1)) & 1) && !((next >> c) & 1))
                        dfs(c + 1, next | (1LL << (c - 1)) | (1LL << c));
                };
                dfs(0, 0);
            }
            dp.swap(ndp);
        }
        return dp[0];
    }

    // --- F.7. «Игра с дыркой»: замощение 2^k×2^k ---
    // Рекурсия по квадрантам: центральное тримино покрывает 3 цент-
    // ральные клетки квадрантов без дырки, каждый квадрант рекурсивно
    // замощается со своей дыркой. Число тримино (2^(2k) − 1)/3 от
    // положения дырки не зависит (индукция: T(k) = 1 + 4·T(k−1)).
    // O(4ᵏ) времени (по числу тримино), O(k) памяти.
    long long tromino_covering(int k, int holeR, int holeC) {
        if (k == 0) return 0;
        return 1 + 4 * tromino_covering(k - 1, holeR, holeC);
    }

    // --- F.8. Полимино: замощение фиксированными фигурами ---
    // Фигура = список (dr, dc), dr ∈ {0, 1} (задевает текущую и
    // следующую строки), нормируется к якорю (0, 0); при клетке c
    // проверяем, что все клетки фигуры свободны (текущая строка —
    // биты prev/covered, следующая — биты next), и размещаем.
    // Симметрии: 8 преобразований (dr, dc) → (±dr, ±dc) и перестановка
    // осей с нормировкой и дедупликацией (F.8). O(n·2ᵐ·|фигуры|).
    long long count_shaped_tilings(int m, int n,
                                   const vector<vector<pair<int, int>>>& shapes,
                                   bool useSymmetries) {
        vector<vector<pair<int, int>>> ss;
        // Нормировка к якорю: клетка (0, 0) текущей строки — минималь-
        // ный dc среди клеток строки dr = 0; клетки следующей строки
        // могут оставаться левее якоря (dc < 0) — проверка cc ≥ 0.
        auto norm_shape = [](vector<pair<int, int>> s) {
            int mindr = s[0].first;
            for (auto& p : s) mindr = min(mindr, p.first);
            for (auto& p : s) p.first -= mindr;
            int mindc0 = INT_MAX;
            for (auto& p : s)
                if (p.first == 0) mindc0 = min(mindc0, p.second);
            for (auto& p : s) p.second -= mindc0;
            sort(s.begin(), s.end());
            return s;
        };
        for (auto& sh : shapes) {
            vector<pair<int, int>> s = norm_shape(sh);
            if (!useSymmetries) {
                ss.push_back(s);
                continue;
            }
            set<vector<pair<int, int>>> seen;
            for (int t = 0; t < 8; t++) {
                vector<pair<int, int>> r;
                for (auto [dr, dc] : s) {
                    int a, b;
                    switch (t) {
                        case 0: a = dr; b = dc; break;
                        case 1: a = -dr; b = dc; break;
                        case 2: a = dr; b = -dc; break;
                        case 3: a = -dr; b = -dc; break;
                        case 4: a = dc; b = dr; break;
                        case 5: a = -dc; b = dr; break;
                        case 6: a = dc; b = -dr; break;
                        default: a = -dc; b = -dr; break;
                    }
                    r.push_back({a, b});
                }
                seen.insert(norm_shape(r));
            }
            for (auto& v : seen) ss.push_back(v);
        }
        vector<long long> dp(1LL << m, 0), ndp;
        dp[0] = 1;
        for (int i = 0; i < n; i++) {
            ndp.assign(1LL << m, 0);
            for (long long prev = 0; prev < (1LL << m); prev++) {
                if (!dp[prev]) continue;
                function<void(int, long long, long long)> dfs =
                    [&](int c, long long covered, long long next) {
                    if (c == m) {
                        ndp[next] += dp[prev];
                        return;
                    }
                    if (((prev >> c) & 1) || ((covered >> c) & 1)) {
                        dfs(c + 1, covered, next);
                        return;
                    }
                    for (auto& sh : ss) {
                        bool ok = true;
                        long long cov2 = covered, nxt2 = next;
                        for (auto [dr, dc] : sh) {
                            int cc = c + dc;
                            if (cc < 0 || cc >= m) {
                                ok = false;
                                break;
                            }
                            if (dr == 0) {
                                if (((prev >> cc) & 1) || ((covered >> cc) & 1)) {
                                    ok = false;
                                    break;
                                }
                                cov2 |= 1LL << cc;
                            } else {
                                if ((next >> cc) & 1) {
                                    ok = false;
                                    break;
                                }
                                nxt2 |= 1LL << cc;
                            }
                        }
                        if (ok) dfs(c + 1, cov2, nxt2);
                    }
                };
                dfs(0, 0, 0);
            }
            dp.swap(ndp);
        }
        return dp[0];
    }

    // --- F.9. Замощение с учётом симметрий: число классов ---
    // Перечисляем все замощения (малые доски) и канонизируем каждое:
    // минимальная форма среди 8 преобразований (квадрат) или 4
    // (прямоугольник m ≠ n); число классов = число канонических форм.
    // О(число замощений · 8 · m·n) времени.
    long long count_domino_tiling_classes(int m, int n) {
        set<string> canon;
        vector<int> cell(m * n, -1);
        function<void()> rec = [&]() {
            int first = -1;
            for (int i = 0; i < m * n; i++)
                if (cell[i] < 0) {
                    first = i;
                    break;
                }
            if (first < 0) {
                canon.insert(canon_form(m, n, cell));
                return;
            }
            int r = first / m, c = first % m;
            if (c + 1 < m && cell[first + 1] < 0) {
                cell[first] = first + 1;
                cell[first + 1] = first;
                rec();
                cell[first] = cell[first + 1] = -1;
            }
            if (r + 1 < n && cell[first + m] < 0) {
                cell[first] = first + m;
                cell[first + m] = first;
                rec();
                cell[first] = cell[first + m] = -1;
            }
        };
        rec();
        return (long long)canon.size();
    }

    // --- F.10. Подсчёт числа способов покрытия в модуле ---
    // Та же рекуррентность, что F.5, с редукцией по модулю при каждом
    // сложении (модулярная арифметика коммутирует со сложением).
    // O(n·2ᵐ·|trans|) времени, O(2ᵐ) памяти.
    long long count_domino_tilings_mod(int m, int n, long long mod) {
        auto trans = row_transitions(m);
        vector<long long> dp(1LL << m, 0), ndp;
        dp[0] = 1 % mod;
        for (int i = 0; i < n; i++) {
            ndp.assign(1LL << m, 0);
            for (long long prev = 0; prev < (1LL << m); prev++)
                if (dp[prev])
                    for (long long next : trans[prev])
                        ndp[next] = (ndp[next] + dp[prev]) % mod;
            dp.swap(ndp);
        }
        return dp[0];
    }

    // --- F.12. Minimum Squares To Represent A Number ---
    // dp[i] = 1 + min(dp[i − k²]) по k² ≤ i — последний квадрат;
    // каркас «1 + min по предыдущим» — b.md (B.1, B.6), d.md (D.3).
    // O(n·√n) времени, O(n) памяти.
    long long min_squares(int n) {
        vector<long long> dp(n + 1, LLONG_MAX / 4);
        dp[0] = 0;
        for (int i = 1; i <= n; i++)
            for (int k = 1; k * k <= i; k++)
                dp[i] = min(dp[i], 1 + dp[i - k * k]);
        return dp[n];
    }

    // --- F.13. Connected-компонентный профиль: гамильтоновы циклы ---
    // Plug DP «клетка за клеткой» (F.3): состояние — метки рёбер
    // линии разлома (m+1 слот, 0 — нет ребра); клетка (r, c):
    // L = слот c (левое ребро), U = слот c+1 (верхнее); случаи
    // (0,0) — новая компонента (вниз и вправо), (L,0)/(0,U) — про-
    // должение ребра, (L ≠ U) — слияние с переметкой и нормировкой,
    // (L = U) — замыкание только в последней клетке при пустом
    // остальном профиле. O(n·m·|состояния|) времени.
    long long count_hamiltonian_cycles(int m, int n) {
        auto norm = [](vector<int> st) {
            vector<int> mp(st.size() + 2, 0);
            int nxt = 1;
            for (int j = 0; j < (int)st.size(); j++)
                if (st[j]) {
                    if (!mp[st[j]]) mp[st[j]] = nxt++;
                    st[j] = mp[st[j]];
                }
            return st;
        };
        map<vector<int>, long long> dp, ndp;
        dp[vector<int>(m + 1, 0)] = 1;
        long long answer = 0;
        for (int r = 0; r < n; r++) {
            // начало строки: линия разлома поворачивает, метки сдвигаются
            // на один слот вправо (левый слот — граница, всегда 0)
            map<vector<int>, long long> shifted;
            for (auto& [state, cnt] : dp) {
                vector<int> ns(m + 1, 0);
                for (int j = 0; j < m; j++) ns[j + 1] = state[j];
                shifted[ns] += cnt;
            }
            dp = shifted;
            for (int c = 0; c < m; c++) {
                ndp.clear();
                for (auto& [state, cnt] : dp) {
                    int L = state[c], U = state[c + 1];
                    if (L == 0 && U == 0) {
                        // новая компонента: рёбра вниз и вправо
                        if (c + 1 < m && r + 1 < n) {
                            vector<int> ns = state;
                            int lab = 0;
                            for (int x : ns) lab = max(lab, x);
                            ns[c] = ns[c + 1] = lab + 1;
                            ndp[norm(ns)] += cnt;
                        }
                    } else if (L != 0 && U == 0) {
                        if (c + 1 < m) {
                            vector<int> ns = state;
                            ns[c] = 0;
                            ns[c + 1] = L;
                            ndp[norm(ns)] += cnt;
                        }
                        if (r + 1 < n) {
                            vector<int> ns = state;
                            ns[c] = L;
                            ns[c + 1] = 0;
                            ndp[norm(ns)] += cnt;
                        }
                    } else if (L == 0 && U != 0) {
                        if (c + 1 < m) {
                            vector<int> ns = state;
                            ns[c] = 0;
                            ns[c + 1] = U;
                            ndp[norm(ns)] += cnt;
                        }
                        if (r + 1 < n) {
                            vector<int> ns = state;
                            ns[c] = U;
                            ns[c + 1] = 0;
                            ndp[norm(ns)] += cnt;
                        }
                    } else {
                        if (L != U) {
                            // слияние компонент: переметить все L в U
                            vector<int> ns = state;
                            for (int j = 0; j <= m; j++)
                                if (ns[j] == L) ns[j] = U;
                            ns[c] = ns[c + 1] = 0;
                            ndp[norm(ns)] += cnt;
                        } else {
                            // замыкание цикла — допустимо только в последней
                            // клетке при пустом остальном профиле
                            bool rest = true;
                            for (int j = 0; j <= m; j++)
                                if (j != c && j != c + 1 && state[j] != 0)
                                    rest = false;
                            if (r == n - 1 && c == m - 1 && rest)
                                answer += cnt;
                        }
                    }
                }
                dp.swap(ndp);
            }
        }
        return answer;
    }

    // --- F.14. Замощение с цветами: без одноцветного 2×2 ---
    // dp по строкам: состояние — цветовая маска строки (m бит);
    // переход prev → next допустим, если для каждого столбца c блок
    // 2×2 (prev c, prev c+1, next c, next c+1) не одноцветен.
    // O(n·2ᵐ·2ᵐ·m) времени, O(2ᵐ) памяти.
    long long count_colorings_no_monochrome_2x2(int m, int n) {
        vector<long long> dp(1LL << m, 1), ndp;
        for (int r = 1; r < n; r++) {
            ndp.assign(1LL << m, 0);
            for (long long prev = 0; prev < (1LL << m); prev++)
                for (long long next = 0; next < (1LL << m); next++) {
                    bool ok = true;
                    for (int c = 0; c + 1 < m && ok; c++) {
                        int b = ((prev >> c) & 1) | (((prev >> (c + 1)) & 1) << 1)
                              | (((next >> c) & 1) << 2)
                              | (((next >> (c + 1)) & 1) << 3);
                        if (b == 0 || b == 15) ok = false;
                    }
                    if (ok) ndp[next] += dp[prev];
                }
            dp.swap(ndp);
        }
        long long total = 0;
        for (long long x : dp) total += x;
        return total;
    }

private:
    // Каноническая форма замощения: минимальный вид среди преобразо-
    // ваний доски — 8 для квадрата (повороты + отражения), 4 для
    // прямоугольника m ≠ n (ид, отражения по осям, поворот 180°).
    string canon_form(int m, int n, const vector<int>& cell) {
        vector<function<pair<int, int>(int, int)>> trs;
        trs.push_back([&](int r, int c) { return make_pair(r, c); });
        trs.push_back([&](int r, int c) { return make_pair(r, m - 1 - c); });
        trs.push_back([&](int r, int c) { return make_pair(n - 1 - r, c); });
        trs.push_back([&](int r, int c) { return make_pair(n - 1 - r, m - 1 - c); });
        if (m == n) {
            trs.push_back([&](int r, int c) { return make_pair(c, m - 1 - r); });
            trs.push_back([&](int r, int c) { return make_pair(m - 1 - c, r); });
            trs.push_back([&](int r, int c) { return make_pair(c, r); });
            trs.push_back([&](int r, int c) { return make_pair(m - 1 - c, n - 1 - r); });
        }
        string best;
        for (auto& T : trs) {
            vector<pair<int, int>> pairs;
            for (int i = 0; i < m * n; i++) {
                int j = cell[i];
                if (j > i) {
                    auto pi = T(i / m, i % m), pj = T(j / m, j % m);
                    int ai = pi.first * m + pi.second;
                    int aj = pj.first * m + pj.second;
                    pairs.push_back({min(ai, aj), max(ai, aj)});
                }
            }
            sort(pairs.begin(), pairs.end());
            string s;
            for (auto& p : pairs)
                s += to_string(p.first) + "," + to_string(p.second) + ";";
            if (best.empty() || s < best) best = s;
        }
        return best;
    }
};

#ifndef PROFILEDP_MAIN
signed main() {
    ProfileDP dp;
    const long long MOD = 1000000007LL;

    auto trans = dp.row_transitions(2);
    cout << "F.2: row_transitions(2):\n";
    for (int prev = 0; prev < 4; prev++) {
        cout << "  trans[" << prev << "] =";
        for (long long nxt : trans[prev]) cout << " " << nxt;
        cout << "\n";
    }
    cout << "  (ожидаем {0, 3}, {2}, {1}, {0})\n";

    cout << "F.4: domino_tiling_2n(5) = " << dp.domino_tiling_2n(5)
         << " (ожидаем 8 = F(5))\n";

    cout << "F.5: count_domino_tilings(2, 3) = " << dp.count_domino_tilings(2, 3)
         << " (ожидаем 3)\n";
    cout << "F.5: count_domino_tilings(4, 4) = " << dp.count_domino_tilings(4, 4)
         << " (ожидаем 36)\n";
    cout << "F.5: count_domino_tilings(3, 3) = " << dp.count_domino_tilings(3, 3)
         << " (ожидаем 0)\n";
    cout << "F.5: count_domino_tilings(2, 5) = " << dp.count_domino_tilings(2, 5)
         << " (ожидаем 8, сверка с F.4)\n";

    cout << "F.6: count_domino_tilings_blocked(3x3, дырка (1,1)) = "
         << dp.count_domino_tilings_blocked(3, 3, {0, 2, 0})
         << " (ожидаем 2)\n";
    cout << "F.6: count_domino_tilings_blocked(2x2, дырка (0,0)) = "
         << dp.count_domino_tilings_blocked(2, 2, {1, 0})
         << " (ожидаем 0)\n";

    cout << "F.7: count_tromino_tilings(2, 3) = "
         << dp.count_tromino_tilings(2, 3) << " (ожидаем 2)\n";
    cout << "F.7: count_tromino_tilings(4, 4) = "
         << dp.count_tromino_tilings(4, 4) << " (ожидаем 0)\n";
    cout << "F.7: tromino_covering(2, 0, 0) = "
         << dp.tromino_covering(2, 0, 0) << " (ожидаем 5 = (16-1)/3)\n";
    cout << "F.7: tromino_covering(3, 5, 3) = "
         << dp.tromino_covering(3, 5, 3) << " (ожидаем 21 = (64-1)/3)\n";

    vector<vector<pair<int, int>>> dominoShapes = {{{0, 0}, {0, 1}},
                                                   {{0, 0}, {1, 0}}};
    cout << "F.8: count_shaped_tilings(2, 3, {H,V}, false) = "
         << dp.count_shaped_tilings(2, 3, dominoShapes, false)
         << " (ожидаем 3, сверка с F.5)\n";
    vector<vector<pair<int, int>>> lTromino = {{{0, 0}, {0, 1}, {1, 0}}};
    cout << "F.8: count_shaped_tilings(2, 3, {L}, true) = "
         << dp.count_shaped_tilings(2, 3, lTromino, true)
         << " (ожидаем 2, сверка с F.7)\n";

    cout << "F.9: count_domino_tiling_classes(2, 2) = "
         << dp.count_domino_tiling_classes(2, 2) << " (ожидаем 1)\n";
    cout << "F.9: count_domino_tiling_classes(2, 3) = "
         << dp.count_domino_tiling_classes(2, 3) << " (ожидаем 2)\n";
    cout << "F.9: count_domino_tiling_classes(2, 4) = "
         << dp.count_domino_tiling_classes(2, 4) << " (ожидаем 4)\n";

    cout << "F.10: count_domino_tilings_mod(4, 4, MOD) = "
         << dp.count_domino_tilings_mod(4, 4, MOD)
         << " (ожидаем 36, сверка с F.5)\n";
    cout << "F.10: count_domino_tilings_mod(2, 10, MOD) = "
         << dp.count_domino_tilings_mod(2, 10, MOD)
         << " (ожидаем 89 = F(10))\n";

    cout << "F.12: min_squares(12) = " << dp.min_squares(12)
         << " (ожидаем 3), min_squares(13) = " << dp.min_squares(13)
         << " (ожидаем 2), min_squares(10) = " << dp.min_squares(10)
         << " (ожидаем 2)\n";

    cout << "F.13: count_hamiltonian_cycles(2, 2) = "
         << dp.count_hamiltonian_cycles(2, 2) << " (ожидаем 1)\n";
    cout << "F.13: count_hamiltonian_cycles(2, 3) = "
         << dp.count_hamiltonian_cycles(2, 3) << " (ожидаем 1)\n";
    cout << "F.13: count_hamiltonian_cycles(3, 3) = "
         << dp.count_hamiltonian_cycles(3, 3) << " (ожидаем 0)\n";
    cout << "F.13: count_hamiltonian_cycles(4, 4) = "
         << dp.count_hamiltonian_cycles(4, 4) << " (ожидаем 6)\n";

    cout << "F.14: count_colorings_no_monochrome_2x2(3, 2) = "
         << dp.count_colorings_no_monochrome_2x2(3, 2) << " (ожидаем 50)\n";
    cout << "F.14: count_colorings_no_monochrome_2x2(1, 3) = "
         << dp.count_colorings_no_monochrome_2x2(1, 3) << " (ожидаем 8)\n";
    cout << "F.14: count_colorings_no_monochrome_2x2(2, 3) = "
         << dp.count_colorings_no_monochrome_2x2(2, 3) << " (ожидаем 50)\n";
}
#endif // PROFILEDP_MAIN
#endif // DYNAMIC_F_CPP
