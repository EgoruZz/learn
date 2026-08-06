#ifndef COMBINATORICS_C_CPP
#define COMBINATORICS_C_CPP

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <algorithm>
using namespace std;

// =============================================================
// C. КОМБИНАТОРИКА РАЦИОНАЛЬНЫХ ЧИСЕЛ
// =============================================================
// Структура md: A. Дерево Штерна-Броко (медианты, поиск, приближения)
//               → B. Ряд Фарея (построение, соседи, связь с SB)
//               → C. Применения (приближения, поиск)
//
// Дроби — пары (числитель, знаменатель); все сравнения — перекрёстным
// умножением в __int128 (в int64 произведение a·d переполняется уже
// при значениях ~3e9). Из базы переиспользуем только include-механику;
// по наследству доступны egcd, modinv, powmod (number-theory b.cpp).
// Уровни дерева Штерна-Броко — stern_brocot_level — реализованы здесь
// (A.2.3); special-sequences (раздел M) переиспользует их как факт.
//
// Содержит:
//   A. Штерн-Броко: frac_less, mediant, sb_search, sb_fraction, sb_bfs
//   A. Приближения: best_rational_approx, cf_expand, cf_convergents,
//      min_denominator_in_interval
//   B. Фарей: phi_sieve, farey_count, farey_build, farey_successor
//   Служебные: gcdll
//
// ВАЖНО (входы): sb_search нормализует вход (в дереве живут только
// несократимые дроби); остальные функции ожидают несократимые a/b, c/d,
// p/q. Прогулки по медиантам для иррациональных входов не завершаются —
// всегда задавайте ограничение по знаменателю или глубине.

#define COMBINATORICS_MAIN
#include "../a/a.cpp"

struct RationalCombinatorics : Combinatorics {

// =============================================================
// A. ДЕРЕВО ШТЕРНА-БРОКО
// =============================================================

// --- A.1.0. Сравнение дробей a/b < c/d через __int128 O(1) ---
// a·d < c·b; переполнение int64 при a·d > 9.2e18 — в __int128
// безопасно до значений ~1e19 по каждой координате.
bool frac_less(long long a, long long b, long long c, long long d) {
    return (__int128)a * d < (__int128)c * b;
}

// --- A.1.1. Медианта O(1) ---
// (a + c)/(b + d): при a/b < c/d и bc − ad = 1 лежит строго между ними
// и несократима (НОД(a + c, b + d) = 1) — «первая» дробь между соседями.
pair<long long, long long> mediant(pair<long long, long long> L,
                                   pair<long long, long long> R) {
    return {L.first + R.first, L.second + R.second};
}

// --- A.1.0. НОД (служебный) O(log min(a, b)) ---
long long gcdll(long long a, long long b) {
    return b == 0 ? a : gcdll(b, a % b);
}

// --- A.2.1. Поиск дроби в дереве Штерна-Броко O(a + b) ---
// Путь от корня (строка из L/R). Границы lo = 0/1, hi = 1/0; пока
// медианта m ≠ x: x < m → «L» (в левое поддерево), hi = m; иначе
// «R», lo = m. Завершается, т.к. знаменатель медианты строго растёт.
// Вход нормализуем: в дереве живут только несократимые дроби.
string sb_search(long long a, long long b) {
    long long g = gcdll(a, b);
    a /= g; b /= g;
    string path;
    long long lo_n = 0, lo_d = 1, hi_n = 1, hi_d = 0;
    while (true) {
        long long m_n = lo_n + hi_n, m_d = lo_d + hi_d;
        if (m_n == a && m_d == b) return path;
        if (frac_less(a, b, m_n, m_d)) { path += 'L'; hi_n = m_n; hi_d = m_d; }
        else { path += 'R'; lo_n = m_n; lo_d = m_d; }
    }
}

// --- A.2.2. Дробь по пути (обратная к sb_search) O(|path|) ---
// «L» → hi = медианта(lo, hi); «R» → lo = медианта(lo, hi);
// ответ — медианта границ (всегда несократим).
pair<long long, long long> sb_fraction(const string& path) {
    long long lo_n = 0, lo_d = 1, hi_n = 1, hi_d = 0;
    for (char ch : path) {
        long long m_n = lo_n + hi_n, m_d = lo_d + hi_d;
        if (ch == 'L') { hi_n = m_n; hi_d = m_d; }
        else { lo_n = m_n; lo_d = m_d; }
    }
    return {lo_n + hi_n, lo_d + hi_d};
}

// --- A.2.3. Обход дерева: все дроби глубины ≤ max_depth (BFS) ---
// Уровень d содержит 2^d дробей (отдельные уровни — stern_brocot_level
// ниже, special-sequences переиспользуют его как факт); здесь —
// суммарный обход по уровням.
vector<pair<long long, long long>> sb_bfs(int max_depth) {
    struct Node { long long n, d, lo_n, lo_d, hi_n, hi_d; };
    vector<pair<long long, long long>> res;
    vector<Node> level = {{1, 1, 0, 1, 1, 0}};
    for (int depth = 0; depth <= max_depth && !level.empty(); depth++) {
        vector<Node> nxt;
        for (const Node& v : level) {
            res.push_back({v.n, v.d});
            nxt.push_back({v.lo_n + v.n, v.lo_d + v.d, v.lo_n, v.lo_d, v.n, v.d});
            nxt.push_back({v.n + v.hi_n, v.d + v.hi_d, v.n, v.d, v.hi_n, v.hi_d});
        }
        level.swap(nxt);
    }
    return res;
}

// --- A.2.3. Уровень дерева: дроби глубины ровно d (BFS по уровням) ---
// Уровень 0 = {1/1}; дети узла a/b — a/(a+b) и (a+b)/b; уровень d
// содержит ровно 2^d дробей. Реализация здесь (комбинаторика C);
// special-sequences (раздел M) переиспользует её как факт.
vector<pair<long long, long long>> stern_brocot_level(int n) {
    vector<pair<long long, long long>> level = {{1, 1}};
    for (int i = 0; i < n; i++) {
        vector<pair<long long, long long>> next;
        for (auto [a, b] : level) {
            next.push_back({a, a + b});
            next.push_back({a + b, b});
        }
        level = next;
    }
    return level;
}

// --- A.3.1. Ближайшая дробь к x = p/q со знаменателем ≤ max_den ---
// Прогулка по медиантам; стоп, когда знаменатель медианты > N. Границы
// lo, hi — соседние дроби ряда Фарея F_N (B.2.1): любая дробь между ними
// имеет знаменатель > N (лемма Фарея) → ответ — ближайшая из границ.
// При равенстве расстояний — дробь с меньшим знаменателем.
// Шагов ≤ N (худший случай x = 1/N), на практике O(log N).
pair<long long, long long> best_rational_approx(long long p, long long q,
                                                long long max_den) {
    long long lo_n = 0, lo_d = 1, hi_n = 1, hi_d = 0;
    while (true) {
        long long m_n = lo_n + hi_n, m_d = lo_d + hi_d;
        if (m_d > max_den) break;
        if (frac_less(p, q, m_n, m_d)) { hi_n = m_n; hi_d = m_d; }
        else { lo_n = m_n; lo_d = m_d; }
    }
    // |p/q − a/b| = |p·b − a·q| / (q·b): сравниваем |p·lo_d − lo_n·q|·hi_d
    // и |p·hi_d − hi_n·q|·lo_d (общий знаменатель q·lo_d·hi_d).
    __int128 d_lo = (__int128)p * lo_d - (__int128)lo_n * q;
    __int128 d_hi = (__int128)p * hi_d - (__int128)hi_n * q;
    if (d_lo < 0) d_lo = -d_lo;
    if (d_hi < 0) d_hi = -d_hi;
    if (d_lo * hi_d <= d_hi * lo_d) return {lo_n, lo_d};
    return {hi_n, hi_d};
}

// --- A.3.2. Цепная дробь числа a/b (алгоритм Евклида) O(log(a + b)) ---
// a/b = [a₀; a₁, ..., aₖ]; возвращает канонический вид: последний
// частичный знаменатель ≥ 2 (т.е. [3; 7, 15, 1] даст [3; 7, 16]).
vector<long long> cf_expand(long long a, long long b) {
    vector<long long> cf;
    while (b != 0) {
        cf.push_back(a / b);
        long long r = a % b;
        a = b; b = r;
    }
    return cf;
}

// --- A.3.2. Подходящие дроби цепной дроби O(k) ---
// pₖ = aₖ·pₖ₋₁ + pₖ₋₂, qₖ = aₖ·qₖ₋₁ + qₖ₋₂ (p₋₁ = 1, q₋₁ = 0).
// Подходящие дроби — наилучшие приближения: |x − pₖ/qₖ| < 1/qₖ²,
// с меньшим знаменателем не приблизить лучше (теорема Лежандра).
vector<pair<long long, long long>> cf_convergents(long long a, long long b) {
    vector<long long> cf = cf_expand(a, b);
    vector<pair<long long, long long>> res;
    long long p_2 = 1, q_2 = 0, p_1 = cf[0], q_1 = 1;
    res.push_back({p_1, q_1});
    for (int i = 1; i < (int)cf.size(); i++) {
        long long p = cf[i] * p_1 + p_2;
        long long q = cf[i] * q_1 + q_2;
        res.push_back({p, q});
        p_2 = p_1; q_2 = q_1; p_1 = p; q_1 = q;
    }
    return res;
}

// --- A.3.3. Дробь с минимальным знаменателем в (a/b, c/d) ---
// Классика олимпиад. Медиантный спуск от (0/1, 1/0): m = медианта
// границ; если m строго внутри интервала — ответ; если m ≤ a/b →
// левая граница = m; иначе правая = m. Корректность: спуск идёт по
// общим предкам всех дробей интервала (каждая дробь внутри — в
// поддереве текущей медианты), а вдоль пути знаменатели строго
// растут → первый узел, попавший внутрь, минимален.
pair<long long, long long> min_denominator_in_interval(long long a, long long b,
                                                       long long c, long long d) {
    long long lo_n = 0, lo_d = 1, hi_n = 1, hi_d = 0;
    while (true) {
        long long m_n = lo_n + hi_n, m_d = lo_d + hi_d;
        if (frac_less(a, b, m_n, m_d) && frac_less(m_n, m_d, c, d))
            return {m_n, m_d};
        if (!frac_less(a, b, m_n, m_d)) { lo_n = m_n; lo_d = m_d; }
        else { hi_n = m_n; hi_d = m_d; }
    }
}

// =============================================================
// B. РЯД ФАРЕЯ
// =============================================================

// --- B.1.2. Решето Эйлера: φ(0..n) O(n log log n) ---
// φ(1) = 1; для каждого простого i вычитаем φ(x)/i из всех кратных.
vector<long long> phi_sieve(int n) {
    vector<long long> phi(n + 1);
    for (int i = 0; i <= n; i++) phi[i] = i;
    for (int i = 2; i <= n; i++)
        if (phi[i] == i)
            for (int j = i; j <= n; j += i)
                phi[j] -= phi[j] / i;
    return phi;
}

// --- B.1.2. Длина ряда Фарея: |F_n| = 1 + Σ_{k=1..n} φ(k) ---
// Для каждого знаменателя k ровно φ(k) числителей (взаимно простые
// с k); |F_n| ~ 3n²/π² — считать в long long.
long long farey_count(int n) {
    vector<long long> phi = phi_sieve(n);
    long long cnt = 1;   // дробь 0/1
    for (int k = 1; k <= n; k++) cnt += phi[k];
    return cnt;
}

// --- B.1.3. Построение ряда Фарея F_n O(|F_n|) ---
// Начало: 0/1, 1/n. Если a/b < c/d соседние, следующая дробь
// e/f = (k·c − a)/(k·d − b), где k = ⌊(n + b)/d⌋.
vector<pair<long long, long long>> farey_build(int n) {
    vector<pair<long long, long long>> res;
    res.push_back({0, 1});
    long long a = 0, b = 1, c = 1, d = n;
    res.push_back({c, d});
    while (!(c == 1 && d == 1)) {
        long long k = (n + b) / d;
        long long e = k * c - a, f = k * d - b;
        a = c; b = d; c = e; d = f;
        res.push_back({c, d});
    }
    return res;
}

// --- B.2.3. Следующая дробь ряда Фарея O(1) ---
// Даны a/b (предыдущая) и c/d (текущая) в F_n; следующая:
// k = ⌊(n + b)/d⌋, e/f = (k·c − a)/(k·d − b).
pair<long long, long long> farey_successor(long long a, long long b,
                                           long long c, long long d,
                                           long long n) {
    long long k = (n + b) / d;
    return {k * c - a, k * d - b};
}

}; // конец struct RationalCombinatorics

#ifndef RATIONAL_COMBINATORICS_MAIN
signed main() {
    RationalCombinatorics rc;

    cout << "=== A. Дерево Штерна-Броко: поиск и путь ===" << endl;
    cout << "sb_search(2/5) = " << rc.sb_search(2, 5) << " (ожид. LLR)" << endl;
    cout << "sb_search(3/4) = " << rc.sb_search(3, 4) << " (ожид. LRR)" << endl;
    cout << "sb_search(1/5) = " << rc.sb_search(1, 5) << " (ожид. LLLL)" << endl;
    cout << "sb_search(5/1) = " << rc.sb_search(5, 1) << " (ожид. RRRR)" << endl;
    cout << "sb_search(1/1) = «" << rc.sb_search(1, 1) << "» (ожид. пусто)" << endl;
    auto f = rc.sb_fraction("LLR");
    cout << "sb_fraction(LLR) = " << f.first << "/" << f.second << " (ожид. 2/5)" << endl;
    f = rc.sb_fraction("LRR");
    cout << "sb_fraction(LRR) = " << f.first << "/" << f.second << " (ожид. 3/4)" << endl;
    int pass = 1;
    for (long long a = 1; a <= 12 && pass; a++)
        for (long long b = 1; b <= 12 && pass; b++)
            if (rc.gcdll(a, b) == 1) {
                auto t = rc.sb_fraction(rc.sb_search(a, b));
                if (t != make_pair(a, b)) { pass = 0; cout << "FAIL " << a << "/" << b << endl; }
            }
    cout << "sb_fraction(sb_search(a/b)) == a/b для всех a,b ≤ 12: "
         << (pass ? "OK" : "FAIL") << endl;

    cout << "\n=== A. Обход дерева ===" << endl;
    auto bfs = rc.sb_bfs(2);
    cout << "sb_bfs(2): ";
    for (auto [n, d] : bfs) cout << n << "/" << d << " ";
    cout << "(ожид. 1/1 1/2 2/1 1/3 2/3 3/2 3/1)" << endl;
    auto lev = rc.stern_brocot_level(2);
    cout << "stern_brocot_level(2): ";
    for (auto [n, d] : lev) cout << n << "/" << d << " ";
    cout << "(ожид. 1/3 3/2 2/3 3/1, размер 2^2 = " << lev.size() << ")" << endl;

    cout << "\n=== A. Рациональные приближения ===" << endl;
    f = rc.best_rational_approx(355, 113, 100);
    cout << "best_rational_approx(π, N=100) = " << f.first << "/" << f.second
         << " (ожид. 311/99)" << endl;
    f = rc.best_rational_approx(355, 113, 1000);
    cout << "best_rational_approx(π, N=1000) = " << f.first << "/" << f.second
         << " (ожид. 355/113)" << endl;
    f = rc.best_rational_approx(99, 70, 20);
    cout << "best_rational_approx(√2, N=20) = " << f.first << "/" << f.second
         << " (ожид. 17/12)" << endl;
    auto cf = rc.cf_expand(355, 113);
    cout << "cf_expand(355/113) = ";
    for (long long x : cf) cout << x << " ";
    cout << "(ожид. 3 7 16 — канон [3; 7, 15, 1])" << endl;
    auto conv = rc.cf_convergents(355, 113);
    cout << "cf_convergents(355/113) = ";
    for (auto [n, d] : conv) cout << n << "/" << d << " ";
    cout << "(ожид. 3/1 22/7 355/113)" << endl;
    f = rc.min_denominator_in_interval(1, 3, 1, 2);
    cout << "min_denominator_in_interval(1/3, 1/2) = " << f.first << "/" << f.second
         << " (ожид. 2/5)" << endl;
    f = rc.min_denominator_in_interval(1, 5, 2, 5);
    cout << "min_denominator_in_interval(1/5, 2/5) = " << f.first << "/" << f.second
         << " (ожид. 1/3)" << endl;
    f = rc.min_denominator_in_interval(2, 5, 3, 7);
    cout << "min_denominator_in_interval(2/5, 3/7) = " << f.first << "/" << f.second
         << " (ожид. 5/12)" << endl;

    // брутфорс: минимальный знаменатель в (a/b, c/d) перебором
    vector<tuple<long long, long long, long long, long long>> ints = {
        {1, 3, 1, 2}, {1, 5, 2, 5}, {2, 5, 3, 7}, {3, 10, 1, 3},
        {1, 2, 2, 3}, {1, 5, 1, 4}, {7, 17, 8, 19}, {2, 7, 1, 3},
        {2, 5, 1, 2}, {1, 4, 2, 5}, {0, 1, 1, 2}, {5, 8, 3, 4}};
    pass = 1;
    for (auto [a, b, c, d] : ints) {
        pair<long long, long long> brute = {-1, -1};
        for (long long q = 1; q <= 300 && brute.first == -1; q++)
            for (long long p = 0; p <= q; p++)
                if (rc.frac_less(a, b, p, q) && rc.frac_less(p, q, c, d)) {
                    brute = {p, q}; break;
                }
        auto ans = rc.min_denominator_in_interval(a, b, c, d);
        if (ans != brute) {
            pass = 0;
            cout << "FAIL min_den " << a << "/" << b << " " << c << "/" << d << ": "
                 << ans.first << "/" << ans.second << " vs брутфорс "
                 << brute.first << "/" << brute.second << endl;
        }
    }
    cout << "брутфорс min_denominator_in_interval: " << (pass ? "OK" : "FAIL") << endl;

    // брутфорс: ближайшая дробь со знаменателем ≤ N
    struct Apx { long long p, q, N; };
    Apx apx[] = {{355, 113, 100}, {355, 113, 1000}, {99, 70, 20}, {355, 113, 500}};
    pass = 1;
    for (auto [p, q, N] : apx) {
        pair<long long, long long> best = {0, 1};
        __int128 best_d = -1;
        for (long long b = 1; b <= N; b++) {
            long long a0 = (__int128)p * b / q;
            for (long long a = max(0LL, a0 - 1); a <= a0 + 1; a++) {
                __int128 dist = (__int128)p * b - (__int128)a * q;
                if (dist < 0) dist = -dist;
                // dist/(q·b) vs best_d/(q·best_d): dist·b' vs best_d·b
                if (best_d == -1 || dist * best.second < best_d * b) {
                    best_d = dist; best = {a, b};
                }
            }
        }
        auto ans = rc.best_rational_approx(p, q, N);
        if (ans != best) {
            pass = 0;
            cout << "FAIL approx " << p << "/" << q << " N=" << N << ": "
                 << ans.first << "/" << ans.second << " vs брутфорс "
                 << best.first << "/" << best.second << endl;
        }
    }
    cout << "брутфорс best_rational_approx: " << (pass ? "OK" : "FAIL") << endl;

    cout << "\n=== B. Ряд Фарея ===" << endl;
    auto F5 = rc.farey_build(5);
    cout << "F_5 = ";
    for (auto [n, d] : F5) cout << n << "/" << d << " ";
    cout << "(ожид. 0/1 1/5 1/4 1/3 2/5 1/2 3/5 2/3 3/4 4/5 1/1)" << endl;
    cout << "farey_count(5) = " << rc.farey_count(5) << " (ожид. 11), "
         << "farey_count(10) = " << rc.farey_count(10) << " (ожид. 33)" << endl;
    auto phi = rc.phi_sieve(10);
    cout << "phi(1..10) = ";
    for (int i = 1; i <= 10; i++) cout << phi[i] << " ";
    cout << "(ожид. 1 1 2 2 4 2 6 4 6 4)" << endl;
    pass = 1;
    for (int i = 1; i < (int)F5.size(); i++)
        // b·c − a·d = 1 для соседей a/b < c/d
        if (F5[i - 1].second * F5[i].first - F5[i - 1].first * F5[i].second != 1) pass = 0;
    cout << "соседи F_5: bc − ad = 1 — " << (pass ? "OK" : "FAIL") << endl;
    f = rc.farey_successor(1, 3, 2, 5, 7);
    cout << "farey_successor(1/3 → 2/5, n=7) = " << f.first << "/" << f.second
         << " (ожид. 3/7)" << endl;
}
#endif // RATIONAL_COMBINATORICS_MAIN
#endif // COMBINATORICS_C_CPP
