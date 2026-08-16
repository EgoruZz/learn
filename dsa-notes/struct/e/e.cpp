#ifndef STRUCT_E_CPP
#define STRUCT_E_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <climits>
#include <map>
#include <set>
using namespace std;

// =============================================================
// V. СТРУКТУРЫ ДЛЯ ЗАПРОСОВ НА ОТРЕЗКАХ
// =============================================================
// Структура md: A. Префиксные суммы
//               → B. Дерево Фенвика
//               → C. Разреженная таблица
//               → D. Дерево отрезков
//               → E. Продвинутые деревья
//
// RangeQueries наследует Heaps (d.cpp). Переиспользует:
//   * HeapIndexedTree (II.A.2) — куча-индексация полного дерева:
//     left/right/has_left для дерева отрезков на массиве (D.1),
//     индексация не переписывается;
//   * мотив ленивой консолидации (IV.B.2) — отложенные операции
//     дерева отрезков (D.6–D.8): платить при спуске, а не сразу;
//   * персистентность PersistentDSU (III.A.7) — механика версий
//     для персистентного дерева отрезков (D.11);
//   * битовые приёмы радикс-кучи (IV.C) — lowbit-индексация
//     дерева Фенвика (B.1), бинарный спуск (B.7);
//   * std::multiset — эталонные сверки в main.
//
// Порядок методов строго соответствует порядку md (A → E).
// A.5 (Prefix Sum with Updates) — мост в B, своего кода не имеет.
// Операции «%=», возведение в степень и битовые на отрезке (D.10)
// описаны в md обобщением схемы Segment Tree Beats (постановка);
// рабочие операции кода — chmin/chmax/add и запросы суммы.
//
// ВНИМАНИЕ (скрытие имён): методы build, update, query, add, sum,
// insert, kth, count_le здесь локальные; одноимённые из других
// веток не подключаются. SegmentTreeArray::build затеняет
// HeapIndexedTree::build: добавляет построение агрегатов (D.1).

#define STRUCT_D_MAIN
#include "../d/d.cpp"
#undef STRUCT_D_MAIN

struct RangeQueries : Heaps {

// =============================================================
// A. ПРЕФИКСНЫЕ СУММЫ
// =============================================================

// --- A.1. Prefix Sum (1D) ---
// p[0] = 0; p[k] — сумма a[0..k-1]; запрос [l, r) — разность двух
// префиксов (включения-исключения в 1D). Без обновлений.
struct PrefixSum1D {
    vector<int> p;

    void build(const vector<int>& a) {
        p.assign(a.size() + 1, 0);
        for (size_t i = 0; i < a.size(); ++i) p[i + 1] = p[i] + a[i];
    }
    int sum(int l, int r) const { return p[r] - p[l]; }
};

// --- A.2. 2D Prefix Sum (включения-исключения прямоугольника) ---
// P[x][y] — сумма подматрицы [0, x) x [0, y); запрос — включения-
// исключения по 4 углам. Полуинтервальные индексы.
struct PrefixSum2D {
    vector<vector<int>> P;

    void build(const vector<vector<int>>& a) {
        int n = (int)a.size();
        int m = n ? (int)a[0].size() : 0;
        P.assign(n + 1, vector<int>(m + 1, 0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                P[i + 1][j + 1] = P[i][j + 1] + P[i + 1][j] - P[i][j] + a[i][j];
    }
    int sum(int x1, int y1, int x2, int y2) const {  // [x1,x2) x [y1,y2)
        return P[x2][y2] - P[x1][y2] - P[x2][y1] + P[x1][y1];
    }
};

// --- A.3. 3D Prefix Sum (обобщение на k измерений) ---
// P[x][y][z] — сумма параллелепипеда [0,x)x[0,y)x[0,z); запрос —
// включения-исключения по 2^3 = 8 углам: знак по чётности числа
// «нижних» границ (угол с чётным числом нижних границ прибавляется).
struct PrefixSum3D {
    vector<vector<vector<int>>> P;

    void build(const vector<vector<vector<int>>>& a) {
        int nx = (int)a.size();
        int ny = nx ? (int)a[0].size() : 0;
        int nz = ny ? (int)a[0][0].size() : 0;
        P.assign(nx + 1, vector<vector<int>>(ny + 1, vector<int>(nz + 1, 0)));
        for (int i = 0; i < nx; ++i)
            for (int j = 0; j < ny; ++j)
                for (int k = 0; k < nz; ++k)
                    P[i + 1][j + 1][k + 1] = P[i][j + 1][k + 1] + P[i + 1][j][k + 1]
                        + P[i + 1][j + 1][k] - P[i][j][k + 1] - P[i][j + 1][k]
                        - P[i + 1][j][k] + P[i][j][k] + a[i][j][k];
    }
    int sum(int x1, int y1, int z1, int x2, int y2, int z2) const {
        int s = 0;
        for (int bx = 0; bx < 2; ++bx)
            for (int by = 0; by < 2; ++by)
                for (int bz = 0; bz < 2; ++bz) {
                    int X = bx ? x2 : x1, Y = by ? y2 : y1, Z = bz ? z2 : z1;
                    int sign = (bx + by + bz) % 2 ? 1 : -1;
                    s += sign * P[X][Y][Z];
                }
        return s;
    }
};

// --- A.4. Difference Array (разностный массив) ---
// d[i] = a[i] - a[i-1]; прибавление x на [l, r): d[l] += x, d[r] -= x;
// значение a[i] — префиксная сумма d («интегрирование»).
struct DifferenceArray {
    vector<long long> d;

    void build(const vector<int>& a) {
        int n = (int)a.size();
        d.assign(n + 1, 0);
        for (int i = 0; i < n; ++i) { d[i] += a[i]; d[i + 1] -= a[i]; }
    }
    void add(int l, int r, long long x) { d[l] += x; d[r] -= x; }
    long long value(int i) const {
        long long s = 0;
        for (int k = 0; k <= i; ++k) s += d[k];
        return s;
    }
    vector<long long> restore() const {
        vector<long long> res;
        long long s = 0;
        for (size_t k = 0; k + 1 < d.size(); ++k) { s += d[k]; res.push_back(s); }
        return res;
    }
};

// A.5. Prefix Sum with Updates — мост в B.1 (Fenwick): префиксы не
// выдерживают обновлений (пересборка O(n)); Fenwick даёт O(log n).

// =============================================================
// B. ДЕРЕВО ФЕНВИКА (FENWICK TREE / BINARY INDEXED TREE)
// =============================================================

// --- B.1. Standard Fenwick Tree (Point Update, Range Query) ---
// Индексы 1..n; p[i] хранит сумму на (i - lowbit(i), i]. Префикс —
// сумма по lowbit-цепочке вниз; обновление — подъём по i += lowbit(i).
struct Fenwick {
    int n;
    vector<int> p;

    Fenwick(int n_ = 0) { init(n_); }
    void init(int n_) { n = n_; p.assign(n + 1, 0); }

    void add(int i, int x) {
        for (; i <= n; i += i & -i) p[i] += x;
    }
    int prefix_sum(int i) const {
        int s = 0;
        for (; i > 0; i -= i & -i) s += p[i];
        return s;
    }
    int sum(int l, int r) const { return prefix_sum(r) - prefix_sum(l - 1); }

    // B.7. Поиск k-го по сумме: наименьший i с prefix_sum(i) >= k
    // (веса >= 0): спуск по степеням двойки, как бинарный поиск.
    int kth(int k) const {
        int idx = 0, step = 1;
        while (step << 1 <= n) step <<= 1;
        for (; step; step >>= 1) {
            int nxt = idx + step;
            if (nxt <= n && p[nxt] < k) { idx = nxt; k -= p[nxt]; }
        }
        return idx + 1;
    }
};

// --- B.2. Maximum Fenwick Tree ---
// Максимум на префиксе [1, i]; работает при обновлениях, не
// ухудшающих значения (увеличение). Уменьшение — не поддерживается.
struct FenwickMax {
    int n;
    vector<int> p;

    FenwickMax(int n_ = 0) { init(n_); }
    void init(int n_) { n = n_; p.assign(n + 1, INT_MIN); }

    void build(const vector<int>& a) {
        init((int)a.size());
        for (int i = 1; i <= n; ++i) p[i] = a[i - 1];
        for (int i = 1; i <= n; ++i) {
            int j = i + (i & -i);
            if (j <= n) p[j] = max(p[j], p[i]);
        }
    }
    void update(int i, int x) {
        for (; i <= n; i += i & -i) p[i] = max(p[i], x);
    }
    int prefix_max(int i) const {
        int r = INT_MIN;
        for (; i > 0; i -= i & -i) r = max(r, p[i]);
        return r;
    }
};

// --- B.3. Minimum Fenwick Tree ---
// Зеркально B.2: минимум на префиксе; обновления, не увеличивающие.
struct FenwickMin {
    int n;
    vector<int> p;

    FenwickMin(int n_ = 0) { init(n_); }
    void init(int n_) { n = n_; p.assign(n + 1, INT_MAX); }

    void build(const vector<int>& a) {
        init((int)a.size());
        for (int i = 1; i <= n; ++i) p[i] = a[i - 1];
        for (int i = 1; i <= n; ++i) {
            int j = i + (i & -i);
            if (j <= n) p[j] = min(p[j], p[i]);
        }
    }
    void update(int i, int x) {
        for (; i <= n; i += i & -i) p[i] = min(p[i], x);
    }
    int prefix_min(int i) const {
        int r = INT_MAX;
        for (; i > 0; i -= i & -i) r = min(r, p[i]);
        return r;
    }
};

// --- B.4. 2D Fenwick Tree (вложенные деревья) ---
// Внешний Fenwick по x, в каждой вершине — Fenwick по y; запрос
// прямоугольника — включения-исключения по 4 префиксам.
struct Fenwick2D {
    int n, m;
    vector<vector<int>> p;

    Fenwick2D(int n_ = 0, int m_ = 0) { init(n_, m_); }
    void init(int n_, int m_) { n = n_; m = m_; p.assign(n + 1, vector<int>(m + 1, 0)); }

    void add(int x, int y, int v) {
        for (int i = x; i <= n; i += i & -i)
            for (int j = y; j <= m; j += j & -j)
                p[i][j] += v;
    }
    int prefix_sum(int x, int y) const {
        int s = 0;
        for (int i = x; i > 0; i -= i & -i)
            for (int j = y; j > 0; j -= j & -j)
                s += p[i][j];
        return s;
    }
    int sum(int x1, int y1, int x2, int y2) const {
        return prefix_sum(x2, y2) - prefix_sum(x1 - 1, y2)
             - prefix_sum(x2, y1 - 1) + prefix_sum(x1 - 1, y1 - 1);
    }
};

// --- B.5. Fenwick with Range Updates ---
// Range Update / Point Query: разностный массив (A.4) в одном Fenwick.
struct FenwickRangeUpdatePointQuery {
    Fenwick d;

    FenwickRangeUpdatePointQuery(int n_ = 0) : d(n_ + 1) {}
    void add_range(int l, int r, int x) { d.add(l, x); d.add(r + 1, -x); }
    int point(int i) const { return d.prefix_sum(i); }
};

// Range Update / Range Query: два Fenwick — B1 по d[i], B2 по i*d[i];
// сумма на [1..i] = (i+1)*B1 - B2.
struct FenwickRangeUpdateRangeQuery {
    int n;
    Fenwick B1, B2;

    FenwickRangeUpdateRangeQuery(int n_ = 0) : n(n_), B1(n_ + 1), B2(n_ + 1) {}
    void add_range(int l, int r, int x) {
        B1.add(l, x); B1.add(r + 1, -x);
        B2.add(l, 1LL * x * l); B2.add(r + 1, -1LL * x * (r + 1));
    }
    long long prefix(int i) const {
        return 1LL * (i + 1) * B1.prefix_sum(i) - B2.prefix_sum(i);
    }
    long long sum_range(int l, int r) const { return prefix(r) - prefix(l - 1); }
};

// --- B.6. Fenwick Tree on Fenwick Tree ---
// Разреженный «дерево на дереве»: внутренние Fenwick — map-узлы,
// создаются по требованию (только для реально обновлявшихся строк).
struct FenwickOnFenwick {
    int n;
    vector<map<int, int>> p;  // внешний Fenwick по x; внутренние — по y

    FenwickOnFenwick(int n_ = 0) : n(n_) { p.assign(n + 1, map<int, int>()); }

    void add(int x, int y, int v) {
        for (int i = x; i <= n; i += i & -i)
            for (int j = y; j <= n; j += j & -j)
                p[i][j] += v;
    }
    int prefix_sum(int x, int y) const {
        int s = 0;
        for (int i = x; i > 0; i -= i & -i)
            for (int j = y; j > 0; j -= j & -j) {
                auto it = p[i].find(j);
                if (it != p[i].end()) s += it->second;
            }
        return s;
    }
    int sum(int x1, int y1, int x2, int y2) const {
        return prefix_sum(x2, y2) - prefix_sum(x1 - 1, y2)
             - prefix_sum(x2, y1 - 1) + prefix_sum(x1 - 1, y1 - 1);
    }
};

// =============================================================
// C. РАЗРЕЖЕННАЯ ТАБЛИЦА (SPARSE TABLE)
// =============================================================

// --- C.1. Sparse Table для RMQ ---
// st[j][i] — минимум на [i, i + 2^j); запрос — минимум двух блоков
// (пересечение допустимо: min идемпотентен). lg[] предвычислен.
struct SparseTableRMQ {
    vector<vector<int>> st;
    vector<int> lg;

    void build(const vector<int>& a) {
        int n = (int)a.size();
        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i) lg[i] = lg[i / 2] + 1;
        int K = lg[n] + 1;
        st.assign(K, vector<int>(n));
        st[0] = a;
        for (int j = 1; j < K; ++j)
            for (int i = 0; i + (1 << j) <= n; ++i)
                st[j][i] = min(st[j - 1][i], st[j - 1][i + (1 << (j - 1))]);
    }
    int query(int l, int r) const {  // [l, r)
        int j = lg[r - l];
        return min(st[j][l], st[j][r - (1 << j)]);
    }
};

// --- C.2. Sparse Table для GCD ---
// gcd ассоциативна и идемпотентна (gcd(x, x) = x) — та же схема.
struct SparseTableGCD {
    vector<vector<int>> st;
    vector<int> lg;

    void build(const vector<int>& a) {
        int n = (int)a.size();
        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i) lg[i] = lg[i / 2] + 1;
        int K = lg[n] + 1;
        st.assign(K, vector<int>(n));
        st[0] = a;
        for (int j = 1; j < K; ++j)
            for (int i = 0; i + (1 << j) <= n; ++i)
                st[j][i] = gcd(st[j - 1][i], st[j - 1][i + (1 << (j - 1))]);
    }
    int query(int l, int r) const {
        int j = lg[r - l];
        return gcd(st[j][l], st[j][r - (1 << j)]);
    }
};

// --- C.3. Sparse Table для произвольных идемпотентных операций ---
// Операция f — параметр: ассоциативность + идемпотентность (f(x,x)=x)
// — ровно то условие, при котором пересекающиеся покрытия допустимы.
struct SparseTableGeneric {
    vector<vector<int>> st;
    vector<int> lg;
    std::function<int(int, int)> f;

    void build(const vector<int>& a, std::function<int(int, int)> op) {
        f = op;
        int n = (int)a.size();
        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i) lg[i] = lg[i / 2] + 1;
        int K = lg[n] + 1;
        st.assign(K, vector<int>(n));
        st[0] = a;
        for (int j = 1; j < K; ++j)
            for (int i = 0; i + (1 << j) <= n; ++i)
                st[j][i] = f(st[j - 1][i], st[j - 1][i + (1 << (j - 1))]);
    }
    int query(int l, int r) const {
        int j = lg[r - l];
        return f(st[j][l], st[j][r - (1 << j)]);
    }
};

// --- C.4. Двумерная Sparse Table ---
// st[k1][k2] — минимум на прямоугольнике 2^k1 x 2^k2; запрос —
// минимум по 4 блокам (идемпотентность).
struct SparseTable2D {
    int n, m;
    vector<vector<vector<vector<int>>>> st;  // [K1][K2][n][m]
    vector<int> lg;

    void build(const vector<vector<int>>& a) {
        n = (int)a.size();
        m = n ? (int)a[0].size() : 0;
        int mx = max(n, m);
        lg.assign(mx + 1, 0);
        for (int i = 2; i <= mx; ++i) lg[i] = lg[i / 2] + 1;
        int K1 = lg[n] + 1, K2 = lg[m] + 1;
        st.assign(K1, vector<vector<vector<int>>>(K2,
                   vector<vector<int>>(n, vector<int>(m))));
        for (int i = 0; i < n; ++i) st[0][0][i] = a[i];
        for (int k2 = 1; k2 < K2; ++k2)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j + (1 << k2) <= m; ++j)
                    st[0][k2][i][j] = min(st[0][k2 - 1][i][j],
                                          st[0][k2 - 1][i][j + (1 << (k2 - 1))]);
        for (int k1 = 1; k1 < K1; ++k1)
            for (int k2 = 0; k2 < K2; ++k2)
                for (int i = 0; i + (1 << k1) <= n; ++i)
                    for (int j = 0; j + (1 << k2) <= m; ++j)
                        st[k1][k2][i][j] = min(st[k1 - 1][k2][i][j],
                                               st[k1 - 1][k2][i + (1 << (k1 - 1))][j]);
    }
    int query(int x1, int y1, int x2, int y2) const {  // [x1,x2) x [y1,y2)
        int k1 = lg[x2 - x1], k2 = lg[y2 - y1];
        return min(min(st[k1][k2][x1][y1], st[k1][k2][x2 - (1 << k1)][y1]),
                   min(st[k1][k2][x1][y2 - (1 << k2)], st[k1][k2][x2 - (1 << k1)][y2 - (1 << k2)]));
    }
};

// --- C.5. Disjoint Sparse Table (неидемпотентные операции) ---
// Уровень j: блоки размера 2^(j+1); в каждом — суффиксы левой
// половины (pref) и префиксы правой (suff). Запрос [l, r): уровень
// k = lg(l xor (r-1)) — середина блока строго между l и r; ответ —
// pref[k][l] + suff[k][r-1] без пересечений. n — степень двойки.
struct DisjointSparseTable {
    int n;
    vector<vector<int>> pref, suff;
    vector<int> lg;

    void build(const vector<int>& a) {
        n = 1;
        while (n < (int)a.size()) n <<= 1;
        vector<int> b(n, 0);
        for (size_t i = 0; i < a.size(); ++i) b[i] = a[i];
        int L = 0;
        while ((1 << (L + 1)) <= n) ++L;  // уровни 0..L-1: блоки 2^(j+1)
        if (L == 0) L = 1;  // n == 1: один уровень, pref[0][0] — единственный элемент
        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i) lg[i] = lg[i / 2] + 1;
        pref.assign(L, vector<int>(n, 0));
        suff.assign(L, vector<int>(n, 0));
        for (int j = 0; j < L; ++j) {
            int bs = 1 << (j + 1), half = 1 << j;
            for (int bl = 0; bl < n; bl += bs) {
                int acc = 0;
                for (int i = bl + half - 1; i >= bl; --i) { acc += b[i]; pref[j][i] = acc; }
                acc = 0;
                for (int i = bl + half; i < bl + bs && i < n; ++i) { acc += b[i]; suff[j][i] = acc; }
            }
        }
    }
    int query(int l, int r) const {  // [l, r)
        int k = lg[l ^ (r - 1)];
        return pref[k][l] + suff[k][r - 1];
    }
};

// --- C.6. Sparse Table с координатным сжатием (для разреженных точек) ---
// Универсум [0, U) огромен, точек m мало: координаты сжимаются
// (сортировка уникальных + lower_bound — тот же приём, что D.5.1),
// значения ложатся на сжатые индексы, между ними — нейтрал INT_MAX.
// Рабочая таблица — готовая C.1 (SparseTableRMQ) на m позициях:
// O(m log m) памяти вместо O(U log U) по всему универсуму.
struct SparseTableCompressed {
    vector<int> coords;
    SparseTableRMQ st;              // переиспользуем C.1

    // Точки (координата, значение); повтор координаты — берётся минимум.
    void build(const vector<pair<int, int>>& points) {
        coords.clear();
        for (auto& p : points) coords.push_back(p.first);
        sort(coords.begin(), coords.end());
        coords.erase(unique(coords.begin(), coords.end()), coords.end());
        vector<int> b(coords.size(), INT_MAX);
        for (auto& p : points) {
            int i = lower_bound(coords.begin(), coords.end(), p.first) - coords.begin();
            b[i] = min(b[i], p.second);
        }
        st.build(b);
    }
    // Минимум на координатах из [l, r); точек нет — нейтрал INT_MAX.
    int query(int l, int r) const {
        int li = lower_bound(coords.begin(), coords.end(), l) - coords.begin();
        int ri = lower_bound(coords.begin(), coords.end(), r) - coords.begin();
        if (li >= ri) return INT_MAX;
        return st.query(li, ri);
    }
};

// =============================================================
// D. ДЕРЕВО ОТРЕЗКОВ (SEGMENT TREE)
// =============================================================

// --- D.1. Segment Tree (на массиве) ---
// Полное дерево по куча-индексации (II.A.2): left/right/has_left
// унаследованы от HeapIndexedTree; хранилище sum — 4n вершин;
// a (унаследованный) — листья. build затеняет HeapIndexedTree::build.
struct SegmentTreeArray : HeapIndexedTree {
    int n;
    vector<int> sum;

    void build(const vector<int>& vals) {
        n = (int)vals.size();
        a = vals;
        sum.assign(4 * n + 4, 0);
        build_rec(0, 0, n);
    }
    int build_rec(int v, int l, int r) {
        if (r - l == 1) return sum[v] = a[l];
        int m = (l + r) / 2;
        return sum[v] = build_rec(left(v), l, m) + build_rec(right(v), m, r);
    }
    void update(int i, int x) { update_rec(0, 0, n, i, x); }
    void update_rec(int v, int l, int r, int i, int x) {
        if (r - l == 1) { sum[v] = x; return; }
        int m = (l + r) / 2;
        if (i < m) update_rec(left(v), l, m, i, x);
        else update_rec(right(v), m, r, i, x);
        sum[v] = sum[left(v)] + sum[right(v)];
    }
    int query(int l, int r) const { return query_rec(0, 0, n, l, r); }
    int query_rec(int v, int l, int r, int ql, int qr) const {
        if (qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr) return sum[v];
        int m = (l + r) / 2;
        return query_rec(left(v), l, m, ql, qr) + query_rec(right(v), m, r, ql, qr);
    }
};

// --- D.2. Segment Tree (на указателях) ---
// Узлы {l, r, sum, lc, rc} с явными границами [l, r); база для
// динамического (D.5) и персистентного (D.11) деревьев.
struct SegmentTreePointers {
    struct Node {
        int l, r;
        long long sum;
        Node* lc;
        Node* rc;
    };

    Node* root = nullptr;
    int n = 0;

    void build(const vector<int>& a) {
        n = (int)a.size();
        root = build_rec(0, n, a);
    }
    Node* build_rec(int l, int r, const vector<int>& a) {
        Node* t = new Node{l, r, 0, nullptr, nullptr};
        if (r - l == 1) { t->sum = a[l]; return t; }
        int m = (l + r) / 2;
        t->lc = build_rec(l, m, a);
        t->rc = build_rec(m, r, a);
        t->sum = t->lc->sum + t->rc->sum;
        return t;
    }
    void update(int i, int x) { update_rec(root, i, x); }
    void update_rec(Node* t, int i, int x) {
        if (t->r - t->l == 1) { t->sum = x; return; }
        int m = (t->l + t->r) / 2;
        if (i < m) update_rec(t->lc, i, x);
        else update_rec(t->rc, i, x);
        t->sum = t->lc->sum + t->rc->sum;
    }
    long long query(int l, int r) const { return query_rec(root, l, r); }
    long long query_rec(Node* t, int ql, int qr) const {
        if (qr <= t->l || t->r <= ql) return 0;
        if (ql <= t->l && t->r <= qr) return t->sum;
        return query_rec(t->lc, ql, qr) + query_rec(t->rc, ql, qr);
    }
};

// --- D.3. Non Recursive Segment Tree ---
// Листья на [sz, 2*sz) (sz — степень двойки), свёртка вверх; запрос —
// два указателя l, r с подъёмом; обновление — путь от листа к корню.
struct SegmentTreeIterative {
    int n, sz;
    vector<int> t;

    void build(const vector<int>& a) {
        n = (int)a.size();
        sz = 1;
        while (sz < n) sz <<= 1;
        t.assign(2 * sz, 0);
        for (int i = 0; i < n; ++i) t[sz + i] = a[i];
        for (int i = sz - 1; i >= 1; --i) t[i] = t[2 * i] + t[2 * i + 1];
    }
    void update(int i, int x) {
        int v = sz + i;
        t[v] = x;
        for (v >>= 1; v; v >>= 1) t[v] = t[2 * v] + t[2 * v + 1];
    }
    int query(int l, int r) const {  // [l, r)
        int res = 0;
        for (l += sz, r += sz; l < r; l >>= 1, r >>= 1) {
            if (l & 1) res += t[l++];
            if (r & 1) res += t[--r];
        }
        return res;
    }

    // D.4. Segment Tree Other: бинарный спуск к k-му по сумме
    // (веса >= 0): наименьший индекс i с суммой [0..i] >= k.
    int kth(int k) const {
        int v = 1;
        while (v < sz) {
            if (t[2 * v] >= k) v = 2 * v;
            else { k -= t[2 * v]; v = 2 * v + 1; }
        }
        return v - sz;
    }
};

// --- D.5. Динамическое дерево отрезков (без сжатия) ---
// Диапазон [0, N) задан заранее; вершины создаются по требованию
// (пул {lc, rc, sum}, 0 — пустой узел с нейтральным ответом).
struct SegmentTreeDynamic {
    struct Node {
        int lc, rc;
        long long sum;
    };

    int N, root;
    vector<Node> pool;  // pool[0] — «пустой» узел (lc=rc=0, sum=0)

    SegmentTreeDynamic(int N_ = 0) : N(N_), root(0) { pool.push_back({0, 0, 0}); }
    int new_node() { pool.push_back({0, 0, 0}); return (int)pool.size() - 1; }

    void update(int pos, long long x) {
        if (root == 0) root = new_node();
        upd(root, 0, N, pos, x);
    }
    // Идём по индексам (не по ссылкам на элементы вектора): new_node
    // может перевыделить пул — ссылка на pool[v].lc стала бы висячей.
    void upd(int v, int l, int r, int pos, long long x) {
        if (r - l == 1) { pool[v].sum = x; return; }
        int m = (l + r) / 2;
        int child = (pos < m) ? pool[v].lc : pool[v].rc;
        if (child == 0) {
            child = new_node();
            if (pos < m) pool[v].lc = child;
            else pool[v].rc = child;
        }
        if (pos < m) upd(child, l, m, pos, x);
        else upd(child, m, r, pos, x);
        pool[v].sum = pool[pool[v].lc].sum + pool[pool[v].rc].sum;
    }
    long long query(int l, int r) const { return query_rec(root, 0, N, l, r); }
    long long query_rec(int v, int l, int r, int ql, int qr) const {
        if (v == 0 || qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr) return pool[v].sum;
        int m = (l + r) / 2;
        return query_rec(pool[v].lc, l, m, ql, qr) + query_rec(pool[v].rc, m, r, ql, qr);
    }
};

// --- D.5.1. Сжатое по координатам дерево отрезков (coordinate compression) ---
// Статический аналог D.5: координаты операций известны заранее
// и сжимаются (сортировка уникальных + lower_bound) — дерево строится
// над сжатыми индексами (m вместо универсума U); механика — D.1
// (SegmentTreeArray) на массиве размера m. Точки между координатами
// не хранятся; границы запросов маппятся lower_bound'ами.
struct SegmentTreeCompressed {
    vector<int> coords;
    SegmentTreeArray tree;          // переиспользуем D.1

    // Зафиксировать координаты (уникальные, сортируются внутри).
    void set_coords(vector<int> xs) {
        sort(xs.begin(), xs.end());
        xs.erase(unique(xs.begin(), xs.end()), xs.end());
        coords = xs;
    }
    // Начальные значения: пары (координата, значение).
    void build(const vector<pair<int, int>>& points) {
        vector<int> b(coords.size(), 0);
        for (auto& p : points) {
            int i = lower_bound(coords.begin(), coords.end(), p.first) - coords.begin();
            if (i < (int)coords.size() && coords[i] == p.first) b[i] = p.second;
        }
        tree.build(b);
    }
    // Присвоить значение координате x (нет в coords — игнорируется).
    void update(int x, int v) {
        int i = lower_bound(coords.begin(), coords.end(), x) - coords.begin();
        if (i < (int)coords.size() && coords[i] == x) tree.update(i, v);
    }
    // Сумма по координатам в [l, r).
    int query(int l, int r) const {
        int li = lower_bound(coords.begin(), coords.end(), l) - coords.begin();
        int ri = lower_bound(coords.begin(), coords.end(), r) - coords.begin();
        return tree.query(li, ri);
    }
};

// --- D.6. Lazy Propagation: Add on segment, sum on segment ---
// Узел хранит отложенный сдвиг lazy, применённый к узлу целиком
// и передаваемый детям при спуске (push). Мотив ленивости — IV.B.2.
struct LazySegTreeAdd {
    int n;
    vector<long long> tree, lazy;

    void build(const vector<int>& a) {
        n = (int)a.size();
        tree.assign(4 * n + 4, 0);
        lazy.assign(4 * n + 4, 0);
        build_rec(0, 0, n, a);
    }
    void build_rec(int v, int l, int r, const vector<int>& a) {
        if (r - l == 1) { tree[v] = a[l]; return; }
        int m = (l + r) / 2;
        build_rec(2 * v + 1, l, m, a);
        build_rec(2 * v + 2, m, r, a);
        tree[v] = tree[2 * v + 1] + tree[2 * v + 2];
    }
    void apply(int v, int l, int r, long long x) {
        tree[v] += x * (r - l);
        lazy[v] += x;
    }
    void push(int v, int l, int r) {
        if (lazy[v] == 0 || r - l == 1) return;
        int m = (l + r) / 2;
        apply(2 * v + 1, l, m, lazy[v]);
        apply(2 * v + 2, m, r, lazy[v]);
        lazy[v] = 0;
    }
    void add_segment(int ql, int qr, long long x) { add_rec(0, 0, n, ql, qr, x); }
    void add_rec(int v, int l, int r, int ql, int qr, long long x) {
        if (qr <= l || r <= ql) return;
        if (ql <= l && r <= qr) { apply(v, l, r, x); return; }
        push(v, l, r);
        int m = (l + r) / 2;
        add_rec(2 * v + 1, l, m, ql, qr, x);
        add_rec(2 * v + 2, m, r, ql, qr, x);
        tree[v] = tree[2 * v + 1] + tree[2 * v + 2];
    }
    long long query(int ql, int qr) { return query_rec(0, 0, n, ql, qr); }
    long long query_rec(int v, int l, int r, int ql, int qr) {
        if (qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr) return tree[v];
        push(v, l, r);
        int m = (l + r) / 2;
        return query_rec(2 * v + 1, l, m, ql, qr) + query_rec(2 * v + 2, m, r, ql, qr);
    }
};

// --- D.7. Lazy Propagation: Assign on segment ---
// Присваивание на отрезке: lazy — «нет set» (INT_MIN) или значение;
// push перезаписывает детей (присваивание не складывается со старым
// lazy — флаг заменяется, а не накапливается).
struct LazySegTreeAssign {
    int n;
    vector<long long> tree;
    vector<int> lazy;  // INT_MIN — нет отложенного присваивания

    void build(const vector<int>& a) {
        n = (int)a.size();
        tree.assign(4 * n + 4, 0);
        lazy.assign(4 * n + 4, INT_MIN);
        build_rec(0, 0, n, a);
    }
    void build_rec(int v, int l, int r, const vector<int>& a) {
        if (r - l == 1) { tree[v] = a[l]; return; }
        int m = (l + r) / 2;
        build_rec(2 * v + 1, l, m, a);
        build_rec(2 * v + 2, m, r, a);
        tree[v] = tree[2 * v + 1] + tree[2 * v + 2];
    }
    void apply(int v, int l, int r, int x) {
        tree[v] = 1LL * x * (r - l);
        lazy[v] = x;
    }
    void push(int v, int l, int r) {
        if (lazy[v] == INT_MIN || r - l == 1) return;
        int m = (l + r) / 2;
        apply(2 * v + 1, l, m, lazy[v]);
        apply(2 * v + 2, m, r, lazy[v]);
        lazy[v] = INT_MIN;
    }
    void assign(int ql, int qr, int x) { assign_rec(0, 0, n, ql, qr, x); }
    void assign_rec(int v, int l, int r, int ql, int qr, int x) {
        if (qr <= l || r <= ql) return;
        if (ql <= l && r <= qr) { apply(v, l, r, x); return; }
        push(v, l, r);
        int m = (l + r) / 2;
        assign_rec(2 * v + 1, l, m, ql, qr, x);
        assign_rec(2 * v + 2, m, r, ql, qr, x);
        tree[v] = tree[2 * v + 1] + tree[2 * v + 2];
    }
    long long query(int ql, int qr) { return query_rec(0, 0, n, ql, qr); }
    long long query_rec(int v, int l, int r, int ql, int qr) {
        if (qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr) return tree[v];
        push(v, l, r);
        int m = (l + r) / 2;
        return query_rec(2 * v + 1, l, m, ql, qr) + query_rec(2 * v + 2, m, r, ql, qr);
    }
};

// --- D.8. Lazy: Add and Assign combination ---
// Два вида отложенных: add и set; приоритет — set перекрывает add
// (set на узел сбрасывает add; add ложится поверх set). Композиция:
// set(v) после set(u) = set(v); add(x) после set(v) = set(v + x);
// add(x) после add(y) = add(x + y).
struct LazySegTreeAddAssign {
    int n;
    vector<long long> tree, lazy_add;
    vector<int> lazy_set;  // INT_MIN — нет отложенного set

    void build(const vector<int>& a) {
        n = (int)a.size();
        tree.assign(4 * n + 4, 0);
        lazy_add.assign(4 * n + 4, 0);
        lazy_set.assign(4 * n + 4, INT_MIN);
        build_rec(0, 0, n, a);
    }
    void build_rec(int v, int l, int r, const vector<int>& a) {
        if (r - l == 1) { tree[v] = a[l]; return; }
        int m = (l + r) / 2;
        build_rec(2 * v + 1, l, m, a);
        build_rec(2 * v + 2, m, r, a);
        tree[v] = tree[2 * v + 1] + tree[2 * v + 2];
    }
    void apply_set(int v, int l, int r, int x) {
        tree[v] = 1LL * x * (r - l);
        lazy_set[v] = x;
        lazy_add[v] = 0;
    }
    void apply_add(int v, int l, int r, long long x) {
        tree[v] += x * (r - l);
        lazy_add[v] += x;
    }
    void push(int v, int l, int r) {
        if (r - l == 1) { lazy_add[v] = 0; lazy_set[v] = INT_MIN; return; }
        int m = (l + r) / 2;
        if (lazy_set[v] != INT_MIN) {
            apply_set(2 * v + 1, l, m, lazy_set[v]);
            apply_set(2 * v + 2, m, r, lazy_set[v]);
            lazy_set[v] = INT_MIN;
        }
        if (lazy_add[v] != 0) {
            apply_add(2 * v + 1, l, m, lazy_add[v]);
            apply_add(2 * v + 2, m, r, lazy_add[v]);
            lazy_add[v] = 0;
        }
    }
    void assign(int ql, int qr, int x) { assign_rec(0, 0, n, ql, qr, x); }
    void assign_rec(int v, int l, int r, int ql, int qr, int x) {
        if (qr <= l || r <= ql) return;
        if (ql <= l && r <= qr) { apply_set(v, l, r, x); return; }
        push(v, l, r);
        int m = (l + r) / 2;
        assign_rec(2 * v + 1, l, m, ql, qr, x);
        assign_rec(2 * v + 2, m, r, ql, qr, x);
        tree[v] = tree[2 * v + 1] + tree[2 * v + 2];
    }
    void add_segment(int ql, int qr, long long x) { add_rec(0, 0, n, ql, qr, x); }
    void add_rec(int v, int l, int r, int ql, int qr, long long x) {
        if (qr <= l || r <= ql) return;
        if (ql <= l && r <= qr) { apply_add(v, l, r, x); return; }
        push(v, l, r);
        int m = (l + r) / 2;
        add_rec(2 * v + 1, l, m, ql, qr, x);
        add_rec(2 * v + 2, m, r, ql, qr, x);
        tree[v] = tree[2 * v + 1] + tree[2 * v + 2];
    }
    long long query(int ql, int qr) { return query_rec(0, 0, n, ql, qr); }
    long long query_rec(int v, int l, int r, int ql, int qr) {
        if (qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr) return tree[v];
        push(v, l, r);
        int m = (l + r) / 2;
        return query_rec(2 * v + 1, l, m, ql, qr) + query_rec(2 * v + 2, m, r, ql, qr);
    }
};

// --- D.9. Массовые операции: параметризация свёртки и нейтрального ---
// Операция op и нейтральный элемент — параметры (sum/min/max/gcd);
// query сводит ответы канонических узлов через op. Обратимость
// операции не нужна (в отличие от Fenwick B.1).
struct SegmentTreeGeneric {
    int n;
    vector<int> t;
    std::function<int(int, int)> op;
    int neut;

    void build(const vector<int>& a, std::function<int(int, int)> f, int neutral) {
        op = f;
        neut = neutral;
        n = (int)a.size();
        t.assign(4 * n + 4, neut);
        build_rec(0, 0, n, a);
    }
    void build_rec(int v, int l, int r, const vector<int>& a) {
        if (r - l == 1) { t[v] = a[l]; return; }
        int m = (l + r) / 2;
        build_rec(2 * v + 1, l, m, a);
        build_rec(2 * v + 2, m, r, a);
        t[v] = op(t[2 * v + 1], t[2 * v + 2]);
    }
    void update(int i, int x) { update_rec(0, 0, n, i, x); }
    void update_rec(int v, int l, int r, int i, int x) {
        if (r - l == 1) { t[v] = x; return; }
        int m = (l + r) / 2;
        if (i < m) update_rec(2 * v + 1, l, m, i, x);
        else update_rec(2 * v + 2, m, r, i, x);
        t[v] = op(t[2 * v + 1], t[2 * v + 2]);
    }
    int query(int l, int r) const { return query_rec(0, 0, n, l, r); }
    int query_rec(int v, int l, int r, int ql, int qr) const {
        if (qr <= l || r <= ql) return neut;
        if (ql <= l && r <= qr) return t[v];
        int m = (l + r) / 2;
        return op(query_rec(2 * v + 1, l, m, ql, qr),
                  query_rec(2 * v + 2, m, r, ql, qr));
    }
};

// --- D.10. Segment Tree Beats (min=, max= + add, запросы) ---
// Узел хранит max1/max2/cnt_max и min1/min2/cnt_min; chmin: если
// x >= max1 — выход; если max2 < x < max1 — пересчёт только
// максимумов за O(1); иначе спуск. Амортизированно O(log n).
// «%=», степень на отрезке и битовые операции — обобщение той же
// схемы (см. md): отсечение по второму экстремуму.
struct SegmentTreeBeats {
    static constexpr long long INF = 4000000000000000000LL;
    int n;
    vector<long long> sum, max1, max2, min1, min2, lazy_add;
    vector<int> cnt_max, cnt_min;

    void build(const vector<int>& a) {
        n = (int)a.size();
        sum.assign(4 * n + 4, 0);
        max1.assign(4 * n + 4, -INF);
        max2.assign(4 * n + 4, -INF);
        min1.assign(4 * n + 4, INF);
        min2.assign(4 * n + 4, INF);
        cnt_max.assign(4 * n + 4, 0);
        cnt_min.assign(4 * n + 4, 0);
        lazy_add.assign(4 * n + 4, 0);
        build_rec(0, 0, n, a);
    }
    void build_rec(int v, int l, int r, const vector<int>& a) {
        if (r - l == 1) {
            sum[v] = a[l];
            max1[v] = min1[v] = a[l];
            max2[v] = -INF;
            min2[v] = INF;
            cnt_max[v] = cnt_min[v] = 1;
            return;
        }
        int m = (l + r) / 2;
        build_rec(2 * v + 1, l, m, a);
        build_rec(2 * v + 2, m, r, a);
        pull(v);
    }
    void pull(int v) {
        int L = 2 * v + 1, R = 2 * v + 2;
        sum[v] = sum[L] + sum[R];
        if (max1[L] > max1[R]) {
            max1[v] = max1[L]; cnt_max[v] = cnt_max[L];
            max2[v] = max(max2[L], max1[R]);
        } else if (max1[L] < max1[R]) {
            max1[v] = max1[R]; cnt_max[v] = cnt_max[R];
            max2[v] = max(max1[L], max2[R]);
        } else {
            max1[v] = max1[L]; cnt_max[v] = cnt_max[L] + cnt_max[R];
            max2[v] = max(max2[L], max2[R]);
        }
        if (min1[L] < min1[R]) {
            min1[v] = min1[L]; cnt_min[v] = cnt_min[L];
            min2[v] = min(min2[L], min1[R]);
        } else if (min1[L] > min1[R]) {
            min1[v] = min1[R]; cnt_min[v] = cnt_min[R];
            min2[v] = min(min1[L], min2[R]);
        } else {
            min1[v] = min1[L]; cnt_min[v] = cnt_min[L] + cnt_min[R];
            min2[v] = min(min2[L], min2[R]);
        }
    }
    void apply_add(int v, int len, long long x) {
        sum[v] += x * len;
        max1[v] += x;
        if (max2[v] != -INF) max2[v] += x;
        min1[v] += x;
        if (min2[v] != INF) min2[v] += x;
        lazy_add[v] += x;
    }
    void apply_chmin(int v, long long x) {
        if (max1[v] <= x) return;
        sum[v] -= 1LL * (max1[v] - x) * cnt_max[v];
        if (min1[v] == max1[v]) min1[v] = x;
        else if (min2[v] == max1[v]) min2[v] = x;
        max1[v] = x;
    }
    void apply_chmax(int v, long long x) {
        if (min1[v] >= x) return;
        sum[v] += 1LL * (x - min1[v]) * cnt_min[v];
        if (max1[v] == min1[v]) max1[v] = x;
        else if (max2[v] == min1[v]) max2[v] = x;
        min1[v] = x;
    }
    void push(int v, int l, int r) {
        if (r - l == 1) { lazy_add[v] = 0; return; }
        int m = (l + r) / 2;
        if (lazy_add[v] != 0) {
            apply_add(2 * v + 1, m - l, lazy_add[v]);
            apply_add(2 * v + 2, r - m, lazy_add[v]);
            lazy_add[v] = 0;
        }
        if (max1[v] < max1[2 * v + 1]) apply_chmin(2 * v + 1, max1[v]);
        if (max1[v] < max1[2 * v + 2]) apply_chmin(2 * v + 2, max1[v]);
        if (min1[v] > min1[2 * v + 1]) apply_chmax(2 * v + 1, min1[v]);
        if (min1[v] > min1[2 * v + 2]) apply_chmax(2 * v + 2, min1[v]);
    }
    void range_chmin(int ql, int qr, long long x) { chmin_rec(0, 0, n, ql, qr, x); }
    void chmin_rec(int v, int l, int r, int ql, int qr, long long x) {
        if (qr <= l || r <= ql || max1[v] <= x) return;
        if (ql <= l && r <= qr && max2[v] < x) { apply_chmin(v, x); return; }
        push(v, l, r);
        int m = (l + r) / 2;
        chmin_rec(2 * v + 1, l, m, ql, qr, x);
        chmin_rec(2 * v + 2, m, r, ql, qr, x);
        pull(v);
    }
    void range_chmax(int ql, int qr, long long x) { chmax_rec(0, 0, n, ql, qr, x); }
    void chmax_rec(int v, int l, int r, int ql, int qr, long long x) {
        if (qr <= l || r <= ql || min1[v] >= x) return;
        if (ql <= l && r <= qr && min2[v] > x) { apply_chmax(v, x); return; }
        push(v, l, r);
        int m = (l + r) / 2;
        chmax_rec(2 * v + 1, l, m, ql, qr, x);
        chmax_rec(2 * v + 2, m, r, ql, qr, x);
        pull(v);
    }
    void range_add(int ql, int qr, long long x) { add_rec(0, 0, n, ql, qr, x); }
    void add_rec(int v, int l, int r, int ql, int qr, long long x) {
        if (qr <= l || r <= ql) return;
        if (ql <= l && r <= qr) { apply_add(v, r - l, x); return; }
        push(v, l, r);
        int m = (l + r) / 2;
        add_rec(2 * v + 1, l, m, ql, qr, x);
        add_rec(2 * v + 2, m, r, ql, qr, x);
        pull(v);
    }
    long long query_sum(int ql, int qr) { return sum_rec(0, 0, n, ql, qr); }
    long long sum_rec(int v, int l, int r, int ql, int qr) {
        if (qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr) return sum[v];
        push(v, l, r);
        int m = (l + r) / 2;
        return sum_rec(2 * v + 1, l, m, ql, qr) + sum_rec(2 * v + 2, m, r, ql, qr);
    }
    long long query_max(int ql, int qr) { return max_rec(0, 0, n, ql, qr); }
    long long max_rec(int v, int l, int r, int ql, int qr) {
        if (qr <= l || r <= ql) return -INF;
        if (ql <= l && r <= qr) return max1[v];
        push(v, l, r);
        int m = (l + r) / 2;
        return max(max_rec(2 * v + 1, l, m, ql, qr), max_rec(2 * v + 2, m, r, ql, qr));
    }
    long long query_min(int ql, int qr) { return min_rec(0, 0, n, ql, qr); }
    long long min_rec(int v, int l, int r, int ql, int qr) {
        if (qr <= l || r <= ql) return INF;
        if (ql <= l && r <= qr) return min1[v];
        push(v, l, r);
        int m = (l + r) / 2;
        return min(min_rec(2 * v + 1, l, m, ql, qr), min_rec(2 * v + 2, m, r, ql, qr));
    }
};

// --- D.11. Персистентное дерево отрезков (PST) ---
// Версии: обновление создаёт новые узлы только на пути от корня
// (O(log n) на версию), остальные поддеревья переиспользуются;
// узлы неизменяемы — старые версии остаются корректными.
struct PersistentSegmentTree {
    struct Node {
        int lc, rc;
        long long sum;
    };

    int n;
    vector<Node> pool;  // pool[0] — «пустой» узел

    PersistentSegmentTree(int n_ = 0) : n(n_) { pool.push_back({0, 0, 0}); }
    int new_node() { pool.push_back({0, 0, 0}); return (int)pool.size() - 1; }

    int build_empty(int l, int r) {
        int v = new_node();
        if (r - l > 1) {
            int m = (l + r) / 2;
            pool[v].lc = build_empty(l, m);
            pool[v].rc = build_empty(m, r);
        }
        return v;
    }
    int update(int root, int pos, long long x) { return update_rec(root, 0, n, pos, x); }
    int update_rec(int prev, int l, int r, int pos, long long x) {
        int v = new_node();
        pool[v] = pool[prev];  // копия: дети переиспользуются
        if (r - l == 1) { pool[v].sum = x; return v; }
        int m = (l + r) / 2;
        if (pos < m) pool[v].lc = update_rec(pool[prev].lc, l, m, pos, x);
        else pool[v].rc = update_rec(pool[prev].rc, m, r, pos, x);
        pool[v].sum = pool[pool[v].lc].sum + pool[pool[v].rc].sum;
        return v;
    }
    long long query(int root, int l, int r) const { return query_rec(root, 0, n, l, r); }
    long long query_rec(int v, int l, int r, int ql, int qr) const {
        if (v == 0 || qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr) return pool[v].sum;
        int m = (l + r) / 2;
        return query_rec(pool[v].lc, l, m, ql, qr) + query_rec(pool[v].rc, m, r, ql, qr);
    }
};

// --- D.12. 2D Segment Tree (дерево деревьев) ---
// Внешнее дерево по x (4n вершин), в каждой — внутреннее дерево
// по y (4m вершин); построение внутренних на внутренних узлах —
// слиянием детей (элементно по y-индексам).
struct SegmentTree2D {
    int n, m;
    vector<vector<long long>> t;

    void build(const vector<vector<int>>& a) {
        n = (int)a.size();
        m = n ? (int)a[0].size() : 0;
        t.assign(4 * n + 4, vector<long long>(4 * m + 4, 0));
        build_x(1, 0, n, a);
    }
    void build_x(int vx, int lx, int rx, const vector<vector<int>>& a) {
        if (rx - lx == 1) {
            build_y(vx, 1, 0, m, a[lx]);
            return;
        }
        int mx = (lx + rx) / 2;
        build_x(2 * vx, lx, mx, a);
        build_x(2 * vx + 1, mx, rx, a);
        for (int j = 1; j < 4 * m + 4; ++j)
            t[vx][j] = t[2 * vx][j] + t[2 * vx + 1][j];
    }
    void build_y(int vx, int vy, int ly, int ry, const vector<int>& row) {
        if (ry - ly == 1) { t[vx][vy] = row[ly]; return; }
        int my = (ly + ry) / 2;
        build_y(vx, 2 * vy, ly, my, row);
        build_y(vx, 2 * vy + 1, my, ry, row);
        t[vx][vy] = t[vx][2 * vy] + t[vx][2 * vy + 1];
    }
    void update(int px, int py, long long v) { update_x(1, 0, n, px, py, v); }
    void update_x(int vx, int lx, int rx, int px, int py, long long v) {
        if (rx - lx == 1) {
            update_y(vx, 1, 0, m, py, v);
            return;
        }
        int mx = (lx + rx) / 2;
        if (px < mx) update_x(2 * vx, lx, mx, px, py, v);
        else update_x(2 * vx + 1, mx, rx, px, py, v);
        // Внутреннее дерево внутреннего узла — сумма внутренних деревьев
        // детей (по y-индексам): пересчёт по пути y = py.
        int vy = 1, ly = 0, ry = m;
        while (ry - ly > 1) {
            int my = (ly + ry) / 2;
            if (py < my) { vy = 2 * vy; ry = my; }
            else { vy = 2 * vy + 1; ly = my; }
        }
        for (; vy >= 1; vy >>= 1)
            t[vx][vy] = t[2 * vx][vy] + t[2 * vx + 1][vy];
    }
    void update_y(int vx, int vy, int ly, int ry, int py, long long v) {
        if (ry - ly == 1) { t[vx][vy] = v; return; }
        int my = (ly + ry) / 2;
        if (py < my) update_y(vx, 2 * vy, ly, my, py, v);
        else update_y(vx, 2 * vy + 1, my, ry, py, v);
        t[vx][vy] = t[vx][2 * vy] + t[vx][2 * vy + 1];
    }
    long long query(int x1, int y1, int x2, int y2) const {
        return query_x(1, 0, n, x1, x2, y1, y2);
    }
    long long query_x(int vx, int lx, int rx, int qx1, int qx2, int qy1, int qy2) const {
        if (qx2 <= lx || rx <= qx1) return 0;
        if (qx1 <= lx && rx <= qx2) return query_y(vx, 1, 0, m, qy1, qy2);
        int mx = (lx + rx) / 2;
        return query_x(2 * vx, lx, mx, qx1, qx2, qy1, qy2)
             + query_x(2 * vx + 1, mx, rx, qx1, qx2, qy1, qy2);
    }
    long long query_y(int vx, int vy, int ly, int ry, int qy1, int qy2) const {
        if (qy2 <= ly || ry <= qy1) return 0;
        if (qy1 <= ly && ry <= qy2) return t[vx][vy];
        int my = (ly + ry) / 2;
        return query_y(vx, 2 * vy, ly, my, qy1, qy2)
             + query_y(vx, 2 * vy + 1, my, ry, qy1, qy2);
    }
};

// =============================================================
// E. ПРОДВИНУТЫЕ ДЕРЕВЬЯ
// =============================================================

// --- E.1. Range Tree (2D, обобщение до k измерений) ---
// Дерево по x; в узле — отсортированный массив y-координат точек
// поддерева (слияние детей); запрос — канонические x-узлы +
// бинарный поиск по y: O(log^2 n). k измерений — O(log^k n).
struct RangeTree2D {
    int n;
    vector<vector<int>> by_x, ys;  // ys[v] — sorted y точек узла v

    void build(const vector<pair<int, int>>& pts) {
        n = 0;
        for (auto& p : pts) n = max(n, p.first + 1);
        by_x.assign(n, {});
        for (auto& p : pts) by_x[p.first].push_back(p.second);
        ys.assign(4 * n + 4, {});
        build_rec(1, 0, n);
    }
    void build_rec(int v, int l, int r) {
        if (r - l == 1) {
            ys[v] = by_x[l];
            sort(ys[v].begin(), ys[v].end());
            return;
        }
        int m = (l + r) / 2;
        build_rec(2 * v, l, m);
        build_rec(2 * v + 1, m, r);
        ys[v].resize(ys[2 * v].size() + ys[2 * v + 1].size());
        merge(ys[2 * v].begin(), ys[2 * v].end(),
              ys[2 * v + 1].begin(), ys[2 * v + 1].end(), ys[v].begin());
    }
    int count(int x1, int x2, int y1, int y2) const {  // x в [x1,x2), y в [y1,y2)
        return count_rec(1, 0, n, x1, x2, y1, y2);
    }
    int count_rec(int v, int l, int r, int qx1, int qx2, int qy1, int qy2) const {
        if (qx2 <= l || r <= qx1) return 0;
        if (qx1 <= l && r <= qx2) {
            const vector<int>& vec = ys[v];
            return (int)(upper_bound(vec.begin(), vec.end(), qy2 - 1)
                       - lower_bound(vec.begin(), vec.end(), qy1));
        }
        int m = (l + r) / 2;
        return count_rec(2 * v, l, m, qx1, qx2, qy1, qy2)
             + count_rec(2 * v + 1, m, r, qx1, qx2, qy1, qy2);
    }
};

// --- E.2. Interval Tree (статические интервалы, stabbing-запросы) ---
// Узел с центром c хранит интервалы, содержащие c: отсортированные
// по l (lefts) и по r (rights); каждый интервал живёт ровно в одном
// узле. Запрос точки — спуск от корня с бинарными поисками.
struct IntervalTree {
    struct Node {
        int lo, hi;                 // диапазон координат узла
        int center;                 // координата-центр
        vector<int> lefts, rights;  // интервалы центра: по l и по r
        int left, right;            // индексы детей (-1 — нет)
    };

    vector<int> coords;
    vector<Node> nodes;

    void build(const vector<pair<int, int>>& ivs) {
        vector<int> tmp;
        for (auto& iv : ivs) { tmp.push_back(iv.first); tmp.push_back(iv.second); }
        sort(tmp.begin(), tmp.end());
        tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());
        coords = tmp;
        nodes.clear();
        build_rec(0, (int)coords.size() - 1);
        for (auto& iv : ivs) insert(iv);
        for (auto& nd : nodes) {
            sort(nd.lefts.begin(), nd.lefts.end());
            sort(nd.rights.begin(), nd.rights.end());
        }
    }
    int build_rec(int lo_i, int hi_i) {
        int id = (int)nodes.size();
        int mid = (lo_i + hi_i) / 2;
        nodes.push_back({coords[lo_i], coords[hi_i], coords[mid], {}, {}, -1, -1});
        if (lo_i < mid) nodes[id].left = build_rec(lo_i, mid - 1);
        if (mid < hi_i) nodes[id].right = build_rec(mid + 1, hi_i);
        return id;
    }
    void insert(const pair<int, int>& iv) {
        int v = 0;
        while (true) {
            Node& nd = nodes[v];
            if (nd.center >= iv.first && nd.center <= iv.second) {
                nd.lefts.push_back(iv.first);
                nd.rights.push_back(iv.second);
                return;
            }
            v = (iv.second < nd.center) ? nd.left : nd.right;
        }
    }
    int query(int p) const {  // число интервалов, содержащих p
        int v = 0, res = 0;
        while (true) {
            const Node& nd = nodes[v];
            if (p <= nd.center) {
                res += (int)(upper_bound(nd.lefts.begin(), nd.lefts.end(), p)
                           - nd.lefts.begin());
                if (p == nd.center || nd.left == -1) break;
                v = nd.left;
            } else {
                res += (int)(nd.rights.end()
                           - lower_bound(nd.rights.begin(), nd.rights.end(), p));
                if (nd.right == -1) break;
                v = nd.right;
            }
        }
        return res;
    }
};

// --- E.3. Segment Tree with Fractional Cascading ---
// Дерево с отсортированными массивами; в узле — переходы toL/toR:
// позиция первого >= s[i] в каждом из детей. Бинарный поиск — один
// раз в корне, дальше O(1) переходов по каскаду (запрос O(log n)).
struct FractionalCascading {
    struct Node {
        vector<int> s, toL, toR;
    };

    int n;
    vector<Node> t;

    void build(const vector<int>& a) {
        n = (int)a.size();
        t.assign(4 * n + 4, Node());
        build_rec(1, 0, n, a);
    }
    vector<int> build_rec(int v, int l, int r, const vector<int>& a) {
        if (r - l == 1) { t[v].s = {a[l]}; return t[v].s; }
        int m = (l + r) / 2;
        vector<int> L = build_rec(2 * v, l, m, a);
        vector<int> R = build_rec(2 * v + 1, m, r, a);
        t[v].s.resize(L.size() + R.size());
        merge(L.begin(), L.end(), R.begin(), R.end(), t[v].s.begin());
        t[v].toL.assign(t[v].s.size() + 1, (int)L.size());
        for (size_t i = 0; i < t[v].s.size(); ++i)
            t[v].toL[i] = (int)(lower_bound(L.begin(), L.end(), t[v].s[i]) - L.begin());
        t[v].toR.assign(t[v].s.size() + 1, (int)R.size());
        for (size_t i = 0; i < t[v].s.size(); ++i)
            t[v].toR[i] = (int)(lower_bound(R.begin(), R.end(), t[v].s[i]) - R.begin());
        return t[v].s;
    }
    int count_le(int ql, int qr, int x) const {  // a[i] <= x для i в [ql, qr)
        int res = 0;
        int pos = (int)(upper_bound(t[1].s.begin(), t[1].s.end(), x) - t[1].s.begin());
        count_rec(1, 0, n, ql, qr, x, pos, res);
        return res;
    }
    void count_rec(int v, int l, int r, int ql, int qr, int x, int pos, int& res) const {
        if (qr <= l || r <= ql) return;
        if (ql <= l && r <= qr) { res += pos; return; }
        int m = (l + r) / 2;
        int posL = t[v].toL[pos];
        while (posL > 0 && t[2 * v].s[posL - 1] > x) --posL;
        count_rec(2 * v, l, m, ql, qr, x, posL, res);
        int posR = t[v].toR[pos];
        while (posR > 0 && t[2 * v + 1].s[posR - 1] > x) --posR;
        count_rec(2 * v + 1, m, r, ql, qr, x, posR, res);
    }
};

// --- E.4. Li Chao Tree ---
// Динамическое дерево по x; в узле — прямая, лучшая в середине;
// вставка — спуск с обменами по знаку на границах; запрос — максимум
// прямых на пути. Диапазон x — параметр конструктора.
struct LiChaoTree {
    struct Line {
        long long k, b;
    };
    struct Node {
        long long k, b;
        int left, right;
    };

    long long xl, xr;
    const long long NEG = LLONG_MIN / 4;
    vector<Node> pool;
    int root;

    LiChaoTree(long long l = 0, long long r = 1) : xl(l), xr(r) {
        pool.push_back({0, NEG, 0, 0});  // pool[0] — «пустой» узел
        root = new_node();
    }
    int new_node() { pool.push_back({0, NEG, 0, 0}); return (int)pool.size() - 1; }

    long long eval(long long k, long long b, long long x) const { return k * x + b; }
    void insert(long long k, long long b) { insert_rec(root, xl, xr, k, b); }
    void insert_rec(int v, long long l, long long r, long long nk, long long nb) {
        if (pool[v].b == NEG) { pool[v] = {nk, nb, 0, 0}; return; }
        long long mid = l + (r - l) / 2;
        bool mid_better = eval(nk, nb, mid) > eval(pool[v].k, pool[v].b, mid);
        bool left_better = eval(nk, nb, l) > eval(pool[v].k, pool[v].b, l);
        if (mid_better) swap(pool[v].k, nk), swap(pool[v].b, nb);
        if (r - l == 1) return;
        if (left_better != mid_better) {
            if (!pool[v].left) pool[v].left = new_node();
            insert_rec(pool[v].left, l, mid, nk, nb);
        } else {
            if (!pool[v].right) pool[v].right = new_node();
            insert_rec(pool[v].right, mid + 1, r, nk, nb);
        }
    }
    long long query_max(long long x) const { return query_rec(root, xl, xr, x); }
    long long query_rec(int v, long long l, long long r, long long x) const {
        if (!v) return NEG;
        long long res = eval(pool[v].k, pool[v].b, x);
        if (r - l == 1) return res;
        long long mid = l + (r - l) / 2;
        if (x <= mid) return max(res, query_rec(pool[v].left, l, mid, x));
        return max(res, query_rec(pool[v].right, mid + 1, r, x));
    }
};

// --- E.5. Merge Sort Tree ---
// Узел хранит отсортированный массив своего отрезка (слияние детей);
// count_le — канонические узлы + upper_bound; kth — бинарный поиск
// по значению с проверкой count_le.
struct MergeSortTree {
    int n;
    vector<vector<int>> t;

    void build(const vector<int>& a) {
        n = (int)a.size();
        t.assign(4 * n + 4, {});
        build_rec(1, 0, n, a);
    }
    vector<int> build_rec(int v, int l, int r, const vector<int>& a) {
        if (r - l == 1) { t[v] = {a[l]}; return t[v]; }
        int m = (l + r) / 2;
        vector<int> L = build_rec(2 * v, l, m, a);
        vector<int> R = build_rec(2 * v + 1, m, r, a);
        t[v].resize(L.size() + R.size());
        merge(L.begin(), L.end(), R.begin(), R.end(), t[v].begin());
        return t[v];
    }
    int count_le(int ql, int qr, int x) const {
        return count_rec(1, 0, n, ql, qr, x);
    }
    int count_rec(int v, int l, int r, int ql, int qr, int x) const {
        if (qr <= l || r <= ql) return 0;
        if (ql <= l && r <= qr)
            return (int)(upper_bound(t[v].begin(), t[v].end(), x) - t[v].begin());
        int m = (l + r) / 2;
        return count_rec(2 * v, l, m, ql, qr, x) + count_rec(2 * v + 1, m, r, ql, qr, x);
    }
    int kth(int ql, int qr, int k) const {
        int lo = INT_MAX, hi = INT_MIN;
        for (const vector<int>& vec : t) {
            if (vec.empty()) continue;
            lo = min(lo, vec.front());
            hi = max(hi, vec.back());
        }
        int res = lo;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (count_le(ql, qr, mid) >= k) { res = mid; hi = mid - 1; }
            else lo = mid + 1;
        }
        return res;
    }
};

}; // конец struct RangeQueries

// =============================================================
// signed main() — демонстрация и проверка всех разделов A–E
// =============================================================

#ifndef STRUCT_E_MAIN
signed main() {
    using H = RangeQueries;

    cout << "=== A. ПРЕФИКСНЫЕ СУММЫ ===" << endl;

    // A.1 Prefix Sum (1D)
    H::PrefixSum1D ps1;
    ps1.build({1, 3, 5, 7, 9, 11});
    cout << "PrefixSum1D sum(1,4) = " << ps1.sum(1, 4) << ", sum(0,6) = " << ps1.sum(0, 6)
         << ", sum(2,5) = " << ps1.sum(2, 5) << " (ожидаем 15 36 21)" << endl;

    // A.2 2D Prefix Sum
    vector<vector<int>> mat3x4 = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };
    H::PrefixSum2D ps2;
    ps2.build(mat3x4);
    cout << "PrefixSum2D sum(0,0,2,3) = " << ps2.sum(0, 0, 2, 3)
         << ", sum(1,1,3,4) = " << ps2.sum(1, 1, 3, 4)
         << ", sum(0,2,3,3) = " << ps2.sum(0, 2, 3, 3)
         << " (ожидаем 24 54 21)" << endl;

    // A.3 3D Prefix Sum (куб 2x2x2)
    vector<vector<vector<int>>> cube = {
        {{1, 2}, {3, 4}},
        {{5, 6}, {7, 8}}
    };
    H::PrefixSum3D ps3;
    ps3.build(cube);
    cout << "PrefixSum3D sum(0,0,0,2,2,2) = " << ps3.sum(0, 0, 0, 2, 2, 2)
         << ", sum(0,0,0,1,2,2) = " << ps3.sum(0, 0, 0, 1, 2, 2)
         << ", sum(1,1,1,2,2,2) = " << ps3.sum(1, 1, 1, 2, 2, 2)
         << ", sum(0,1,0,2,2,2) = " << ps3.sum(0, 1, 0, 2, 2, 2)
         << " (ожидаем 36 10 8 22)" << endl;

    // A.4 Difference Array
    H::DifferenceArray da;
    da.build({0, 0, 0, 0, 0});
    da.add(1, 3, 5);
    da.add(0, 5, 1);
    da.add(2, 4, -3);
    vector<long long> restored = da.restore();
    cout << "DifferenceArray after adds: ";
    for (long long x : restored) cout << x << " ";
    cout << "| value(3) = " << da.value(3) << " (ожидаем 1 6 3 -2 1 | -2)" << endl;

    // A.5 Prefix Sum with Updates: пересборка префиксов vs Fenwick
    {
        vector<int> a = {1, 3, 5, 7, 9, 11};
        a[2] = 10;  // обновление точки
        H::PrefixSum1D rebuild;
        rebuild.build(a);
        H::Fenwick fw(6);
        for (int i = 0; i < 6; ++i) fw.add(i + 1, (i == 2 ? 10 : a[i]));
        cout << "A.5 пересборка префиксов = " << rebuild.sum(1, 4)
             << ", Fenwick = " << fw.sum(2, 5)
             << " (ожидаем 20 29)" << endl;
    }

    cout << "\n=== B. ДЕРЕВО ФЕНВИКА ===" << endl;

    // B.1 Standard Fenwick (Point Update, Range Query)
    H::Fenwick f1(5);
    for (int x : {3, 1, 4, 1, 5}) { static int i = 1; f1.add(i++, x); }
    f1.add(2, 10);
    cout << "Fenwick sum(1,4) = " << f1.sum(1, 4) << ", prefix(3) = " << f1.prefix_sum(3)
         << ", sum(1,5) = " << f1.sum(1, 5) << " (ожидаем 19 18 24)" << endl;

    // B.2 Maximum Fenwick Tree
    H::FenwickMax fmx;
    fmx.build({3, 1, 4, 1, 5, 9, 2});
    cout << "FenwickMax prefix_max(3) = " << fmx.prefix_max(3)
         << ", prefix_max(6) = " << fmx.prefix_max(6) << " (ожидаем 4 9)" << endl;
    fmx.update(5, 12);
    cout << "FenwickMax after update(5,12): prefix_max(5) = " << fmx.prefix_max(5)
         << ", prefix_max(7) = " << fmx.prefix_max(7) << " (ожидаем 12 12)" << endl;

    // B.3 Minimum Fenwick Tree
    H::FenwickMin fmn;
    fmn.build({3, 1, 4, 1, 5, 9, 2});
    cout << "FenwickMin prefix_min(3) = " << fmn.prefix_min(3)
         << ", prefix_min(7) = " << fmn.prefix_min(7) << " (ожидаем 1 1)" << endl;
    fmn.update(6, 0);
    fmn.update(2, 0);
    cout << "FenwickMin after update(6,0),update(2,0): prefix_min(3) = " << fmn.prefix_min(3)
         << ", prefix_min(7) = " << fmn.prefix_min(7) << " (ожидаем 0 0)" << endl;

    // B.4 2D Fenwick Tree
    H::Fenwick2D f2d(3, 4);
    for (int i = 1; i <= 3; ++i)
        for (int j = 1; j <= 4; ++j)
            f2d.add(i, j, mat3x4[i - 1][j - 1]);
    cout << "Fenwick2D sum(1,1,2,3) = " << f2d.sum(1, 1, 2, 3)
         << ", sum(2,2,3,4) = " << f2d.sum(2, 2, 3, 4) << " (ожидаем 24 54)" << endl;
    f2d.add(3, 4, 5);
    cout << "Fenwick2D after add(3,4,5): sum(3,3,3,4) = " << f2d.sum(3, 3, 3, 4)
         << " (ожидаем 28)" << endl;

    // B.5 Fenwick with Range Updates
    H::FenwickRangeUpdatePointQuery frp(6);
    frp.add_range(1, 3, 5);
    frp.add_range(2, 6, 1);
    cout << "FenwickRUPQ point(2) = " << frp.point(2) << ", point(3) = " << frp.point(3)
         << ", point(6) = " << frp.point(6) << " (ожидаем 6 6 1)" << endl;
    H::FenwickRangeUpdateRangeQuery frr(6);
    frr.add_range(1, 3, 5);
    cout << "FenwickRURQ sum_range(1,3) = " << frr.sum_range(1, 3)
         << ", sum_range(1,6) = " << frr.sum_range(1, 6) << " (ожидаем 15 15)" << endl;
    frr.add_range(2, 6, 1);
    cout << "FenwickRURQ after add(2,6,1): sum_range(2,4) = " << frr.sum_range(2, 4)
         << ", sum_range(1,6) = " << frr.sum_range(1, 6) << " (ожидаем 13 20)" << endl;

    // B.6 Fenwick Tree on Fenwick Tree (разреженный)
    H::FenwickOnFenwick fof(4);
    for (int i = 1; i <= 3; ++i)
        for (int j = 1; j <= 4; ++j)
            fof.add(i, j, mat3x4[i - 1][j - 1]);
    cout << "FenwickOnFenwick sum(1,1,2,3) = " << fof.sum(1, 1, 2, 3)
         << ", sum(2,2,3,4) = " << fof.sum(2, 2, 3, 4) << " (ожидаем 24 54)" << endl;

    // B.7 k-й по сумме
    H::Fenwick fk(5);
    fk.add(1, 2); fk.add(3, 3); fk.add(4, 1); fk.add(5, 4);
    cout << "Fenwick kth(1) = " << fk.kth(1) << ", kth(3) = " << fk.kth(3)
         << ", kth(6) = " << fk.kth(6) << ", kth(10) = " << fk.kth(10)
         << " (ожидаем 1 3 4 5)" << endl;

    cout << "\n=== C. РАЗРЕЖЕННАЯ ТАБЛИЦА ===" << endl;

    // C.1 Sparse Table RMQ
    H::SparseTableRMQ st1;
    st1.build({3, 1, 4, 1, 5, 9, 2, 6});
    cout << "SparseTableRMQ query(1,4) = " << st1.query(1, 4)
         << ", query(3,7) = " << st1.query(3, 7) << ", query(0,8) = " << st1.query(0, 8)
         << " (ожидаем 1 1 1)" << endl;

    // C.2 Sparse Table GCD
    H::SparseTableGCD stg;
    stg.build({12, 18, 24, 36, 48, 60});
    cout << "SparseTableGCD query(0,6) = " << stg.query(0, 6)
         << ", query(1,4) = " << stg.query(1, 4) << ", query(2,5) = " << stg.query(2, 5)
         << " (ожидаем 6 6 12)" << endl;

    // C.3 Sparse Table Generic (min и max)
    H::SparseTableGeneric stgn;
    stgn.build({3, 1, 4, 1, 5, 9, 2, 6},
               [](int a, int b) { return a < b ? a : b; });
    cout << "SparseTableGeneric min query(1,4) = " << stgn.query(1, 4)
         << ", query(4,8) = " << stgn.query(4, 8) << " (ожидаем 1 2)" << endl;
    stgn.build({3, 1, 4, 1, 5, 9, 2, 6},
               [](int a, int b) { return a > b ? a : b; });
    cout << "SparseTableGeneric max query(1,4) = " << stgn.query(1, 4)
         << ", query(0,8) = " << stgn.query(0, 8) << " (ожидаем 4 9)" << endl;

    // C.4 Двумерная Sparse Table
    H::SparseTable2D st2d;
    st2d.build(mat3x4);
    cout << "SparseTable2D query(0,0,2,2) = " << st2d.query(0, 0, 2, 2)
         << ", query(1,1,3,3) = " << st2d.query(1, 1, 3, 3)
         << ", query(0,2,2,4) = " << st2d.query(0, 2, 2, 4)
         << " (ожидаем 1 6 3)" << endl;

    // C.5 Disjoint Sparse Table (сумма — неидемпотентная операция)
    H::DisjointSparseTable dst;
    dst.build({1, 3, 5, 7, 9, 11});
    cout << "DisjointST query(1,5) = " << dst.query(1, 5)
         << ", query(0,6) = " << dst.query(0, 6) << ", query(2,6) = " << dst.query(2, 6)
         << " (ожидаем 24 36 32)" << endl;

    // C.6 Sparse Table с координатным сжатием
    H::SparseTableCompressed stc;
    stc.build({{1, 3}, {10, 5}, {100, 7}, {5, 2}, {10, 1}});
    cout << "SparseTableCompressed query(0,6) = " << stc.query(0, 6)
         << ", query(6,20) = " << stc.query(6, 20)
         << ", query(0,1000) = " << stc.query(0, 1000)
         << ", query(0,2) = " << stc.query(0, 2)
         << " (ожидаем 2 1 1 3)" << endl;

    cout << "\n=== D. ДЕРЕВО ОТРЕЗКОВ ===" << endl;

    // D.1 Segment Tree (на массиве)
    H::SegmentTreeArray sta;
    sta.build({1, 3, 5, 7, 9, 11});
    cout << "SegmentTreeArray query(1,4) = " << sta.query(1, 4) << " (ожидаем 15)" << endl;
    sta.update(3, 10);
    cout << "SegmentTreeArray after update(3,10): query(0,6) = " << sta.query(0, 6)
         << ", query(2,5) = " << sta.query(2, 5) << " (ожидаем 39 24)" << endl;

    // D.2 Segment Tree (на указателях)
    H::SegmentTreePointers stp;
    stp.build({1, 3, 5, 7, 9, 11});
    cout << "SegmentTreePointers query(1,4) = " << stp.query(1, 4) << " (ожидаем 15)" << endl;
    stp.update(3, 10);
    cout << "SegmentTreePointers after update: query(0,6) = " << stp.query(0, 6)
         << " (ожидаем 39)" << endl;

    // D.3 Non Recursive Segment Tree
    H::SegmentTreeIterative sti;
    sti.build({1, 3, 5, 7, 9, 11});
    cout << "SegmentTreeIterative query(1,4) = " << sti.query(1, 4) << " (ожидаем 15)" << endl;
    sti.update(3, 10);
    cout << "SegmentTreeIterative after update: query(0,6) = " << sti.query(0, 6)
         << " (ожидаем 39)" << endl;

    // D.4 Segment Tree Other: спуск к k-му по сумме
    cout << "SegmentTreeIterative kth(13) = " << sti.kth(13) << ", kth(9) = " << sti.kth(9)
         << " (ожидаем 3 2)" << endl;

    // D.5 Динамическое дерево отрезков (без сжатия)
    H::SegmentTreeDynamic stdyn(1000);
    stdyn.update(100, 5);
    stdyn.update(500, 3);
    stdyn.update(999, 7);
    cout << "SegmentTreeDynamic query(0,1000) = " << stdyn.query(0, 1000)
         << ", query(50,150) = " << stdyn.query(50, 150)
         << ", query(100,999) = " << stdyn.query(100, 999)
         << " (ожидаем 15 5 8)" << endl;
    stdyn.update(100, 10);
    cout << "SegmentTreeDynamic after update(100,10): query(0,1000) = "
         << stdyn.query(0, 1000) << " (ожидаем 20)" << endl;

    // D.5.1 Сжатое по координатам дерево отрезков
    H::SegmentTreeCompressed stcmp;
    stcmp.set_coords({1, 5, 10, 20, 100});
    stcmp.build({{1, 3}, {10, 5}, {100, 7}});
    cout << "SegmentTreeCompressed query(0,1000) = " << stcmp.query(0, 1000)
         << ", query(5,11) = " << stcmp.query(5, 11)
         << ", query(1,20) = " << stcmp.query(1, 20)
         << " (ожидаем 15 5 8)" << endl;
    stcmp.update(10, 10);
    cout << "SegmentTreeCompressed after update(10,10): query(0,1000) = "
         << stcmp.query(0, 1000) << " (ожидаем 20)" << endl;

    // D.6 Lazy: Add on segment, sum on segment
    H::LazySegTreeAdd lza;
    lza.build({1, 3, 5, 7, 9, 11});
    lza.add_segment(1, 5, 10);
    cout << "LazySegTreeAdd after add(1,5,10): sum(0,6) = " << lza.query(0, 6)
         << ", sum(1,4) = " << lza.query(1, 4) << " (ожидаем 76 45)" << endl;
    lza.add_segment(0, 6, 1);
    cout << "LazySegTreeAdd after add(0,6,1): sum(0,6) = " << lza.query(0, 6)
         << ", sum(5,6) = " << lza.query(5, 6) << " (ожидаем 82 12)" << endl;

    // D.7 Lazy: Assign on segment
    H::LazySegTreeAssign lzs;
    lzs.build({1, 3, 5, 7, 9, 11});
    lzs.assign(1, 5, 7);
    lzs.assign(2, 4, 0);
    cout << "LazySegTreeAssign sum(0,6) = " << lzs.query(0, 6)
         << ", sum(1,3) = " << lzs.query(1, 3) << " (ожидаем 26 7)" << endl;
    lzs.assign(0, 6, 3);
    cout << "LazySegTreeAssign after assign(0,6,3): sum(0,6) = " << lzs.query(0, 6)
         << " (ожидаем 18)" << endl;

    // D.8 Lazy: Add and Assign combination
    H::LazySegTreeAddAssign lzc;
    lzc.build({1, 3, 5, 7, 9, 11});
    lzc.assign(1, 4, 100);
    lzc.add_segment(0, 6, 10);
    cout << "LazySegTreeAddAssign sum(0,6) = " << lzc.query(0, 6) << " (ожидаем 381)" << endl;
    lzc.add_segment(2, 5, 5);
    lzc.assign(3, 6, 0);
    cout << "LazySegTreeAddAssign sum(2,4) = " << lzc.query(2, 4)
         << ", sum(0,6) = " << lzc.query(0, 6) << " (ожидаем 115 236)" << endl;
    lzc.assign(1, 2, 50);   // set перекрывает add
    lzc.add_segment(1, 3, 7);
    cout << "LazySegTreeAddAssign set+add: sum(0,3) = " << lzc.query(0, 3)
         << ", sum(0,6) = " << lzc.query(0, 6) << " (ожидаем 190 190)" << endl;

    // D.9 Массовые операции: параметризация свёртки
    H::SegmentTreeGeneric segm;
    segm.build({1, 3, 5, 7, 9, 11},
               [](int a, int b) { return a < b ? a : b; }, INT_MAX);
    cout << "SegmentTreeGeneric min query(1,4) = " << segm.query(1, 4)
         << " (ожидаем 3)" << endl;
    segm.update(3, 10);
    cout << "SegmentTreeGeneric min after update: query(2,5) = " << segm.query(2, 5)
         << " (ожидаем 5)" << endl;
    segm.build({12, 18, 24, 36, 48, 60},
               [](int a, int b) { return gcd(a, b); }, 0);
    cout << "SegmentTreeGeneric gcd query(0,6) = " << segm.query(0, 6)
         << ", query(1,4) = " << segm.query(1, 4) << " (ожидаем 6 6)" << endl;
    segm.update(2, 25);
    cout << "SegmentTreeGeneric gcd after update(2,25): query(0,6) = " << segm.query(0, 6)
         << " (ожидаем 1)" << endl;

    // D.10 Segment Tree Beats
    H::SegmentTreeBeats beats;
    beats.build({1, 3, 5, 7, 9, 11});
    beats.range_chmin(2, 5, 8);
    cout << "Beats after chmin(2,5,8): sum = " << beats.query_sum(0, 6)
         << ", max = " << beats.query_max(0, 6) << " (ожидаем 35 11)" << endl;
    beats.range_add(0, 6, 2);
    cout << "Beats after add(0,6,2): sum = " << beats.query_sum(0, 6)
         << " (ожидаем 47)" << endl;
    beats.range_chmax(0, 6, 8);
    cout << "Beats after chmax(0,6,8): sum = " << beats.query_sum(0, 6)
         << ", min = " << beats.query_min(0, 6) << " (ожидаем 56 8)" << endl;
    beats.range_chmin(0, 3, 4);
    cout << "Beats after chmin(0,3,4): sum = " << beats.query_sum(0, 6)
         << " (ожидаем 44)" << endl;
    beats.range_chmax(3, 6, 12);
    cout << "Beats after chmax(3,6,12): sum = " << beats.query_sum(0, 6)
         << ", max = " << beats.query_max(0, 6) << " (ожидаем 49 13)" << endl;

    // D.11 Персистентное дерево отрезков
    H::PersistentSegmentTree pst(6);
    int r0 = pst.build_empty(0, 6);
    int r1 = pst.update(r0, 2, 100);
    int r2 = pst.update(r1, 5, 200);
    cout << "PST query(v2,0,6) = " << pst.query(r2, 0, 6)
         << ", query(v1,0,6) = " << pst.query(r1, 0, 6)
         << ", query(v0,0,6) = " << pst.query(r0, 0, 6)
         << " (ожидаем 300 100 0)" << endl;
    cout << "PST query(v2,1,4) = " << pst.query(r2, 1, 4)
         << ", query(v2,5,6) = " << pst.query(r2, 5, 6)
         << " (ожидаем 100 200)" << endl;

    // D.12 2D Segment Tree (дерево деревьев)
    H::SegmentTree2D s2d;
    s2d.build(mat3x4);
    cout << "SegmentTree2D sum(0,0,2,3) = " << s2d.query(0, 0, 2, 3)
         << ", sum(1,1,3,4) = " << s2d.query(1, 1, 3, 4)
         << " (ожидаем 24 54)" << endl;
    s2d.update(2, 3, 50);
    cout << "SegmentTree2D after update(2,3,50): sum(2,2,3,4) = " << s2d.query(2, 2, 3, 4)
         << ", sum(0,0,3,4) = " << s2d.query(0, 0, 3, 4)
         << " (ожидаем 61 116)" << endl;

    cout << "\n=== E. ПРОДВИНУТЫЕ ДЕРЕВЬЯ ===" << endl;

    // E.1 Range Tree (2D)
    H::RangeTree2D rt;
    rt.build({{0, 1}, {0, 3}, {1, 5}, {2, 2}, {3, 4}, {3, 0}, {4, 3}});
    cout << "RangeTree2D count(1,4,2,6) = " << rt.count(1, 4, 2, 6)
         << ", count(0,5,0,2) = " << rt.count(0, 5, 0, 2)
         << ", count(2,5,3,6) = " << rt.count(2, 5, 3, 6)
         << ", count(0,5,0,10) = " << rt.count(0, 5, 0, 10)
         << " (ожидаем 3 2 2 7)" << endl;

    // E.2 Interval Tree
    H::IntervalTree it;
    it.build({{1, 3}, {2, 6}, {3, 4}, {4, 8}, {5, 5}, {7, 9}});
    cout << "IntervalTree query(3) = " << it.query(3) << ", query(5) = " << it.query(5)
         << ", query(7) = " << it.query(7) << ", query(10) = " << it.query(10)
         << ", query(2) = " << it.query(2)
         << " (ожидаем 3 3 2 0 2)" << endl;

    // E.3 Segment Tree with Fractional Cascading
    H::FractionalCascading fc;
    fc.build({3, 1, 4, 1, 5, 9, 2, 6});
    cout << "FractionalCascading count_le(0,8,4) = " << fc.count_le(0, 8, 4)
         << ", count_le(2,6,3) = " << fc.count_le(2, 6, 3)
         << ", count_le(1,7,6) = " << fc.count_le(1, 7, 6)
         << ", count_le(0,8,1) = " << fc.count_le(0, 8, 1)
         << " (ожидаем 5 1 5 2)" << endl;

    // E.4 Li Chao Tree
    H::LiChaoTree lc(0, 10);
    lc.insert(2, 1);
    lc.insert(-1, 9);
    lc.insert(3, -5);
    cout << "LiChaoTree max(1) = " << lc.query_max(1) << ", max(3) = " << lc.query_max(3)
         << ", max(5) = " << lc.query_max(5) << ", max(9) = " << lc.query_max(9)
         << ", max(0) = " << lc.query_max(0)
         << " (ожидаем 8 7 11 22 9)" << endl;

    // E.5 Merge Sort Tree
    H::MergeSortTree mst;
    mst.build({3, 1, 4, 1, 5, 9, 2, 6});
    cout << "MergeSortTree count_le(2,6,3) = " << mst.count_le(2, 6, 3)
         << ", count_le(0,8,5) = " << mst.count_le(0, 8, 5)
         << " (ожидаем 1 6)" << endl;
    cout << "MergeSortTree kth(2,6,2) = " << mst.kth(2, 6, 2)
         << ", kth(0,8,3) = " << mst.kth(0, 8, 3)
         << ", kth(0,8,5) = " << mst.kth(0, 8, 5)
         << ", kth(4,8,2) = " << mst.kth(4, 8, 2)
         << " (ожидаем 4 2 4 5)" << endl;

    cout << "\n=== ОБЩЕЕ: одинаковые серии ===" << endl;
    vector<int> base = {1, 3, 5, 7, 9, 11};
    vector<int> want_static = {15, 36, 32};       // sum(1,4), sum(0,6), sum(2,6)
    vector<int> want_upd = {20, 41, 37};          // после update(2, 10)

    auto check = [](const char* name, const vector<int>& got, const vector<int>& want) {
        cout << name << " series match (expect 1) = " << (got == want) << endl;
    };

    auto static_answers = [&](auto&& q) {
        return vector<int>{(int)q(1, 4), (int)q(0, 6), (int)q(2, 6)};
    };

    { H::PrefixSum1D t; t.build(base); check("PrefixSum1D", static_answers([&](int l, int r) { return t.sum(l, r); }), want_static); }
    { H::DisjointSparseTable t; t.build(base); check("DisjointST", static_answers([&](int l, int r) { return t.query(l, r); }), want_static); }
    { H::Fenwick t(6); for (int i = 0; i < 6; ++i) t.add(i + 1, base[i]); t.add(3, 5);
      check("Fenwick", static_answers([&](int l, int r) { return t.sum(l + 1, r); }), want_upd); }
    { H::SegmentTreeArray t; t.build(base); t.update(2, 10);
      check("SegmentTreeArray", static_answers([&](int l, int r) { return t.query(l, r); }), want_upd); }
    { H::SegmentTreePointers t; t.build(base); t.update(2, 10);
      check("SegmentTreePointers", static_answers([&](int l, int r) { return (int)t.query(l, r); }), want_upd); }
    { H::SegmentTreeIterative t; t.build(base); t.update(2, 10);
      check("SegmentTreeIterative", static_answers([&](int l, int r) { return t.query(l, r); }), want_upd); }
    { H::SegmentTreeDynamic t(6); for (int i = 0; i < 6; ++i) t.update(i, base[i]); t.update(2, 10);
      check("SegmentTreeDynamic", static_answers([&](int l, int r) { return (int)t.query(l, r); }), want_upd); }
    { H::SegmentTreeGeneric t; t.build(base, [](int a, int b) { return a + b; }, 0); t.update(2, 10);
      check("SegmentTreeGeneric", static_answers([&](int l, int r) { return t.query(l, r); }), want_upd); }
    { H::PersistentSegmentTree t(6); int r = t.build_empty(0, 6);
      for (int i = 0; i < 6; ++i) r = t.update(r, i, base[i]);
      r = t.update(r, 2, 10);
      check("PersistentST", static_answers([&](int l, int rq) { return (int)t.query(r, l, rq); }), want_upd); }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_E_MAIN

#endif // STRUCT_E_CPP
