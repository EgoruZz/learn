#ifndef STRUCT_G_CPP
#define STRUCT_G_CPP

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
// VII. ПЕРСИСТЕНТНЫЕ СТРУКТУРЫ
// =============================================================
// Структура md: A. Основы персистентности
//               → B. Персистентное дерево отрезков
//               → C. Персистентное декартово дерево
//               → D. Структуры с откатами
//               → E. Fully persistent → F. Confluently persistent
//               → G. Functional структуры
//
// PersistentStructures наследует SqrtStructures (f.cpp).
// Переиспользует из базы:
//   * PersistentSegmentTree (V.D.11) — PST (B.1) и Persistent Array
//     (A.2) как его частный случай; частотное применение (B.2);
//   * RollbackDSU (III.A.6) — стек изменений и откат (D.1);
//   * PersistentDSU (III.A.7) — пример Fat Node (A.3);
//   * next_priority (II.F) — приоритеты персистентного treap
//     (C.1, G.2);
//   * ceil_sqrt (VI) — размер блока Persistent Vector (G.1);
//   * мотив «полные куски + хвосты» (VI.A) — хребет + блоки (G.1).
//
// Порядок структур строго соответствует порядку md (A → G).
// A.1, A.3, B.1, B.3, D.1, E.1, F.1, G.3 — анализ и/или прямое
// переиспользование готовых структур (PersistentSegmentTree,
// RollbackDSU) и демо в main: нового кода не имеют.
//
// ВНИМАНИЕ (скрытие имён): build, update, query, push, pop, top,
// insert, erase, split, merge здесь локальные; одноимённые из
// других веток не подключаются.

#define STRUCT_F_MAIN
#include "../f/f.cpp"
#undef STRUCT_F_MAIN

struct PersistentStructures : SqrtStructures {

// =============================================================
// A. ОСНОВЫ ПЕРСИСТЕНТНОСТИ
// =============================================================

// --- A.2. Persistent Array (path copying) — базовый примитив ---
// Версия — корень двоичного дерева над [0, n): обновление копирует
// путь от корня до листа (O(log n) новых узлов), остальные
// поддеревья разделяются между версиями. Реализуется готовым
// PersistentSegmentTree (e D.11) как его частный случай: точечный
// доступ — query(p, p + 1); версии — список корней.
struct PersistentArray {
    PersistentSegmentTree st;  // e D.11 (агрегат = значение точки)
    vector<int> roots;         // версии: корни; roots[0] — версия 0

    void build(const vector<long long>& a) {
        st = PersistentSegmentTree((int)a.size());
        int root = st.build_empty(0, (int)a.size());
        for (int i = 0; i < (int)a.size(); ++i) root = st.update(root, i, a[i]);
        roots.assign(1, root);
    }
    // set в версии v → новая версия (возвращает её номер)
    int set_value(int v, int pos, long long x) {
        roots.push_back(st.update(roots[v], pos, x));
        return (int)roots.size() - 1;
    }
    long long get(int v, int pos) const { return st.query(roots[v], pos, pos + 1); }
    int versions() const { return (int)roots.size(); }
};

// =============================================================
// B. ПЕРСИСТЕНТНОЕ ДЕРЕВО ОТРЕЗКОВ
// =============================================================

// --- B.2. Приложение PST: k-я порядковая статистика на отрезке ---
// Частотное PST: версия roots[i] — счётчики значений префикса
// [0, i); распределение на [l, r) = разность версий (root_r −
// root_l); k-е значение — спуск по паре корней за O(log σ),
// σ — диапазон значений. Строится поверх готового
// PersistentSegmentTree (e D.11): сумма листа = частота значения.
struct PersistentSegmentTreeKth {
    int sigma;                  // значения [0, sigma)
    PersistentSegmentTree pst;  // суммы = частоты (e D.11)
    vector<int> roots;          // roots[i] — префикс [0, i)

    void build(const vector<int>& a, int sigma_) {
        sigma = sigma_;
        pst = PersistentSegmentTree(sigma);
        int root = pst.build_empty(0, sigma);
        roots.assign(a.size() + 1, root);
        for (int i = 0; i < (int)a.size(); ++i) {
            int v = a[i];
            long long cnt = pst.query(root, v, v + 1);
            root = pst.update(root, v, cnt + 1);
            roots[i + 1] = root;
        }
    }
    // k-е по возрастанию значение на [l, r) (k 1-based)
    int kth(int l, int r, int k) const {
        int v = roots[r], u = roots[l];
        int lo = 0, hi = sigma;
        while (hi - lo > 1) {
            int mid = (lo + hi) / 2;
            int lc_v = pst.pool[v].lc, lc_u = pst.pool[u].lc;
            long long cnt = pst.pool[lc_v].sum - pst.pool[lc_u].sum;
            if (k <= cnt) { v = lc_v; u = lc_u; hi = mid; }
            else { k -= (int)cnt; v = pst.pool[v].rc; u = pst.pool[u].rc; lo = mid; }
        }
        return lo;
    }
    // число значений ≤ x на [l, r) (rank)
    long long count_leq(int l, int r, int x) const {
        if (x < 0) return 0;
        if (x >= sigma) return r - l;
        int v = roots[r], u = roots[l];
        int lo = 0, hi = sigma;
        long long res = 0;
        while (hi - lo > 1) {
            int mid = (lo + hi) / 2;
            int lc_v = pst.pool[v].lc, lc_u = pst.pool[u].lc;
            if (x < mid) { v = lc_v; u = lc_u; hi = mid; }
            else {
                res += pst.pool[lc_v].sum - pst.pool[lc_u].sum;
                v = pst.pool[v].rc; u = pst.pool[u].rc; lo = mid;
            }
        }
        res += pst.pool[v].sum - pst.pool[u].sum;  // вклад листа (значение x)
        return res;
    }
};

// =============================================================
// C. ПЕРСИСТЕНТНОЕ ДЕКАРТОВО ДЕРЕВО
// =============================================================

// --- C.1. Persistent Treap (split/merge без модификации старых) ---
// split/merge клонируют узлы на пути спуска (path copying) — старые
// корни остаются корректными версиями. Структура и приоритеты — как
// в Treap (b.cpp II.F): next_priority — детерминированный генератор.
// Узлы в пуле, без удаления: память — суммарное число копий.
struct PersistentTreap {
    struct Node { int key, pr, sz, l, r; };
    vector<Node> pool;  // pool[0] — «пустой» узел

    PersistentTreap() { pool.push_back({0, 0, 0, 0, 0}); }
    int new_node(int key) {
        pool.push_back({key, next_priority(), 1, 0, 0});
        return (int)pool.size() - 1;
    }
    int clone(int t) { pool.push_back(pool[t]); return (int)pool.size() - 1; }
    int sz(int t) const { return t ? pool[t].sz : 0; }
    void upd(int t) { pool[t].sz = 1 + sz(pool[t].l) + sz(pool[t].r); }

    // split: (ключи < key, ключи ≥ key) — копии узлов на пути
    pair<int, int> split(int t, int key) {
        if (!t) return {0, 0};
        if (pool[t].key < key) {
            auto [a, b] = split(pool[t].r, key);
            int n = clone(t);
            pool[n].r = a;
            upd(n);
            return {n, b};
        } else {
            auto [a, b] = split(pool[t].l, key);
            int n = clone(t);
            pool[n].l = b;
            upd(n);
            return {a, n};
        }
    }
    int merge(int a, int b) {
        if (!a) return b;
        if (!b) return a;
        if (pool[a].pr > pool[b].pr) {
            int n = clone(a);
            pool[n].r = merge(pool[n].r, b);
            upd(n);
            return n;
        } else {
            int n = clone(b);
            pool[n].l = merge(a, pool[n].l);
            upd(n);
            return n;
        }
    }
    // вставка → новая версия; существующий ключ — без изменений (set)
    int insert(int root, int key) {
        if (search(root, key)) return root;
        auto [a, b] = split(root, key);
        return merge(merge(a, new_node(key)), b);
    }
    int erase(int root, int key) {
        auto [a, b] = split(root, key);
        auto [b2, c] = split(b, key + 1);
        return merge(a, c);
    }
    bool search(int root, int key) const {
        int t = root;
        while (t) {
            if (pool[t].key == key) return true;
            t = key < pool[t].key ? pool[t].l : pool[t].r;
        }
        return false;
    }
    // k-й элемент по порядку (1-based)
    int kth(int root, int k) const {
        int t = root;
        while (t) {
            int ls = sz(pool[t].l);
            if (k == ls + 1) return pool[t].key;
            if (k <= ls) t = pool[t].l;
            else { k -= ls + 1; t = pool[t].r; }
        }
        return -1;
    }
    // число элементов строго меньше key
    int rank(int root, int key) const {
        int t = root, res = 0;
        while (t) {
            if (pool[t].key < key) { res += sz(pool[t].l) + 1; t = pool[t].r; }
            else t = pool[t].l;
        }
        return res;
    }
    vector<int> inorder(int root) const {
        vector<int> res;
        function<void(int)> go = [&](int t) {
            if (!t) return;
            go(pool[t].l);
            res.push_back(pool[t].key);
            go(pool[t].r);
        };
        go(root);
        return res;
    }
};

// =============================================================
// D. СТРУКТУРЫ С ОТКАТАМИ
// =============================================================

// --- D.2. Persistent Stack (списки версий) ---
// Версия = индекс вершины односвязного списка (пул узлов);
// push создаёт новый узел, pop — переход по next; старые версии
// неизменяемы и разделяют хвосты. Узел 0 — пустой стек.
struct PersistentStack {
    struct Node { int val, next; };
    vector<Node> pool;

    PersistentStack() : pool(1) {}  // pool[0] — пустой стек
    int push(int v, int x) { pool.push_back({x, v}); return (int)pool.size() - 1; }
    int pop(int v) const { return pool[v].next; }
    int top(int v) const { return pool[v].val; }
    bool empty(int v) const { return v == 0; }
};

// --- D.3. Persistent Queue (на двух стеках) ---
// Очередь — пара персистентных стеков (D.2): front — начало, back —
// перевёрнутый хвост. push — в back; pop — из front; при пустом
// front — переворот back поэлементным push (разово O(длина)).
// Инвариант: front непуст при непустой очереди. Каждый элемент
// переносится в front не более одного раза — O(1) амортизированно.
struct PersistentQueue {
    PersistentStack st;  // D.2 — переиспользуется
    struct State { int front, back; };

    State push(State q, int x) {
        q.back = st.push(q.back, x);
        return fix(q);  // инвариант: front непуст при непустой очереди
    }
    State fix(State q) {  // пустой front → переворот всего back
        if (q.front) return q;
        while (q.back) {
            q.front = st.push(q.front, st.top(q.back));
            q.back = st.pop(q.back);
        }
        return q;
    }
    State pop(State q) { return fix({st.pop(q.front), q.back}); }
    int front(State q) const { return st.top(q.front); }
};

// =============================================================
// G. FUNCTIONAL СТРУКТУРЫ
// =============================================================

// --- G.1. Persistent Vector (блочный path copying) ---
// Версия — «хребет»: массив указателей на блоки + число элементов
// (копируется O(n/B)); изменение копирует один блок (O(B)).
// Блоки неизменяемы и разделяются между версиями; копия изменяемого
// блока — безопасность разделения. B = ⌈√n⌉ → O(√n) на операцию;
// дерево хребта (RRB-vector, I.F) даёт O(log n). Копирование блока
// всегда (без refcount) — простота; обобщение — подсчёт ссылок.
struct PersistentVector {
    int B;
    struct Version { vector<int> buckets; int size; };
    vector<vector<int>> blk;  // пул блоков
    vector<Version> versions;

    PersistentVector(int B_ = 0) : B(B_) {}
    void build(const vector<int>& a) {
        if (B <= 0) B = (int)SqrtStructures::ceil_sqrt(max(1, (int)a.size()));
        blk.clear();
        versions.clear();
        Version v0;
        for (int i = 0; i < (int)a.size(); i += B) {
            int e = min(i + B, (int)a.size());
            blk.push_back(vector<int>(a.begin() + i, a.begin() + e));
            v0.buckets.push_back((int)blk.size() - 1);
        }
        v0.size = (int)a.size();
        versions.push_back(v0);
    }
    int update(int v, int pos, int x) {  // → новая версия
        Version nv = versions[v];        // копия хребта O(n/B)
        int b = pos / B;
        vector<int> nb = blk[nv.buckets[b]];  // копия блока O(B)
        nb[pos % B] = x;
        blk.push_back(std::move(nb));
        nv.buckets[b] = (int)blk.size() - 1;
        versions.push_back(nv);
        return (int)versions.size() - 1;
    }
    int push_back(int v, int x) {  // → новая версия
        Version nv = versions[v];
        if (nv.size % B == 0) {
            blk.push_back(vector<int>());      // свежий блок — не разделяется
            nv.buckets.push_back((int)blk.size() - 1);
        } else {
            vector<int> nb = blk[nv.buckets.back()];
            blk.push_back(std::move(nb));
            nv.buckets.back() = (int)blk.size() - 1;
        }
        blk[nv.buckets.back()].push_back(x);
        ++nv.size;
        versions.push_back(nv);
        return (int)versions.size() - 1;
    }
    int get(int v, int pos) const {
        return blk[versions[v].buckets[pos / B]][pos % B];
    }
    int size(int v) const { return versions[v].size; }
};

// --- G.2. Persistent Map (персистентный treap, ключ → значение) ---
// Те же split/merge, что C.1, узел дополнительно хранит значение;
// замена значения — split по двум границам и вставка нового узла.
// Set — частный случай (значение = ключ). find по любой версии.
struct PersistentMap {
    struct Node { int key, val, pr, sz, l, r; };
    vector<Node> pool;

    PersistentMap() { pool.push_back({0, 0, 0, 0, 0, 0}); }
    int new_node(int key, int val) {
        pool.push_back({key, val, next_priority(), 1, 0, 0});
        return (int)pool.size() - 1;
    }
    int clone(int t) { pool.push_back(pool[t]); return (int)pool.size() - 1; }
    int sz(int t) const { return t ? pool[t].sz : 0; }
    void upd(int t) { pool[t].sz = 1 + sz(pool[t].l) + sz(pool[t].r); }

    pair<int, int> split(int t, int key) {
        if (!t) return {0, 0};
        if (pool[t].key < key) {
            auto [a, b] = split(pool[t].r, key);
            int n = clone(t);
            pool[n].r = a;
            upd(n);
            return {n, b};
        } else {
            auto [a, b] = split(pool[t].l, key);
            int n = clone(t);
            pool[n].l = b;
            upd(n);
            return {a, n};
        }
    }
    int merge(int a, int b) {
        if (!a) return b;
        if (!b) return a;
        if (pool[a].pr > pool[b].pr) {
            int n = clone(a);
            pool[n].r = merge(pool[n].r, b);
            upd(n);
            return n;
        } else {
            int n = clone(b);
            pool[n].l = merge(a, pool[n].l);
            upd(n);
            return n;
        }
    }
    // вставка/замена ключа → новая версия
    int insert(int root, int key, int val) {
        auto [a, b] = split(root, key);
        auto [b2, c] = split(b, key + 1);   // старый узел key выпадает
        return merge(merge(a, new_node(key, val)), c);
    }
    int erase(int root, int key) {
        auto [a, b] = split(root, key);
        auto [b2, c] = split(b, key + 1);
        return merge(a, c);
    }
    bool find(int root, int key, int& val) const {
        int t = root;
        while (t) {
            if (pool[t].key == key) { val = pool[t].val; return true; }
            t = key < pool[t].key ? pool[t].l : pool[t].r;
        }
        return false;
    }
};

};  // struct PersistentStructures

#ifndef STRUCT_G_MAIN
#define STRUCT_G_MAIN

int main() {
    using H = PersistentStructures;
    cout << "=== VII. ПЕРСИСТЕНТНЫЕ СТРУКТУРЫ ===" << endl;

    // ---------- A.2 Persistent Array ----------
    {
        H::PersistentArray pa;
        pa.build({1, 3, 5, 7, 9, 11});
        int v1 = pa.set_value(0, 2, 100);
        int v2 = pa.set_value(v1, 5, 200);
        cout << "PersistentArray get(v0,2) = " << pa.get(0, 2)
             << ", get(v1,2) = " << pa.get(v1, 2)
             << ", get(v2,2) = " << pa.get(v2, 2)
             << ", get(v0,5) = " << pa.get(0, 5)
             << ", get(v2,5) = " << pa.get(v2, 5)
             << " (ожидаем 5 100 100 11 200)" << endl;
    }

    // ---------- B.1 PST (готовый, e D.11): версии и история ----------
    {
        H::PersistentSegmentTree pst(6);
        int r0 = pst.build_empty(0, 6);
        int r1 = pst.update(r0, 2, 100);
        int r2 = pst.update(r1, 5, 200);
        cout << "PST query(v2,0,6) = " << pst.query(r2, 0, 6)
             << ", query(v1,0,6) = " << pst.query(r1, 0, 6)
             << ", query(v0,0,6) = " << pst.query(r0, 0, 6)
             << " (ожидаем 300 100 0)" << endl;
    }

    // ---------- B.2 k-я статистика на отрезке ----------
    {
        vector<int> a = {2, 1, 3, 1, 4, 2, 5, 1};
        H::PersistentSegmentTreeKth kth;
        kth.build(a, 6);
        cout << "kth(0,8,1) = " << kth.kth(0, 8, 1)
             << ", kth(0,8,4) = " << kth.kth(0, 8, 4)
             << ", kth(0,8,8) = " << kth.kth(0, 8, 8)
             << ", kth(2,6,2) = " << kth.kth(2, 6, 2)
             << ", count_leq(0,8,2) = " << kth.count_leq(0, 8, 2)
             << " (ожидаем 1 2 5 2 5)" << endl;
    }

    // ---------- C.1 Persistent Treap ----------
    {
        H::PersistentTreap tr;
        int r0 = 0;
        int r1 = tr.insert(r0, 5);
        int r2 = tr.insert(r1, 3);
        int r3 = tr.insert(r2, 8);
        int r4 = tr.insert(r3, 1);
        int r5 = tr.erase(r4, 3);
        vector<int> in5 = tr.inorder(r5), in2 = tr.inorder(r2);
        cout << "Treap inorder(v5) =";
        for (int x : in5) cout << " " << x;
        cout << "; inorder(v2) =";
        for (int x : in2) cout << " " << x;
        cout << " (ожидаем 1 5 8; 3 5)" << endl;
        cout << "Treap kth(v4,2) = " << tr.kth(r4, 2)
             << ", rank(v4,5) = " << tr.rank(r4, 5)
             << ", search(v3,8) = " << tr.search(r3, 8)
             << " (ожидаем 3 2 1)" << endl;
    }

    // ---------- D.1 Rollback DSU (готовый, c A.6) ----------
    {
        H::RollbackDSU dsu(6);
        dsu.unite(0, 1);
        dsu.unite(2, 3);
        int snap = dsu.snapshot();
        dsu.unite(0, 2);
        dsu.rollback_to(snap);
        cout << "RollbackDSU same(0,3) = " << (dsu.find(0) == dsu.find(3))
             << ", size(1) = " << dsu.size(1)
             << " (ожидаем 0 2)" << endl;
    }

    // ---------- D.2 Persistent Stack ----------
    {
        H::PersistentStack st;
        int v1 = st.push(0, 10);
        int v2 = st.push(v1, 20);
        int v3 = st.push(v1, 30);   // ветка от v1 (полная персистентность)
        cout << "Stack top(v2) = " << st.top(v2)
             << ", top(v3) = " << st.top(v3)
             << ", pop(v2) → top = " << st.top(st.pop(v2))
             << ", empty(pop(v1)) = " << st.empty(st.pop(v1))
             << " (ожидаем 20 30 10 1)" << endl;
    }

    // ---------- D.3 Persistent Queue ----------
    {
        H::PersistentQueue q;
        H::PersistentQueue::State q0 = {0, 0};
        H::PersistentQueue::State q1 = q.push(q0, 1);
        H::PersistentQueue::State q2 = q.push(q1, 2);
        H::PersistentQueue::State q3 = q.push(q2, 3);
        cout << "Queue front(q3) = " << q.front(q3);
        H::PersistentQueue::State q4 = q.pop(q3);
        cout << ", front(q4) = " << q.front(q4);
        H::PersistentQueue::State q5 = q.pop(q4);
        cout << ", front(q5) = " << q.front(q5);
        H::PersistentQueue::State q6 = q.push(q5, 4);
        H::PersistentQueue::State q7 = q.pop(q6);
        cout << ", front(q7) = " << q.front(q7);
        cout << " (ожидаем 1 2 3 4)" << endl;
    }

    // ---------- E.1 Fully persistent (модификация старой версии) ----------
    {
        H::PersistentArray pa;
        pa.build({1, 3, 5, 7, 9, 11});
        int v1 = pa.set_value(0, 2, 100);
        int v2 = pa.set_value(0, 0, 77);   // ветка от версии 0
        cout << "FullyPersistent get(v0,0) = " << pa.get(0, 0)
             << ", get(v2,0) = " << pa.get(v2, 0)
             << ", get(v1,0) = " << pa.get(v1, 0)
             << ", get(v2,2) = " << pa.get(v2, 2)
             << " (ожидаем 1 77 1 5)" << endl;
    }

    // ---------- F.1 Confluently persistent (merge версий) ----------
    {
        H::PersistentTreap tr;
        int rA = tr.insert(tr.insert(tr.insert(0, 1), 2), 3);
        int rB = tr.insert(tr.insert(0, 5), 4);   // ключи > ключей rA
        int rM = tr.merge(rA, rB);                // новая версия — слияние
        vector<int> inM = tr.inorder(rM);
        cout << "Confluent merge inorder =";
        for (int x : inM) cout << " " << x;
        cout << "; old vA =";
        for (int x : tr.inorder(rA)) cout << " " << x;
        cout << " (ожидаем 1 2 3 4 5; 1 2 3)" << endl;
    }

    // ---------- G.1 Persistent Vector ----------
    {
        H::PersistentVector pv;
        pv.build({1, 3, 5, 7, 9, 11, 13});
        int v1 = pv.update(0, 2, 100);
        int v2 = pv.push_back(v1, 15);
        int v3 = pv.push_back(v2, 17);
        cout << "PV get(v0,2) = " << pv.get(0, 2)
             << ", get(v1,2) = " << pv.get(v1, 2)
             << ", get(v0,6) = " << pv.get(0, 6)
             << ", size(v2) = " << pv.size(v2)
             << ", get(v2,7) = " << pv.get(v2, 7)
             << ", size(v3) = " << pv.size(v3)
             << ", get(v3,8) = " << pv.get(v3, 8)
             << " (ожидаем 5 100 13 8 15 9 17)" << endl;
    }

    // ---------- G.2 Persistent Map ----------
    {
        H::PersistentMap pm;
        int m1 = pm.insert(0, 5, 50);
        int m2 = pm.insert(m1, 3, 30);
        int m3 = pm.insert(m2, 8, 80);
        int m4 = pm.insert(m3, 3, 33);   // замена значения
        int val;
        cout << "PM find(m4,3) = " << pm.find(m4, 3, val) << " (val " << val << ")"
             << ", find(m3,3) = " << pm.find(m3, 3, val) << " (val " << val << ")"
             << ", find(m2,8) = " << pm.find(m2, 8, val)
             << ", find(m4,8) = " << pm.find(m4, 8, val) << " (val " << val << ")"
             << " (ожидаем 1 (33) 1 (30) 0 1 (80))" << endl;
        int m5 = pm.erase(m4, 5);
        cout << "PM after erase(5): find(m5,5) = " << pm.find(m5, 5, val)
             << ", find(m3,5) = " << pm.find(m3, 5, val) << " (val " << val << ")"
             << " (ожидаем 0 1 (50))" << endl;
    }

    cout << "\n=== ОБЩЕЕ: одинаковые серии ===" << endl;
    // PST и PersistentArray обязаны совпадать на точечных запросах
    {
        vector<long long> want_v2 = {5, 100, 100, 11, 200};
        H::PersistentArray pa;
        pa.build({1, 3, 5, 7, 9, 11});
        int v1 = pa.set_value(0, 2, 100);
        int v2 = pa.set_value(v1, 5, 200);
        vector<long long> got = {pa.get(0, 2), pa.get(v1, 2), pa.get(v2, 2),
                                 pa.get(0, 5), pa.get(v2, 5)};
        cout << "PersistentArray series match (expect 1) = " << (got == want_v2) << endl;
    }
    // kth обязана совпадать с сортировкой
    {
        vector<int> a = {2, 1, 3, 1, 4, 2, 5, 1};
        H::PersistentSegmentTreeKth kth;
        kth.build(a, 6);
        bool ok = true;
        for (int l = 0; l < (int)a.size() && ok; ++l)
            for (int r = l + 1; r <= (int)a.size() && ok; ++r) {
                vector<int> s(a.begin() + l, a.begin() + r);
                sort(s.begin(), s.end());
                for (int k = 1; k <= (int)s.size(); ++k)
                    if (kth.kth(l, r, k) != s[k - 1]) ok = false;
            }
        cout << "PersistentSegmentTreeKth kth vs sort (expect 1) = " << ok << endl;
    }
    // PersistentVector и массив обязаны совпадать на общей серии
    {
        vector<int> base = {1, 3, 5, 7, 9, 11, 13};
        H::PersistentVector pv;
        pv.build(base);
        int v1 = pv.update(0, 2, 100);
        int v2 = pv.push_back(v1, 15);
        vector<int> want = {5, 100, 13, 8, 15};
        vector<int> got = {pv.get(0, 2), pv.get(v1, 2), pv.get(0, 6),
                           pv.size(v2), pv.get(v2, 7)};
        cout << "PersistentVector series match (expect 1) = " << (got == want) << endl;
    }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_G_MAIN

#endif // STRUCT_G_CPP
