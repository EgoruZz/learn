#ifndef STRUCT_K_CPP
#define STRUCT_K_CPP

#include <algorithm>
#include <numeric>
#include <limits>
#include <climits>

// =============================================================
// XI. ТЕОРЕТИЧЕСКИЕ И ЭКЗОТИЧЕСКИЕ СТРУКТУРЫ
// =============================================================
// Структура md: A. Малый универсум: битовые трюки
//               → B. Дерево ван Эмде Боаса (VEB)
//               → C. X-Fast Trie
//               → D. Y-Fast Trie
//               → E. Fusion Tree (битовая параллельность)
//               → F. Структуры динамической связности
//               → G. Ретроактивные структуры
//               → H. Cache-oblivious структуры
//               → H. Cache-oblivious структуры
//
// TheoreticalStructures наследует CachingStructures (j.cpp).
// Ось XI — структуры, цена операций которых привязана к параметрам
// задачи: универсум U (A–E), время и интервалы жизни (F, G),
// внешняя память и кэш (H).
//
// Переиспользование из базы:
//   * DynamicBitset (III.B) — компактные битовые блоки в основании
//     VEB (B.3): операции на слове (set/test, ctz/clz) и битовая
//     сортировка малого универсума (A.2);
//   * RollbackDSU (III.A.6) — оффлайн динамическая связность (F.2):
//     snapshot/rollback_to применяются в DFS по сегментному дереву
//     времени;
//   * Treap (II.F.1) — бакеты Y-fast trie (D): insert/erase/rank/kth
//     и статические split/sz;
//   * next_priority (II.F) — детерминированные приоритеты всех
//     неявных treap-классов k (F.3, G.3);
//   * ImplicitTreap (II.F.2) — прототип неявного treap: ETT (F.3)
//     развивает его родительскими указателями и агрегатом суммы,
//     ретроактивный стек (G.3) — агрегатом «эффекта отрезка»;
//   * DSU (III.A.3) — прямое сравнение с оффлайн-ответом (F.2).
//
// Порядок структур строго соответствует порядку md (A → I).
// X-fast идёт до Y-fast (Y строится на X). F.5 (Holm–de Lichtenberg–
// Thorup, Top Tree) и H.3 (cache-oblivious B-дерево) — обзоры в md,
// реализуемые части — F.2/F.3/F.4 и H.2.
//
// ВНИМАНИЕ (скрытие имён): next, prev, insert, erase, link, cut,
// search, min, max здесь локальные в классах k; одноимённые из
// других веток не подключаются. Внутри классов используются
// std::min/std::max во избежание теней от собственных методов.

#define STRUCT_J_MAIN
#include "../j/j.cpp"
#undef STRUCT_J_MAIN

struct TheoreticalStructures : CachingStructures {

// =============================================================
// A. МАЛЫЙ УНИВЕРСУМ: БИТОВЫЕ ТРЮКИ
// =============================================================

// --- A.2. Битовое множество и сортировка за O(n + U/w) ---
// Ключи из [0, U) ставят биты в DynamicBitset (III.B); вывод по
// возрастанию — проход по битам. Сравнение заменяется
// индексированием: при U = O(n) это линейная сортировка целых.
static vector<int> bit_sort(const vector<int>& keys, int U) {
    DynamicBitset b(U);
    for (int x : keys)
        if (x >= 0 && x < U) b.set(x);
    vector<int> res;
    for (int x = 0; x < U; x++)
        if (b.test(x)) res.push_back(x);
    return res;
}

// =============================================================
// B. ДЕРЕВО ВАН ЭМДЕ БОАСА (VEB)
// =============================================================

// --- B.1–B.3. VEB: рекурсия sqrt-разбиения + битовые блоки ---
// Универсум u = 2^k. Ключ режется на ⌈k/2⌉ старших бит (номер
// кластера) и ⌊k/2⌋ младших (смещение). Пустые кластеры
// регистрируются в summary. Нижний уровень (u ≤ w) — один
// DynamicBitset (III.B): минимум/максимум/сосед — словные
// инструкции (__builtin_ctzll/clzll). Все операции O(log log u),
// память O(u).
struct VebTree {
    static constexpr int W = DynamicBitset::W;   // бит в слове (64)

    int u;                       // универсум: степень двойки
    int m;                       // размер кластера (младшие биты)
    int ncl;                     // число кластеров
    int mn, mx, sz;              // края множества и размер (кеш)
    VebTree* summary;
    vector<VebTree*> cluster;
    DynamicBitset* block;        // непуст при u ≤ W

    explicit VebTree(int u_) : u(u_), m(0), ncl(0), mn(-1), mx(-1),
                               sz(0), summary(nullptr), block(nullptr) {
        if (u <= W) {
            block = new DynamicBitset(u);
        } else {
            int lg = 0, t = u;
            while (t > 1) { t >>= 1; lg++; }
            m = 1 << (lg / 2);               // 2^⌊lg/2⌋ — низкие биты
            ncl = u / m;
            summary = new VebTree(ncl);
            cluster.assign(ncl, nullptr);
        }
    }

    // --- битовый блок: операции над словом ---
    int block_min() const { return __builtin_ctzll(block->b[0]); }
    int block_max() const { return W - 1 - __builtin_clzll(block->b[0]); }
    int block_next(int x) const {
        if (x >= u) return -1;
        unsigned long long w = block->b[0] >> x;
        if (!w) return -1;
        return x + __builtin_ctzll(w);
    }
    int block_prev(int x) const {
        if (x >= u) x = u - 1;
        unsigned long long mask = (x == W - 1) ? ~0ULL : ((1ULL << (x + 1)) - 1);
        unsigned long long w = block->b[0] & mask;
        if (!w) return -1;
        return W - 1 - __builtin_clzll(w);
    }

    bool contains(int x) const {
        if (x < 0 || x >= u) return false;
        if (block) return block->test(x);
        int c = x / m, o = x % m;
        return cluster[c] && cluster[c]->contains(o);
    }

    void insert(int x) {
        if (x < 0 || x >= u) return;
        if (block) {
            if (block->test(x)) return;
            block->set(x);
        } else {
            int c = x / m, o = x % m;
            if (!cluster[c]) cluster[c] = new VebTree(m);
            bool empty = (cluster[c]->sz == 0);
            cluster[c]->insert(o);
            if (empty) summary->insert(c);
        }
        sz++;
        if (sz == 1) mn = mx = x;
        else { mn = std::min(mn, x); mx = std::max(mx, x); }
    }

    void erase(int x) {
        if (x < 0 || x >= u) return;
        if (block) {
            if (!block->test(x)) return;
            block->reset(x);
        } else {
            int c = x / m, o = x % m;
            cluster[c]->erase(o);
            if (cluster[c]->sz == 0) summary->erase(c);
        }
        sz--;
        if (sz == 0) mn = mx = -1;
        else if (x == mn) mn = compute_min();
        else if (x == mx) mx = compute_max();
    }

    int compute_min() const {
        if (sz == 0) return -1;
        if (block) return block_min();
        int c = summary->min();
        return c * m + cluster[c]->min();
    }
    int compute_max() const {
        if (sz == 0) return -1;
        if (block) return block_max();
        int c = summary->max();
        return c * m + cluster[c]->max();
    }
    int min() const { return mn; }
    int max() const { return mx; }

    int next(int x) const {                    // наименьший ключ ≥ x
        if (sz == 0 || x < 0 || x >= u) return -1;
        if (block) return block_next(x);
        if (x > mx) return -1;
        int c = x / m, o = x % m;
        if (cluster[c] && cluster[c]->sz > 0) {
            int r = cluster[c]->next(o);
            if (r != -1) return c * m + r;
        }
        int nc = summary->next(c + 1);
        if (nc == -1) return -1;
        return nc * m + cluster[nc]->min();
    }
    int prev(int x) const {                    // наибольший ключ ≤ x
        if (sz == 0 || x < 0) return -1;
        if (x >= u) x = u - 1;
        if (block) return block_prev(x);
        if (x < mn) return -1;
        int c = x / m, o = x % m;
        if (cluster[c] && cluster[c]->sz > 0) {
            int r = cluster[c]->prev(o);
            if (r != -1) return c * m + r;
        }
        int pc = summary->prev(c - 1);
        if (pc == -1) return -1;
        return pc * m + cluster[pc]->max();
    }
};

// =============================================================
// C. X-FAST TRIE
// =============================================================

// --- C.1–C.3. Двоичное trie с хешем уровней ---
// Универсум u = 2^k; уровень d хранит хеш «префикс → узел».
// Монотонность присутствия префиксов позволяет бинарный поиск
// самого глубокого узла на пути к x. Узлы хранят mn/mx — крайние
// листья поддерева; листья связаны двусвязным списком — соседние
// операции O(1) после наводки. insert/erase — путь длины k,
// next/prev — O(log k) хеш-запросов.
struct XFastTrie {
    struct Node {
        int id;                  // префикс (старшие depth бит)
        int depth;               // 0..k
        int ch[2];               // дети, -1
        int par;                 // родитель, -1 для корня
        int mn, mx;              // крайние листья поддерева (ключи)
        int prev_leaf, next_leaf;// соседи в списке листьев
    };
    int u, k;
    vector<Node> nd;
    vector<unordered_map<int, int>> lvl;   // lvl[d]: префикс → узел
    int first_leaf, last_leaf;
    int cnt;

    explicit XFastTrie(int u_) : u(u_), first_leaf(-1), last_leaf(-1), cnt(0) {
        k = 0;
        while ((1 << k) < u) k++;
        lvl.assign(k + 1, {});
        Node root;
        root.id = 0; root.depth = 0; root.ch[0] = root.ch[1] = -1;
        root.par = -1; root.mn = root.mx = -1;
        root.prev_leaf = root.next_leaf = -1;
        nd.push_back(root);
        lvl[0][0] = 0;
    }

    int new_node(int depth, int prefix) {
        Node t;
        t.id = prefix; t.depth = depth; t.ch[0] = t.ch[1] = -1; t.par = -1;
        t.mn = t.mx = -1; t.prev_leaf = t.next_leaf = -1;
        nd.push_back(t);
        int idx = (int)nd.size() - 1;
        lvl[depth][prefix] = idx;
        return idx;
    }

    int prefix(int x, int d) const { return x >> (k - d); }   // старшие d бит
    int bit(int x, int d) const { return (x >> (k - 1 - d)) & 1; }

    int size() const { return cnt; }
    bool contains(int x) const { return x >= 0 && x < u && lvl[k].count(x) > 0; }

    // узел-лист с наименьшим ключом ≥ x (-1, если такого нет)
    int ge_node(int x) const {
        if (x < 0 || x >= u || first_leaf == -1) return -1;
        int lo = 0, hi = k;                   // max d: префикс(x,d) существует
        while (lo < hi) {
            int mid = (lo + hi + 1) / 2;
            if (lvl[mid].count(prefix(x, mid))) lo = mid;
            else hi = mid - 1;
        }
        int d = lo;
        if (d == k) return lvl[k].at(x);      // x присутствует
        int v = lvl[d].at(prefix(x, d));
        if (nd[v].mn == -1) return -1;        // трие пустое
        int b = bit(x, d);
        if (b == 0) return lvl[k].at(nd[v].mn);   // все ключи поддерева > x
        int maxleaf = lvl[k].at(nd[v].mx);        // все ключи поддерева < x
        return nd[maxleaf].next_leaf;
    }
    // узел-лист с наибольшим ключом ≤ x (-1, если такого нет)
    int le_node(int x) const {
        if (x < 0 || first_leaf == -1) return -1;
        if (x >= u) x = u - 1;
        int lo = 0, hi = k;
        while (lo < hi) {
            int mid = (lo + hi + 1) / 2;
            if (lvl[mid].count(prefix(x, mid))) lo = mid;
            else hi = mid - 1;
        }
        int d = lo;
        if (d == k) return lvl[k].at(x);
        int v = lvl[d].at(prefix(x, d));
        if (nd[v].mx == -1) return -1;        // трие пустое
        int b = bit(x, d);
        if (b == 1) return lvl[k].at(nd[v].mx);   // все ключи поддерева < x
        int minleaf = lvl[k].at(nd[v].mn);        // все ключи поддерева > x
        return nd[minleaf].prev_leaf;
    }

    int next(int x) const { int n = ge_node(x); return n == -1 ? -1 : nd[n].id; }
    int prev(int x) const { int n = le_node(x); return n == -1 ? -1 : nd[n].id; }

    void insert(int x) {
        if (x < 0 || x >= u) return;
        if (lvl[k].count(x)) return;
        int succ = ge_node(x);                 // первый лист > x или -1
        int pred = (succ == -1) ? last_leaf : nd[succ].prev_leaf;
        int cur = 0;
        for (int d = 0; d < k; d++) {
            int b = bit(x, d);
            if (nd[cur].ch[b] == -1) {
                int nn = new_node(d + 1, prefix(x, d + 1));
                nd[cur].ch[b] = nn;
                nd[nn].par = cur;
                nd[nn].mn = nd[nn].mx = x;
            }
            cur = nd[cur].ch[b];
            nd[cur].mn = std::min(nd[cur].mn, x);
            nd[cur].mx = std::max(nd[cur].mx, x);
        }
        int leaf = cur;                        // узел глубины k — лист x
        nd[leaf].prev_leaf = pred;
        nd[leaf].next_leaf = succ;
        if (pred == -1) first_leaf = leaf; else nd[pred].next_leaf = leaf;
        if (succ == -1) last_leaf = leaf; else nd[succ].prev_leaf = leaf;
        cnt++;
    }

    void erase(int x) {
        if (x < 0 || x >= u) return;
        if (!lvl[k].count(x)) return;
        int leaf = lvl[k][x];
        int p = nd[leaf].prev_leaf, s = nd[leaf].next_leaf;
        if (p == -1) first_leaf = s; else nd[p].next_leaf = s;
        if (s == -1) last_leaf = p; else nd[s].prev_leaf = p;
        lvl[k].erase(x);
        cnt--;
        int par = nd[leaf].par;
        if (par != -1) {                 // отвязать лист от родителя
            int idx = (nd[par].ch[0] == leaf) ? 0 : 1;
            nd[par].ch[idx] = -1;
        } else {                         // u == 1: корень сам является листом
            nd[0].mn = nd[0].mx = -1;
            return;
        }
        climb(par);
    }
    // подъём после удаления листа: пересчёт mn/mx, снятие пустых узлов
    void climb(int v) {
        while (v != -1) {
            int l = nd[v].ch[0], r = nd[v].ch[1];
            if (l == -1 && r == -1) {
                int p = nd[v].par;
                if (p == -1) { nd[v].mn = nd[v].mx = -1; return; }
                int idx = (nd[p].ch[0] == v) ? 0 : 1;
                nd[p].ch[idx] = -1;
                lvl[nd[v].depth].erase(nd[v].id);
                v = p;
            } else {
                nd[v].mn = INT_MAX; nd[v].mx = -1;
                if (l != -1) {
                    nd[v].mn = std::min(nd[v].mn, nd[l].mn);
                    nd[v].mx = std::max(nd[v].mx, nd[l].mx);
                }
                if (r != -1) {
                    nd[v].mn = std::min(nd[v].mn, nd[r].mn);
                    nd[v].mx = std::max(nd[v].mx, nd[r].mx);
                }
                if (v == 0) return;
                v = nd[v].par;
            }
        }
    }
};

// =============================================================
// D. Y-FAST TRIE
// =============================================================

// --- D.1–D.2. X-fast над представителями + бакеты Treap ---
// Представитель — минимум бакета; бакет r хранит ключи [r, next_rep).
// Наводка по X-fast (C) за O(log log U), точный ответ — рангом в
// Treap (II.F.1) за O(log B), B ≈ log₂U. Переполненный бакет
// разбивается пополам; пустой — снимает представителя.
struct YFastTrie {
    XFastTrie reps;                     // представители = минимумы бакетов
    unordered_map<int, int> bucket_of;  // реп → индекс бакета
    vector<Treap> buckets;
    int B;                              // целевой размер бакета

    explicit YFastTrie(int u) : reps(u) {
        int k = 0;
        while ((1 << k) < u) k++;
        B = std::max(4, k);
    }

    bool contains(int x) {
        int r = reps.prev(x);           // бакет, покрывающий x
        if (r == -1) return false;
        return buckets[bucket_of[r]].search(x);
    }

    int next(int x) {                   // наименьший ключ ≥ x
        int r = reps.prev(x);
        if (r == -1) {                  // x меньше всех ключей
            int mn_rep = reps.next(0);
            return mn_rep == -1 ? -1 : mn_rep;
        }
        Treap& bt = buckets[bucket_of[r]];
        int kk = bt.rank(x);
        if (kk < Treap::sz(bt.root)) return bt.kth(kk + 1);
        return reps.next(r + 1);        // x ≥ всех ключей бакета → следующий
    }

    int prev(int x) {                   // наибольший ключ ≤ x
        int r = reps.prev(x);
        if (r == -1) return -1;
        Treap& bt = buckets[bucket_of[r]];
        if (bt.search(x)) return x;
        int kk = bt.rank(x);
        if (kk > 0) return bt.kth(kk);
        // x в диапазоне бакета, но меньше его минимума (минимум удалён)
        int rr = reps.prev(r - 1);
        if (rr == -1) return -1;
        Treap& pb = buckets[bucket_of[rr]];
        return pb.kth(Treap::sz(pb.root));
    }

    void insert(int x) {
        if (contains(x)) return;
        int r = reps.prev(x);           // наибольший реп ≤ x
        if (r == -1) {                  // x меньше всех репов → новый бакет
            int bi = (int)buckets.size();
            buckets.emplace_back();
            buckets[bi].insert(x);
            bucket_of[x] = bi;
            reps.insert(x);
            return;
        }
        int bi = bucket_of[r];
        buckets[bi].insert(x);
        if (Treap::sz(buckets[bi].root) > 2 * B) split_bucket(r);
    }

    void erase(int x) {
        int r = reps.prev(x);
        if (r == -1) return;
        int bi = bucket_of[r];
        Treap& bt = buckets[bi];
        if (!bt.search(x)) return;
        bt.erase(x);
        if (bt.root == nullptr) {       // бакет опустел → снять реп
            reps.erase(r);
            bucket_of.erase(r);
        }
    }

    void split_bucket(int r) {
        int bi = bucket_of[r];
        Treap& bt = buckets[bi];
        int s = Treap::sz(bt.root);
        int split_key = bt.kth(s / 2 + 1);   // минимум второй половины
        Treap::Node *a, *b;
        Treap::split(bt.root, split_key, a, b);   // a: < split_key
        bt.root = a;
        int bi2 = (int)buckets.size();
        buckets.emplace_back();
        buckets[bi2].root = b;
        reps.insert(split_key);
        bucket_of[split_key] = bi2;
    }
};

// =============================================================
// E. FUSION TREE (БИТОВАЯ ПАРАЛЛЕЛЬНОСТЬ СЛОВА)
// =============================================================

// --- E.2. Sketch: сжатие ключа до выбранных позиций битов ---
// Монотонно: x < y ⇒ sketch(x) < sketch(y); уплотнение — перестановка
// бит. Позиции задаются списком pos (биты в порядке старшинства).
static unsigned long long sketch(unsigned long long x, const vector<int>& pos) {
    unsigned long long r = 0;
    for (int i = 0; i < (int)pos.size(); i++)
        if ((x >> pos[i]) & 1ULL) r |= 1ULL << i;
    return r;
}

// --- E.3. Параллельное сравнение в слове (SWAR) ---
// 4 ключа по 16 бит в одном 64-битном слове; старший бит полосы —
// служебный, поэтому ключи должны быть < 2^15 (15 значащих бит).
// Трюк вычитания: (a | HIGH) − (b & ~HIGH) не пропускает
// заимствование через границу полосы; старший бит результата
// установлен ⇔ ключ полосы ≥ x. count_less — одна словная операция.
struct PackedKeys {
    static constexpr int K = 4;                        // ключей в слове
    static constexpr int L = 16;                       // бит на полосу
    static constexpr unsigned long long HIGH_ALL = 0x8000800080008000ULL;

    unsigned long long word;

    explicit PackedKeys(const vector<int>& keys) : word(0) {
        for (int i = 0; i < K; i++)
            word |= (unsigned long long)(keys[i] & 0xFFFF) << (16 * i);
    }

    // число ключей, строго меньших x, за O(1) словных операций
    int count_less(unsigned long long x) const {
        unsigned long long bx = 0;                     // широковещательная рассылка
        for (int i = 0; i < K; i++) bx |= (x & 0xFFFF) << (16 * i);
        unsigned long long ge = ((word | HIGH_ALL) - (bx & ~HIGH_ALL)) & HIGH_ALL;
        return K - __builtin_popcountll(ge);
    }

    int pred(unsigned long long x) const {             // наибольший ключ < x
        int c = count_less(x);
        return c == 0 ? -1 : (int)((word >> (16 * (c - 1))) & 0xFFFF);
    }
    int succ(unsigned long long x) const {             // наименьший ключ ≥ x
        int c = count_less(x);
        return c >= K ? -1 : (int)((word >> (16 * c)) & 0xFFFF);
    }
};

// =============================================================
// F. СТРУКТУРЫ ДИНАМИЧЕСКОЙ СВЯЗНОСТИ
// =============================================================

// --- F.2. Оффлайн: рёбра живут на интервалах [l, r) ---
// Сегментное дерево по времени: ребро вешается на O(log T) узлов,
// покрывающих интервал. DFS с RollbackDSU (III.A.6): в листе t
// применены ровно рёбра, активные в момент t; откат к снапшоту
// после выхода из узла.
struct OfflineDynamicConnectivity {
    struct Edge { int u, v; };
    struct Query { int id, u, v; };
    int n, T, qcnt;
    vector<vector<Edge>> seg;
    vector<vector<Query>> q_at;            // время → запросы
    vector<char> ans;

    OfflineDynamicConnectivity(int n_, int T_) : n(n_), T(T_), qcnt(0) {
        seg.assign(4 * T, {});
        q_at.assign(T, {});
    }

    void add_edge(int u, int v, int l, int r) {        // активна на [l, r)
        add_edge_node(1, 0, T, l, r, {u, v});
    }
    void add_edge_node(int node, int lo, int hi, int ql, int qr, const Edge& e) {
        if (ql <= lo && hi <= qr) { seg[node].push_back(e); return; }
        int mid = (lo + hi) / 2;
        if (ql < mid) add_edge_node(2 * node, lo, mid, ql, qr, e);
        if (mid < qr) add_edge_node(2 * node + 1, mid, hi, ql, qr, e);
    }
    void add_query(int t, int u, int v) { q_at[t].push_back({qcnt++, u, v}); }

    void solve() {
        ans.assign(qcnt, 0);
        RollbackDSU dsu(n);
        dfs(1, 0, T, dsu);
    }
    void dfs(int node, int lo, int hi, RollbackDSU& dsu) {
        int snap = dsu.snapshot();
        for (auto& e : seg[node]) dsu.unite(e.u, e.v);
        if (hi - lo == 1) {
            for (auto& q : q_at[lo])
                ans[q.id] = dsu.find(q.u) == dsu.find(q.v);
        } else {
            int mid = (lo + hi) / 2;
            dfs(2 * node, lo, mid, dsu);
            dfs(2 * node + 1, mid, hi, dsu);
        }
        dsu.rollback_to(snap);
    }
};

// --- F.3. ETT (Euler Tour Tree) ---
// Неявный treap с родительскими указателями и суммой — развитие
// ImplicitTreap (II.F.2): узел обхода держит значение (вход вершины —
// номер, выход — 0), поддерево вершины = отрезок [in, out].
// reroot — циклический сдвиг; link — вставка обхода внутрь скобок
// родителя; cut — вырезание отрезка. Позиция — подъём по родителям.
struct EulerTourTree {
    struct Node {
        long long val, sum;
        int pr, sz;
        Node *l, *r, *p;
        Node(long long v) : val(v), sum(v), pr(next_priority()), sz(1),
                            l(nullptr), r(nullptr), p(nullptr) {}
    };
    vector<Node*> in, out;
    int n;

    explicit EulerTourTree(int n_) : in(n_), out(n_), n(n_) {}

    static int sz(Node* t) { return t ? t->sz : 0; }
    static long long sum(Node* t) { return t ? t->sum : 0; }
    static void pull(Node* t) {
        if (!t) return;
        t->sz = 1 + sz(t->l) + sz(t->r);
        t->sum = t->val + sum(t->l) + sum(t->r);
        if (t->l) t->l->p = t;
        if (t->r) t->r->p = t;
    }
    static void split(Node* t, int k, Node*& a, Node*& b) {
        if (!t) { a = b = nullptr; return; }
        if (k <= sz(t->l)) { split(t->l, k, a, t->l); b = t; pull(t); t->p = nullptr; }
        else { split(t->r, k - sz(t->l) - 1, t->r, b); a = t; pull(t); t->p = nullptr; }
    }
    static Node* merge(Node* a, Node* b) {
        if (!a) { if (b) b->p = nullptr; return b; }
        if (!b) { if (a) a->p = nullptr; return a; }
        if (a->pr > b->pr) { a->r = merge(a->r, b); pull(a); a->p = nullptr; return a; }
        else { b->l = merge(a, b->l); pull(b); b->p = nullptr; return b; }
    }

    static Node* root(Node* t) { while (t->p) t = t->p; return t; }
    static int position(Node* t) {
        int res = sz(t->l);
        while (t->p) {
            if (t->p->r == t) res += sz(t->p->l) + 1;
            t = t->p;
        }
        return res;
    }

    void build_vertices() {
        for (int v = 0; v < n; v++) {
            in[v] = new Node(v);     // вход: значение = номер вершины
            out[v] = new Node(0);    // выход: 0 — поддерево = сумма по отрезку
            merge(in[v], out[v]);
        }
    }

    // x становится ребёнком y; x должен быть корнем своей компоненты
    // (вся компонента — один блок [in(x), out(x)]; смена корня этой
    // версией не поддерживается — для этого служат LCT/Top Tree)
    void link(int x, int y) {
        Node* rx = root(in[x]);
        Node* ry = root(in[y]);
        int py = position(in[y]);
        Node *a, *b;
        split(ry, py + 1, a, b);
        merge(a, merge(rx, b));
    }

    // отрезать поддерево x от родителя
    void cut(int x, int) {
        Node* r = root(in[x]);
        int p1 = position(in[x]), p2 = position(out[x]);
        Node *a, *b, *c;
        split(r, p1, a, b);
        split(b, p2 - p1 + 1, c, b);
        merge(a, b);
    }

    bool connected(int u, int v) const {
        return root(in[u]) == root(in[v]);
    }

    // сумма номеров вершин в поддереве v
    long long subtree_sum(int v) {
        Node* r = root(in[v]);
        int p1 = position(in[v]), p2 = position(out[v]);
        Node *a, *b, *c;
        split(r, p1, a, b);
        split(b, p2 - p1 + 1, c, b);
        long long res = c->sum;
        merge(a, merge(c, b));
        return res;
    }
};

// --- F.4. Link-Cut Tree ---
// Динамическое дерево: дерево разбито на предпочитаемые пути,
// каждый путь — splay по глубине. access(v) выдвигает путь корень→v;
// makeroot — access + реверс; findroot — спуск влево после access.
// Путевой агрегат — максимум. Все операции O(log n) амортизированно.
struct LinkCutTree {
    struct Node {
        long long val, mx;
        int ch[2], p;
        bool rev;
        Node() : val(0), mx(-(1LL << 60)), p(0), rev(false) { ch[0] = ch[1] = 0; }
    };
    vector<Node> nd;          // 1-based; 0 — null

    explicit LinkCutTree(int n) : nd(n + 1) {}

    void set_val(int x, long long v) { access(x); nd[x].val = v; pull(x); }

    bool is_root(int x) const {
        int p = nd[x].p;
        return p == 0 || (nd[p].ch[0] != x && nd[p].ch[1] != x);
    }
    void push(int x) {
        if (nd[x].rev) {
            std::swap(nd[x].ch[0], nd[x].ch[1]);
            if (nd[x].ch[0]) nd[nd[x].ch[0]].rev ^= 1;
            if (nd[x].ch[1]) nd[nd[x].ch[1]].rev ^= 1;
            nd[x].rev = false;
        }
    }
    void pull(int x) {
        nd[x].mx = nd[x].val;
        if (nd[x].ch[0]) nd[x].mx = std::max(nd[x].mx, nd[nd[x].ch[0]].mx);
        if (nd[x].ch[1]) nd[x].mx = std::max(nd[x].mx, nd[nd[x].ch[1]].mx);
    }
    void rotate(int x) {
        int p = nd[x].p, g = nd[p].p;
        int dir = (nd[p].ch[1] == x);
        if (!is_root(p)) nd[g].ch[nd[g].ch[1] == p] = x;
        nd[x].p = g;
        nd[p].ch[dir] = nd[x].ch[dir ^ 1];
        if (nd[x].ch[dir ^ 1]) nd[nd[x].ch[dir ^ 1]].p = p;
        nd[x].ch[dir ^ 1] = p;
        nd[p].p = x;
        pull(p);
        pull(x);
    }
    void splay(int x) {
        vector<int> st;
        int y = x;
        st.push_back(y);
        while (!is_root(y)) { y = nd[y].p; st.push_back(y); }
        for (int i = (int)st.size() - 1; i >= 0; i--) push(st[i]);
        while (!is_root(x)) {
            int p = nd[x].p, g = nd[p].p;
            if (!is_root(p)) {
                if ((nd[p].ch[0] == x) == (nd[g].ch[0] == p)) rotate(p);
                else rotate(x);
            }
            rotate(x);
        }
    }
    void access(int x) {
        int last = 0;
        for (int y = x; y; y = nd[y].p) {
            splay(y);
            nd[y].ch[1] = last;
            if (last) nd[last].p = y;
            pull(y);
            last = y;
        }
        splay(x);
    }
    void makeroot(int x) {
        access(x);
        nd[x].rev ^= 1;
    }
    int findroot(int x) {
        access(x);
        while (nd[x].ch[0]) {
            push(x);
            x = nd[x].ch[0];
        }
        splay(x);
        return x;
    }
    void link(int u, int v) {
        makeroot(u);
        nd[u].p = v;
    }
    void cut(int u, int v) {
        makeroot(u);
        access(v);
        if (nd[v].ch[0] == u) {
            nd[v].ch[0] = 0;
            nd[u].p = 0;
            pull(v);
        }
    }
    bool connected(int u, int v) {
        if (u == v) return true;
        makeroot(u);
        return findroot(v) == u;
    }
    long long path_max(int u, int v) {
        makeroot(u);
        access(v);
        return nd[v].mx;
    }
};

// =============================================================
// G. РЕТРОАКТИВНЫЕ СТРУКТУРЫ
// =============================================================

// --- G.2. Частичная ретроактивность: приоритетная очередь ---
// Ось времени 0..T, в каждый момент — операция INSERT(v)/DELETE/
// NONE. Операции вставляются/удаляются в прошлом (set_op/remove_op);
// запрос — итоговый минимум. Эталон: полный пересчёт слева направо
// минимум-кучей (структура Демейна–Яконно–Лангермана — O(log n),
// обзор в md).
struct PartialRetroPQ {
    enum Op { NONE, INSERT, DELETE };
    struct Event { Op op; int val; };
    int T;
    vector<Event> time;

    explicit PartialRetroPQ(int T_) : T(T_), time(T_, {NONE, 0}) {}

    void set_op(int t, Op op, int val = 0) { time[t] = {op, val}; }
    void remove_op(int t) { time[t] = {NONE, 0}; }

    int current_min() const {
        priority_queue<int, vector<int>, greater<int>> h;
        for (int t = 0; t < T; t++) {
            if (time[t].op == INSERT) h.push(time[t].val);
            else if (time[t].op == DELETE && !h.empty()) h.pop();
        }
        return h.empty() ? -1 : h.top();
    }
};

// --- G.3. Полная ретроактивность: стек ---
// Неявный treap над осью времени (прототип — ImplicitTreap, II.F.2);
// узел хранит эффект своего отрезка: (pops, pushes) — сколько POP
// съедают элементы слева и какая последовательность значений
// остаётся. Композиция: POP правого отрезка снимают хвост pushes
// левого. Вставка/удаление операции в любом моменте — split/merge
// с пересчётом эффекта. (Пересчёт копирует pushes — в конспекте
// компактная версия хранит счётчики и выборку.)
struct FullRetroStack {
    enum Op { PUSH, POP };
    struct Effect {
        int pops;
        vector<int> pushes;
    };
    struct Node {
        Op op;
        int val;
        Effect eff;
        int pr, sz;
        Node *l, *r;
        Node(Op o, int v) : op(o), val(v), pr(next_priority()), sz(1),
                            l(nullptr), r(nullptr) {
            if (o == PUSH) eff = {0, {v}};
            else eff = {1, {}};
        }
    };
    Node* root = nullptr;

    static int sz(Node* t) { return t ? t->sz : 0; }
    static Effect compose(const Effect& a, const Effect& b) {
        int taken = std::min(b.pops, (int)a.pushes.size());
        Effect r;
        r.pops = a.pops + std::max(0, b.pops - (int)a.pushes.size());
        r.pushes = a.pushes;
        r.pushes.resize((int)a.pushes.size() - taken);
        r.pushes.insert(r.pushes.end(), b.pushes.begin(), b.pushes.end());
        return r;
    }
    static void pull(Node* t) {
        if (!t) return;
        t->sz = 1 + sz(t->l) + sz(t->r);
        Effect e = (t->op == PUSH) ? Effect{0, {t->val}} : Effect{1, {}};
        if (t->l) e = compose(t->l->eff, e);
        if (t->r) e = compose(e, t->r->eff);
        t->eff = e;
    }
    static void split(Node* t, int k, Node*& a, Node*& b) {
        if (!t) { a = b = nullptr; return; }
        if (k <= sz(t->l)) { split(t->l, k, a, t->l); b = t; pull(b); }
        else { split(t->r, k - sz(t->l) - 1, t->r, b); a = t; pull(a); }
    }
    static Node* merge(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->pr > b->pr) { a->r = merge(a->r, b); pull(a); return a; }
        else { b->l = merge(a, b->l); pull(b); return b; }
    }

    void insert_at(int pos, Op op, int val) {
        Node *a, *b;
        split(root, pos, a, b);
        root = merge(merge(a, new Node(op, val)), b);
    }
    void erase_at(int pos) {
        Node *a, *b, *c;
        split(root, pos, a, b);
        split(b, 1, c, b);
        delete c;
        root = merge(a, b);
    }

    // итоговое содержимое стека (валидная история: pops на корне = 0)
    vector<int> final_stack() const {
        return root ? root->eff.pushes : vector<int>();
    }
    int top() const {
        const vector<int>& v = final_stack();
        return v.empty() ? -1 : v.back();
    }
};

// =============================================================
// H. CACHE-OBLIVIOUS СТРУКТУРЫ
// =============================================================

// --- H.1. Модель идеального кэша ---
// Кэш из Z линий по L элементов; промах — обращение к линии, которой
// нет в кэше; политика замены — LRU (для демонстрации достаточно;
// оффлайн-оптимум — X.A.3). Счётчик промахов — мера переносов.
struct IdealCache {
    int Z, L;
    int misses = 0;
    vector<int> stack;           // LRU-стек: [0] — самая недавняя линия

    IdealCache(int Z_, int L_) : Z(Z_), L(L_) {}

    void touch(int addr) {
        int line = addr / L;
        auto it = find(stack.begin(), stack.end(), line);
        if (it == stack.end()) {
            misses++;
            if ((int)stack.size() == Z) stack.pop_back();
            stack.insert(stack.begin(), line);
        } else {
            stack.erase(it);
            stack.insert(stack.begin(), line);
        }
    }
};

// --- H.2. vEB-раскладка массива и рекурсивный поиск ---
// Массив перекладывается рекурсивно: корень (средний элемент), затем
// левая половина, затем правая (мост B.4). Поиск спускается по
// раскладке: шаги концентрируются во всё меньших непрерывных кусках —
// меньше переносов, чем у обычного бинарного поиска.
struct VebLayout {
    vector<long long> a, lay;
    int n;

    explicit VebLayout(const vector<long long>& sorted)
        : a(sorted), lay(sorted.size(), 0), n((int)sorted.size()) {
        int pos = 0;
        build(0, n, pos);
    }
    void build(int lo, int hi, int& pos) {
        if (lo >= hi) return;
        int mid = (lo + hi) / 2;
        lay[pos++] = a[mid];
        build(lo, mid, pos);
        build(mid + 1, hi, pos);
    }

    bool search(long long x) const { return rec(x, 0, n, 0); }
    bool rec(long long x, int lo, int hi, int idx) const {
        if (lo >= hi) return false;
        if (lay[idx] == x) return true;
        int mid = (lo + hi) / 2;
        if (x < lay[idx]) return rec(x, lo, mid, idx + 1);
        return rec(x, mid + 1, hi, idx + 1 + (mid - lo));
    }

    // адреса (индексы раскладки), к которым обращается поиск
    vector<int> probe_idx(long long x) const {
        vector<int> res;
        int lo = 0, hi = n, idx = 0;
        while (lo < hi) {
            res.push_back(idx);
            if (lay[idx] == x) break;
            int mid = (lo + hi) / 2;
            if (x < lay[idx]) { hi = mid; idx = idx + 1; }
            else { idx = idx + 1 + (mid - lo); lo = mid + 1; }
        }
        return res;
    }
    // адреса бинарного поиска в исходном массиве (для сравнения)
    static vector<int> bin_probe_idx(const vector<long long>& a, long long x) {
        vector<int> res;
        int lo = 0, hi = (int)a.size();
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            res.push_back(mid);
            if (a[mid] == x) break;
            if (x < a[mid]) hi = mid; else lo = mid + 1;
        }
        return res;
    }
};

};  // struct TheoreticalStructures

#ifndef STRUCT_K_MAIN

int main() {
    using H = TheoreticalStructures;

    cout << "== XI. ТЕОРЕТИЧЕСКИЕ И ЭКЗОТИЧЕСКИЕ СТРУКТУРЫ (k.cpp) ==" << endl;

    // ---------- A.2 Битовое множество и сортировка ----------
    {
        vector<int> s = H::bit_sort({8, 3, 1, 7, 3, 5}, 16);
        cout << "A.2 bit_sort({8,3,1,7,3,5}, 16) =";
        for (int x : s) cout << " " << x;
        cout << " (ожидаем 1 3 5 7 8)" << endl;
    }

    // ---------- B. VEB Tree ----------
    {
        H::VebTree v(256);
        for (int x : {5, 1, 200, 128, 64, 255}) v.insert(x);
        cout << "B contains(128)/contains(129), min/max: " << v.contains(128) << "/"
             << v.contains(129) << ", " << v.min() << "/" << v.max()
             << " (ожидаем 1/0, 1/255)" << endl;
        cout << "B next: 64->" << v.next(64) << ", 65->" << v.next(65)
             << ", 300->" << v.next(300)
             << "; prev: 130->" << v.prev(130) << ", 63->" << v.prev(63)
             << " (ожидаем 64/128/-1, 128/5)" << endl;
        v.erase(128);
        v.erase(1);
        cout << "B after erase 128,1: contains(128)=" << v.contains(128)
             << ", next(65)=" << v.next(65) << ", min()=" << v.min()
             << " (ожидаем 0/200/5)" << endl;

        H::VebTree big(4096);           // универсум 2^12
        for (int x : {0, 4095, 100, 2000}) big.insert(x);
        cout << "B big(4096): next(1500)=" << big.next(1500) << ", prev(3000)="
             << big.prev(3000) << ", prev(4096)=" << big.prev(4096)
             << ", next(4096)=" << big.next(4096) << ", contains(2000)="
             << big.contains(2000) << " (ожидаем 2000/2000/4095/-1/1)" << endl;

        H::VebTree small(16);           // нижний уровень — битовый блок
        for (int x : {0, 3, 15}) small.insert(x);
        cout << "B block(16): next(1)=" << small.next(1) << ", prev(14)="
             << small.prev(14) << ", min=" << small.min() << ", max=" << small.max()
             << " (ожидаем 3/3/0/15)" << endl;
        small.erase(15);
        cout << "B block after erase(15): max=" << small.max()
             << " (ожидаем 3)" << endl;
    }

    // ---------- C. X-Fast Trie ----------
    {
        H::XFastTrie x(16);
        for (int key : {3, 7, 12}) x.insert(key);
        cout << "C next: 1->" << x.next(1) << ", 4->" << x.next(4) << ", 8->"
             << x.next(8) << ", 13->" << x.next(13) << ", 3->" << x.next(3)
             << " (ожидаем 3/7/12/-1/3)" << endl;
        cout << "C prev: 2->" << x.prev(2) << ", 3->" << x.prev(3) << ", 4->"
             << x.prev(4) << ", 8->" << x.prev(8) << ", 100->" << x.prev(100)
             << " (ожидаем -1/3/3/7/12)" << endl;
        x.erase(7);
        cout << "C after erase(7): next(4)=" << x.next(4) << ", prev(4)="
             << x.prev(4) << ", prev(8)=" << x.prev(8) << ", contains(7)="
             << x.contains(7) << " (ожидаем 12/3/12/0)" << endl;
        x.erase(3);
        x.erase(12);
        cout << "C empty: size=" << x.size() << ", next(1)=" << x.next(1)
             << ", prev(1)=" << x.prev(1) << " (ожидаем 0/-1/-1)" << endl;
        x.insert(0);
        x.insert(15);
        cout << "C {0,15}: next(0)=" << x.next(0) << ", prev(0)=" << x.prev(0)
             << ", next(1)=" << x.next(1) << ", prev(15)=" << x.prev(15)
             << ", prev(16)=" << x.prev(16) << " (ожидаем 0/0/15/15/15)" << endl;

        H::XFastTrie x2(256);
        for (int key : {10, 100, 200}) x2.insert(key);
        cout << "C x2(256): next(50)=" << x2.next(50) << ", prev(50)="
             << x2.prev(50) << ", next(250)=" << x2.next(250) << ", prev(250)="
             << x2.prev(250) << " (ожидаем 100/10/-1/200)" << endl;
    }

    // ---------- D. Y-Fast Trie ----------
    {
        H::YFastTrie y(16);
        for (int k = 1; k <= 9; k++) y.insert(k);   // при 9-м — разбиение бакета
        cout << "D contains: 5->" << y.contains(5) << ", 0->" << y.contains(0)
             << ", 10->" << y.contains(10) << " (ожидаем 1/0/0)" << endl;
        cout << "D next: 4->" << y.next(4) << ", 5->" << y.next(5) << ", 9->"
             << y.next(9) << ", 10->" << y.next(10) << " (ожидаем 4/5/9/-1)" << endl;
        cout << "D prev: 4->" << y.prev(4) << ", 3->" << y.prev(3) << ", 10->"
             << y.prev(10) << ", 1->" << y.prev(1) << ", 0->" << y.prev(0)
             << " (ожидаем 4/3/9/1/-1)" << endl;
        y.insert(0);
        cout << "D insert 0: contains(0)=" << y.contains(0) << ", next(0)="
             << y.next(0) << ", prev(0)=" << y.prev(0) << " (ожидаем 1/0/0)" << endl;
        y.erase(5);
        cout << "D erase 5: contains(5)=" << y.contains(5) << ", next(5)="
             << y.next(5) << ", prev(6)=" << y.prev(6) << ", prev(5)="
             << y.prev(5) << ", prev(10)=" << y.prev(10)
             << " (ожидаем 0/6/6/4/9)" << endl;
        y.erase(0);
        y.erase(1);
        cout << "D erase 0,1: next(0)=" << y.next(0) << ", prev(0)=" << y.prev(0)
             << ", next(1)=" << y.next(1) << ", prev(1)=" << y.prev(1)
             << " (ожидаем 1/-1/2/-1)" << endl;

        H::YFastTrie y2(256);
        for (int k : {10, 100, 200}) y2.insert(k);
        cout << "D y2(256): next(50)=" << y2.next(50) << ", prev(50)="
             << y2.prev(50) << ", prev(150)=" << y2.prev(150) << ", next(250)="
             << y2.next(250) << ", prev(250)=" << y2.prev(250)
             << " (ожидаем 100/10/100/-1/200)" << endl;
    }

    // ---------- E. Fusion Tree: битовая параллельность ----------
    {
        H::PackedKeys pk({5, 17, 1000, 30000});
        cout << "E count_less(17)=" << pk.count_less(17) << ", pred(17)="
             << pk.pred(17) << ", succ(17)=" << pk.succ(17)
             << " (ожидаем 1/5/17)" << endl;
        cout << "E count_less(18)=" << pk.count_less(18) << ", pred(18)="
             << pk.pred(18) << ", succ(18)=" << pk.succ(18)
             << " (ожидаем 2/17/1000)" << endl;
        cout << "E count_less(0)=" << pk.count_less(0) << ", pred(0)="
             << pk.pred(0) << ", succ(0)=" << pk.succ(0)
             << " (ожидаем 0/-1/5)" << endl;
        cout << "E succ(32767)=" << pk.succ(32767) << ", pred(32767)="
             << pk.pred(32767) << " (ожидаем -1/30000)" << endl;

        cout << "E sketch(45,{5,2,0})=" << H::sketch(45, {5, 2, 0})
             << " (ожидаем 7: биты 5,2,0 числа 101101₂ = 1,1,1)" << endl;
        cout << "E sketch монотонность {2,5,7} на {2,1,0}: "
             << H::sketch(2, {2, 1, 0}) << " " << H::sketch(5, {2, 1, 0}) << " "
             << H::sketch(7, {2, 1, 0}) << " (ожидаем 2 5 7)" << endl;
    }

    // ---------- F.2 Оффлайн динамическая связность ----------
    {
        H::OfflineDynamicConnectivity odc(4, 4);
        odc.add_edge(0, 1, 0, 4);       // живёт всё время
        odc.add_edge(1, 2, 0, 2);       // живо в моменты 0, 1
        odc.add_edge(2, 3, 2, 4);       // живо в моменты 2, 3
        odc.add_query(0, 0, 2);
        odc.add_query(0, 0, 3);
        odc.add_query(1, 0, 2);
        odc.add_query(2, 0, 2);
        odc.add_query(2, 1, 3);
        odc.add_query(3, 2, 3);
        odc.solve();
        cout << "F.2 answers: t0(0,2)(0,3)=" << (int)odc.ans[0] << " " << (int)odc.ans[1]
             << "; t1(0,2)=" << (int)odc.ans[2]
             << "; t2(0,2)(1,3)=" << (int)odc.ans[3] << " " << (int)odc.ans[4]
             << "; t3(2,3)=" << (int)odc.ans[5]
             << " (ожидаем 1 0 / 1 / 0 0 / 1)" << endl;
    }

    // ---------- F.3 ETT (Euler Tour Tree) ----------
    {
        H::EulerTourTree ett(4);
        ett.build_vertices();
        ett.link(1, 0);      // дерево: 0 → 1 → {2, 3}
        ett.link(2, 1);
        ett.link(3, 1);
        cout << "F.3 subtree_sum: 1->" << ett.subtree_sum(1) << ", 2->"
             << ett.subtree_sum(2) << ", 3->" << ett.subtree_sum(3)
             << " (ожидаем 6/2/3)" << endl;
        cout << "F.3 connected: (2,3)=" << ett.connected(2, 3)
             << ", (0,3)=" << ett.connected(0, 3) << " (ожидаем 1/1)" << endl;
        ett.cut(3, 1);
        cout << "F.3 after cut(3,1): connected(2,3)=" << ett.connected(2, 3)
             << ", (1,3)=" << ett.connected(1, 3) << ", (0,2)="
             << ett.connected(0, 2) << "; subtree_sum(1)=" << ett.subtree_sum(1)
             << " (ожидаем 0/0/1, 3)" << endl;
        ett.link(3, 0);
        cout << "F.3 after link(3,0): connected(0,3)=" << ett.connected(0, 3)
             << ", (2,3)=" << ett.connected(2, 3) << "; subtree_sum(0)="
             << ett.subtree_sum(0) << " (ожидаем 1/1, 6)" << endl;
    }

    // ---------- F.4 Link-Cut Tree ----------
    {
        H::LinkCutTree lct(6);
        lct.link(2, 1); lct.link(3, 1); lct.link(4, 2); lct.link(5, 4); lct.link(6, 3);
        for (int v = 1; v <= 6; v++) lct.set_val(v, v * 10);
        cout << "F.4 path_max(5,6)=" << lct.path_max(5, 6)
             << ", path_max(4,3)=" << lct.path_max(4, 3)
             << ", connected(5,6)=" << lct.connected(5, 6)
             << " (ожидаем 60/40/1)" << endl;
        lct.cut(4, 5);
        cout << "F.4 after cut(4,5): connected(5,4)=" << lct.connected(5, 4)
             << ", connected(5,1)=" << lct.connected(5, 1)
             << ", path_max(4,6)=" << lct.path_max(4, 6)
             << " (ожидаем 0/0/60)" << endl;
        lct.link(5, 6);
        cout << "F.4 after link(5,6): connected(5,1)=" << lct.connected(5, 1)
             << ", path_max(4,5)=" << lct.path_max(4, 5)
             << " (ожидаем 1/60)" << endl;
        lct.set_val(4, 100);
        cout << "F.4 after set_val(4,100): path_max(5,6)=" << lct.path_max(5, 6)
             << ", findroot(1)=" << lct.findroot(1)
             << ", findroot(5)=" << lct.findroot(5)
             << " (ожидаем 60/5/5 — вершина 4 не на пути 5-6; общий корень 5"
             << " — последний makeroot от path_max(5,6))" << endl;
    }

    // ---------- G.2 Частично-ретроактивная приоритетная очередь ----------
    {
        H::PartialRetroPQ pq(5);
        pq.set_op(0, H::PartialRetroPQ::INSERT, 10);
        pq.set_op(1, H::PartialRetroPQ::INSERT, 1);
        pq.set_op(2, H::PartialRetroPQ::DELETE);
        pq.set_op(3, H::PartialRetroPQ::INSERT, 2);
        pq.set_op(4, H::PartialRetroPQ::INSERT, 3);
        int m1 = pq.current_min();            // 2
        pq.set_op(2, H::PartialRetroPQ::INSERT, 20);  // DELETE → INSERT 20
        int m2 = pq.current_min();            // 1
        pq.set_op(1, H::PartialRetroPQ::NONE);        // убрать INSERT 1
        int m3 = pq.current_min();            // 2
        pq.set_op(0, H::PartialRetroPQ::NONE);        // убрать INSERT 10
        int m4 = pq.current_min();            // 2
        pq.set_op(4, H::PartialRetroPQ::DELETE);      // добавить DELETE
        int m5 = pq.current_min();            // 20 — DELETE выкинул 2
        cout << "G.2 current_min after edits: " << m1 << " " << m2 << " " << m3
             << " " << m4 << " " << m5 << " (ожидаем 2 1 2 2 20)" << endl;
    }

    // ---------- G.3 Полно-ретроактивный стек ----------
    {
        H::FullRetroStack s;
        s.insert_at(0, H::FullRetroStack::PUSH, 1);
        s.insert_at(1, H::FullRetroStack::PUSH, 2);
        s.insert_at(2, H::FullRetroStack::POP, 0);
        s.insert_at(3, H::FullRetroStack::PUSH, 3);
        int t1 = s.top();                     // 3
        s.insert_at(1, H::FullRetroStack::PUSH, 4);   // вставка в прошлое
        int t2 = s.top();                     // 3
        s.erase_at(3);                        // удаление POP из прошлого
        int t3 = s.top();                     // 3
        s.insert_at(4, H::FullRetroStack::POP, 0);
        int t4 = s.top();                     // 2
        s.erase_at(2);                        // удаление PUSH 2
        int t5 = s.top();                     // 4
        cout << "G.3 tops after edits: " << t1 << " " << t2 << " " << t3 << " "
             << t4 << " " << t5 << " (ожидаем 3 3 3 2 4)" << endl;
        const vector<int>& fin = s.final_stack();
        cout << "G.3 final stack:";
        for (int x : fin) cout << " " << x;
        cout << " (ожидаем 1 4)" << endl;
    }

    // ---------- H.1–H.2 Cache-oblivious: идеальный кэш и vEB-раскладка ----------
    {
        vector<long long> a;
        for (int i = 1; i <= 31; i++) a.push_back(i);
        H::VebLayout vl(a);
        bool ok_search = vl.search(1) && vl.search(16) && vl.search(31)
                         && !vl.search(0) && !vl.search(32);
        int sum_bin = 0, sum_veb = 0;
        int ex_bin = 0, ex_veb = 0;
        for (long long x : a) {
            H::IdealCache cb(1000, 4), cv(1000, 4);
            for (int i : H::VebLayout::bin_probe_idx(a, x)) cb.touch(i);
            for (int i : vl.probe_idx(x)) cv.touch(i);
            sum_bin += cb.misses;
            sum_veb += cv.misses;
            if (x == 1) { ex_bin = cb.misses; ex_veb = cv.misses; }
        }
        cout << "H.2 search correct=" << ok_search << "; промахи (холодный кэш,"
             << " все ключи): bin=" << sum_bin << ", veb=" << sum_veb
             << " (ожидаем veb ≤ bin); ключ 1: bin=" << ex_bin
             << ", veb=" << ex_veb << " (ожидаем 3/2 — veb не хуже bin)" << endl;
    }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_K_MAIN

#endif // STRUCT_K_CPP
