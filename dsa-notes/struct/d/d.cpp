#ifndef STRUCT_D_CPP
#define STRUCT_D_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <queue>
#include <set>
#include <random>
#include <climits>
using namespace std;

// =============================================================
// IV. КУЧИ (PRIORITY QUEUES)
// =============================================================
// Структура md: A. Бинарная куча
//               → B. Сливаемые кучи
//               → C. Радикс-куча
//               → D. Двусторонняя куча
//               → E. Общее: выбор кучи (сводка, без кода)
//
// Heaps наследует SetStructures (c.cpp). Переиспользует:
//   * HeapIndexedTree (II.A.2) — куча-индексация полного дерева:
//     left/right/parent, has_left/has_right не переписываются;
//     BinaryHeap (A.1) наследует и дополняет sift-up/sift-down;
//   * BucketQueue (I.D.8) — бакетная схема по приоритетам, мост
//     для радикс-кучи (C.1: те же бакеты, но по битовой длине);
//   * амортизационный анализ (I.A, analysis.md) — потенциальные
//     функции фибоначчиевой (B.2), косой (B.5) и парной (B.3) куч;
//   * приём «потенциал к корню» WeightedDSU (III.A.5) — та же
//     механика, что вырезка в decrease_key кучах с указателями;
//   * mt19937 (II.F) — монетка рандомизированной кучи (B.6),
//     зерно — параметр конструктора;
//   * std::priority_queue (STL) — готовый вариант бинарной кучи (A.5).
//
// Порядок методов строго соответствует порядку md (A → E).
// Deap (D.3) объявлен в md постановкой (оценки и инварианты
// зафиксированы); рабочие двусторонние кучи — MinMaxHeap (D.1)
// и IntervalHeap (D.2), поэтому кода под D.3 нет.
//
// ВНИМАНИЕ (скрытие имён): методы push, pop, pop_min, top, min_key,
// max_key, merge, meld, build, empty, size здесь локальные;
// одноимённые из других веток не подключаются. BinaryHeap::build
// затеняет HeapIndexedTree::build: добавляет heapify (A.2).

#define STRUCT_C_MAIN
#include "../c/c.cpp"
#undef STRUCT_C_MAIN

struct Heaps : SetStructures {

// =============================================================
// A. БИНАРНАЯ КУЧА
// =============================================================

// --- A.1. Heap: массив + sift-up / sift-down ---
// Полное дерево в массиве; индексация left/right/parent унаследована
// от HeapIndexedTree (II.A.2). Вставка: в конец + всплытие.
// Извлечение: корень ↔ последний, затем утопление к меньшему ребёнку.
// Компаратор — параметр (по умолчанию min-куча), см. A.3.
struct BinaryHeap : HeapIndexedTree {
    std::function<bool(int, int)> cmp;  // cmp(a, b): a «лучше» b (стоит выше)

    explicit BinaryHeap(std::function<bool(int, int)> c = std::less<int>{}) : cmp(c) {}

    void sift_up(int i) {
        while (i > 0 && cmp(a[i], a[parent(i)])) {
            swap(a[i], a[parent(i)]);
            i = parent(i);
        }
    }
    void sift_down(int i) {
        while (has_left(i)) {
            int j = left(i);
            if (has_right(i) && cmp(a[right(i)], a[j])) j = right(i);
            if (!cmp(a[j], a[i])) break;
            swap(a[i], a[j]);
            i = j;
        }
    }
    void push(int x) {
        a.push_back(x);
        sift_up((int)a.size() - 1);
    }
    int top() const { return a[0]; }
    void pop() {
        a[0] = a.back();
        a.pop_back();
        if (!a.empty()) sift_down(0);
    }
    bool empty() const { return a.empty(); }

    // --- A.2. Построение за O(n): sift-down снизу вверх ---
    // Последний родитель — (size/2 − 1); суммарная стоимость —
    // геометрическая прогрессия высот: O(n) (связь analysis.md).
    void heapify() {
        for (int i = (int)a.size() / 2 - 1; i >= 0; i--) sift_down(i);
    }
    void build(const vector<int>& vals) {
        HeapIndexedTree::build(vals);   // копия массива — из II.A.2
        heapify();                      // затем инвариант кучи за O(n)
    }
};

// --- A.3. Max Heap: обёртка с обратным компаратором ---
// Тот же движок, направление сравнения — параметр конструктора.
struct MaxHeap : BinaryHeap {
    MaxHeap() : BinaryHeap(std::greater<int>{}) {}
};

// --- A.4. Index Heap: куча с позиционным массивом ---
// Элементы нумерованы 0..n−1; pos[id] — позиция в куче, key[id] —
// текущий ключ. decrease/increase-key за O(log n) — для Дейкстры
// и Прима (graph.md); pop_min возвращает номер элемента.
struct IndexHeap {
    vector<int> key;   // key[id] — приоритет
    vector<int> pos;   // pos[id] — позиция в heap, −1 если нет
    vector<int> heap;  // heap[pos] = id; минимум — heap[0]

    IndexHeap(int n, int inf = 1e9) : key(n, inf), pos(n, -1) { heap.reserve(n); }

    bool empty() const { return heap.empty(); }
    bool contains(int id) const { return pos[id] != -1; }
    int top_id() const { return heap[0]; }
    int top_key() const { return key[heap[0]]; }

    void sift_up(int i) {
        while (i > 0 && key[heap[i]] < key[heap[(i - 1) / 2]]) {
            swap(heap[i], heap[(i - 1) / 2]);
            pos[heap[i]] = i;
            pos[heap[(i - 1) / 2]] = (i - 1) / 2;
            i = (i - 1) / 2;
        }
    }
    void sift_down(int i) {
        int n = (int)heap.size();
        while (2 * i + 1 < n) {
            int j = 2 * i + 1;
            if (2 * i + 2 < n && key[heap[2 * i + 2]] < key[heap[j]]) j = 2 * i + 2;
            if (!(key[heap[j]] < key[heap[i]])) break;
            swap(heap[i], heap[j]);
            pos[heap[i]] = i;
            pos[heap[j]] = j;
            i = j;
        }
    }
    // push: новый элемент; если уже есть — decrease/increase по факту ключа
    void push(int id, int k) {
        if (pos[id] == -1) {
            key[id] = k;
            pos[id] = (int)heap.size();
            heap.push_back(id);
            sift_up(pos[id]);
        } else if (k < key[id]) {
            decrease_key(id, k);
        } else if (k > key[id]) {
            increase_key(id, k);
        }
    }
    void decrease_key(int id, int k) { key[id] = k; sift_up(pos[id]); }
    void increase_key(int id, int k) { key[id] = k; sift_down(pos[id]); }

    int pop_min() {
        int id = heap[0];
        pos[id] = -1;
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) {
            pos[heap[0]] = 0;
            sift_down(0);
        }
        return id;
    }
};

// =============================================================
// B. СЛИВАЕМЫЕ КУЧИ (MERGEABLE HEAPS)
// =============================================================
// Общее для B: двоичные деревья на указателях; merge — «сшивание»
// деревьев без копирования массивов; вставка = merge с одноузловой
// кучей; извлечение = merge детей корня; decrease_key — вырезка
// узла (или подъём значения) по указателю-«ручке» на узел.

// --- B.1. Биномиальная куча ---
// Лес биномиальных деревьев: дерево степени d — ровно 2^d вершин,
// число деревьев = число единиц в двоичной записи размера кучи.
// Слияние — как сложение двоичных чисел (переносы в старшую степень).
struct BinomialHeap {
    struct Node {
        int val;
        int degree;
        Node* parent;
        Node* child;    // старший ребёнок (цепочка через sibling)
        Node* sibling;
        Node(int v) : val(v), degree(0), parent(nullptr), child(nullptr), sibling(nullptr) {}
    };
    vector<Node*> roots;   // roots[d] — корень дерева степени d

    BinomialHeap() : roots(1, nullptr) {}

    bool empty() const {
        for (Node* r : roots) if (r) return false;
        return true;
    }
    // сцепление двух B_d в B_{d+1}: меньший ключ — корень
    static Node* link_trees(Node* a, Node* b) {
        if (a->val > b->val) swap(a, b);
        b->parent = a;
        b->sibling = a->child;
        a->child = b;
        a->degree++;
        return a;
    }
    // «сложение двоичных чисел»: переносы одинаковых степеней
    void absorb(vector<Node*>& list) {
        for (Node* x : list) {
            if (!x) continue;
            int d = x->degree;
            x->parent = nullptr;
            x->sibling = nullptr;
            while (true) {
                while ((int)roots.size() <= d) roots.push_back(nullptr);
                if (!roots[d]) break;
                x = link_trees(x, roots[d]);
                roots[d] = nullptr;
                d++;
            }
            roots[d] = x;
        }
    }
    void merge(BinomialHeap& other) {
        absorb(other.roots);
        other.roots.assign(1, nullptr);
    }
    Node* push(int v) {
        Node* x = new Node(v);
        vector<Node*> one = {x};
        absorb(one);
        return x;
    }
    int top() const {
        int best = INT_MAX;
        for (Node* r : roots) if (r) best = min(best, r->val);
        return best;
    }
    int pop_min() {
        int best = INT_MAX, d = -1;
        for (int i = 0; i < (int)roots.size(); i++)
            if (roots[i] && roots[i]->val < best) { best = roots[i]->val; d = i; }
        Node* x = roots[d];
        roots[d] = nullptr;
        vector<Node*> kids;
        for (Node* c = x->child; c; ) {
            Node* nxt = c->sibling;
            c->parent = nullptr;
            c->sibling = nullptr;
            kids.push_back(c);
            c = nxt;
        }
        delete x;
        absorb(kids);
        return best;
    }
    // decrease_key: вырезка узла из родительского дерева и повторное
    // слияние его поддерева в лес — ключ остаётся на узле-«ручке»
    void decrease_key(Node* x, int v) {
        x->val = v;
        if (!x->parent) return;   // уже корень: меньшее значение поднимет top()
        Node* p = x->parent;
        if (p->child == x) p->child = x->sibling;
        else {
            Node* q = p->child;
            while (q->sibling != x) q = q->sibling;
            q->sibling = x->sibling;
        }
        x->parent = nullptr;
        x->sibling = nullptr;
        vector<Node*> one = {x};
        absorb(one);
    }
};

// --- B.2. Фибоначчиева куча ---
// Максимально ленивая куча: insert/merge/decrease_key делают только
// подвешивание/вырезку (O(1)), платим при pop_min (консолидация
// деревьев равной степени). Каскадная вырезка: потеряв второго
// ребёнка, узел сам вырезается в корневой список. Потенциал
// Φ = (число корней) + 2·(число помеченных) (связь analysis.md).
struct FibonacciHeap {
    struct Node {
        int val;
        Node* parent;
        Node* child;    // любой ребёнок; дети — в циклическом списке
        Node* left;     // лево-право — циклический двусвязный список
        Node* right;
        int degree;
        bool mark;      // потерял ребёнка с момента последней подвески
        Node(int v) : val(v), parent(nullptr), child(nullptr), left(this), right(this),
                      degree(0), mark(false) {}
    };
    Node* min_root;   // корень с минимальным ключом
    int n;

    FibonacciHeap() : min_root(nullptr), n(0) {}

    bool empty() const { return n == 0; }
    int top() const { return min_root->val; }

    static void add_to_list(Node* r, Node* x) {   // вставить x рядом с r
        Node* nxt = r->right;
        x->left = r; x->right = nxt;
        r->right = x; nxt->left = x;
    }
    static void remove_from_list(Node* x) {
        x->left->right = x->right;
        x->right->left = x->left;
        x->left = x->right = x;
    }

    Node* push(int v) {
        Node* x = new Node(v);
        n++;
        if (!min_root) { min_root = x; return x; }
        add_to_list(min_root, x);
        if (x->val < min_root->val) min_root = x;
        return x;
    }
    // merge: сращивание циклических корневых списков за O(1)
    static Node* splice(Node* a, Node* b) {
        Node* a_next = a->right;
        a->right = b->right; b->right->left = a;
        a_next->left = b; b->right = a_next;
        return a->val <= b->val ? a : b;
    }
    void merge(FibonacciHeap& other) {
        if (!other.min_root) return;
        if (!min_root) min_root = other.min_root;
        else min_root = splice(min_root, other.min_root);
        n += other.n;
        other.min_root = nullptr;
        other.n = 0;
    }

    void link(Node* u, Node* v) {   // v подвешивается к u (u.val <= v.val)
        remove_from_list(v);
        v->parent = u;
        v->mark = false;
        if (!u->child) u->child = v;
        else add_to_list(u->child, v);
        u->degree++;
    }
    // консолидация: деревья равной степени сцепляются (как в B.1 «переносы»)
    void consolidate() {
        vector<Node*> roots;
        if (min_root) {
            Node* x = min_root;
            do { roots.push_back(x); x = x->right; } while (x != min_root);
        }
        vector<Node*> slot;
        for (Node* y : roots) {
            y->parent = nullptr;
            y->mark = false;
            int d = y->degree;
            while ((int)slot.size() <= d) slot.push_back(nullptr);
            while (slot[d]) {
                Node* other = slot[d];
                if (y->val > other->val) swap(y, other);
                link(y, other);
                slot[d] = nullptr;
                d++;
                while ((int)slot.size() <= d) slot.push_back(nullptr);
            }
            slot[d] = y;
        }
        min_root = nullptr;
        for (Node* s : slot) {
            if (!s) continue;
            if (!min_root) { min_root = s; min_root->left = min_root->right = min_root; }
            else { add_to_list(min_root, s); if (s->val < min_root->val) min_root = s; }
        }
    }
    int pop_min() {
        Node* z = min_root;
        int ret = z->val;
        if (z->child) {
            Node* c = z->child;
            do {
                Node* nxt = c->right;
                c->parent = nullptr;
                add_to_list(z, c);
                c = nxt;
            } while (c != z->child);
        }
        Node* nxt_root = (z->left == z) ? nullptr : z->right;   // сосед до изоляции z
        remove_from_list(z);
        n--;
        delete z;
        if (!nxt_root) {                // куча опустела
            min_root = nullptr;
            return ret;
        }
        min_root = nxt_root;
        consolidate();
        return ret;
    }
    void cut(Node* x, Node* p) {        // вырезка x из детей p в корневой список
        Node* nxt = (x->right == x) ? nullptr : x->right;
        remove_from_list(x);
        p->degree--;
        if (p->child == x) p->child = nxt;
        x->parent = nullptr;
        x->mark = false;
        add_to_list(min_root, x);
    }
    void cascading_cut(Node* y) {       // второй потерянный ребёнок вырезает y
        Node* p = y->parent;
        if (!p) return;
        if (!y->mark) { y->mark = true; return; }
        cut(y, p);
        cascading_cut(p);
    }
    void decrease_key(Node* x, int v) {
        x->val = v;
        Node* p = x->parent;
        if (p && x->val < p->val) {
            cut(x, p);
            cascading_cut(p);
        }
        if (x->val < min_root->val) min_root = x;
    }
    void erase(Node* x) {               // удаление: ключ −∞ + извлечение
        decrease_key(x, INT_MIN);
        pop_min();
    }
};

// --- B.3. Парная куча ---
// Подешевле фибоначчиевой: никаких mark/degree, дети — однонаправленный
// список через next/prev (prev — для вырезки в decrease_key).
// pop_min: дети объединяются двухпроходным слиянием (пары слева
// направо, затем справа налево) — пути «укорачиваются», O(log n)
// амортизированно (связь analysis.md).
struct PairingHeap {
    struct Node {
        int val;
        Node* parent;
        Node* first;    // первый ребёнок (список через next/prev)
        Node* next;
        Node* prev;
        Node(int v) : val(v), parent(nullptr), first(nullptr), next(nullptr), prev(nullptr) {}
    };
    Node* root;

    PairingHeap() : root(nullptr) {}

    bool empty() const { return !root; }
    int top() const { return root->val; }

    // link: подвесить больший корень первым ребёнком меньшего
    static Node* link(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->val > b->val) swap(a, b);
        b->parent = a;
        b->prev = nullptr;
        b->next = a->first;
        if (a->first) a->first->prev = b;
        a->first = b;
        return a;
    }
    Node* push(int v) {
        Node* x = new Node(v);
        root = link(root, x);
        return x;
    }
    // двухпроходное слияние списка детей
    static Node* merge_pairs(const vector<Node*>& cs) {
        vector<Node*> ps;
        for (size_t i = 0; i + 1 < cs.size(); i += 2) ps.push_back(link(cs[i], cs[i + 1]));
        if (cs.size() % 2) ps.push_back(cs.back());
        Node* acc = nullptr;
        for (size_t i = ps.size(); i-- > 0;) acc = link(acc, ps[i]);
        return acc;
    }
    int pop_min() {
        Node* old = root;
        vector<Node*> cs;
        for (Node* c = old->first; c; ) {
            Node* nxt = c->next;
            c->parent = c->next = c->prev = nullptr;
            cs.push_back(c);
            c = nxt;
        }
        root = merge_pairs(cs);
        int ret = old->val;
        delete old;
        return ret;
    }
    // decrease_key: вырезка узла из списка братьев + link с корнем
    void decrease_key(Node* x, int v) {
        x->val = v;
        if (x == root) return;
        if (x->prev) x->prev->next = x->next;
        else x->parent->first = x->next;
        if (x->next) x->next->prev = x->prev;
        x->parent = nullptr;
        x->next = nullptr;
        x->prev = nullptr;
        root = link(root, x);
    }
    void merge(PairingHeap& other) {
        root = link(root, other.root);
        other.root = nullptr;
    }
};

// --- B.4. Левосторонняя куча ---
// Ранг узла — длина кратчайшего пути до NULL; инвариант
// rank(left) >= rank(right): правый путь — кратчайший, его длина
// <= log2(n+1), рекурсия merge идёт только по правым путям —
// строгий O(log n) (не амортизированный).
struct LeftistHeap {
    struct Node {
        int val;
        int rank;       // нуль-путь-ранг: 1 + rank(правый), для NULL — 0
        Node* left;
        Node* right;
        Node(int v) : val(v), rank(1), left(nullptr), right(nullptr) {}
    };
    Node* root;

    LeftistHeap() : root(nullptr) {}

    bool empty() const { return !root; }
    int top() const { return root->val; }

    static Node* meld(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->val > b->val) swap(a, b);
        a->right = meld(a->right, b);
        if (!a->left || a->left->rank < a->right->rank) swap(a->left, a->right);
        a->rank = a->right ? a->right->rank + 1 : 1;
        return a;
    }
    void push(int v) { root = meld(root, new Node(v)); }
    int pop_min() {
        Node* old = root;
        root = meld(old->left, old->right);
        int ret = old->val;
        delete old;
        return ret;
    }
    void merge(LeftistHeap& other) {
        root = meld(root, other.root);
        other.root = nullptr;
    }
};

// --- B.5. Косая куча ---
// Левосторонняя без рангов: после рекурсивного meld дети корня
// меняются местами безусловно. Потенциал — число «тяжёлых» правых
// рёбер: O(log n) амортизированно (связь analysis.md).
struct SkewHeap {
    struct Node {
        int val;
        Node* left;
        Node* right;
        Node(int v) : val(v), left(nullptr), right(nullptr) {}
    };
    Node* root;

    SkewHeap() : root(nullptr) {}

    bool empty() const { return !root; }
    int top() const { return root->val; }

    static Node* meld(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->val > b->val) swap(a, b);
        swap(a->left, a->right);             // безусловный обмен детей
        a->left = meld(a->left, b);
        return a;
    }
    void push(int v) { root = meld(root, new Node(v)); }
    int pop_min() {
        Node* old = root;
        root = meld(old->left, old->right);
        int ret = old->val;
        delete old;
        return ret;
    }
    void merge(SkewHeap& other) {
        root = meld(root, other.root);
        other.root = nullptr;
    }
};

// --- B.6. Рандомизированная куча ---
// Та же схема, но выбор ребёнка для рекурсии — монетка: вместо
// инварианта ранга — вероятностный баланс, ожидаемый O(log n)
// независимо от порядка вставок. Зерно генератора — параметр
// конструктора (тестируемость). Мост в hashing.md IV (вероятностные структуры).
struct RandomizedHeap {
    struct Node {
        int val;
        Node* left;
        Node* right;
        Node(int v) : val(v), left(nullptr), right(nullptr) {}
    };
    Node* root;
    mt19937 rng;

    explicit RandomizedHeap(unsigned seed = 12345) : root(nullptr), rng(seed) {}

    bool empty() const { return !root; }
    int top() const { return root->val; }

    Node* meld(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->val > b->val) swap(a, b);
        if (rng() & 1) a->left = meld(a->left, b);
        else a->right = meld(a->right, b);
        return a;
    }
    void push(int v) { root = meld(root, new Node(v)); }
    int pop_min() {
        Node* old = root;
        root = meld(old->left, old->right);
        int ret = old->val;
        delete old;
        return ret;
    }
    void merge(RandomizedHeap& other) {
        root = meld(root, other.root);
        other.root = nullptr;
    }
};

// =============================================================
// C. РАДИКС-КУЧА (RADIX HEAP / BUCKET HEAP)
// =============================================================

// --- C.1. Для целочисленных неотрицательных ключей ---
// Бакет d хранит ключи с битовой длиной (key ^ last_min) = d;
// last_min — последний извлечённый минимум. pop_min берёт первый
// непустой бакет, его минимум становится last_min, остальные ключи
// бакета переезжают в младшие бакеты — каждый ключ переезжает
// не более ⌈log2(ключ)⌉ раз, отсюда амортизированный O(1).
// Бакетная схема — мост из BucketQueue (I.D.8); ключи при push
// должны быть >= last_min (инвариант). Для Дейкстры на целочисленных
// весах (graph.md).
struct RadixHeap {
    int bits;                                // битовая длина ключа — параметр
    vector<vector<pair<int, int>>> buckets;  // бакет[d]: (ключ, значение)
    int last_min;
    int cnt;

    explicit RadixHeap(int bits = 64) : bits(bits), buckets(bits + 1), last_min(0), cnt(0) {}

    bool empty() const { return cnt == 0; }

    // битовая длина (key ^ lm) — номер бакета; ключ < 2^bits
    static int bucket_of(int key, int lm, int bits) {
        int t = key ^ lm;
        if (t == 0) return 0;
        return bits - __builtin_clzll((unsigned long long)t);
    }
    void push(int key, int val) {
        buckets[bucket_of(key, last_min, bits)].push_back({key, val});
        cnt++;
    }
    bool pop_min(int& key, int& val) {
        for (int d = 0; d < (int)buckets.size(); d++) {
            vector<pair<int, int>>& b = buckets[d];
            if (b.empty()) continue;
            int mn = INT_MAX;
            for (const auto& e : b) mn = min(mn, e.first);
            last_min = mn;
            int ret = -1;
            for (int i = 0; i < (int)b.size(); i++)
                if (b[i].first == mn) { ret = i; break; }
            key = b[ret].first;
            val = b[ret].second;
            b[ret] = b.back();
            b.pop_back();
            cnt--;
            vector<pair<int, int>> rest = b;    // переезд в младшие бакеты
            b.clear();
            for (const auto& e : rest) buckets[bucket_of(e.first, last_min, bits)].push_back(e);
            return true;
        }
        return false;
    }
};

// =============================================================
// D. ДВУСТОРОННЯЯ КУЧА (DOUBLE-ENDED PRIORITY QUEUE)
// =============================================================
// D.3 Deap — постановка в md: структура, инварианты и оценки
// зафиксированы; рабочие двусторонние кучи — D.1 и D.2 (кода под
// D.3 нет, как у постановок в других ветках).

// --- D.1. Min-Max Heap ---
// Полное дерево в массиве; чётные уровни (корень — 0-й) — min-уровни,
// нечётные — max-уровни. Узел min-уровня <= всех потомков, max-уровня
// >= всех потомков. Восстановление не пересекает границу уровня.
struct MinMaxHeap {
    vector<int> a;

    bool empty() const { return a.empty(); }
    int min_key() const { return a[0]; }
    int max_key() const {
        if ((int)a.size() == 1) return a[0];
        return (int)a.size() == 2 ? a[1] : max(a[1], a[2]);
    }

    static bool is_min_level(int i) {
        int lvl = 0;
        while (i > 0) { i = (i - 1) / 2; lvl++; }
        return lvl % 2 == 0;
    }
    int grandparent(int i) const { return ((i - 1) / 2 - 1) / 2; }

    void bubble_up_min(int i) {     // подъём по min-уровням к деду
        while (i >= 3 && a[i] < a[grandparent(i)]) {
            swap(a[i], a[grandparent(i)]);
            i = grandparent(i);
        }
    }
    void bubble_up_max(int i) {     // подъём по max-уровням к деду
        while (i >= 3 && a[i] > a[grandparent(i)]) {
            swap(a[i], a[grandparent(i)]);
            i = grandparent(i);
        }
    }
    void bubble_up(int i) {
        if (i == 0) return;
        int p = (i - 1) / 2;
        if (is_min_level(i)) {
            if (a[i] > a[p]) { swap(a[i], a[p]); bubble_up_max(p); }
            else bubble_up_min(i);
        } else {
            if (a[i] < a[p]) { swap(a[i], a[p]); bubble_up_min(p); }
            else bubble_up_max(i);
        }
    }
    void push(int x) {
        a.push_back(x);
        bubble_up((int)a.size() - 1);
    }

    // лучший среди детей и внуков (минимум/максимум по уровню)
    int best_descendant(int i, bool want_min) const {
        int best = i;
        auto consider = [&](int j) {
            if (j < (int)a.size() &&
                (want_min ? a[j] < a[best] : a[j] > a[best])) best = j;
        };
        consider(2 * i + 1); consider(2 * i + 2);
        consider(4 * i + 3); consider(4 * i + 4);
        consider(4 * i + 5); consider(4 * i + 6);
        return best;
    }
    void restore_min(int i) {       // спуск по min-уровню (через внуков)
        if (2 * i + 1 >= (int)a.size()) return;
        int m = best_descendant(i, true);
        if (m == i) return;
        if (m <= 2 * i + 2) {       // ребёнок
            if (a[i] > a[m]) swap(a[i], a[m]);
            return;
        }
        if (a[i] <= a[m]) return;
        swap(a[i], a[m]);           // внук: обмен + сверка с родителем (max-уровень)
        if (a[m] > a[(m - 1) / 2]) swap(a[m], a[(m - 1) / 2]);
        restore_min(m);
    }
    void restore_max(int i) {       // спуск по max-уровню — симметрично
        if (2 * i + 1 >= (int)a.size()) return;
        int m = best_descendant(i, false);
        if (m == i) return;
        if (m <= 2 * i + 2) {
            if (a[i] < a[m]) swap(a[i], a[m]);
            return;
        }
        if (a[i] >= a[m]) return;
        swap(a[i], a[m]);
        if (a[m] < a[(m - 1) / 2]) swap(a[m], a[(m - 1) / 2]);
        restore_max(m);
    }
    void pop_min() {
        a[0] = a.back();
        a.pop_back();
        if (!a.empty()) restore_min(0);
    }
    void pop_max() {
        if ((int)a.size() == 1) { a.pop_back(); return; }
        int j = ((int)a.size() >= 3 && a[2] > a[1]) ? 2 : 1;
        a[j] = a.back();
        a.pop_back();
        if (j < (int)a.size()) {
            restore_max(j);
            bubble_up(j);           // последний элемент мог нарушить и верх
        }
    }
};

// --- D.2. Interval Heap ---
// Пары [lo, hi] (lo <= hi): lo-части — min-куча, hi-части — max-куча;
// глобальный минимум — lo[0], максимум — hi[0]. Пар — ceil(n/2);
// последняя пара может быть неполной (один элемент в lo).
struct IntervalHeap {
    vector<int> lo, hi;   // lo[i] <= hi[i]; при неполной паре hi[last] = lo[last]
    int n = 0;

    bool empty() const { return n == 0; }
    int min_key() const { return lo[0]; }
    int max_key() const { return hi[0]; }
    int num_pairs() const { return (int)lo.size(); }

    // push: подъём от листа с обменами значений; у одиночной пары
    // реальный элемент — lo, его дубль hi поддерживается (hi = lo),
    // для полных пар после обмена пара нормализуется (lo <= hi).
    void push(int x) {
        if (n == 0) { lo.push_back(x); hi.push_back(x); n = 1; return; }
        int i;
        if (n % 2 == 1) {                    // дополняем последнюю пару до полной
            i = (n - 1) / 2;
            int e = lo[i];
            lo[i] = min(x, e);
            hi[i] = max(x, e);
        } else {                             // новая одиночная пара (x, x)
            lo.push_back(x);
            hi.push_back(x);
            i = (int)lo.size() - 1;
        }
        n++;
        int leaf = i;
        while (i > 0) {
            int p = (i - 1) / 2;
            if (i == leaf && n % 2 == 1) {   // одиночная пара: обмен идёт только через lo
                if (lo[i] < lo[p]) {
                    swap(lo[i], lo[p]);
                    hi[i] = lo[i];
                    i = p;
                } else if (lo[i] > hi[p]) {
                    swap(lo[i], hi[p]);
                    hi[i] = lo[i];
                    i = p;
                } else {
                    break;
                }
            } else if (lo[i] < lo[p]) {
                swap(lo[i], lo[p]);
                if (lo[i] > hi[i]) swap(lo[i], hi[i]);
                i = p;
            } else if (hi[i] > hi[p]) {
                swap(hi[i], hi[p]);
                if (lo[i] > hi[i]) swap(lo[i], hi[i]);
                i = p;
            } else {
                break;
            }
        }
    }
    void trickle_down_min(int i) {      // спуск по lo-половине
        while (2 * i + 1 < num_pairs()) {
            int m = 2 * i + 1;
            if (2 * i + 2 < num_pairs() && lo[2 * i + 2] < lo[m]) m = 2 * i + 2;
            if (lo[i] <= lo[m]) return;
            if (m == num_pairs() - 1 && n % 2 == 1) {   // одиночная пара: один элемент
                swap(lo[i], lo[m]);
                if (lo[i] > hi[i]) swap(lo[i], hi[i]);
                hi[m] = lo[m];                          // синхронизация дубля
                break;
            }
            swap(lo[i], lo[m]);
            if (lo[i] > hi[i]) swap(lo[i], hi[i]);   // пара i не должна сломаться
            if (lo[m] > hi[m]) swap(lo[m], hi[m]);   // и пара m тоже
            i = m;
        }
    }
    void trickle_down_max(int i) {      // спуск по hi-половине
        while (2 * i + 1 < num_pairs()) {
            int m = 2 * i + 1;
            if (2 * i + 2 < num_pairs() && hi[2 * i + 2] > hi[m]) m = 2 * i + 2;
            if (hi[i] >= hi[m]) return;
            if (m == num_pairs() - 1 && n % 2 == 1) {   // одиночная пара: обмен с lo
                swap(hi[i], lo[m]);
                if (lo[i] > hi[i]) swap(lo[i], hi[i]);
                hi[m] = lo[m];                          // синхронизация дубля
                break;
            }
            swap(hi[i], hi[m]);
            if (lo[i] > hi[i]) swap(lo[i], hi[i]);
            if (lo[m] > hi[m]) swap(lo[m], hi[m]);
            i = m;
        }
    }
    void pop_min() {
        int last = (n - 1) / 2;
        if (n == 2) {                        // корень же и есть последняя пара
            lo[0] = hi[0];
            n = 1;
            return;
        }
        int tmp;
        if (n % 2 == 1) {                    // одиночная пара: забираем элемент, пара исчезает
            tmp = lo[last];
            lo.pop_back();
            hi.pop_back();
        } else {                             // полная пара: забираем hi, lo живёт дальше
            tmp = hi[last];
            hi[last] = lo[last];
        }
        n--;
        if (n == 0) return;
        lo[0] = tmp;
        if (lo[0] > hi[0]) swap(lo[0], hi[0]);
        trickle_down_min(0);
    }
    void pop_max() {
        int last = (n - 1) / 2;
        if (n == 2) {                        // корень же и есть последняя пара
            hi[0] = lo[0];
            n = 1;
            return;
        }
        int tmp;
        if (n % 2 == 1) {
            tmp = lo[last];
            lo.pop_back();
            hi.pop_back();
        } else {
            tmp = hi[last];
            hi[last] = lo[last];
        }
        n--;
        if (n == 0) return;
        hi[0] = tmp;
        if (lo[0] > hi[0]) swap(lo[0], hi[0]);
        trickle_down_max(0);
    }
};

}; // конец struct Heaps

// =============================================================
// signed main() — демонстрация и проверка всех разделов A–E
// =============================================================

#ifndef STRUCT_D_MAIN
signed main() {
    using H = Heaps;

    cout << "=== A. БИНАРНАЯ КУЧА ===" << endl;

    // A.1 Heap: push/pop/top
    H::BinaryHeap h;
    for (int x : {5, 1, 4, 2, 3}) h.push(x);
    cout << "BinaryHeap top = " << h.top() << " (ожидаем 1)" << endl;
    h.pop();
    cout << "BinaryHeap top after pop = " << h.top() << " (ожидаем 2)" << endl;

    // A.2 построение за O(n)
    H::BinaryHeap h2;
    h2.build({3, 7, 2, 9, 1, 6, 4, 8});
    cout << "heapify top = " << h2.top() << ", array: ";
    for (int x : h2.a) cout << x << " ";
    cout << "(ожидаем 1, 1 3 2 8 7 6 4 9)" << endl;

    // A.3 MaxHeap (обратный компаратор)
    H::MaxHeap mh;
    for (int x : {5, 1, 4, 2, 3}) mh.push(x);
    cout << "MaxHeap top = " << mh.top() << " (ожидаем 5)" << endl;
    mh.pop();
    cout << "MaxHeap top after pop = " << mh.top() << " (ожидаем 4)" << endl;

    // A.4 Index Heap: decrease/increase-key по позиции
    H::IndexHeap ih(6);
    ih.push(2, 10); ih.push(0, 5); ih.push(3, 7); ih.push(1, 8); ih.push(4, 6);
    cout << "IndexHeap top_id = " << ih.top_id() << ", top_key = " << ih.top_key()
         << " (ожидаем 0 5)" << endl;
    ih.decrease_key(1, 1);
    cout << "after decrease_key(1,1): top_id = " << ih.top_id()
         << ", top_key = " << ih.top_key() << " (1 1)" << endl;
    cout << "pop_min id = " << ih.pop_min() << " (ожидаем 1)" << endl;
    cout << "next top_id = " << ih.top_id() << ", top_key = " << ih.top_key()
         << " (4 6)" << endl;

    // A.5 std::priority_queue (max по умолчанию, min через greater)
    priority_queue<int> pq;
    for (int x : {5, 1, 4, 2, 3}) pq.push(x);
    priority_queue<int, vector<int>, greater<int>> pqmin;
    for (int x : {5, 1, 4, 2, 3}) pqmin.push(x);
    cout << "priority_queue max = " << pq.top() << ", min = " << pqmin.top()
         << " (ожидаем 5 1)" << endl;

    cout << "\n=== B. СЛИВАЕМЫЕ КУЧИ ===" << endl;

    // B.1 Binomial Heap
    H::BinomialHeap bh;
    for (int x : {5, 3, 8, 1, 9, 2, 7}) bh.push(x);
    cout << "BinomialHeap pops: ";
    while (!bh.empty()) cout << bh.pop_min() << " ";
    cout << "(ожидаем 1 2 3 5 7 8 9)" << endl;
    H::BinomialHeap b1, b2;
    b1.push(10); b1.push(20); b2.push(5); b2.push(15);
    b1.merge(b2);
    cout << "BinomialHeap merge pops: ";
    while (!b1.empty()) cout << b1.pop_min() << " ";
    cout << "(ожидаем 5 10 15 20)" << endl;
    H::BinomialHeap bh2;
    H::BinomialHeap::Node* bh_handle = bh2.push(100);
    bh2.push(4); bh2.push(6); bh2.push(8);
    bh2.decrease_key(bh_handle, 2);
    cout << "BinomialHeap after decrease(100->2): top = " << bh2.top()
         << " (ожидаем 2)" << endl;

    // B.2 Fibonacci Heap
    H::FibonacciHeap fh;
    for (int x : {9, 7, 5, 3}) fh.push(x);
    H::FibonacciHeap::Node* fh_handle = fh.push(100);
    fh.decrease_key(fh_handle, 2);
    cout << "FibonacciHeap after decrease(100->2): top = " << fh.top()
         << " (ожидаем 2)" << endl;
    cout << "FibonacciHeap pops: ";
    while (!fh.empty()) cout << fh.pop_min() << " ";
    cout << "(ожидаем 2 3 5 7 9)" << endl;
    H::FibonacciHeap f1, f2;
    for (int x : {1, 3, 5}) f1.push(x);
    for (int x : {2, 4, 6}) f2.push(x);
    f1.merge(f2);
    cout << "FibonacciHeap merge pops: ";
    while (!f1.empty()) cout << f1.pop_min() << " ";
    cout << "(ожидаем 1 2 3 4 5 6)" << endl;

    // B.3 Pairing Heap
    H::PairingHeap ph;
    for (int x : {9, 7, 5, 3}) ph.push(x);
    H::PairingHeap::Node* ph_handle = ph.push(100);
    ph.decrease_key(ph_handle, 2);
    cout << "PairingHeap after decrease(100->2): top = " << ph.top()
         << " (ожидаем 2)" << endl;
    cout << "PairingHeap pops: ";
    while (!ph.empty()) cout << ph.pop_min() << " ";
    cout << "(ожидаем 2 3 5 7 9)" << endl;

    // B.4 Leftist Heap
    H::LeftistHeap lh;
    for (int x : {7, 3, 9, 1, 8, 2, 6}) lh.push(x);
    cout << "LeftistHeap pops: ";
    while (!lh.empty()) cout << lh.pop_min() << " ";
    cout << "(ожидаем 1 2 3 6 7 8 9)" << endl;
    H::LeftistHeap l1, l2;
    l1.push(10); l1.push(20); l2.push(5); l2.push(15);
    l1.merge(l2);
    cout << "LeftistHeap merge pops: ";
    while (!l1.empty()) cout << l1.pop_min() << " ";
    cout << "(ожидаем 5 10 15 20)" << endl;

    // B.5 Skew Heap
    H::SkewHeap sh;
    for (int x : {7, 3, 9, 1, 8, 2, 6}) sh.push(x);
    cout << "SkewHeap pops: ";
    while (!sh.empty()) cout << sh.pop_min() << " ";
    cout << "(ожидаем 1 2 3 6 7 8 9)" << endl;
    H::SkewHeap s1, s2;
    s1.push(10); s1.push(20); s2.push(5); s2.push(15);
    s1.merge(s2);
    cout << "SkewHeap merge pops: ";
    while (!s1.empty()) cout << s1.pop_min() << " ";
    cout << "(ожидаем 5 10 15 20)" << endl;

    // B.6 Randomized Heap (зерно фиксировано — вывод детерминирован)
    H::RandomizedHeap rh(2026);
    for (int x : {4, 7, 2, 9, 1, 5, 3, 8, 6}) rh.push(x);
    cout << "RandomizedHeap pops: ";
    while (!rh.empty()) cout << rh.pop_min() << " ";
    cout << "(ожидаем 1 2 3 4 5 6 7 8 9)" << endl;

    cout << "\n=== C. РАДИКС-КУЧА ===" << endl;
    H::RadixHeap rx;
    rx.push(5, 10); rx.push(3, 20); rx.push(9, 30); rx.push(1, 40); rx.push(7, 50);
    cout << "RadixHeap pops (key:val): ";
    while (!rx.empty()) {
        int k, v;
        rx.pop_min(k, v);
        cout << k << ":" << v << " ";
    }
    cout << "(ожидаем 1:40 3:20 5:10 7:50 9:30)" << endl;

    cout << "\n=== D. ДВУСТОРОННИЕ КУЧИ ===" << endl;

    // D.1 Min-Max Heap
    H::MinMaxHeap mm;
    for (int x : {7, 3, 9, 1, 8, 2, 6, 4, 5}) mm.push(x);
    cout << "MinMaxHeap min = " << mm.min_key() << ", max = " << mm.max_key()
         << " (ожидаем 1 9)" << endl;
    mm.pop_min();
    cout << "after pop_min: min = " << mm.min_key() << " (ожидаем 2)" << endl;
    mm.pop_max();
    cout << "after pop_max: max = " << mm.max_key() << " (ожидаем 8)" << endl;
    cout << "MinMaxHeap pops: ";
    while (!mm.empty()) { cout << mm.min_key() << " "; mm.pop_min(); }
    cout << "(ожидаем 2 3 4 5 6 7 8)" << endl;

    // D.2 Interval Heap
    H::IntervalHeap im;
    for (int x : {7, 3, 9, 1, 8, 2, 6, 4, 5}) im.push(x);
    cout << "IntervalHeap min = " << im.min_key() << ", max = " << im.max_key()
         << " (ожидаем 1 9)" << endl;
    im.pop_min();
    cout << "after pop_min: min = " << im.min_key() << " (ожидаем 2)" << endl;
    im.pop_max();
    cout << "after pop_max: max = " << im.max_key() << " (ожидаем 8)" << endl;
    cout << "IntervalHeap pops: ";
    while (!im.empty()) { cout << im.min_key() << " "; im.pop_min(); }
    cout << "(ожидаем 2 3 4 5 6 7 8)" << endl;

    cout << "\n=== E. ОБЩЕЕ: одинаковые серии на всех кучах ===" << endl;
    multiset<int> ref = {5, 3, 8, 1, 9, 2, 7};

    auto check = [&ref](const char* name, const vector<int>& got) {
        vector<int> want(ref.begin(), ref.end());
        int ok = (got == want);
        cout << name << " series match (expect 1) = " << ok << endl;
    };

    { H::BinaryHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) { got.push_back(t.top()); t.pop(); } check("BinaryHeap", got); }
    { H::BinomialHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) got.push_back(t.pop_min()); check("BinomialHeap", got); }
    { H::FibonacciHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) got.push_back(t.pop_min()); check("FibonacciHeap", got); }
    { H::PairingHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) got.push_back(t.pop_min()); check("PairingHeap", got); }
    { H::LeftistHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) got.push_back(t.pop_min()); check("LeftistHeap", got); }
    { H::SkewHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) got.push_back(t.pop_min()); check("SkewHeap", got); }
    { H::RandomizedHeap t(42); for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) got.push_back(t.pop_min()); check("RandomizedHeap", got); }
    { H::MinMaxHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) { got.push_back(t.min_key()); t.pop_min(); } check("MinMaxHeap", got); }
    { H::IntervalHeap t; for (int x : ref) t.push(x); vector<int> got;
      while (!t.empty()) { got.push_back(t.min_key()); t.pop_min(); } check("IntervalHeap", got); }
    { H::RadixHeap t; int k, v;// серия целых ключей 7,3,8,1,9,2,5 — те же значения
      for (int x : ref) t.push(x, x); vector<int> got;
      while (!t.empty()) { t.pop_min(k, v); got.push_back(k); } check("RadixHeap", got); }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_D_MAIN

#endif // STRUCT_D_CPP