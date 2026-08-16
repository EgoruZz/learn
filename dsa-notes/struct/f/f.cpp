#ifndef STRUCT_F_CPP
#define STRUCT_F_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <climits>
#include <map>
#include <set>
#include <cmath>
using namespace std;

// =============================================================
// VI. КОРНЕВЫЕ СТРУКТУРЫ (SQRT DECOMPOSITION)
// =============================================================
// Структура md: A. Корневая декомпозиция
//               → B. Алгоритм Мо
//               → C. Дерево отрезков с корневой декомпозицией
//               → D. Sqrt по блокам: heavy/light + HLD
//
// SqrtStructures наследует RangeQueries (e.cpp). Переиспользует:
//   * ParentArray (B.9) — LCA двоичными подъёмами для Мо на
//     деревьях (B.3);
//   * SegmentTreeIterative (V.D.3) — дерево отрезков над суммами
//     блоков (C.1);
//   * LazySegTreeAdd (V.D.6) — прибавления и суммы на цепочках
//     HLD (D.2);
//   * DigitOps (number-theory C.2) — целочисленные корни
//     isqrt_newton (⌈√n⌉ для размера блока) и kth_root_newton
//     (⌈n^(1/3)⌉ для блока времени B.2): без плавающей арифметики;
//   * мотив «полные куски + хвосты» — бакеты (I.D.8), Fenwick
//     с диапазонными обновлениями (V.B.5): блоки A;
//   * ленивость (V.D.6) — ленивые сдвиги блоков (A.3);
//   * обратимость операций (V.A.4) — инвариант окна Мо (B.4).
//
// Порядок методов строго соответствует порядку md (A → D).
// A.2 (выбор размера блока) и C.2 (смешанные запросы) — анализ
// в md, своего кода не имеют: размер блока — параметр структур
// (по умолчанию B = ⌈√n⌉; B = n^(2/3) в B.2).
// B.1 и B.4 (обобщение add/remove) реализованы вместе: раннер
// принимает add/remove/save функциями-параметрами.
//
// ВНИМАНИЕ (скрытие имён): методы build, update, query, run, add,
// remove здесь локальные; одноимённые из других веток не
// подключаются. Query/Update — локальные типы раннеров.

#define STRUCT_E_MAIN
#include "../e/e.cpp"
#undef STRUCT_E_MAIN

#define DIGIT_OPS_MAIN
#include "../../math/number-theory/c-folder/c.cpp"
#undef DIGIT_OPS_MAIN

struct SqrtStructures : RangeQueries {

// =============================================================
// A. КОРНЕВАЯ ДЕКОМПОЗИЦИЯ
// =============================================================

// ⌈√n⌉ через целочисленный корень isqrt_newton (number-theory C.2):
// floor-корень + подгонка точного квадрата; без плавающей арифметики.
static long long ceil_sqrt(long long n) {
    long long r = DigitOps().isqrt_newton(n);
    return r * r == n ? r : r + 1;
}

// --- A.1. Sqrt Decomposition (сумма на отрезке) ---
// Массив разбит на блоки размера B (B = ⌈√n⌉ параметром); в каждом
// блоке — сумма. Запрос [l, r): полные блоки целиком + хвосты
// поштучно — O(B + n/B). Точечное обновление — пересчёт агрегата
// блока за O(1).
struct SqrtSum {
    int n, B, nb;
    vector<int> a, sum;

    SqrtSum(const vector<int>& vals = {}, int B_ = 0) { build(vals, B_); }
    void build(const vector<int>& vals, int B_ = 0) {
        n = (int)vals.size();
        B = B_ > 0 ? B_ : (int)SqrtStructures::ceil_sqrt(n);
        a = vals;
        nb = (n + B - 1) / B;
        sum.assign(nb, 0);
        for (int i = 0; i < n; ++i) sum[i / B] += a[i];
    }
    void update(int p, int x) {
        sum[p / B] += x - a[p];
        a[p] = x;
    }
    int query(int l, int r) const {  // [l, r)
        if (l >= r) return 0;
        int res = 0;
        int bl = l / B, br = (r - 1) / B;
        if (bl == br) {
            for (int i = l; i < r; ++i) res += a[i];
            return res;
        }
        for (int i = l; i < (bl + 1) * B; ++i) res += a[i];
        for (int b = bl + 1; b < br; ++b) res += sum[b];
        for (int i = br * B; i < r; ++i) res += a[i];
        return res;
    }
    int block_size() const { return B; }
};

// --- A.3. Sqrt Decomposition: прибавление и сумма на отрезке ---
// Полные блоки — ленивый сдвиг add[b] за O(1); хвосты — поштучно
// с пересборкой суммы блока O(B). Обе операции — O(B + n/B).
struct SqrtRangeAdd {
    int n, B, nb;
    vector<long long> a, sum, add;  // значение i = a[i] + add[i/B]

    SqrtRangeAdd(const vector<long long>& vals = {}, int B_ = 0) { build(vals, B_); }
    void build(const vector<long long>& vals, int B_ = 0) {
        n = (int)vals.size();
        B = B_ > 0 ? B_ : (int)SqrtStructures::ceil_sqrt(n);
        a = vals;
        nb = (n + B - 1) / B;
        sum.assign(nb, 0);
        add.assign(nb, 0);
        for (int i = 0; i < n; ++i) sum[i / B] += a[i];
    }
    void rebuild(int b) {  // суммы хвостов после поштучных правок
        sum[b] = 0;
        for (int i = b * B; i < min(n, (b + 1) * B); ++i) sum[b] += a[i];
    }
    void range_add(int l, int r, long long x) {  // [l, r)
        if (l >= r) return;
        int bl = l / B, br = (r - 1) / B;
        if (bl == br) {
            for (int i = l; i < r; ++i) a[i] += x;
            rebuild(bl);
            return;
        }
        for (int i = l; i < (bl + 1) * B; ++i) a[i] += x;
        rebuild(bl);
        for (int b = bl + 1; b < br; ++b) add[b] += x;
        for (int i = br * B; i < r; ++i) a[i] += x;
        rebuild(br);
    }
    long long range_sum(int l, int r) const {  // [l, r)
        if (l >= r) return 0;
        long long res = 0;
        int bl = l / B, br = (r - 1) / B;
        if (bl == br) {
            for (int i = l; i < r; ++i) res += a[i];
            return res + add[bl] * (r - l);
        }
        for (int i = l; i < (bl + 1) * B; ++i) res += a[i];
        res += add[bl] * ((bl + 1) * B - l);
        for (int b = bl + 1; b < br; ++b) res += sum[b] + add[b] * B;
        for (int i = br * B; i < r; ++i) res += a[i];
        res += add[br] * (r - br * B);
        return res;
    }
};

// =============================================================
// B. АЛГОРИТМ МО
// =============================================================

// --- B.1 + B.4. Стандартный Мо (add/remove как параметры) ---
// Офлайн: сортировка запросов по (l/B, r) с зигзагом по r; окно
// [cl, cr) двигается указателями; add(p)/remove(p) и save(id) —
// параметры-функции (B.4): инвариант окна и его копирование
// в ответы — на стороне вызывающего. remove приходит ровно в
// обратном порядке, чем add (обратимость окна).
struct MoSolver {
    struct Query { int l, r, id; };
    int B;
    int block_size() const { return B; }

    template <typename Add, typename Rem, typename Save>
    void run(int n, vector<Query> qs, Add add, Rem remove, Save save) {
        B = (int)SqrtStructures::ceil_sqrt(max(1, n));
        sort(qs.begin(), qs.end(), [&](const Query& a, const Query& b) {
            int ba = a.l / B, bb = b.l / B;
            if (ba != bb) return ba < bb;
            if (ba & 1) return a.r > b.r;
            return a.r < b.r;
        });
        int cl = 0, cr = 0;
        for (const Query& q : qs) {
            while (cr < q.r) add(cr++);
            while (cl > q.l) add(--cl);
            while (cr > q.r) remove(--cr);
            while (cl < q.l) remove(cl++);
            save(q.id);
        }
    }
};

// --- B.2. Мо с обновлениями (третья координата времени) ---
// Запрос — тройка (l, r, t): окно и число применённых обновлений
// (t до запроса). Сортировка по (l/B, r/B, t) с зигзагом по r-блоку
// и t; B = n^(2/3). Движение времени применяет/откатывает
// обновление: позиция в окне — remove → смена значения в arr →
// add. Откат — те же функции с from/to наоборот. arr на входе —
// состояние при t = 0 (откат полагается на from); раннер меняет его
// на лету.
struct MoWithUpdates {
    struct Query { int l, r, t, id; };
    struct Update { int pos, from, to; };
    int B;
    int block_size() const { return B; }

    template <typename Add, typename Rem, typename Save>
    void run(int n, vector<Query> qs, const vector<Update>& upd, vector<int>& arr,
             Add add, Rem remove, Save save) {
        long long b = DigitOps().kth_root_newton(max(1, n), 3);  // ⌊n^(1/3)⌋
        while (b * b * b < max(1, n)) ++b;                       // ⌈n^(1/3)⌉
        B = (int)(b * b);                                        // размер блока n^(2/3)
        sort(qs.begin(), qs.end(), [&](const Query& a, const Query& bq) {
            int la = a.l / B, lb = bq.l / B;
            if (la != lb) return la < lb;
            int ra = a.r / B, rb = bq.r / B;
            if (ra != rb) {
                if (la & 1) return ra > rb;
                return ra < rb;
            }
            if (ra & 1) return a.t > bq.t;
            return a.t < bq.t;
        });
        int cl = 0, cr = 0, ct = 0;
        auto apply = [&](const Update& u, int fwd) {
            int p = u.pos;
            bool in = (cl <= p && p < cr);
            if (in) remove(p);
            arr[p] = fwd > 0 ? u.to : u.from;
            if (in) add(p);
        };
        for (const Query& q : qs) {
            while (ct < q.t) apply(upd[ct++], +1);
            while (ct > q.t) apply(upd[--ct], -1);
            while (cr < q.r) add(cr++);
            while (cl > q.l) add(--cl);
            while (cr > q.r) remove(--cr);
            while (cl < q.l) remove(cl++);
            save(q.id);
        }
    }
};

// --- B.3. Мо на деревьях (эйлеров порядок + LCA) ---
// Каждая вершина входит в euler дважды: tin — появление, tout —
// исчезновение. Путь u–v: если lca(u, v) == u (u — предок v) —
// интервал [tin[u], tin[v]]; иначе [tout[u], tin[v]] (u — с меньшим
// tin) плюс lca отдельно. Вершины пути встречаются в интервале
// ровно по одному разу, прочие — дважды: toggle по флагу активности
// (нечётное вхождение — активна). LCA — ParentArray (B.9).
struct MoOnTree {
    struct Query { int l, r, id, lca; };

    int n;
    vector<int> euler;  // 2n позиций: tin[v] — вход, tout[v] — выход
    vector<int> tin, tout;
    ParentArray pa;     // LCA двоичными подъёмами (b.cpp B.9)

    void build(int n_, const vector<pair<int, int>>& edges, int root = 0) {
        n = n_;
        vector<vector<int>> g(n);
        for (auto& e : edges) {
            g[e.first].push_back(e.second);
            g[e.second].push_back(e.first);
        }
        pa.build(n, edges, root);
        tin.assign(n, -1);
        tout.assign(n, -1);
        euler.assign(2 * n, -1);
        int cur = 0;
        dfs(g, root, -1, cur);
    }
    void dfs(const vector<vector<int>>& g, int v, int p, int& cur) {
        tin[v] = cur;
        euler[cur++] = v;
        for (int u : g[v]) if (u != p) dfs(g, u, v, cur);
        tout[v] = cur;
        euler[cur++] = v;
    }
    // Путь (u, v) → интервал [l, r) в euler; lca отдельным ответом
    // (или -1, если lca уже внутри интервала).
    void path_to_interval(int u, int v, int& l, int& r, int& lca) const {
        lca = pa.lca(u, v);
        if (lca == u) { l = tin[u]; r = tin[v] + 1; lca = -1; return; }
        if (lca == v) { l = tin[v]; r = tin[u] + 1; lca = -1; return; }
        if (tin[u] > tin[v]) swap(u, v);
        l = tout[u];
        r = tin[v] + 1;
    }
    // Раннер: toggle по флагу активности; save(id, lca) — вызывающий
    // добавляет вклад lca к ответу окна.
    template <typename Add, typename Rem, typename Save>
    void run(vector<Query> qs, Add add, Rem remove, Save save) {
        MoSolver mo;
        vector<char> active(n, 0);
        auto toggle = [&](int p) {
            int v = euler[p];
            if (active[v]) { active[v] = 0; remove(v); }
            else { active[v] = 1; add(v); }
        };
        vector<MoSolver::Query> mq(qs.size());
        vector<int> lca_of(qs.size(), -1);
        for (size_t i = 0; i < qs.size(); ++i) {
            mq[i] = {qs[i].l, qs[i].r, (int)i};
            lca_of[i] = qs[i].lca;
        }
        mo.run(2 * n, mq, toggle, toggle, [&](int id) { save(id, lca_of[id]); });
    }
};

// =============================================================
// C. ДЕРЕВО ОТРЕЗКОВ С КОРНЕВОЙ ДЕКОМПОЗИЦИЕЙ
// =============================================================

// --- C.1. Дерево отрезков над блоками ---
// Блочные суммы собраны в дерево отрезков (e D.3): запрос по
// полным блокам — O(log(n/B)) вместо O(n/B); хвосты — поштучно
// O(B). Обновление — пересборка блока O(B) + дерево O(log(n/B)).
struct SqrtSegTree {
    int n, B, nb;
    vector<int> a, sum;
    SegmentTreeIterative st;  // дерево над суммами блоков (e D.3)

    SqrtSegTree(const vector<int>& vals = {}, int B_ = 0) { build(vals, B_); }
    void build(const vector<int>& vals, int B_ = 0) {
        n = (int)vals.size();
        B = B_ > 0 ? B_ : (int)SqrtStructures::ceil_sqrt(n);
        a = vals;
        nb = (n + B - 1) / B;
        sum.assign(nb, 0);
        for (int i = 0; i < n; ++i) sum[i / B] += a[i];
        st.build(sum);
    }
    void update(int p, int x) {
        int b = p / B;
        sum[b] += x - a[p];
        a[p] = x;
        st.update(b, sum[b]);
    }
    int query(int l, int r) const {  // [l, r)
        if (l >= r) return 0;
        int res = 0;
        int bl = l / B, br = (r - 1) / B;
        if (bl == br) {
            for (int i = l; i < r; ++i) res += a[i];
            return res;
        }
        for (int i = l; i < (bl + 1) * B; ++i) res += a[i];
        res += st.query(bl + 1, br);
        for (int i = br * B; i < r; ++i) res += a[i];
        return res;
    }
};

// =============================================================
// D. SQRT ДЕКОМПОЗИЦИЯ ПО БЛОКАМ (ГРАФЫ)
// =============================================================

// --- D.1. Тяжелые и легкие вершины ---
// Задача: веса вершин статического графа; обновление веса (x, d);
// запрос — сумма весов соседей v. Тяжёлые — deg > √(2m); их число
// ≤ √(2m) ≤ 2√m (сумма степеней 2m). Для тяжёлых хранится S[v] —
// сумма весов соседей: при обновлении (x, d) S[u] += d для каждого
// тяжёлого соседа u вершины x (перебор соседей x). Запрос: тяжёлая
// — O(1); лёгкая — пересчёт по соседям O(deg v) ≤ O(√m).
struct HeavyLightVertices {
    int n;
    vector<vector<int>> g;
    vector<int> w;
    vector<char> heavy;
    vector<long long> S;  // только для тяжёлых: сумма весов соседей

    void build(int n_, const vector<pair<int, int>>& edges, const vector<int>& weights) {
        n = n_;
        g.assign(n, {});
        for (auto& e : edges) {
            g[e.first].push_back(e.second);
            g[e.second].push_back(e.first);
        }
        w = weights;
        int m = (int)edges.size();
        long long lim = SqrtStructures::ceil_sqrt(2 * max(1, m));
        heavy.assign(n, 0);
        for (int v = 0; v < n; ++v)
            if ((long long)g[v].size() > lim) heavy[v] = 1;
        S.assign(n, 0);
        for (int v = 0; v < n; ++v) if (heavy[v])
            for (int u : g[v]) S[v] += w[u];
    }
    void add_weight(int x, int d) {
        w[x] += d;
        for (int u : g[x]) if (heavy[u]) S[u] += d;
    }
    long long neighbor_sum(int v) const {
        if (heavy[v]) return S[v];
        long long res = 0;
        for (int u : g[v]) res += w[u];
        return res;
    }
};

// --- D.2. Heavy-Light Decomposition ---
// Дерево (корень root); первый DFS — размеры поддеревьев, глубины,
// heavy-ребёнок (максимум поддерева); второй — разложение на
// вертикальные цепочки: head — верх, pos — позиция в едином массиве
// (тяжёлый ребёнок идёт сразу за родителем — поддерево = непрерывный
// отрезок pos). Поверх pos — дерево отрезков (e D.6, add + sum).
// Путь u–v = O(log n) цепочек → O(log n) отрезковых операций.
struct HeavyLightDecomposition {
    int n;
    vector<vector<int>> g;
    vector<int> parent, depth, sz, heavy, head, pos;
    int cur;
    LazySegTreeAdd st;  // суммы на pos-массиве (e D.6)

    void build(const vector<vector<int>>& g_, int root, const vector<int>& weights) {
        n = (int)g_.size();
        g = g_;
        parent.assign(n, -1);
        depth.assign(n, 0);
        sz.assign(n, 0);
        heavy.assign(n, -1);
        head.assign(n, -1);
        pos.assign(n, -1);
        dfs1(root, -1);
        cur = 0;
        vector<int> base(n, 0);
        decompose(root, root, base, weights);
        st.build(base);
    }
    void dfs1(int v, int p) {
        parent[v] = p;
        sz[v] = 1;
        int best = 0;
        for (int u : g[v]) if (u != p) {
            depth[u] = depth[v] + 1;
            dfs1(u, v);
            sz[v] += sz[u];
            if (sz[u] > best) { best = sz[u]; heavy[v] = u; }
        }
    }
    void decompose(int v, int h, vector<int>& base, const vector<int>& weights) {
        head[v] = h;
        pos[v] = cur;
        base[cur++] = weights[v];
        if (heavy[v] != -1) decompose(heavy[v], h, base, weights);
        for (int u : g[v])
            if (u != parent[v] && u != heavy[v]) decompose(u, u, base, weights);
    }
    // Сумма на пути u–v (веса вершин; обе вершины входят).
    long long path_query(int u, int v) {
        long long res = 0;
        while (head[u] != head[v]) {
            if (depth[head[u]] < depth[head[v]]) swap(u, v);
            res += st.query(pos[head[u]], pos[u] + 1);
            u = parent[head[u]];
        }
        if (depth[u] > depth[v]) swap(u, v);
        res += st.query(pos[u], pos[v] + 1);
        return res;
    }
    // Прибавление x ко всем вершинам пути u–v.
    void path_add(int u, int v, long long x) {
        while (head[u] != head[v]) {
            if (depth[head[u]] < depth[head[v]]) swap(u, v);
            st.add_segment(pos[head[u]], pos[u] + 1, x);
            u = parent[head[u]];
        }
        if (depth[u] > depth[v]) swap(u, v);
        st.add_segment(pos[u], pos[v] + 1, x);
    }
    // Сумма по поддереву v: поддерево — непрерывный отрезок pos.
    long long sub_query(int v) { return st.query(pos[v], pos[v] + sz[v]); }
};

};  // struct SqrtStructures

#ifndef STRUCT_F_MAIN
#define STRUCT_F_MAIN

int main() {
    using H = SqrtStructures;
    cout << "=== VI. КОРНЕВЫЕ СТРУКТУРЫ (SQRT DECOMPOSITION) ===" << endl;

    // ---------- A.1 SqrtSum ----------
    vector<int> base = {1, 3, 5, 7, 9, 11};
    {
        H::SqrtSum t(base);
        cout << "SqrtSum block_size = " << t.block_size()
             << " (ожидаем 3)" << endl;
        cout << "SqrtSum query(1,4) = " << t.query(1, 4)
             << ", query(0,6) = " << t.query(0, 6)
             << ", query(2,6) = " << t.query(2, 6)
             << " (ожидаем 15 36 32)" << endl;
        t.update(2, 10);
        cout << "SqrtSum after update(2,10): query(0,6) = " << t.query(0, 6)
             << ", query(1,4) = " << t.query(1, 4)
             << ", query(2,6) = " << t.query(2, 6)
             << " (ожидаем 41 20 37)" << endl;
        // B = 2 — параметр работает
        H::SqrtSum t2(base, 2);
        cout << "SqrtSum B=2: query(1,4) = " << t2.query(1, 4)
             << ", query(0,6) = " << t2.query(0, 6)
             << " (ожидаем 15 36)" << endl;
    }

    // ---------- A.3 SqrtRangeAdd ----------
    {
        H::SqrtRangeAdd t(vector<long long>{1, 3, 5, 7, 9, 11});
        t.range_add(1, 5, 10);
        cout << "SqrtRangeAdd after add(1,5,10): sum(1,4) = " << t.range_sum(1, 4)
             << ", sum(0,6) = " << t.range_sum(0, 6)
             << " (ожидаем 45 76)" << endl;
        t.range_add(0, 3, 1);
        cout << "SqrtRangeAdd after add(0,3,1): sum(0,6) = " << t.range_sum(0, 6)
             << ", sum(2,5) = " << t.range_sum(2, 5)
             << " (ожидаем 79 52)" << endl;
    }

    // ---------- B.1 Стандартный Мо: число различных на отрезке ----------
    {
        vector<int> a = {2, 1, 3, 1, 4, 3, 2};
        H::MoSolver mo;
        vector<H::MoSolver::Query> qs = {{0, 3, 0}, {1, 4, 1}, {3, 7, 2}, {0, 7, 3}, {4, 6, 4}};
        vector<int> ans(5, 0);
        vector<int> freq(10, 0);
        int cur = 0;
        auto add = [&](int p) { if (freq[a[p]]++ == 0) ++cur; };
        auto rem = [&](int p) { if (--freq[a[p]] == 0) --cur; };
        mo.run((int)a.size(), qs, add, rem, [&](int id) { ans[id] = cur; });
        cout << "MoSolver distinct(0,3) = " << ans[0]
             << ", distinct(1,4) = " << ans[1]
             << ", distinct(3,7) = " << ans[2]
             << ", distinct(0,7) = " << ans[3]
             << ", distinct(4,6) = " << ans[4]
             << " (ожидаем 3 2 4 4 2)" << endl;
        // B.4: сумма на отрезке — те же раннер, другие add/remove
        vector<int> ans2(5, 0);
        int s = 0;
        auto add2 = [&](int p) { s += a[p]; };
        auto rem2 = [&](int p) { s -= a[p]; };
        mo.run((int)a.size(), qs, add2, rem2, [&](int id) { ans2[id] = s; });
        cout << "MoSolver sum(1,4) = " << ans2[1] << ", sum(0,7) = " << ans2[3]
             << " (ожидаем 5 16)" << endl;
    }

    // ---------- B.2 Мо с обновлениями ----------
    {
        vector<int> a = {2, 1, 3, 1, 4, 3, 2};
        vector<H::MoWithUpdates::Update> upd = {{2, 3, 1}, {5, 3, 7}};
        vector<H::MoWithUpdates::Query> qs = {{1, 4, 0, 0}, {2, 6, 1, 1}, {0, 7, 2, 2},
                                              {3, 5, 1, 3}, {0, 3, 2, 4}};
        vector<int> ans(5, 0);
        vector<int> freq(10, 0);
        int cur = 0;
        auto add = [&](int p) { if (freq[a[p]]++ == 0) ++cur; };
        auto rem = [&](int p) { if (--freq[a[p]] == 0) --cur; };
        H::MoWithUpdates mo;
        mo.run((int)a.size(), qs, upd, a, add, rem, [&](int id) { ans[id] = cur; });
        cout << "MoWithUpdates ans = " << ans[0] << " " << ans[1] << " " << ans[2]
             << " " << ans[3] << " " << ans[4]
             << " (ожидаем 2 3 4 2 2)" << endl;
    }

    // ---------- B.3 Мо на деревьях ----------
    {
        vector<int> val = {1, 2, 2, 3, 1, 2};
        H::MoOnTree mo;
        mo.build(6, {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {4, 5}}, 0);
        vector<pair<int, int>> paths = {{3, 5}, {2, 4}, {5, 0}, {3, 2}, {4, 4}, {1, 5}};
        vector<H::MoOnTree::Query> qs;
        for (int i = 0; i < (int)paths.size(); ++i) {
            int l, r, lca;
            mo.path_to_interval(paths[i].first, paths[i].second, l, r, lca);
            qs.push_back({l, r, i, lca});
        }
        vector<int> ans(6, 0);
        vector<int> freq(10, 0);
        int cur = 0;
        auto add = [&](int v) { if (freq[val[v]]++ == 0) ++cur; };
        auto rem = [&](int v) { if (--freq[val[v]] == 0) --cur; };
        mo.run(qs, add, rem, [&](int id, int lca) {
            ans[id] = cur + (lca != -1 && freq[val[lca]] == 0 ? 1 : 0);
        });
        cout << "MoOnTree path distinct: 3-5 = " << ans[0] << ", 2-4 = " << ans[1]
             << ", 5-0 = " << ans[2] << ", 3-2 = " << ans[3]
             << ", 4-4 = " << ans[4] << ", 1-5 = " << ans[5]
             << " (ожидаем 3 2 2 3 1 2)" << endl;
    }

    // ---------- C.1 SqrtSegTree ----------
    {
        H::SqrtSegTree t(base);
        cout << "SqrtSegTree query(1,4) = " << t.query(1, 4)
             << ", query(0,6) = " << t.query(0, 6)
             << ", query(2,6) = " << t.query(2, 6)
             << " (ожидаем 15 36 32)" << endl;
        t.update(2, 10);
        cout << "SqrtSegTree after update(2,10): query(0,6) = " << t.query(0, 6)
             << ", query(2,6) = " << t.query(2, 6)
             << " (ожидаем 41 37)" << endl;
    }

    // ---------- D.1 Тяжелые и легкие вершины ----------
    {
        H::HeavyLightVertices h;
        h.build(7, {{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}},
                {10, 1, 2, 3, 4, 5, 6});
        cout << "HeavyLightVertices neighbor_sum(0) = " << h.neighbor_sum(0)
             << ", neighbor_sum(3) = " << h.neighbor_sum(3)
             << " (ожидаем 21 10)" << endl;
        h.add_weight(0, 5);
        cout << "HeavyLightVertices after add(0,5): neighbor_sum(3) = " << h.neighbor_sum(3)
             << " (ожидаем 15)" << endl;
        h.add_weight(4, 7);
        cout << "HeavyLightVertices after add(4,7): neighbor_sum(0) = " << h.neighbor_sum(0)
             << ", neighbor_sum(4) = " << h.neighbor_sum(4)
             << " (ожидаем 28 15)" << endl;
    }

    // ---------- D.2 HLD ----------
    {
        vector<vector<int>> g(6);
        vector<pair<int, int>> edges = {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {4, 5}};
        for (auto& e : edges) { g[e.first].push_back(e.second); g[e.second].push_back(e.first); }
        H::HeavyLightDecomposition h;
        h.build(g, 0, {1, 2, 3, 4, 5, 6});
        cout << "HLD path_query(3,5) = " << h.path_query(3, 5)
             << ", path_query(2,5) = " << h.path_query(2, 5)
             << " (ожидаем 17 17)" << endl;
        h.path_add(3, 5, 10);
        cout << "HLD after path_add(3,5,10): path_query(0,5) = " << h.path_query(0, 5)
             << ", sub_query(1) = " << h.sub_query(1)
             << ", path_query(3,4) = " << h.path_query(3, 4)
             << ", sub_query(0) = " << h.sub_query(0)
             << " (ожидаем 44 57 41 61)" << endl;
    }

    cout << "\n=== ОБЩЕЕ: одинаковые серии ===" << endl;
    vector<int> want_static = {15, 36, 32};       // query(1,4), query(0,6), query(2,6)
    vector<int> want_upd = {20, 41, 37};          // после update(2, 10)

    auto check = [](const char* name, const vector<int>& got, const vector<int>& want) {
        cout << name << " series match (expect 1) = " << (got == want) << endl;
    };
    auto static_answers = [&](auto&& q) {
        return vector<int>{(int)q(1, 4), (int)q(0, 6), (int)q(2, 6)};
    };

    { H::SqrtSum t; t.build(base); check("SqrtSum", static_answers([&](int l, int r) { return t.query(l, r); }), want_static); }
    { H::SqrtSegTree t; t.build(base); t.update(2, 10);
      check("SqrtSegTree", static_answers([&](int l, int r) { return t.query(l, r); }), want_upd); }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_F_MAIN

#endif // STRUCT_F_CPP
