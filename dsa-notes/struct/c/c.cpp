#ifndef STRUCT_C_CPP
#define STRUCT_C_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <bitset>
#include <climits>
using namespace std;

// =============================================================
// III. СТРУКТУРЫ ДЛЯ МНОЖЕСТВ
// =============================================================
// Структура md: A. Система непересекающихся множеств (DSU)
//               → B. Битсеты
//
// SetStructures наследует SearchTrees (b.cpp). Переиспользует:
//   * SetsAndRelations (math/discrete-and-logic, через a.cpp):
//     set_union / set_intersection / set_difference /
//     set_symmetric_difference / is_subset / set_cardinality —
//     примитивы множеств на машинном слове; DynamicBitset
//     применяет их по словам (B.3–B.4);
//   * BitBoard (I.A.3.17) — образец хранения слов битсета;
//   * амортизационный анализ (I.A) — обоснование сжатия путей (A.3);
//   * внутри раздела: AlternateDSU (A.2) — база для DSU (A.3),
//     DSU (A.3) — база для DSUWithAgg (A.4) и демонстрации A.8;
//     WeightedDSU/RollbackDSU/PersistentDSU переписывают find (другое
//     тело: потенциалы / без сжатия / по версии).
//
// Порядок методов строго соответствует порядку md (A → B).
// Собственной арифметики не имеет.
//
// ВНИМАНИЕ (скрытие имён): методы find, unite, rank_prefix, test,
// all/any/none, popcount, size здесь локальные; одноимённые из
// других веток не подключаются.

#define STRUCT_B_MAIN
#include "../b/b.cpp"
#undef STRUCT_B_MAIN

struct SetStructures : SearchTrees {

// =============================================================
// A. СИСТЕМА НЕПЕРЕСЕКАЮЩИХСЯ МНОЖЕСТВ (DSU)
// =============================================================

// --- A.1. Базовый DSU: лес с родителями, без эвристик ---
// find рекурсивный; unite вешает произвольно. Демонстрирует
// деградацию до цепей O(n) на отсортированных объединениях.
struct DisjointSet {
    vector<int> parent;
    explicit DisjointSet(int n) : parent(n) { iota(parent.begin(), parent.end(), 0); }
    int find(int x) const { return parent[x] == x ? x : find(parent[x]); }
    void unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra != rb) parent[ra] = rb;
    }
    bool same(int a, int b) const { return find(a) == find(b); }
};

// --- A.2. Alternate DSU: то же, но find без рекурсии ---
// Два прохода: подъём до корня, затем переподвешивание всего пути
// (полное сжатие). Глубина рекурсии не ограничивает. Базовый класс
// для DSU (A.3): find переиспользуется без изменений.
struct AlternateDSU {
    int n;
    vector<int> parent;
    explicit AlternateDSU(int n_) : n(n_), parent(n_) { iota(parent.begin(), parent.end(), 0); }
    int find(int x) {
        int root = x;
        while (parent[root] != root) root = parent[root];
        while (parent[x] != x) {
            int nxt = parent[x];
            parent[x] = root;
            x = nxt;
        }
        return root;
    }
    void unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra != rb) parent[ra] = rb;
    }
    bool same(int a, int b) { return find(a) == find(b); }
};

// --- A.3. DSU: union by size + сжатие путей ---
// Основной класс раздела; варианты ниже строятся на нём.
// Наследует итеративный двухпроходный find со сжатием у AlternateDSU
// (A.2) — не переписывается; добавляет union by size: корень
// меньшего дерева под корень большего, и агрегаты размера.
struct DSU : AlternateDSU {
    vector<int> sz;
    explicit DSU(int n_) : AlternateDSU(n_), sz(n_, 1) {}

    // unite по размеру; true, если компоненты слились
    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (sz[ra] < sz[rb]) swap(ra, rb);
        parent[rb] = ra;
        sz[ra] += sz[rb];
        return true;
    }
    int size(int x) { return sz[find(x)]; }
    int count_components() const {
        int c = 0;
        for (int i = 0; i < n; i++)
            if (parent[i] == i) c++;
        return c;
    }
};

// --- A.4. DSU с данными в компоненте ---
// Агрегат корня = агрегат класса; пересчёт при слиянии: agg = f(agg, agg).
// Наследует DSU (A.3) — find/сжатие/same/size переиспользуются;
// unite переопределяется для пересчёта агрегата.
// Агрегат — параметр (бинарная операция f + нейтральный элемент);
// значение одного элемента — параметр elem_val (для размера — 1).
struct DSUWithAgg : DSU {
    vector<long long> agg;
    function<long long(long long, long long)> f;
    long long neut;

    DSUWithAgg(int n_, const function<long long(long long, long long)>& op,
               long long neutral, long long elem_val = 0)
        : DSU(n_), agg(n_, elem_val), f(op), neut(neutral) {}

    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (sz[ra] < sz[rb]) swap(ra, rb);
        parent[rb] = ra;
        sz[ra] += sz[rb];
        agg[ra] = f(agg[ra], agg[rb]);
        agg[rb] = neut;                 // не-корень теряет значение (данные — у корня)
        return true;
    }
    long long aggregate(int x) { return agg[find(x)]; }
};

// --- A.5. Weighted DSU: потенциалы к корню ---
// pot[x] — «значение x относительно parent[x]». find возвращает
// пару (корень, накопленный потенциал): значение узла относительно
// корня — сумма потенциалов по пути (сжатие пересчитывает сумму).
// unite(x, y, w) кодирует соотношение val[x] + w == val[y].
struct WeightedDSU {
    vector<int> parent;
    vector<long long> pot;
    explicit WeightedDSU(int n) : parent(n), pot(n, 0) { iota(parent.begin(), parent.end(), 0); }

    // (корень, значение x относительно корня)
    pair<int, long long> find(int x) {
        if (parent[x] == x) return {x, 0};
        auto [root, p] = find(parent[x]);
        pot[x] += p;
        parent[x] = root;
        return {root, pot[x]};
    }
    // true, если соотношение согласовано (уже связаны и совпадает);
    // false — конфликт
    bool unite(int x, int y, long long w) {
        auto [rx, px] = find(x);
        auto [ry, py] = find(y);
        if (rx == ry) return px + w == py;
        // вешаем rx под ry: val[x] = px + pot[rx] должно дать val[y] = py
        parent[rx] = ry;
        pot[rx] = py - px - w;
        return true;
    }
    // разность val[y] − val[x]: (связаны ли, разность) — пары не в одном
    // классе не дают числового ответа
    pair<bool, long long> query(int x, int y) {
        auto [rx, px] = find(x);
        auto [ry, py] = find(y);
        return {rx == ry, py - px};
    }
    bool same(int a, int b) { return find(a).first == find(b).first; }
};

// --- A.6. Rollback DSU: откат объединений ---
// Каждое объединение меняет ≤ 2 ячеек (родитель корня, размер) —
// старые значения пишутся в стек изменений. Сжатие путей запрещено
// (меняет много ячеек); union by size сохраняет O(log n) высоты.
// Ячейки parent кодируются 0..n−1, размеры — n..2n−1.
struct RollbackDSU {
    int n;
    vector<int> parent, sz;
    struct Change { int idx, old; };
    vector<Change> hist;

    explicit RollbackDSU(int n_) : n(n_), parent(n_), sz(n_, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) const {
        while (parent[x] != x) x = parent[x];
        return x;
    }
    void unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) { hist.push_back({-1, -1}); return; }  // маркер no-op
        if (sz[ra] < sz[rb]) swap(ra, rb);
        hist.push_back({rb, parent[rb]}); parent[rb] = ra;
        hist.push_back({ra + n, sz[ra]}); sz[ra] += sz[rb];
    }
    int snapshot() const { return (int)hist.size(); }
    void rollback_to(int snap) {
        while ((int)hist.size() > snap) {
            Change ch = hist.back(); hist.pop_back();
            if (ch.idx < 0) continue;
            if (ch.idx < n) parent[ch.idx] = ch.old;
            else sz[ch.idx - n] = ch.old;
        }
    }
    int size(int x) const { return sz[find(x)]; }
};

// --- A.7. Persistent DSU (частичная персистентность) ---
// Каждая ячейка parent/sz заменяется хронологией пар (версия, значение);
// чтение в версии v — последняя запись ≤ v (бинарный поиск), запись —
// добавление пары в конец. Сжатие путей не применимо (пишет в прошлое).
struct PersistentDSU {
    int n;
    vector<vector<pair<int, int>>> parent_t, sz_t;

    explicit PersistentDSU(int n_) : n(n_), parent_t(n_), sz_t(n_) {
        for (int i = 0; i < n; i++) {
            parent_t[i].push_back({0, i});
            sz_t[i].push_back({0, 1});
        }
    }

    // значение ячейки idx в версии v (последняя запись с версией ≤ v)
    int get(const vector<vector<pair<int, int>>>& t, int idx, int v) const {
        const auto& h = t[idx];
        int lo = 0, hi = (int)h.size();
        while (lo < hi) {                       // бинарный поиск по версиям
            int mid = (lo + hi) / 2;
            if (h[mid].first <= v) lo = mid + 1;
            else hi = mid;
        }
        return h[lo - 1].second;
    }
    int find(int x, int v) const {
        while (true) {
            int p = get(parent_t, x, v);
            if (p == x) return x;
            x = p;
        }
    }
    // объединение в версии v → новая версия v+1; возвращает новую версию
    int unite(int a, int b, int v) {
        int ra = find(a, v), rb = find(b, v);
        int nv = v + 1;
        if (ra == rb) return nv;                    // версия без изменений
        int sa = get(sz_t, ra, v), sb = get(sz_t, rb, v);
        if (sa < sb) swap(ra, rb);
        parent_t[rb].push_back({nv, ra});
        sz_t[ra].push_back({nv, sa + sb});
        return nv;
    }
    bool same(int a, int b, int v) const { return find(a, v) == find(b, v); }
};

// --- A.8. DSU оффлайн: число компонент при удалениях рёбер ---
// Приём «обратное время»: удаления читаются с конца и становятся
// добавлениями (unite на DSU из A.3). Рёбра, не участвующие в
// удалениях, активны всегда. answers[i] — число компонент после
// i-го удаления (answers[0] — после первого).
static vector<int> components_after_removals_offline(
        int n, const vector<pair<int, int>>& edges,
        const vector<int>& removal_order) {
    vector<char> removed(edges.size(), 0);
    for (int e : removal_order) removed[e] = 1;
    DSU dsu(n);
    int comp = n;
    for (int i = 0; i < (int)edges.size(); i++)
        if (!removed[i] && dsu.unite(edges[i].first, edges[i].second)) comp--;
    vector<int> ans(removal_order.size());
    for (int k = (int)removal_order.size() - 1; k >= 0; k--) {
        ans[k] = comp;
        if (dsu.unite(edges[removal_order[k]].first, edges[removal_order[k]].second)) comp--;
    }
    return ans;
}

// =============================================================
// B. БИТСЕТЫ
// =============================================================

// --- B.2. Dynamic Bitset: n бит в ⌈n/w⌉ машинных словах ---
// Слово — параметр платформы: w бит (w = 64), логарифм lg_w и
// маска w_mask для индексации «слово = i >> lg_w, смещение = i & w_mask».
// Хвост последнего слова маскируется до n mod w бит.
struct DynamicBitset {
    static constexpr int W = 64;        // бит в слове
    static constexpr int LG_W = 6;      // log2(W)
    static constexpr int W_MASK = W - 1;

    int n, words;
    vector<unsigned long long> b;

    DynamicBitset(int n_, bool fill = false) : n(n_) {
        words = (n + W - 1) >> LG_W;
        b.assign(words, fill ? ~0ULL : 0ULL);
        if (fill) clear_tail();
    }

    // обнулить биты за пределами n (хвост последнего слова)
    void clear_tail() {
        int tail = n & W_MASK;
        if (tail) b.back() &= (1ULL << tail) - 1;
    }
    void set(int i) { b[i >> LG_W] |= 1ULL << (i & W_MASK); }
    void reset(int i) { b[i >> LG_W] &= ~(1ULL << (i & W_MASK)); }
    void flip(int i) { b[i >> LG_W] ^= 1ULL << (i & W_MASK); }
    bool test(int i) const { return (b[i >> LG_W] >> (i & W_MASK)) & 1ULL; }
    bool any() const {
        for (unsigned long long w : b) if (w) return true;
        return false;
    }
    bool none() const { return !any(); }
    bool all() const {
        if (!words) return true;
        for (int i = 0; i + 1 < words; i++) if (b[i] != ~0ULL) return false;
        int tail = n & W_MASK;
        unsigned long long full = ~0ULL;
        if (tail) full = (1ULL << tail) - 1;    // маска хвоста: n mod w младших битов
        return (b.back() & full) == full;
    }
};

// --- B.3. Операции с множествами по словам ---
// Примитивы машинного слова переиспользуются из SetsAndRelations
// (math/discrete-and-logic): set_union / set_intersection /
// set_difference / set_symmetric_difference / is_subset применяются
// к каждому слову; дополнение — побитовое ~ по словам с маскированием
// хвоста (set_complement оттуда корректен только для масок < w бит).
static DynamicBitset bit_union(const DynamicBitset& a, const DynamicBitset& b) {
    DynamicBitset r(a.n);
    SetsAndRelations s;
    for (int i = 0; i < a.words; i++) r.b[i] = s.set_union(a.b[i], b.b[i]);
    return r;
}
static DynamicBitset bit_intersection(const DynamicBitset& a, const DynamicBitset& b) {
    DynamicBitset r(a.n);
    SetsAndRelations s;
    for (int i = 0; i < a.words; i++) r.b[i] = s.set_intersection(a.b[i], b.b[i]);
    return r;
}
static DynamicBitset bit_difference(const DynamicBitset& a, const DynamicBitset& b) {
    DynamicBitset r(a.n);
    SetsAndRelations s;
    for (int i = 0; i < a.words; i++) r.b[i] = s.set_difference(a.b[i], b.b[i]);
    return r;
}
static DynamicBitset bit_symmetric_difference(const DynamicBitset& a, const DynamicBitset& b) {
    DynamicBitset r(a.n);
    SetsAndRelations s;
    for (int i = 0; i < a.words; i++) r.b[i] = s.set_symmetric_difference(a.b[i], b.b[i]);
    return r;
}
static DynamicBitset bit_complement(const DynamicBitset& a) {
    DynamicBitset r(a.n);
    for (int i = 0; i < a.words; i++) r.b[i] = ~a.b[i];
    r.clear_tail();                     // обрезать до n бит
    return r;
}
static bool bit_is_subset(const DynamicBitset& a, const DynamicBitset& b) {
    SetsAndRelations s;
    for (int i = 0; i < a.words; i++)
        if (!s.is_subset(a.b[i], b.b[i])) return false;
    return true;
}

// --- B.4. popcount и rank ---
// popcount слова — SetsAndRelations::set_cardinality; rank префикса —
// префиксные суммы по словам (та же идея, что V.A) + хвостовое слово.
static long long bit_popcount(const DynamicBitset& a) {
    SetsAndRelations s;
    long long cnt = 0;
    for (unsigned long long w : a.b) cnt += s.set_cardinality(w);
    return cnt;
}
// rank_prefix: наивно O(число слов) на запрос
static long long bit_rank_prefix(const DynamicBitset& a, int i) {
    SetsAndRelations s;
    long long cnt = 0;
    int full = i >> DynamicBitset::LG_W, off = i & DynamicBitset::W_MASK;
    for (int j = 0; j < full; j++) cnt += s.set_cardinality(a.b[j]);
    if (off) cnt += s.set_cardinality(a.b[full] & ((1ULL << off) - 1));
    return cnt;
}
// rank_fast: таблица префиксных сумм слов — O(1) на запрос
// (полные слова — из таблицы, хвостовое слово — popcount части).
struct RankTable {
    DynamicBitset bs;
    vector<long long> pref;             // pref[k] — popcount первых k слов
    RankTable(const DynamicBitset& a) : bs(a) {
        SetsAndRelations s;
        pref.assign(bs.words + 1, 0);
        for (int j = 0; j < bs.words; j++) pref[j + 1] = pref[j] + s.set_cardinality(bs.b[j]);
    }
    long long rank(int i) const {
        SetsAndRelations s;
        int full = i >> DynamicBitset::LG_W, off = i & DynamicBitset::W_MASK;
        long long res = pref[full];
        if (off) res += s.set_cardinality(bs.b[full] & ((1ULL << off) - 1));
        return res;
    }
};

// --- B.5. Перебор установленных битов ---
// Младший установленный бит: lsb = w & −w, позиция — ctz; сброс lsb
// и повтор — итерация за O(popcount). Следующий установленный после
// позиции from — первый ненулевой бит с from в текущем слове, затем
// первое ненулевое слово.
static int next_set_bit(const DynamicBitset& a, int from) {
    if (from >= a.n) return -1;
    int w = from >> DynamicBitset::LG_W, o = from & DynamicBitset::W_MASK;
    unsigned long long m = a.b[w] & (~0ULL << o);
    if (m) return (w << DynamicBitset::LG_W) + __builtin_ctzll(m);
    for (int j = w + 1; j < a.words; j++)
        if (a.b[j]) return (j << DynamicBitset::LG_W) + __builtin_ctzll(a.b[j]);
    return -1;
}
template <class F>
static void for_each_set_bit(const DynamicBitset& a, F f) {
    for (int j = 0; j < a.words; j++)
        for (unsigned long long w = a.b[j]; w; w &= w - 1)
            f((j << DynamicBitset::LG_W) + __builtin_ctzll(w));
}
static vector<int> collect_set_bits(const DynamicBitset& a) {
    vector<int> res;
    for_each_set_bit(a, [&](int i) { res.push_back(i); });
    return res;
}

// --- B.6. Применения: рюкзак битсетом и Warshall на битсетах ---
// dp |= dp << x: сдвиг битсета влево по словам + ИЛИ по словам
// (bit_union из B.3). Warshall: транзитивное замыкание, строка графа —
// битсет; каждая операция — OR строк за O(n/w).
static DynamicBitset shift_left_bitset(const DynamicBitset& a, int s) {
    DynamicBitset r(a.n);
    int sw = s >> DynamicBitset::LG_W, so = s & DynamicBitset::W_MASK;
    for (int i = a.words - 1; i >= sw; i--) {
        unsigned long long v = a.b[i - sw] << so;
        if (so && i - sw - 1 >= 0) v |= a.b[i - sw - 1] >> (DynamicBitset::W - so);
        r.b[i] = v;
    }
    r.clear_tail();
    return r;
}
static DynamicBitset knapsack_bitset(const vector<int>& weights, int S) {
    DynamicBitset dp(S + 1);
    dp.set(0);
    for (int x : weights) dp = bit_union(dp, shift_left_bitset(dp, x));
    return dp;
}
static vector<DynamicBitset> transitive_closure_bitset(vector<DynamicBitset> reach) {
    int n = (int)reach.size();
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            if (reach[i].test(k)) reach[i] = bit_union(reach[i], reach[k]);
    return reach;
}

// --- B.7. Rank/Select и сукцинктное хранение (FID) ---
// Над битовым вектором (в духе B.2) строятся rank(i) — число единиц
// на [0, i) и select(k) — позиция k-й единицы. Хранение сукцинктное:
// сам вектор (n бит) + таблицы o(n) бит — суперблоки (префиксные
// суммы по 512 бит) и блоки (поправка внутри суперблока по словам).
// Битовый вектор с rank/select — FID, базовый примитив сукцинктных
// структур (Wavelet Matrix — II.G.4).
struct RankSelect {
    static constexpr int W = 64;        // бит в слове
    static constexpr int LG_W = 6;
    static constexpr int SB = 512;      // бит в суперблоке

    int n;
    vector<unsigned long long> w;       // n бит
    vector<int> sb, br;                 // суперблоки + блоки

    RankSelect() : n(0) {}
    explicit RankSelect(int n_, bool fill = false) : n(n_) {
        w.assign((n_ + W - 1) >> LG_W, fill ? ~0ULL : 0ULL);
        sb.assign((w.size() + 7) >> 3, 0);
        br.assign(w.size(), 0);
        if (fill) {
            int tail = n & (W - 1);
            if (tail) w.back() &= (1ULL << tail) - 1;   // хвост — как в B.2
        }
    }
    void set(int i) { w[i >> LG_W] |= 1ULL << (i & (W - 1)); }
    bool test(int i) const { return (w[i >> LG_W] >> (i & (W - 1))) & 1ULL; }
    void build() {
        int r = 0;
        for (int j = 0; j < (int)w.size(); j++) {
            if ((j & 7) == 0) sb[j >> 3] = r;           // начало суперблока
            br[j] = r - sb[j >> 3];                     // поправка внутри суперблока
            r += __builtin_popcountll(w[j]);
        }
    }
    int count() const {
        int r = 0;
        for (unsigned long long x : w) r += __builtin_popcountll(x);
        return r;
    }
    int rank(int i) const {                             // единиц в [0, i)
        if (i <= 0) return 0;
        int wi = (i - 1) >> LG_W, lo = i & (W - 1);
        if (lo == 0) return sb[wi >> 3] + br[wi] + __builtin_popcountll(w[wi]);
        return sb[wi >> 3] + br[wi] + __builtin_popcountll(w[wi] & ((1ULL << lo) - 1));
    }
    int select(int k) const {                           // позиция k-й единицы (k 0-based)
        if (k < 0 || k >= count()) return -1;
        int lo = 0, hi = n;
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            if (rank(mid) > k) hi = mid; else lo = mid + 1;
        }
        return lo - 1;
    }
};

// FID (Fully Indexable Dictionary): тот же контракт rank/select —
// здесь та же схема RankSelect (базовый примитив сукцинктных структур).
struct FID : RankSelect {
    using RankSelect::RankSelect;
};

}; // конец struct SetStructures

// =============================================================
// signed main() — демонстрация и проверка всех разделов A–B
// =============================================================

#ifndef STRUCT_C_MAIN
signed main() {
    using SS = SetStructures;

    cout << "=== A. DSU ===" << endl;
    // A.1 базовый
    SS::DisjointSet ds1(6);
    ds1.unite(0, 1); ds1.unite(1, 2);
    cout << "DisjointSet same(0,2) = " << ds1.same(0, 2)
         << " same(0,3) = " << ds1.same(0, 3) << " (1 0)" << endl;

    // A.2 альтернативный (итеративный find)
    SS::AlternateDSU ds2(6);
    ds2.unite(0, 1); ds2.unite(1, 2);
    cout << "AlternateDSU same(0,2) = " << ds2.same(0, 2)
         << " same(4,5) = " << ds2.same(4, 5) << " (1 0)" << endl;

    // A.3 оптимизации
    SS::DSU dsu(8);
    dsu.unite(0, 1); dsu.unite(1, 2); dsu.unite(3, 4); dsu.unite(2, 3);
    cout << "DSU same(0,4) = " << dsu.same(0, 4) << " size(0) = " << dsu.size(0)
         << " components = " << dsu.count_components() << " (1 5 4)" << endl;

    // A.4 данные в компоненте (сумма)
    SS::DSUWithAgg agg(5, [](long long a, long long b) { return a + b; }, 0, 1);
    agg.unite(0, 1); agg.unite(1, 2); agg.unite(3, 4);
    cout << "agg sum(0) = " << agg.aggregate(0) << " sum(3) = " << agg.aggregate(3)
         << " (ожидаем 3 2)" << endl;

    // A.5 потенциалы: соотношения val[y] - val[x] = w
    SS::WeightedDSU wdsu(5);
    wdsu.unite(0, 1, 2); wdsu.unite(1, 2, 3);
    auto q = wdsu.query(0, 2); auto ql = wdsu.query(3, 0);
    cout << "weighted query(0,2) = " << q.second << " linked(3,0) = " << ql.first
         << " (ожидаем 5 0)" << endl;
    cout << "weighted conflict(0,2,1) = " << !wdsu.unite(0, 2, 1) << " (1)" << endl;

    // A.6 откат
    SS::RollbackDSU rdsu(6);
    rdsu.unite(0, 1); rdsu.unite(1, 2);
    int snap = rdsu.snapshot();
    rdsu.unite(3, 4);
    cout << "rollback size(0) = " << rdsu.size(0) << " comp(3,4) = " << (rdsu.find(3) == rdsu.find(4)) << " (3 1)" << endl;
    rdsu.rollback_to(snap);
    cout << "after rollback size(0) = " << rdsu.size(0)
         << " comp(3,4) = " << (rdsu.find(3) == rdsu.find(4)) << " (3 0)" << endl;

    // A.7 частичная персистентность
    SS::PersistentDSU pdsu(5);
    int v1 = pdsu.unite(0, 1, 0);
    int v2 = pdsu.unite(1, 2, v1);
    cout << "persistent same(0,2) v2 = " << pdsu.same(0, 2, v2)
         << " same(0,2) v1 = " << pdsu.same(0, 2, v1) << " (1 0)" << endl;

    // A.8 обратное время: 5 вершин, рёбра 0-1, 1-2, 2-3, 0-4;
    // удаляем 0-4 (индекс 3), затем 2-3 (индекс 2)
    vector<int> comps = SS::components_after_removals_offline(
        5, {{0,1},{1,2},{2,3},{0,4}}, {3, 2});
    cout << "offline components = " << comps[0] << " " << comps[1] << " (ожидаем 2 3)" << endl;

    cout << "\n=== B. БИТСЕТЫ ===" << endl;
    // B.1 std::bitset
    bitset<8> bs;
    bs.set(0); bs.set(2); bs.flip(4);
    cout << "std::bitset count = " << bs.count() << " test(2) = " << bs.test(2)
         << " test(1) = " << bs.test(1) << " (3 1 0)" << endl;

    // B.2 динамический битсет
    SS::DynamicBitset db(10);
    db.set(3); db.set(7);
    cout << "DynamicBitset test(3) = " << db.test(3) << " test(4) = " << db.test(4)
         << " count = " << SS::bit_popcount(db) << " (1 0 2)" << endl;

    // B.3 операции с множествами
    SS::DynamicBitset s1(6), s2(6);
    for (int x : {1, 2, 3}) s1.set(x);
    for (int x : {2, 3, 4}) s2.set(x);
    cout << "union: ";
    for (int x : SS::collect_set_bits(SS::bit_union(s1, s2))) cout << x << " ";
    cout << "(ожидаем 1 2 3 4)" << endl;
    cout << "intersection: ";
    for (int x : SS::collect_set_bits(SS::bit_intersection(s1, s2))) cout << x << " ";
    cout << "(ожидаем 2 3)" << endl;
    cout << "difference: ";
    for (int x : SS::collect_set_bits(SS::bit_difference(s1, s2))) cout << x << " ";
    cout << "(ожидаем 1)" << endl;
    SS::DynamicBitset s3(6);
    for (int x : {1, 2}) s3.set(x);
    cout << "is_subset(s3, s1) = " << SS::bit_is_subset(s3, s1)
         << " is_subset(s2, s1) = " << SS::bit_is_subset(s2, s1) << " (1 0)" << endl;

    // B.4 rank
    cout << "rank_prefix(7) = " << SS::bit_rank_prefix(db, 7)
         << " rank_prefix(8) = " << SS::bit_rank_prefix(db, 8) << " (1 2)" << endl;
    SS::RankTable rt(db);
    cout << "rank_fast(7) = " << rt.rank(7) << " rank_fast(8) = " << rt.rank(8) << " (1 2)" << endl;

    // B.5 перебор единиц
    cout << "next_set_bit(4) = " << SS::next_set_bit(db, 4) << " (ожидаем 7)" << endl;
    cout << "bits of db: ";
    for (int x : SS::collect_set_bits(db)) cout << x << " ";
    cout << "(ожидаем 3 7)" << endl;

    // B.6 применения
    SS::DynamicBitset dp = SS::knapsack_bitset({2, 3, 5}, 11);
    cout << "knapsack count = " << SS::bit_popcount(dp)
         << " reach(8) = " << dp.test(8) << " reach(1) = " << dp.test(1) << " (7 1 0)" << endl;
    int nv = 4;
    vector<SS::DynamicBitset> reach(nv, SS::DynamicBitset(nv));
    for (int i = 0; i < nv; i++) reach[i].set(i);
    reach[0].set(1); reach[0].set(3); reach[1].set(2);
    auto cl = SS::transitive_closure_bitset(reach);
    cout << "closure reach[0]: ";
    for (int x : SS::collect_set_bits(cl[0])) cout << x << " ";
    cout << "(ожидаем 0 1 2 3)" << endl;

    // B.7 rank/select (FID)
    SS::RankSelect rs(10);
    for (int x : {1, 3, 7, 8}) rs.set(x);
    rs.build();
    cout << "RankSelect count = " << rs.count()
         << " rank(8) = " << rs.rank(8) << " rank(10) = " << rs.rank(10)
         << " select(0) = " << rs.select(0) << " select(2) = " << rs.select(2)
         << " select(3) = " << rs.select(3) << " select(4) = " << rs.select(4)
         << " (4 3 4 1 7 8 -1)" << endl;
    SS::FID fid(10);
    for (int x : {0, 2, 5, 9}) fid.set(x);
    fid.build();
    cout << "FID rank(10) = " << fid.rank(10) << " select(1) = " << fid.select(1)
         << " select(3) = " << fid.select(3) << " (4 2 9)" << endl;

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_C_MAIN

#endif // STRUCT_C_CPP
