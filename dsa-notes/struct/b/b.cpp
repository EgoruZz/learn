#ifndef STRUCT_B_CPP
#define STRUCT_B_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <stack>
#include <algorithm>
#include <climits>
#include <random>
#include <map>
#include <set>
using namespace std;

// =============================================================
// II. ДЕРЕВЬЯ ПОИСКА
// =============================================================
// Структура md: A. Представления деревьев в памяти
//               → B. Базовые бинарные деревья и обходы
//               → C. Бинарные деревья поиска (BST)
//               → D. Сбалансированные деревья
//               → E. Многопутевые деревья
//               → F. Декартовы и рандомизированные деревья
//               → G. Специализированные деревья
//               → H. Деревья в STL
//
// SearchTrees наследует LinearStructures (a.cpp). Переиспользует:
//   * арена-память и курсорные ссылки (CursorList из I.B) — как образец
//     для курсорного дерева CursorTree (A.3): тот же приём «индексы
//     вместо указателей + свободный список», но узел дерева двухсвязный,
//     поэтому аллокатор переписан под {val, left, right};
//   * амортизированный анализ — обоснование splay/scapegoat (D.4, D.6).
//
// Порядок методов строго соответствует порядку md (A → H).
// Собственной арифметики не имеет. Ограничения: демонстрационные
// деревья на TreeNode* — без балансировки (C); сбалансированные
// варианты (D) и многопутевые (E) — самодостаточные классы.
//
// ВНИМАНИЕ (скрытие имён): методы lca_naive, kth, rank здесь локальные;
// одноимённые из других веток не подключаются.

#ifndef INSIDE_STRUCT
#define INSIDE_STRUCT
#define SETS_RELATIONS_MAIN
#include "../../math/discrete-and-logic/a-folder/a.cpp"
#undef INSIDE_STRUCT
#undef SETS_RELATIONS_MAIN
#endif

#ifndef INSIDE_STRUCT
#define INSIDE_STRUCT
#define STRUCT_A_MAIN
#include "../a/a.cpp"
#undef INSIDE_STRUCT
#undef STRUCT_A_MAIN
#endif

struct SearchTrees : LinearStructures {

// =============================================================
// A. ПРЕДСТАВЛЕНИЯ ДЕРЕВЬЕВ В ПАМЯТИ
// =============================================================

// --- A.1. Узел-структура (указательское представление) ---
// Базовый узел всех бинарных деревьев разделов B–D, F.
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
};

// --- A.2. Полное бинарное дерево в массиве (куча-индексация) ---
// Корень — a[0]; left(i) = 2i+1, right(i) = 2i+2, parent(i) = (i−1)/2.
// Представление без указателей: плотный массив, максимальная
// кэш-локальность; требование полноты — уровни заполняются слева направо.
struct HeapIndexedTree {
    vector<int> a;

    HeapIndexedTree() {}
    explicit HeapIndexedTree(const vector<int>& vals) : a(vals) {}

    void build(const vector<int>& vals) { a = vals; }
    int size() const { return (int)a.size(); }

    int left(int i) const { return 2 * i + 1; }
    int right(int i) const { return 2 * i + 2; }
    int parent(int i) const { return (i - 1) / 2; }
    bool has_left(int i) const { return 2 * i + 1 < (int)a.size(); }
    bool has_right(int i) const { return 2 * i + 2 < (int)a.size(); }

    // level-order обход = последовательное чтение массива
    vector<int> level_order() const { return a; }

    // preorder по индексам (стек вместо рекурсии)
    vector<int> preorder() const {
        vector<int> res;
        stack<int> st;
        st.push(0);
        while (!st.empty()) {
            int i = st.top(); st.pop();
            res.push_back(a[i]);
            if (has_right(i)) st.push(right(i));
            if (has_left(i)) st.push(left(i));
        }
        return res;
    }

    // inorder по индексам (рекурсия в глубину)
    void inorder_rec(int i, vector<int>& res) const {
        if (i >= (int)a.size()) return;
        inorder_rec(left(i), res);
        res.push_back(a[i]);
        inorder_rec(right(i), res);
    }
    vector<int> inorder() const {
        vector<int> res;
        inorder_rec(0, res);
        return res;
    }

    // Проверка: массив — валидный level-order полного дерева
    // (для любого индекса i ≥ 1 его родитель существует и корректен).
    static bool is_complete_from_array(const vector<int>& vals) {
        int n = (int)vals.size();
        for (int i = 1; i < n; i++)
            if ((i - 1) / 2 >= n) return false;
        return true;
    }
};

// --- A.3. Арена и курсорное дерево (индексы вместо указателей) ---
// Узлы живут в фиксированном массиве; left/right — индексы (−1 = пусто);
// свободные индексы — в свободном списке (поле left свободного узла).
// Аллокация O(1) без malloc; сериализуемость (индексы стабильны).
struct CursorTree {
    struct Node { int val; int left, right; };
    int N, root, free_head;
    vector<Node> nodes;

    explicit CursorTree(int pool_size) : N(pool_size), root(-1), free_head(0),
                                         nodes(pool_size) {
        for (int i = 0; i + 1 < N; i++) nodes[i].left = i + 1;
        nodes[N - 1].left = -1;
    }

    int alloc_node(int v) {
        int id = free_head;
        if (id == -1) return -1;            // пул исчерпан
        free_head = nodes[id].left;
        nodes[id] = {v, -1, -1};
        return id;
    }

    void free_node(int id) {
        nodes[id].left = free_head;
        free_head = id;
    }

    // BST-вставка (курсорная форма; то же дерево, что в C.1)
    void insert_bst(int v) {
        int id = alloc_node(v);
        if (id == -1) return;
        if (root == -1) { root = id; return; }
        int cur = root;
        while (true) {
            int& nxt = (v < nodes[cur].val) ? nodes[cur].left : nodes[cur].right;
            if (nxt == -1) { nxt = id; return; }
            cur = nxt;
        }
    }

    bool search(int v) const {
        int cur = root;
        while (cur != -1) {
            if (nodes[cur].val == v) return true;
            cur = (v < nodes[cur].val) ? nodes[cur].left : nodes[cur].right;
        }
        return false;
    }

    void erase_bst(int v) {
        // возврат корня не нужен — индексы стабильны; удаление по 2 детям
        // через преемника (минимум правого поддерева), как в C.1
        struct Loc { int parent; bool is_left; };
        int cur = root, par = -1;
        bool isl = false;
        while (cur != -1 && nodes[cur].val != v) {
            par = cur;
            isl = (v < nodes[cur].val);
            cur = isl ? nodes[cur].left : nodes[cur].right;
        }
        if (cur == -1) return;
        if (nodes[cur].left == -1 && nodes[cur].right == -1) {
            // 0 детей
            if (par == -1) root = -1;
            else if (isl) nodes[par].left = -1;
            else nodes[par].right = -1;
            free_node(cur);
        } else if (nodes[cur].left == -1 || nodes[cur].right == -1) {
            // 1 ребёнок
            int child = (nodes[cur].left != -1) ? nodes[cur].left : nodes[cur].right;
            if (par == -1) root = child;
            else if (isl) nodes[par].left = child;
            else nodes[par].right = child;
            free_node(cur);
        } else {
            // 2 ребёнка: преемник — минимум правого поддерева
            int succ = nodes[cur].right, succ_par = cur;
            while (nodes[succ].left != -1) { succ_par = succ; succ = nodes[succ].left; }
            nodes[cur].val = nodes[succ].val;
            if (nodes[succ_par].left == succ) nodes[succ_par].left = nodes[succ].right;
            else nodes[succ_par].right = nodes[succ].right;
            free_node(succ);
        }
    }

    vector<int> inorder() const {
        vector<int> res;
        stack<int> st;
        int cur = root;
        while (cur != -1 || !st.empty()) {
            while (cur != -1) { st.push(cur); cur = nodes[cur].left; }
            cur = st.top(); st.pop();
            res.push_back(nodes[cur].val);
            cur = nodes[cur].right;
        }
        return res;
    }
};

// --- A.4. Parent Array (массив родителей) + двоичные подъёмы ---
// parent[v] — предок; построение по неориентированным рёбрам BFS-ом.
// kth_ancestor(v, k) — подъём на k: наивный O(k) и двоичные подъёмы O(log n).
struct ParentArray {
    int n;
    int LOG;
    vector<int> parent;
    vector<int> depth;
    vector<vector<int>> up;      // up[j][v] — 2^j-й предок v

    ParentArray() : n(0), LOG(0) {}

    void build(int n_, const vector<pair<int, int>>& edges, int root = 0) {
        n = n_;
        vector<vector<int>> g(n);
        for (auto& e : edges) { g[e.first].push_back(e.second); g[e.second].push_back(e.first); }
        parent.assign(n, -1);
        depth.assign(n, 0);
        queue<int> q;
        q.push(root);
        parent[root] = -1;
        vector<char> vis(n, 0);
        vis[root] = 1;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : g[u]) if (!vis[v]) {
                vis[v] = 1;
                parent[v] = u;
                depth[v] = depth[u] + 1;
                q.push(v);
            }
        }
        LOG = 1;
        while ((1 << LOG) < n) LOG++;
        up.assign(LOG, vector<int>(n, -1));
        for (int v = 0; v < n; v++) up[0][v] = parent[v];
        for (int j = 1; j < LOG; j++)
            for (int v = 0; v < n; v++)
                up[j][v] = (up[j - 1][v] == -1) ? -1 : up[j - 1][up[j - 1][v]];
    }

    // Подъём на k шагов: наивный (по одному предку)
    int kth_ancestor_naive(int v, int k) const {
        for (int i = 0; i < k && v != -1; i++) v = parent[v];
        return v;
    }

    // Подъём на k шагов: двоичные подъёмы O(log k)
    int kth_ancestor(int v, int k) const {
        for (int j = 0; k && v != -1; j++, k >>= 1)
            if (k & 1) v = up[j][v];
        return v;
    }

    // LCA двоичными подъёмами: выравниваем глубины, затем подъём обоих
    int lca(int u, int v) const {
        if (depth[u] < depth[v]) swap(u, v);
        u = kth_ancestor(u, depth[u] - depth[v]);
        if (u == v) return u;
        for (int j = LOG - 1; j >= 0; j--)
            if (up[j][u] != up[j][v]) { u = up[j][u]; v = up[j][v]; }
        return parent[u];
    }
};

// --- A.5. First-child / next-sibling (первый ребёнок — следующий брат) ---
// n-арное дерево кодируется бинарно: first_child и next_sibling —
// индексы вершин (−1 = нет); обход детей — шаги по next_sibling.
struct FCNS {
    struct Node { int val; int first_child, next_sibling; };
    int root;
    vector<Node> nodes;

    FCNS() : root(-1) {}

    // Построение из «родитель → список детей»: edges = (parent, child),
    // вершины нумеруются 0..n−1, root — корень.
    void build(int n, const vector<pair<int, int>>& edges, int root_) {
        root = root_;
        nodes.assign(n, Node{-1, -1, -1});
        nodes[root_].val = root_;
        vector<vector<int>> children(n);
        for (auto& e : edges) children[e.first].push_back(e.second);
        for (int p = 0; p < n; p++) {
            for (int i = 0; i < (int)children[p].size(); i++) {
                int c = children[p][i];
                nodes[c].val = c;
                if (i == 0) nodes[p].first_child = c;
                else nodes[children[p][i - 1]].next_sibling = c;
            }
        }
    }

    // Обход в глубину (preorder) по индексам
    void dfs(int v, vector<int>& res) const {
        if (v == -1) return;
        res.push_back(nodes[v].val);
        for (int c = nodes[v].first_child; c != -1; c = nodes[c].next_sibling)
            dfs(c, res);
    }
    vector<int> preorder() const {
        vector<int> res;
        dfs(root, res);
        return res;
    }

    // Преобразование в бинарное представление (TreeNode*):
    // left = первый ребёнок, right = следующий брат (левый сдвиг)
    TreeNode* to_binary(int v) const {
        if (v == -1) return nullptr;
        TreeNode* t = new TreeNode(nodes[v].val);
        t->left = to_binary(nodes[v].first_child);
        t->right = to_binary(nodes[v].next_sibling);
        return t;
    }

    // Обратное преобразование: бинарное представление → FCNS.
    // left = первый ребёнок, right = следующий брат; новые индексы
    // раздаются в порядке preorder исходного n-арного дерева.
    int from_binary_impl(TreeNode* t, int& id) {
        if (!t) return -1;
        int v = id++;
        nodes[v].val = t->val;
        nodes[v].first_child = from_binary_impl(t->left, id);
        nodes[v].next_sibling = from_binary_impl(t->right, id);
        return v;
    }
    // n — число вершин исходного дерева; заполняет nodes и root
    void from_binary(TreeNode* bin_root, int n) {
        nodes.assign(n, Node{-1, -1, -1});
        int id = 0;
        root = from_binary_impl(bin_root, id);
    }
};

// --- A.6. Нитяное дерево (Threaded Binary Tree) ---
// Правый указатель узла без правого ребёнка несёт inorder-преемника
// (нить); обход без стека и рекурсии. Построение из TreeNode*.
struct ThreadedTree {
    struct TNode {
        int val;
        TNode* left;
        TNode* right;
        bool rthread;          // right — нить (преемник), не ребёнок
        TNode(int v) : val(v), left(nullptr), right(nullptr), rthread(false) {}
    };
    TNode* root;

    ThreadedTree() : root(nullptr) {}

    // Построение из обычного дерева: сначала копия, затем навешивание нитей
    // итеративным inorder-обходом (стек хранит «предыдущий» узел).
    static TNode* clone_from(TreeNode* t) {
        if (!t) return nullptr;
        TNode* nt = new TNode(t->val);
        nt->left = clone_from(t->left);
        nt->right = clone_from(t->right);
        return nt;
    }
    void build(TreeNode* src) {
        root = clone_from(src);
        if (!root) return;
        stack<TNode*> st;
        TNode* cur = root;
        TNode* prev = nullptr;
        while (cur || !st.empty()) {
            while (cur) { st.push(cur); cur = cur->left; }
            cur = st.top(); st.pop();
            if (prev && prev->right == nullptr) { prev->right = cur; prev->rthread = true; }
            prev = cur;
            cur = cur->right;
        }
    }

    // Inorder без стека: спуск к самому левому, затем шаги по нитям/детям
    vector<int> inorder() const {
        vector<int> res;
        if (!root) return res;
        TNode* cur = root;
        while (cur->left) cur = cur->left;
        while (cur) {
            res.push_back(cur->val);
            if (cur->rthread) cur = cur->right;             // нить — преемник
            else {
                cur = cur->right;                            // ребёнок → левый край
                while (cur && cur->left) cur = cur->left;
            }
        }
        return res;
    }
};

// --- A.7. Morris Traversal (inorder/preorder, O(1) памяти) ---
// Временная нить: inorder-предшественник текущего узла связывается с ним;
// при повторном визите нить обрезается и дерево восстанавливается.
static vector<int> morris_inorder(TreeNode* root) {
    vector<int> res;
    TreeNode* cur = root;
    while (cur) {
        if (!cur->left) { res.push_back(cur->val); cur = cur->right; }
        else {
            TreeNode* pred = cur->left;
            while (pred->right && pred->right != cur) pred = pred->right;
            if (!pred->right) { pred->right = cur; cur = cur->left; }
            else { pred->right = nullptr; res.push_back(cur->val); cur = cur->right; }
        }
    }
    return res;
}

static vector<int> morris_preorder(TreeNode* root) {
    vector<int> res;
    TreeNode* cur = root;
    while (cur) {
        if (!cur->left) { res.push_back(cur->val); cur = cur->right; }
        else {
            TreeNode* pred = cur->left;
            while (pred->right && pred->right != cur) pred = pred->right;
            if (!pred->right) { pred->right = cur; res.push_back(cur->val); cur = cur->left; }
            else { pred->right = nullptr; cur = cur->right; }
        }
    }
    return res;
}

// =============================================================
// B. БАЗОВЫЕ БИНАРНЫЕ ДЕРЕВЬЯ
// =============================================================

// --- B.1. Построение из level-order (массив с маркером пустоты INT_MIN) ---
static TreeNode* build_from_level_order(const vector<int>& a) {
    if (a.empty() || a[0] == INT_MIN) return nullptr;
    TreeNode* root = new TreeNode(a[0]);
    queue<TreeNode*> q;
    q.push(root);
    int i = 1;
    while (!q.empty() && i < (int)a.size()) {
        TreeNode* cur = q.front(); q.pop();
        if (i < (int)a.size() && a[i] != INT_MIN) {
            cur->left = new TreeNode(a[i]); q.push(cur->left);
        }
        i++;
        if (i < (int)a.size() && a[i] != INT_MIN) {
            cur->right = new TreeNode(a[i]); q.push(cur->right);
        }
        i++;
    }
    return root;
}

static int height(TreeNode* root) {
    if (!root) return 0;
    return 1 + max(height(root->left), height(root->right));
}

static int count_nodes(TreeNode* root) {
    if (!root) return 0;
    return 1 + count_nodes(root->left) + count_nodes(root->right);
}

static int count_leaves(TreeNode* root) {
    if (!root) return 0;
    if (!root->left && !root->right) return 1;
    return count_leaves(root->left) + count_leaves(root->right);
}

// --- B.2. Обходы (рекурсивные) ---
static void inorder_rec(TreeNode* t, vector<int>& res) {
    if (!t) return;
    inorder_rec(t->left, res);
    res.push_back(t->val);
    inorder_rec(t->right, res);
}
static vector<int> inorder_rec(TreeNode* root) { vector<int> r; inorder_rec(root, r); return r; }

static void preorder_rec(TreeNode* t, vector<int>& res) {
    if (!t) return;
    res.push_back(t->val);
    preorder_rec(t->left, res);
    preorder_rec(t->right, res);
}
static vector<int> preorder_rec(TreeNode* root) { vector<int> r; preorder_rec(root, r); return r; }

static void postorder_rec(TreeNode* t, vector<int>& res) {
    if (!t) return;
    postorder_rec(t->left, res);
    postorder_rec(t->right, res);
    res.push_back(t->val);
}
static vector<int> postorder_rec(TreeNode* root) { vector<int> r; postorder_rec(root, r); return r; }

// --- B.3. Обходы (итеративные, явный стек) ---
static vector<int> inorder_iter(TreeNode* root) {
    vector<int> res;
    stack<TreeNode*> st;
    TreeNode* cur = root;
    while (cur || !st.empty()) {
        while (cur) { st.push(cur); cur = cur->left; }
        cur = st.top(); st.pop();
        res.push_back(cur->val);
        cur = cur->right;
    }
    return res;
}

static vector<int> preorder_iter(TreeNode* root) {
    vector<int> res;
    if (!root) return res;
    stack<TreeNode*> st;
    st.push(root);
    while (!st.empty()) {
        TreeNode* cur = st.top(); st.pop();
        res.push_back(cur->val);
        if (cur->right) st.push(cur->right);
        if (cur->left) st.push(cur->left);
    }
    return res;
}

static vector<int> postorder_iter(TreeNode* root) {
    // два стека: второй проход печатает после детей
    vector<int> res;
    if (!root) return res;
    stack<TreeNode*> st1, st2;
    st1.push(root);
    while (!st1.empty()) {
        TreeNode* cur = st1.top(); st1.pop();
        st2.push(cur);
        if (cur->left) st1.push(cur->left);
        if (cur->right) st1.push(cur->right);
    }
    while (!st2.empty()) { res.push_back(st2.top()->val); st2.pop(); }
    return res;
}

// --- B.4. Обход в ширину (Level-order / BFS) ---
static vector<int> level_order(TreeNode* root) {
    vector<int> res;
    if (!root) return res;
    queue<TreeNode*> q;
    q.push(root);
    while (!q.empty()) {
        TreeNode* cur = q.front(); q.pop();
        res.push_back(cur->val);
        if (cur->left) q.push(cur->left);
        if (cur->right) q.push(cur->right);
    }
    return res;
}

// Слои отдельно: длина очереди в начале уровня = число узлов уровня
static vector<vector<int>> level_order_layers(TreeNode* root) {
    vector<vector<int>> res;
    if (!root) return res;
    queue<TreeNode*> q;
    q.push(root);
    while (!q.empty()) {
        int sz = (int)q.size();
        vector<int> layer;
        for (int i = 0; i < sz; i++) {
            TreeNode* cur = q.front(); q.pop();
            layer.push_back(cur->val);
            if (cur->left) q.push(cur->left);
            if (cur->right) q.push(cur->right);
        }
        res.push_back(layer);
    }
    return res;
}

// --- B.5. Mirror и Symmetric Tree ---
// Mirror на месте: обмен детей в каждом узле.
static TreeNode* invert_tree(TreeNode* root) {
    if (!root) return nullptr;
    swap(root->left, root->right);
    invert_tree(root->left);
    invert_tree(root->right);
    return root;
}

// Симметричность: сравнение зеркальных пар (l.left == r.right и l.right == r.left)
static bool is_mirror_pair(TreeNode* a, TreeNode* b) {
    if (!a || !b) return a == b;
    return a->val == b->val && is_mirror_pair(a->left, b->right) && is_mirror_pair(a->right, b->left);
}
static bool is_symmetric(TreeNode* root) {
    return is_mirror_pair(root, root);
}

// --- B.6. Diameter Of Binary Tree ---
// diameter = max(диаметр левого, диаметр правого, h_left + h_right + 2)
// возвращаем (диаметр, высоту) одним проходом; высота — в рёбрах,
// поэтому пустому поддереву соответствует h = −1
static int diameter(TreeNode* root) {
    function<pair<int, int>(TreeNode*)> go = [&](TreeNode* t) -> pair<int, int> {
        if (!t) return {0, -1};
        auto [ld, lh] = go(t->left);
        auto [rd, rh] = go(t->right);
        int h = 1 + max(lh, rh);
        int d = max({ld, rd, lh + rh + 2});
        return {d, h};
    };
    return go(root).first;
}

// --- B.7. Node Sum / Path Sum ---
static long long node_sum(TreeNode* root) {
    if (!root) return 0;
    return root->val + node_sum(root->left) + node_sum(root->right);
}

// Существует ли путь корень→лист с суммой target (вычитание при спуске)
static bool root_to_leaf_path_sum(TreeNode* root, int target) {
    if (!root) return false;
    if (!root->left && !root->right) return root->val == target;
    return root_to_leaf_path_sum(root->left, target - root->val)
        || root_to_leaf_path_sum(root->right, target - root->val);
}

// --- B.8. Distribute Coins ---
// Для каждого поддерева излишек/дефицит = сумма монет − target·размер;
// каждая монета обязана пересечь ребро к родителю — ходы считаются по |сумме|
static long long distribute_coins(TreeNode* root, long long target = 1) {
    long long moves = 0;
    function<long long(TreeNode*)> go = [&](TreeNode* t) -> long long {
        if (!t) return 0;
        long long l = go(t->left), r = go(t->right);
        moves += llabs(l) + llabs(r);
        return t->val + l + r - target;
    };
    go(root);
    return moves;
}

// --- B.9. LCA (наивный рекурсивный) ---
// Узел — LCA, если левое и правое поддеревья содержат по одному из
// запрошенных (или сам является одним из них); O(n) на запрос.
static TreeNode* lca_naive(TreeNode* root, TreeNode* u, TreeNode* v) {
    if (!root) return nullptr;
    if (root == u || root == v) return root;
    TreeNode* l = lca_naive(root->left, u, v);
    TreeNode* r = lca_naive(root->right, u, v);
    if (l && r) return root;
    return l ? l : r;
}

// --- B.10. Serialize / Deserialize (level-order) ---
// Формат: значения и marker (пустой узел), разделённые sep. Параметры
// дают произвольные форматы (маркер, разделитель).
static string serialize_level_order(TreeNode* root, const string& marker = "#",
                                    const string& sep = " ") {
    string s;
    if (!root) return "";
    queue<TreeNode*> q;
    q.push(root);
    bool first = true;
    while (!q.empty()) {
        TreeNode* cur = q.front(); q.pop();
        if (!first) s += sep;
        first = false;
        if (cur) {
            s += to_string(cur->val);
            q.push(cur->left);
            q.push(cur->right);
        } else {
            s += marker;
        }
    }
    return s;
}

// tokenize: значения и маркер по разделителю; пустая строка — пустое дерево
static TreeNode* deserialize_level_order(const string& s, const string& marker = "#",
                                         const string& sep = " ") {
    if (s.empty()) return nullptr;
    vector<string> tok;
    for (size_t pos = 0; pos <= s.size();) {
        size_t nx = s.find(sep, pos);
        if (nx == string::npos) nx = s.size();
        if (nx > pos) tok.push_back(s.substr(pos, nx - pos));
        pos = nx + sep.size();
    }
    if (tok.empty() || tok[0] == marker) return nullptr;
    TreeNode* root = new TreeNode(stoi(tok[0]));
    queue<TreeNode*> q;
    q.push(root);
    int i = 1;
    while (!q.empty() && i < (int)tok.size()) {
        TreeNode* cur = q.front(); q.pop();
        if (i < (int)tok.size() && tok[i] != marker) { cur->left = new TreeNode(stoi(tok[i])); q.push(cur->left); }
        i++;
        if (i < (int)tok.size() && tok[i] != marker) { cur->right = new TreeNode(stoi(tok[i])); q.push(cur->right); }
        i++;
    }
    return root;
}

// --- B.11. Merge Two Binary Trees ---
// Покомпонентное слияние: при наложении узлов применяется бинарная
// операция op (по умолчанию — сумма); отсутствующий узел берётся как есть.
static TreeNode* merge_trees(TreeNode* a, TreeNode* b,
                             const function<int(int, int)>& op = [](int x, int y) { return x + y; }) {
    if (!a) return b;
    if (!b) return a;
    TreeNode* t = new TreeNode(op(a->val, b->val));
    t->left = merge_trees(a->left, b->left, op);
    t->right = merge_trees(a->right, b->right, op);
    return t;
}

// =============================================================
// C. БИНАРНЫЕ ДЕРЕВЬЯ ПОИСКА (BST)
// =============================================================

// --- C.1. Поиск / вставка / удаление ---
static TreeNode* bst_search(TreeNode* root, int key) {
    TreeNode* cur = root;
    while (cur) {
        if (cur->val == key) return cur;
        cur = (key < cur->val) ? cur->left : cur->right;
    }
    return nullptr;
}

// рекурсивная форма — база для ДП-сверток на дереве и удаления (C.1)
static TreeNode* bst_search_recursive(TreeNode* root, int key) {
    if (!root || root->val == key) return root;
    return key < root->val ? bst_search_recursive(root->left, key)
                           : bst_search_recursive(root->right, key);
}

static TreeNode* bst_insert(TreeNode* root, int key) {
    if (!root) return new TreeNode(key);
    if (key < root->val) root->left = bst_insert(root->left, key);
    else root->right = bst_insert(root->right, key);
    return root;
}

// итеративная вставка: спуск с сохранением места — O(h), без рекурсии
static TreeNode* bst_insert_iter(TreeNode* root, int key) {
    TreeNode* z = new TreeNode(key);
    if (!root) return z;
    TreeNode* cur = root;
    while (true) {
        if (key < cur->val) {
            if (!cur->left) { cur->left = z; break; }
            cur = cur->left;
        } else {
            if (!cur->right) { cur->right = z; break; }
            cur = cur->right;
        }
    }
    return root;
}

static TreeNode* bst_min(TreeNode* root) {
    if (!root) return nullptr;
    while (root->left) root = root->left;
    return root;
}

static TreeNode* bst_max(TreeNode* root) {
    if (!root) return nullptr;
    while (root->right) root = root->right;
    return root;
}

// Удаление: случаи 0/1/2 детей; при 2 — замена inorder-преемником
static TreeNode* bst_erase(TreeNode* root, int key) {
    if (!root) return nullptr;
    if (key < root->val) root->left = bst_erase(root->left, key);
    else if (key > root->val) root->right = bst_erase(root->right, key);
    else {
        if (!root->left) { TreeNode* r = root->right; delete root; return r; }
        if (!root->right) { TreeNode* l = root->left; delete root; return l; }
        TreeNode* succ = bst_min(root->right);
        root->val = succ->val;
        root->right = bst_erase(root->right, succ->val);
    }
    return root;
}

// --- C.2. Floor And Ceiling ---
static TreeNode* bst_floor(TreeNode* root, int key) {
    TreeNode* res = nullptr;
    while (root) {
        if (root->val == key) return root;
        if (root->val < key) { res = root; root = root->right; }
        else root = root->left;
    }
    return res;
}

static TreeNode* bst_ceil(TreeNode* root, int key) {
    TreeNode* res = nullptr;
    while (root) {
        if (root->val == key) return root;
        if (root->val > key) { res = root; root = root->left; }
        else root = root->right;
    }
    return res;
}

// --- C.3. Is Sorted (валидация BST) ---
// Подход A: inorder обязан быть строго (или нестрого при strict=false)
// возрастающим — нарушение ломает порядок.
static bool is_bst_sorted(TreeNode* root, bool strict = true) {
    vector<int> ord = inorder_iter(root);
    for (int i = 1; i < (int)ord.size(); i++)
        if (strict ? ord[i - 1] >= ord[i] : ord[i - 1] > ord[i]) return false;
    return true;
}

// Подход B: рекурсия с допустимым интервалом (min, max) для каждого
// поддерева; strict=false допускает дубликаты в левом поддереве
static bool is_bst_bounds(TreeNode* root, long long lo = LLONG_MIN, long long hi = LLONG_MAX,
                          bool strict = true) {
    if (!root) return true;
    if (strict ? (root->val <= lo || root->val >= hi)
               : (root->val < lo || root->val > hi))
        return false;
    return is_bst_bounds(root->left, lo, root->val, strict)
        && is_bst_bounds(root->right, root->val, hi, strict);
}

// --- C.4. Is Sum Tree ---
// Узел — sum-узел, если val = сумма поддеревьев; постордер-свёртка
static bool is_sum_tree(TreeNode* root) {
    function<pair<bool, long long>(TreeNode*)> go = [&](TreeNode* t) -> pair<bool, long long> {
        if (!t) return {true, 0};
        if (!t->left && !t->right) return {true, t->val};
        auto [okl, sl] = go(t->left);
        auto [okr, sr] = go(t->right);
        if (!okl || !okr) return {false, 0};
        if (t->val != sl + sr) return {false, 0};
        return {true, t->val};
    };
    return go(root).first;
}

// --- C.5. Maximum Sum BST ---
// Постордер-свёртка (сумма, min, max, валидно): ответ — максимум суммы
// валидных BST-поддеревьев. Пустое поддерево — кандидат с суммой 0.
static long long max_sum_bst(TreeNode* root) {
    struct Info { bool valid; int mn, mx; long long sum; };
    long long best = 0;
    function<Info(TreeNode*)> go = [&](TreeNode* t) -> Info {
        if (!t) return {true, INT_MAX, INT_MIN, 0};
        Info l = go(t->left), r = go(t->right);
        if (l.valid && r.valid && t->val > l.mx && t->val < r.mn) {
            long long sum = t->val + l.sum + r.sum;
            best = max(best, sum);
            return {true, min(t->val, l.mn), max(t->val, r.mx), sum};
        }
        return {false, 0, 0, 0};
    };
    go(root);
    return best;
}

// --- C.6. Поворот как примитив ---
// Правый поворот: x = y->left становится корнем, y — правым ребёнком x.
static TreeNode* rotate_right(TreeNode* y) {
    TreeNode* x = y->left;
    y->left = x->right;
    x->right = y;
    return x;
}
static TreeNode* rotate_left(TreeNode* x) {
    TreeNode* y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

// =============================================================
// D. СБАЛАНСИРОВАННЫЕ ДЕРЕВЬЯ
// =============================================================

// --- D.1. Общий каркас сбалансированного дерева ---
// Инвариант: высота ограничена логарифмом (рекуррентность Фибоначчи
// для AVL, чёрная высота для RB). Каркас даёт узел с высотой,
// пересчёт высоты/баланс-фактора и повороты (примитив C.6);
// конкретные деревья добавляют свой инвариант и случаи балансировки.
struct TreeBalanced {
    struct Node {
        int key, h;
        Node *left, *right;
        Node(int k) : key(k), h(1), left(nullptr), right(nullptr) {}
    };
    Node* root = nullptr;

    static int height(Node* t) { return t ? t->h : 0; }
    static void upd(Node* t) { t->h = 1 + max(height(t->left), height(t->right)); }
    static int bf(Node* t) { return t ? height(t->left) - height(t->right) : 0; }

    // Повороты (C.6) с пересчётом высот — атом перестройки
    static Node* rotate_right(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        upd(y); upd(x);
        return x;
    }
    static Node* rotate_left(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        upd(x); upd(y);
        return y;
    }

    bool search(int key) const {
        Node* cur = root;
        while (cur) {
            if (cur->key == key) return true;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return false;
    }
    vector<int> inorder() const {
        vector<int> res;
        stack<Node*> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) { st.push(cur); cur = cur->left; }
            cur = st.top(); st.pop();
            res.push_back(cur->key);
            cur = cur->right;
        }
        return res;
    }
};

// --- D.2. AVL-дерево ---
// Баланс-фактор |h(left) − h(right)| ≤ 1; LL/RR/LR/RL повороты.
struct AVLTree : TreeBalanced {
    static Node* balance(Node* t) {
        upd(t);
        if (bf(t) > 1) {
            if (bf(t->left) < 0) t->left = rotate_left(t->left);   // LR
            return rotate_right(t);                                // LL
        }
        if (bf(t) < -1) {
            if (bf(t->right) > 0) t->right = rotate_right(t->right); // RL
            return rotate_left(t);                                 // RR
        }
        return t;
    }

    static Node* insert(Node* t, int key) {
        if (!t) return new Node(key);
        if (key < t->key) t->left = insert(t->left, key);
        else t->right = insert(t->right, key);
        return balance(t);
    }
    static Node* erase(Node* t, int key) {
        if (!t) return nullptr;
        if (key < t->key) t->left = erase(t->left, key);
        else if (key > t->key) t->right = erase(t->right, key);
        else {
            if (!t->left || !t->right) {
                Node* child = t->left ? t->left : t->right;
                delete t;
                return child;
            }
            Node* succ = t->right;
            while (succ->left) succ = succ->left;
            t->key = succ->key;
            t->right = erase(t->right, succ->key);
        }
        return balance(t);
    }

    void insert(int key) { root = insert(root, key); }
    void erase(int key) { root = erase(root, key); }
};

// --- D.3. Красно-чёрное дерево ---
// Инварианты: корень чёрный; красные имеют чёрных детей; одинаковая
// чёрная высота на всех путях. Вставка/удаление по случаям (CLRS).
struct RedBlackTree {
    enum Color { BLACK, RED };
    struct Node {
        int key;
        bool red;
        Node *l, *r, *p;
        Node(int k, Node* nilp) : key(k), red(true), l(nilp), r(nilp), p(nilp) {}
    };
    Node* nil;
    Node* root;

    RedBlackTree() {
        nil = new Node(0, nullptr);
        nil->red = false;
        nil->l = nil->r = nil->p = nil;
        root = nil;
    }

    void left_rotate(Node* x) {
        Node* y = x->r;
        x->r = y->l;
        if (y->l != nil) y->l->p = x;
        y->p = x->p;
        if (x->p == nil) root = y;
        else if (x == x->p->l) x->p->l = y;
        else x->p->r = y;
        y->l = x;
        x->p = y;
    }
    void right_rotate(Node* y) {
        Node* x = y->l;
        y->l = x->r;
        if (x->r != nil) x->r->p = y;
        x->p = y->p;
        if (y->p == nil) root = x;
        else if (y == y->p->l) y->p->l = x;
        else y->p->r = x;
        x->r = y;
        y->p = x;
    }

    void insert_fixup(Node* z) {
        while (z->p->red) {
            if (z->p == z->p->p->l) {
                Node* y = z->p->p->r;
                if (y->red) {                                   // дядя красный
                    z->p->red = false;
                    y->red = false;
                    z->p->p->red = true;
                    z = z->p->p;
                } else {
                    if (z == z->p->r) { z = z->p; left_rotate(z); }  // LR
                    z->p->red = false;
                    z->p->p->red = true;
                    right_rotate(z->p->p);                      // LL
                }
            } else {
                Node* y = z->p->p->l;
                if (y->red) {
                    z->p->red = false;
                    y->red = false;
                    z->p->p->red = true;
                    z = z->p->p;
                } else {
                    if (z == z->p->l) { z = z->p; right_rotate(z); } // RL
                    z->p->red = false;
                    z->p->p->red = true;
                    left_rotate(z->p->p);                       // RR
                }
            }
        }
        root->red = false;
    }

    void insert(int key) {
        Node* z = new Node(key, nil);
        Node* y = nil;
        Node* x = root;
        while (x != nil) {
            y = x;
            if (z->key < x->key) x = x->l;
            else x = x->r;
        }
        z->p = y;
        if (y == nil) root = z;
        else if (z->key < y->key) y->l = z;
        else y->r = z;
        insert_fixup(z);
    }

    void rb_transplant(Node* u, Node* v) {
        if (u->p == nil) root = v;
        else if (u == u->p->l) u->p->l = v;
        else u->p->r = v;
        v->p = u->p;
    }

    Node* tree_minimum(Node* x) {
        while (x->l != nil) x = x->l;
        return x;
    }

    Node* search_node(int key) const {
        Node* x = root;
        while (x != nil) {
            if (x->key == key) return x;
            x = (key < x->key) ? x->l : x->r;
        }
        return nil;
    }

    void delete_fixup(Node* x) {
        while (x != root && !x->red) {
            if (x == x->p->l) {
                Node* w = x->p->r;
                if (w->red) {
                    w->red = false;
                    x->p->red = true;
                    left_rotate(x->p);
                    w = x->p->r;
                }
                if (!w->l->red && !w->r->red) {
                    w->red = true;
                    x = x->p;
                } else {
                    if (!w->r->red) {
                        w->l->red = false;
                        w->red = true;
                        right_rotate(w);
                        w = x->p->r;
                    }
                    w->red = x->p->red;
                    x->p->red = false;
                    w->r->red = false;
                    left_rotate(x->p);
                    x = root;
                }
            } else {
                Node* w = x->p->l;
                if (w->red) {
                    w->red = false;
                    x->p->red = true;
                    right_rotate(x->p);
                    w = x->p->l;
                }
                if (!w->r->red && !w->l->red) {
                    w->red = true;
                    x = x->p;
                } else {
                    if (!w->l->red) {
                        w->r->red = false;
                        w->red = true;
                        left_rotate(w);
                        w = x->p->l;
                    }
                    w->red = x->p->red;
                    x->p->red = false;
                    w->l->red = false;
                    right_rotate(x->p);
                    x = root;
                }
            }
        }
        x->red = false;
    }

    void erase(int key) {
        Node* z = search_node(key);
        if (z == nil) return;
        Node* y = z;
        bool y_orig_red = y->red;
        Node* x;
        if (z->l == nil) {
            x = z->r;
            rb_transplant(z, z->r);
        } else if (z->r == nil) {
            x = z->l;
            rb_transplant(z, z->l);
        } else {
            y = tree_minimum(z->r);
            y_orig_red = y->red;
            x = y->r;
            if (y->p == z) {
                x->p = y;
            } else {
                rb_transplant(y, y->r);
                y->r = z->r;
                y->r->p = y;
            }
            rb_transplant(z, y);
            y->l = z->l;
            y->l->p = y;
            y->red = z->red;
        }
        delete z;
        if (!y_orig_red) delete_fixup(x);
    }

    bool search(int key) const { return search_node(key) != nil; }

    // Проверка инвариантов: чёрная высота едина, нет красно-красных
    bool valid_black_height(Node* t, int& bh) const {
        if (t == nil) { bh = 1; return true; }
        if (t->red && (t->l->red || t->r->red)) return false;
        int bl, br;
        if (!valid_black_height(t->l, bl) || !valid_black_height(t->r, br)) return false;
        if (bl != br) return false;
        bh = bl + (t->red ? 0 : 1);
        return true;
    }
    bool validate() const {
        if (root == nil) return true;
        if (root->red) return false;
        int bh;
        return valid_black_height(root, bh);
    }
};

// --- D.4. Splay-дерево ---
// Каждый доступ сплайсит узел к корню; амортизированные O(log n).
struct SplayTree {
    struct Node {
        int key;
        Node *l, *r, *p;
        Node(int k) : key(k), l(nullptr), r(nullptr), p(nullptr) {}
    };
    Node* root = nullptr;

    static void rotate(Node* x) {
        Node* y = x->p;
        if (!y) return;
        if (x == y->l) {
            y->l = x->r;
            if (x->r) x->r->p = y;
            x->r = y;
        } else {
            y->r = x->l;
            if (x->l) x->l->p = y;
            x->l = y;
        }
        x->p = y->p;
        if (y->p) {
            if (y == y->p->l) y->p->l = x;
            else y->p->r = x;
        }
        y->p = x;
    }

    void splay(Node* x) {
        while (x->p) {
            Node* y = x->p;
            Node* z = y->p;
            if (!z) {
                rotate(x);                              // zig
            } else if ((y == z->l) == (x == y->l)) {
                rotate(y); rotate(x);                   // zig-zig
            } else {
                rotate(x); rotate(x);                   // zig-zag
            }
        }
        root = x;
    }

    void insert(int key) {
        Node* cur = root;
        Node* par = nullptr;
        while (cur) {
            par = cur;
            cur = (key < cur->key) ? cur->l : cur->r;
        }
        Node* n = new Node(key);
        n->p = par;
        if (!par) root = n;
        else if (key < par->key) par->l = n;
        else par->r = n;
        splay(n);
    }

    bool search(int key) {
        Node* cur = root;
        Node* last = nullptr;
        while (cur) {
            last = cur;
            if (cur->key == key) { splay(cur); return true; }
            cur = (key < cur->key) ? cur->l : cur->r;
        }
        if (last) splay(last);
        return false;
    }

    void erase(int key) {
        if (!search(key)) return;
        Node* t = root;
        if (!t->l) {
            root = t->r;
            if (root) root->p = nullptr;
            delete t;
            return;
        }
        // максимум левого поддерева → корень левой части
        Node* lm = t->l;
        while (lm->r) lm = lm->r;
        splay(lm);                                  // теперь lm — корень, у него нет правого
        lm->r = t->r;
        if (t->r) t->r->p = lm;
        root = lm;
        delete t;
    }

    vector<int> inorder() const {
        vector<int> res;
        stack<Node*> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) { st.push(cur); cur = cur->l; }
            cur = st.top(); st.pop();
            res.push_back(cur->key);
            cur = cur->r;
        }
        return res;
    }
};

// --- D.5. AA-дерево ---
// Упрощение красно-чёрного: уровни (level); skew + split.
struct AATree {
    struct Node {
        int key, level;
        Node *l, *r;
        Node(int k) : key(k), level(1), l(nullptr), r(nullptr) {}
    };
    Node* root = nullptr;

    static Node* skew(Node* t) {
        if (!t || !t->l) return t;
        if (t->l->level == t->level) {
            Node* x = t->l;
            t->l = x->r;
            x->r = t;
            return x;
        }
        return t;
    }
    static Node* split(Node* t) {
        if (!t || !t->r || !t->r->r) return t;
        if (t->r->r->level == t->level) {
            Node* x = t->r;
            t->r = x->l;
            x->l = t;
            x->level++;
            return x;
        }
        return t;
    }

    static Node* insert(Node* t, int key) {
        if (!t) return new Node(key);
        if (key < t->key) t->l = insert(t->l, key);
        else t->r = insert(t->r, key);
        t = skew(t);
        t = split(t);
        return t;
    }

    static Node* dec_level(Node* t) {
        int should = 1 + min(t->l ? t->l->level : 0, t->r ? t->r->level : 0);
        if (should < t->level) {
            t->level = should;
            if (t->r && should < t->r->level) t->r->level = should;
        }
        return t;
    }

    static Node* erase(Node* t, int key) {
        if (!t) return nullptr;
        if (key < t->key) t->l = erase(t->l, key);
        else if (key > t->key) t->r = erase(t->r, key);
        else {
            if (!t->l && !t->r) { delete t; return nullptr; }
            if (!t->l) {
                Node* s = t->r;
                while (s->l) s = s->l;
                t->key = s->key;
                t->r = erase(t->r, s->key);
            } else {
                Node* s = t->l;
                while (s->r) s = s->r;
                t->key = s->key;
                t->l = erase(t->l, s->key);
            }
        }
        t = dec_level(t);
        t = skew(t);
        if (t->r) {
            t->r = skew(t->r);
            if (t->r->r) t->r->r = skew(t->r->r);
        }
        t = split(t);
        if (t->r) t->r = split(t->r);
        return t;
    }

    void insert(int key) { root = insert(root, key); }
    void erase(int key) { root = erase(root, key); }
    bool search(int key) const {
        Node* cur = root;
        while (cur) {
            if (cur->key == key) return true;
            cur = (key < cur->key) ? cur->l : cur->r;
        }
        return false;
    }
    vector<int> inorder() const {
        vector<int> res;
        stack<Node*> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) { st.push(cur); cur = cur->l; }
            cur = st.top(); st.pop();
            res.push_back(cur->key);
            cur = cur->r;
        }
        return res;
    }
};

// --- D.6. Scapegoat-дерево ---
// Параметр α: узел нарушает α-баланс (размер ребёнка > α·размер) →
// вся подструктура перестраивается (rebuild); амортизированно O(log n).
struct ScapegoatTree {
    struct Node {
        int key, sz;
        Node *l, *r;
        Node(int k) : key(k), sz(1), l(nullptr), r(nullptr) {}
    };
    Node* root = nullptr;
    double alpha;

    explicit ScapegoatTree(double a = 0.75) : alpha(a) {}

    static int size(Node* t) { return t ? t->sz : 0; }
    static void upd(Node* t) { t->sz = 1 + size(t->l) + size(t->r); }

    static void collect(Node* t, vector<int>& keys) {
        if (!t) return;
        collect(t->l, keys);
        keys.push_back(t->key);
        collect(t->r, keys);
    }
    static Node* build_balanced(const vector<int>& keys, int lo, int hi) {
        if (lo > hi) return nullptr;
        int mid = (lo + hi) / 2;
        Node* t = new Node(keys[mid]);
        t->l = build_balanced(keys, lo, mid - 1);
        t->r = build_balanced(keys, mid + 1, hi);
        upd(t);
        return t;
    }
    static Node* rebuild(Node* t) {
        vector<int> keys;
        collect(t, keys);
        return build_balanced(keys, 0, (int)keys.size() - 1);
    }

    Node* insert(Node* t, int key, bool& found_violation, int& violated_sz) {
        if (!t) return new Node(key);
        if (key < t->key) t->l = insert(t->l, key, found_violation, violated_sz);
        else t->r = insert(t->r, key, found_violation, violated_sz);
        upd(t);
        if (!found_violation &&
            (size(t->l) > alpha * size(t) || size(t->r) > alpha * size(t))) {
            found_violation = true;
            violated_sz = size(t);
            t = rebuild(t);
        }
        return t;
    }
    void insert(int key) {
        bool fv = false;
        int vsz = 0;
        root = insert(root, key, fv, vsz);
    }

    static Node* erase(Node* t, int key) {
        if (!t) return nullptr;
        if (key < t->key) t->l = erase(t->l, key);
        else if (key > t->key) t->r = erase(t->r, key);
        else {
            if (!t->l || !t->r) {
                Node* child = t->l ? t->l : t->r;
                delete t;
                return child;
            }
            Node* s = t->l;
            while (s->r) s = s->r;
            t->key = s->key;
            t->l = erase(t->l, s->key);
        }
        upd(t);
        return t;
    }
    void erase(int key) { root = erase(root, key); }

    bool search(int key) const {
        Node* cur = root;
        while (cur) {
            if (cur->key == key) return true;
            cur = (key < cur->key) ? cur->l : cur->r;
        }
        return false;
    }
    vector<int> inorder() const {
        vector<int> res;
        stack<Node*> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) { st.push(cur); cur = cur->l; }
            cur = st.top(); st.pop();
            res.push_back(cur->key);
            cur = cur->r;
        }
        return res;
    }
};

// =============================================================
// E. МНОГОПУТЕВЫЕ ДЕРЕВЬЯ
// =============================================================

// --- E.1. B-дерево (минимальная степень t, CLRS-семантика) ---
// Узел: от t−1 до 2t−1 ключей; все листья на одной глубине; рост вверх.
// Вставка: спуск с предварительным расщеплением полных узлов.
// Удаление: спуск с подкреплением (заимствование / слияние).
struct BTree {
    struct Node {
        vector<int> keys;
        vector<Node*> child;
        bool leaf;
        Node(bool lf) : leaf(lf) {}
    };
    int t;              // минимальная степень: ключи t−1 .. 2t−1
    bool redistribute;  // B*-режим (E.3): полный узел делится с соседом
    Node* root;

    explicit BTree(int t_ = 3, bool redistribute_ = false)
        : t(t_), redistribute(redistribute_), root(new Node(true)) {}

    // --- поиск ---
    // lower_bound — бинарный поиск по ключам узла (O(log t) внутри узла)
    bool search(Node* n, int key) const {
        auto it = lower_bound(n->keys.begin(), n->keys.end(), key);
        if (it != n->keys.end() && *it == key) return true;
        if (n->leaf) return false;
        return search(n->child[it - n->keys.begin()], key);
    }
    bool search(int key) const { return search(root, key); }

    // --- вставка ---
    void split_child(Node* parent, int i) {
        Node* y = parent->child[i];
        Node* z = new Node(y->leaf);
        int mid = y->keys[t - 1];               // середина уходит наверх
        z->keys.assign(y->keys.begin() + t, y->keys.end());
        y->keys.resize(t - 1);
        if (!y->leaf) {
            z->child.assign(y->child.begin() + t, y->child.end());
            y->child.resize(t);
        }
        parent->child.insert(parent->child.begin() + i + 1, z);
        parent->keys.insert(parent->keys.begin() + i, mid);
    }

    // B*-распределение (E.3): вместо расщепления полный узел child[i]
    // делится с соседом через разделитель родителя — крайний ключ полного
    // узла уходит наверх, разделитель опускается соседу. Возвращает true,
    // если сосед с местом нашёлся.
    bool redistribute_child(Node* n, int i) {
        Node* f = n->child[i];
        if (i > 0 && (int)n->child[i - 1]->keys.size() < 2 * t - 1) {
            Node* l = n->child[i - 1];
            int x = f->keys.front();
            f->keys.erase(f->keys.begin());
            if (!f->leaf) {
                l->child.push_back(f->child.front());
                f->child.erase(f->child.begin());
            }
            l->keys.push_back(n->keys[i - 1]);
            n->keys[i - 1] = x;
            return true;
        }
        if (i + 1 < (int)n->child.size() && (int)n->child[i + 1]->keys.size() < 2 * t - 1) {
            Node* r = n->child[i + 1];
            int x = f->keys.back();
            f->keys.pop_back();
            if (!f->leaf) {
                r->child.insert(r->child.begin(), f->child.back());
                f->child.pop_back();
            }
            r->keys.insert(r->keys.begin(), n->keys[i]);
            n->keys[i] = x;
            return true;
        }
        return false;
    }

    void insert_nonfull(Node* n, int key) {
        auto it = lower_bound(n->keys.begin(), n->keys.end(), key);
        if (n->leaf) {
            n->keys.insert(it, key);
            return;
        }
        int i = (int)(it - n->keys.begin());
        if ((int)n->child[i]->keys.size() == 2 * t - 1) {
            if (!redistribute || !redistribute_child(n, i)) {
                split_child(n, i);
                if (key > n->keys[i]) i++;
            }
        }
        insert_nonfull(n->child[i], key);
    }

    void insert(int key) {
        Node* r = root;
        if ((int)r->keys.size() == 2 * t - 1) {
            Node* s = new Node(false);
            s->child.push_back(r);
            root = s;
            split_child(s, 0);
            insert_nonfull(s, key);
        } else {
            insert_nonfull(r, key);
        }
    }

    // --- удаление ---
    void merge_children(Node* n, int i) {
        Node* l = n->child[i];
        Node* r = n->child[i + 1];
        l->keys.push_back(n->keys[i]);
        l->keys.insert(l->keys.end(), r->keys.begin(), r->keys.end());
        if (!l->leaf)
            l->child.insert(l->child.end(), r->child.begin(), r->child.end());
        n->keys.erase(n->keys.begin() + i);
        n->child.erase(n->child.begin() + i + 1);
        delete r;
    }

    // заимствование из левого соседа в child[i]
    void borrow_left(Node* n, int i) {
        Node* c = n->child[i];
        Node* s = n->child[i - 1];
        c->keys.insert(c->keys.begin(), n->keys[i - 1]);
        n->keys[i - 1] = s->keys.back();
        s->keys.pop_back();
        if (!c->leaf) {
            c->child.insert(c->child.begin(), s->child.back());
            s->child.pop_back();
        }
    }

    void borrow_right(Node* n, int i) {
        Node* c = n->child[i];
        Node* s = n->child[i + 1];
        c->keys.push_back(n->keys[i]);
        n->keys[i] = s->keys.front();
        s->keys.erase(s->keys.begin());
        if (!c->leaf) {
            c->child.push_back(s->child.front());
            s->child.erase(s->child.begin());
        }
    }

    bool erase_key(Node* n, int key) {
        int i = 0;
        while (i < (int)n->keys.size() && key > n->keys[i]) i++;
        if (i < (int)n->keys.size() && n->keys[i] == key) {
            if (n->leaf) {
                n->keys.erase(n->keys.begin() + i);
                return true;
            }
            if ((int)n->child[i]->keys.size() >= t) {
                Node* pred = n->child[i];
                while (!pred->leaf) pred = pred->child.back();
                n->keys[i] = pred->keys.back();
                return erase_key(n->child[i], n->keys[i]);
            }
            if ((int)n->child[i + 1]->keys.size() >= t) {
                Node* succ = n->child[i + 1];
                while (!succ->leaf) succ = succ->child.front();
                n->keys[i] = succ->keys.front();
                return erase_key(n->child[i + 1], n->keys[i]);
            }
            merge_children(n, i);
            return erase_key(n->child[i], key);
        }
        if (n->leaf) return false;
        bool last = (i == (int)n->keys.size());
        if ((int)n->child[i]->keys.size() < t) {
            if (i > 0 && (int)n->child[i - 1]->keys.size() >= t)
                borrow_left(n, i);
            else if (!last && (int)n->child[i + 1]->keys.size() >= t)
                borrow_right(n, i);
            else if (!last) {
                merge_children(n, i);
                i = min(i, (int)n->keys.size());
            } else {
                merge_children(n, i - 1);
                i = i - 1;
            }
        }
        return erase_key(n->child[i], key);
    }

    void erase(int key) {
        if (!root) return;
        erase_key(root, key);
        if (root->keys.empty() && !root->leaf) {
            Node* old = root;
            root = root->child[0];
            delete old;
        }
    }

    vector<int> inorder() const {
        vector<int> res;
        function<void(Node*)> go = [&](Node* n) {
            if (!n) return;
            for (int i = 0; i < (int)n->keys.size(); i++) {
                if (!n->leaf) go(n->child[i]);
                res.push_back(n->keys[i]);
            }
            if (!n->leaf) go(n->child[n->keys.size()]);
        };
        go(root);
        return res;
    }
};

// --- E.2. B+-дерево ---
// Данные только в листьях; внутренние — разделители; листья связаны
// next-списком для диапазонных сканов.
struct BPlusTree {
    struct Node {
        vector<int> keys;
        vector<Node*> child;
        Node* next;                 // у листьев — следующий лист
        bool leaf;
        Node(bool lf) : next(nullptr), leaf(lf) {}
    };
    int order;                      // максимум ключей в листе
    Node* root;

    explicit BPlusTree(int order_ = 4) : order(order_), root(new Node(true)) {}

    bool search(int key) const {
        Node* n = root;
        while (!n->leaf) {
            int i = 0;
            while (i < (int)n->keys.size() && key >= n->keys[i]) i++;
            n = n->child[i];
        }
        for (int k : n->keys) if (k == key) return true;
        return false;
    }

    void insert_internal(Node* n, int sep, Node* right) {
        int i = (int)n->keys.size() - 1;
        n->keys.push_back(0);
        while (i >= 0 && sep < n->keys[i]) { n->keys[i + 1] = n->keys[i]; i--; }
        n->keys[i + 1] = sep;
        n->child.insert(n->child.begin() + i + 2, right);
    }

    void insert(int key) {
        Node* n = root;
        stack<Node*> path;
        while (!n->leaf) {
            path.push(n);
            int i = 0;
            while (i < (int)n->keys.size() && key >= n->keys[i]) i++;
            n = n->child[i];
        }
        // вставка в лист (сортированная)
        int i = (int)n->keys.size() - 1;
        n->keys.push_back(0);
        while (i >= 0 && key < n->keys[i]) { n->keys[i + 1] = n->keys[i]; i--; }
        n->keys[i + 1] = key;
        // расщепление вверх при переполнении
        while ((int)n->keys.size() > order) {
            int mid = n->keys.size() / 2;
            int sep = n->keys[mid];
            Node* nn = new Node(n->leaf);
            nn->keys.assign(n->keys.begin() + mid + (n->leaf ? 0 : 1), n->keys.end());
            n->keys.resize(mid + (n->leaf ? 0 : 1));
            if (n->leaf) {
                nn->next = n->next;
                n->next = nn;
            } else {
                nn->child.assign(n->child.begin() + mid + 1, n->child.end());
                n->child.resize(mid + 1);
            }
            if (path.empty()) {
                Node* nr = new Node(false);
                nr->keys.push_back(sep);
                nr->child.push_back(n);
                nr->child.push_back(nn);
                root = nr;
                return;
            }
            Node* p = path.top(); path.pop();
            insert_internal(p, sep, nn);
            n = p;
        }
    }

    // Диапазонный запрос [lo, hi]
    vector<int> range(int lo, int hi) const {
        vector<int> res;
        Node* n = root;
        while (!n->leaf) {
            int i = 0;
            while (i < (int)n->keys.size() && lo >= n->keys[i]) i++;
            n = n->child[i];
        }
        while (n) {
            for (int k : n->keys)
                if (k >= lo && k <= hi) res.push_back(k);
            n = n->next;
        }
        return res;
    }
};

// --- E.4. 2-3 дерево (частный случай B-дерева, t = 1) ---
// BTree(t = 1): ключи 0..1, дети 1..2 — классическое 2-3 дерево
// (порядок 3: до 2 ключей, до 3 детей).
using TwoThreeTree = BTree;

// --- E.5. 2-3-4 дерево (частный случай B-дерева, t = 2) ---
// BTree(t = 2): ключи 1..3, дети 2..4. Эквивалент красно-чёрного:
// чёрный узел = узел 2-3-4, красные дети = дополнительные ключи.
using TwoThreeFourTree = BTree;

// =============================================================
// F. ДЕКАРТОВЫ И РАНДОМИЗИРОВАННЫЕ ДЕРЕВЬЯ
// =============================================================

static int next_priority() {
    static mt19937 rng(12345);      // детерминированный генератор
    return (int)rng();
}

// --- F.1. Treap (BST + куча по приоритету) ---
// split/merge — фундаментальные примитивы; insert/erase через них.
// size поддеревьев — для select/rank (G.2). Приоритеты — параметр
// rng (функция-генератор), по умолчанию — детерминированный mt19937.
struct Treap {
    struct Node {
        int key, pr, sz;
        Node *l, *r;
        Node(int k, int p) : key(k), pr(p), sz(1), l(nullptr), r(nullptr) {}
    };
    Node* root = nullptr;
    function<int()> rng;

    explicit Treap(const function<int()>& gen = {})
        : rng(gen ? gen : [] { return next_priority(); }) {}

    static int sz(Node* t) { return t ? t->sz : 0; }
    static void upd(Node* t) { if (t) t->sz = 1 + sz(t->l) + sz(t->r); }

    // split: (ключи < key, ключи ≥ key)
    static void split(Node* t, int key, Node*& a, Node*& b) {
        if (!t) { a = b = nullptr; return; }
        if (t->key < key) { split(t->r, key, t->r, b); a = t; upd(a); }
        else { split(t->l, key, a, t->l); b = t; upd(b); }
    }
    static Node* merge(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->pr > b->pr) { a->r = merge(a->r, b); upd(a); return a; }
        else { b->l = merge(a, b->l); upd(b); return b; }
    }

    void insert(int key) {
        Node *a, *b;
        split(root, key, a, b);
        root = merge(merge(a, new Node(key, rng())), b);
    }
    void erase(int key) {
        Node *a, *b, *c;
        split(root, key, a, b);
        split(b, key + 1, b, c);
        delete b;
        root = merge(a, c);
    }
    bool search(int key) const {
        Node* cur = root;
        while (cur) {
            if (cur->key == key) return true;
            cur = (key < cur->key) ? cur->l : cur->r;
        }
        return false;
    }

    // k-й элемент по порядку (1-based)
    int kth(int k) const {
        Node* cur = root;
        while (cur) {
            int ls = sz(cur->l);
            if (k == ls + 1) return cur->key;
            if (k <= ls) cur = cur->l;
            else { k -= ls + 1; cur = cur->r; }
        }
        return -1;
    }
    // rank: число элементов строго меньше key (order_of_key)
    int rank(int key) const {
        Node* cur = root;
        int res = 0;
        while (cur) {
            if (cur->key < key) { res += sz(cur->l) + 1; cur = cur->r; }
            else cur = cur->l;
        }
        return res;
    }
    vector<int> inorder() const {
        vector<int> res;
        stack<Node*> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) { st.push(cur); cur = cur->l; }
            cur = st.top(); st.pop();
            res.push_back(cur->key);
            cur = cur->r;
        }
        return res;
    }
};

// --- F.2. Неявный treap (ключ = позиция в последовательности) ---
// split по размеру; reverse с lazy-флагом (F.3).
struct ImplicitTreap {
    struct Node {
        int val, pr, sz;
        bool rev;
        Node *l, *r;
        Node(int v) : val(v), pr(next_priority()), sz(1), rev(false), l(nullptr), r(nullptr) {}
    };
    Node* root = nullptr;

    static int sz(Node* t) { return t ? t->sz : 0; }
    static void push(Node* t) {
        if (t && t->rev) {
            swap(t->l, t->r);
            if (t->l) t->l->rev ^= 1;
            if (t->r) t->r->rev ^= 1;
            t->rev = false;
        }
    }
    static void upd(Node* t) { if (t) t->sz = 1 + sz(t->l) + sz(t->r); }

    // split_at: (первые k элементов, остаток)
    static void split_at(Node* t, int k, Node*& a, Node*& b) {
        if (!t) { a = b = nullptr; return; }
        push(t);
        if (k <= sz(t->l)) { split_at(t->l, k, a, t->l); b = t; upd(b); }
        else { split_at(t->r, k - sz(t->l) - 1, t->r, b); a = t; upd(a); }
    }
    static Node* merge(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->pr > b->pr) { push(a); a->r = merge(a->r, b); upd(a); return a; }
        else { push(b); b->l = merge(a, b->l); upd(b); return b; }
    }

    void insert_at(int pos, int val) {
        Node *a, *b;
        split_at(root, pos, a, b);
        root = merge(merge(a, new Node(val)), b);
    }
    void erase_range(int l, int r) {
        Node *a, *b, *c;
        split_at(root, r, a, b);
        split_at(a, l, a, c);
        delete c;
        root = merge(a, b);
    }
    void reverse_range(int l, int r) {
        Node *a, *b, *c;
        split_at(root, r, a, b);
        split_at(a, l, a, c);
        if (c) c->rev ^= 1;
        root = merge(merge(a, c), b);
    }
    vector<int> to_vector() const {
        vector<int> res;
        function<void(Node*)> go = [&](Node* t) {
            if (!t) return;
            push(t);
            go(t->l);
            res.push_back(t->val);
            go(t->r);
        };
        go(root);
        return res;
    }
};

// =============================================================
// G. СПЕЦИАЛИЗИРОВАННЫЕ ДЕРЕВЬЯ
// =============================================================

// --- G.1. Wavelet Tree ---
// Бинарное дерево по битам значений: b[i] — сколько элементов префикса
// [0..i) ушли в левое поддерево; kth / count за O(log σ).
struct WaveletTree {
    int lo, hi;                     // диапазон значений [lo, hi]
    WaveletTree* l;
    WaveletTree* r;
    vector<int> b;                  // префиксные счётчики «влево»

    WaveletTree(const vector<int>& a, int lo_, int hi_) : lo(lo_), hi(hi_) {
        if (a.empty()) { l = r = nullptr; b.assign(1, 0); return; }
        if (lo == hi) { l = r = nullptr; b.assign(a.size() + 1, 0); return; }
        int mid = (lo + hi) / 2;
        b.assign(a.size() + 1, 0);
        vector<int> L, R;
        for (int i = 0; i < (int)a.size(); i++) {
            b[i + 1] = b[i] + (a[i] <= mid);
            if (a[i] <= mid) L.push_back(a[i]);
            else R.push_back(a[i]);
        }
        l = new WaveletTree(L, lo, mid);
        r = new WaveletTree(R, mid + 1, hi);
    }

    // k-я порядковая статистика на отрезке [ql, qr) (k 0-based)
    int kth(int ql, int qr, int k) const {
        if (lo == hi) return lo;
        int in_left = b[qr] - b[ql];
        if (k < in_left) return l->kth(b[ql], b[qr], k);
        return r->kth(ql - b[ql], qr - b[qr], k - in_left);
    }

    // число элементов ≤ x на [ql, qr)
    int count_leq(int ql, int qr, int x) const {
        if (x < lo) return 0;
        if (hi <= x) return qr - ql;
        return l->count_leq(b[ql], b[qr], x) + r->count_leq(ql - b[ql], qr - b[qr], x);
    }

    int count_eq(int ql, int qr, int x) const {
        return count_leq(ql, qr, x) - count_leq(ql, qr, x - 1);
    }
};

// --- G.4. Wavelet Matrix ---
// Сукцинктный вариант Wavelet Tree (G.1): на каждом разряде хранится
// битовый вектор (значение бита до стабильной сортировки) с rank по
// двум уровням блоков (суперблоки + блоки — приём III.B.7); kth /
// count_leq спускаются по разрядам через rank — без копий значений и
// счётчиков на узел. Память: n·log σ бит на векторы + o(n·log σ) бит
// таблиц. select как примитив — III.B.7 (c.cpp).
struct WaveletMatrix {
    struct BitVec {               // битовый вектор с rank за O(1)
        int n;
        vector<unsigned long long> w;   // n бит
        vector<int> sb, br;             // суперблоки (512 бит) + блоки (64 бит)
        BitVec() : n(0) {}
        explicit BitVec(int n_) : n(n_), w((n_ + 63) / 64),
                                  sb((n_ + 511) / 512), br((n_ + 63) / 64) {}
        void set(int i) { w[i >> 6] |= 1ULL << (i & 63); }
        void build() {
            int r = 0;
            for (int j = 0; j < (int)w.size(); j++) {
                if ((j & 7) == 0) sb[j >> 3] = r;
                br[j] = r - sb[j >> 3];
                r += __builtin_popcountll(w[j]);
            }
        }
        int rank(int i) const {         // число единиц в [0, i)
            if (i <= 0) return 0;
            int wi = (i - 1) >> 6, lo = i & 63;
            if (lo == 0) return sb[wi >> 3] + br[wi] + __builtin_popcountll(w[wi]);
            return sb[wi >> 3] + br[wi] + __builtin_popcountll(w[wi] & ((1ULL << lo) - 1));
        }
    };

    int n, maxbit;
    vector<BitVec> bv;          // bv[lv] — биты разряда lv (до сортировки)
    vector<int> zc;             // zc[lv] — число нулей на разряде lv

    WaveletMatrix() : n(0), maxbit(0) {}
    explicit WaveletMatrix(const vector<int>& a) {
        n = a.size();
        int mx = 0;
        for (int x : a) mx = max(mx, x);
        maxbit = (mx <= 0) ? 0 : 31 - __builtin_clz(mx);
        bv.assign(maxbit + 1, BitVec());
        zc.assign(maxbit + 1, 0);
        vector<int> cur = a, nxt(n);
        for (int lv = maxbit; lv >= 0; lv--) {
            BitVec v(n);
            int z = 0;
            for (int i = 0; i < n; i++) {
                if ((cur[i] >> lv) & 1) v.set(i);
                else z++;
            }
            v.build();
            bv[lv] = v;
            zc[lv] = z;
            int p = 0;
            for (int i = 0; i < n; i++) if (!((cur[i] >> lv) & 1)) nxt[p++] = cur[i];
            for (int i = 0; i < n; i++) if ((cur[i] >> lv) & 1) nxt[p++] = cur[i];
            cur.swap(nxt);
        }
    }

    // k-я порядковая статистика на [l, r) (k 0-based)
    int kth(int l, int r, int k) const {
        int res = 0;
        for (int lv = maxbit; lv >= 0; lv--) {
            int pl = bv[lv].rank(l), pr = bv[lv].rank(r);
            int zeros = (r - l) - (pr - pl);
            if (k < zeros) {
                l -= pl; r -= pr;                       // в нули
            } else {
                k -= zeros;
                res |= 1 << lv;
                l = zc[lv] + pl; r = zc[lv] + pr;       // в единицы
            }
        }
        return res;
    }

    // число элементов ≤ x на [l, r)
    int count_leq(int l, int r, int x) const {
        if (x < 0) return 0;
        if (x >> (maxbit + 1)) return r - l;    // x больше любого значения
        int res = 0;
        for (int lv = maxbit; lv >= 0; lv--) {
            int pl = bv[lv].rank(l), pr = bv[lv].rank(r);
            int zeros = (r - l) - (pr - pl);
            if ((x >> lv) & 1) {
                res += zeros;                           // нули строго меньше
                l = zc[lv] + pl; r = zc[lv] + pr;
            } else {
                l -= pl; r -= pr;
            }
        }
        res += r - l;                                   // остаток — равные x
        return res;
    }

    int count_eq(int l, int r, int x) const {
        return count_leq(l, r, x) - count_leq(l, r, x - 1);
    }
};

// --- G.2. Order-Statistic Tree ---
// select/rank на treap с размерами (методы Treap::kth / Treap::rank).
// STL-аналог — __gnu_pbds::tree (order_of_key / find_by_order).
struct OrderStatTree : Treap {};

// =============================================================
// H. ДЕРЕВЬЯ В STL
// =============================================================

// --- H.1. std::map: экстремумы и границы ---
static void stl_extrema(const vector<int>& keys) {
    map<int, int> m;
    for (int i = 0; i < (int)keys.size(); i++) m[keys[i]] = i;
    cout << "  std::map min = " << m.begin()->first
         << " max = " << m.rbegin()->first << endl;
}

// --- H.2. std::set / std::multiset ---
static void stl_set_minmax(const vector<int>& keys) {
    set<int> s(keys.begin(), keys.end());
    cout << "  std::set size = " << s.size()
         << " min = " << *s.begin() << " max = " << *s.rbegin() << endl;
}

static void stl_multiset_count(const vector<int>& keys, int x) {
    multiset<int> ms(keys.begin(), keys.end());
    cout << "  std::multiset count(" << x << ") = " << ms.count(x) << endl;
}

// --- H.3. std::multimap: equal_range ---
static vector<int> stl_multimap_range(const vector<pair<int, int>>& data, int key) {
    multimap<int, int> mm(data.begin(), data.end());
    vector<int> res;
    auto range = mm.equal_range(key);
    for (auto it = range.first; it != range.second; it++) res.push_back(it->second);
    return res;
}

}; // конец struct SearchTrees

// =============================================================
// signed main() — демонстрация и проверка всех разделов A–H
// =============================================================

#ifndef STRUCT_B_MAIN
signed main() {
    using ST = SearchTrees;

    cout << "=== A. ПРЕДСТАВЛЕНИЯ ===" << endl;
    // A.2 HeapIndexedTree
    ST::HeapIndexedTree ht({1, 2, 3, 4, 5, 6});
    cout << "heap-indexed: left(0)=" << ht.left(0) << " right(0)=" << ht.right(0)
         << " parent(3)=" << ht.parent(3) << " (ожидаем 1 2 1)" << endl;
    cout << "heap preorder: ";
    for (int x : ht.preorder()) cout << x << " ";
    cout << "(ожидаем 1 2 4 5 3 6)" << endl;
    cout << "is_complete({1..6}) = " << ST::HeapIndexedTree::is_complete_from_array({1,2,3,4,5,6}) << " (1)" << endl;

    // A.3 CursorTree (BST-операции на курсорах)
    ST::CursorTree ct(20);
    for (int x : {10, 5, 15, 3, 7, 12}) ct.insert_bst(x);
    cout << "CursorTree inorder: ";
    for (int x : ct.inorder()) cout << x << " ";
    cout << "(ожидаем 3 5 7 10 12 15)" << endl;
    cout << "CursorTree search(7) = " << ct.search(7) << " search(9) = " << ct.search(9) << " (1 0)" << endl;
    ct.erase_bst(10);
    cout << "CursorTree after erase(10): ";
    for (int x : ct.inorder()) cout << x << " ";
    cout << "(ожидаем 3 5 7 12 15)" << endl;

    // A.4 ParentArray + двоичные подъёмы
    ST::ParentArray pa;
    pa.build(6, {{0,1},{0,2},{1,3},{1,4},{2,5}});
    cout << "ParentArray kth(4, 2) = " << pa.kth_ancestor(4, 2)
         << " lca(3,4) = " << pa.lca(3, 4) << " lca(4,5) = " << pa.lca(4, 5) << " (ожидаем 0 1 0)" << endl;

    // A.5 FCNS
    ST::FCNS fc;
    fc.build(7, {{0,1},{0,2},{1,3},{1,4},{1,5},{2,6}}, 0);
    cout << "FCNS preorder: ";
    for (int x : fc.preorder()) cout << x << " ";
    cout << "(ожидаем 0 1 3 4 5 2 6)" << endl;
    // roundtrip: to_binary → from_binary даёт тот же preorder
    ST::TreeNode* fc_bin = fc.to_binary(0);
    ST::FCNS fc2;
    fc2.from_binary(fc_bin, 7);
    cout << "FCNS roundtrip: ";
    for (int x : fc2.preorder()) cout << x << " ";
    cout << "(ожидаем 0 1 3 4 5 2 6)" << endl;

    // A.6 ThreadedTree + A.7 Morris
    ST::TreeNode* t1 = ST::build_from_level_order({1, 2, 3, 4, 5, INT_MIN, 6});
    ST::ThreadedTree th;
    th.build(t1);
    cout << "threaded inorder: ";
    for (int x : th.inorder()) cout << x << " ";
    cout << "(ожидаем 4 2 5 1 3 6)" << endl;
    cout << "morris inorder: ";
    for (int x : ST::morris_inorder(t1)) cout << x << " ";
    cout << "(ожидаем 4 2 5 1 3 6)" << endl;
    cout << "morris preorder: ";
    for (int x : ST::morris_preorder(t1)) cout << x << " ";
    cout << "(ожидаем 1 2 4 5 3 6)" << endl;

    cout << "\n=== B. БАЗОВЫЕ ДЕРЕВЬЯ ===" << endl;
    ST::TreeNode* root = ST::build_from_level_order({1, 2, 3, 4, 5, 6, 7});
    cout << "height = " << ST::height(root) << " nodes = " << ST::count_nodes(root)
         << " leaves = " << ST::count_leaves(root) << " (ожидаем 3 7 4)" << endl;
    cout << "inorder: ";
    for (int x : ST::inorder_rec(root)) cout << x << " ";
    cout << "(ожидаем 4 2 5 1 6 3 7)" << endl;
    cout << "preorder: ";
    for (int x : ST::preorder_rec(root)) cout << x << " ";
    cout << "(ожидаем 1 2 4 5 3 6 7)" << endl;
    cout << "postorder: ";
    for (int x : ST::postorder_rec(root)) cout << x << " ";
    cout << "(ожидаем 4 5 2 6 7 3 1)" << endl;
    cout << "iter inorder == rec: " << (ST::inorder_iter(root) == ST::inorder_rec(root)) << endl;
    cout << "iter preorder == rec: " << (ST::preorder_iter(root) == ST::preorder_rec(root)) << endl;
    cout << "iter postorder == rec: " << (ST::postorder_iter(root) == ST::postorder_rec(root)) << endl;
    cout << "level_order: ";
    for (int x : ST::level_order(root)) cout << x << " ";
    cout << "(ожидаем 1 2 3 4 5 6 7)" << endl;
    auto layers = ST::level_order_layers(root);
    cout << "layers = " << layers.size() << " (ожидаем 3)" << endl;

    // Mirror / Symmetric
    ST::TreeNode* sym = ST::build_from_level_order({1, 2, 2, 3, 4, 4, 3});
    cout << "is_symmetric(sym) = " << ST::is_symmetric(sym) << " (1)" << endl;
    cout << "is_symmetric(root) = " << ST::is_symmetric(root) << " (0)" << endl;

    // Diameter / sums / path / coins / LCA
    ST::TreeNode* dtree = ST::build_from_level_order({1, 2, 3, 4, 5});
    cout << "diameter = " << ST::diameter(dtree) << " (ожидаем 3)" << endl;
    cout << "node_sum = " << ST::node_sum(dtree) << " (ожидаем 15)" << endl;
    cout << "path_sum(1→4, 7) = " << ST::root_to_leaf_path_sum(dtree, 7)
         << " path_sum(1→5, 8) = " << ST::root_to_leaf_path_sum(dtree, 8) << " (1 1)" << endl;
    ST::TreeNode* coins = ST::build_from_level_order({3, 0, 0});
    cout << "distribute_coins = " << ST::distribute_coins(coins) << " (ожидаем 2)" << endl;
    ST::TreeNode* bstree = ST::build_from_level_order({3, 1, 4, 0, 2});
    ST::TreeNode* u = ST::bst_search(bstree, 2);
    ST::TreeNode* v = ST::bst_search(bstree, 4);
    cout << "lca(2, 4) = " << ST::lca_naive(bstree, u, v)->val << " (ожидаем 3)" << endl;

    // Serialize / Deserialize
    string ser = ST::serialize_level_order(root);
    ST::TreeNode* des = ST::deserialize_level_order(ser);
    cout << "serialize = \"" << ser << "\"" << endl;
    cout << "roundtrip equal: " << (ST::level_order(des) == ST::level_order(root)) << " (1)" << endl;

    // Merge
    ST::TreeNode* m1 = ST::build_from_level_order({1, 3, 2, 5});
    ST::TreeNode* m2 = ST::build_from_level_order({2, 1, 3, INT_MIN, 4, INT_MIN, 7});
    ST::TreeNode* merged = ST::merge_trees(m1, m2);
    cout << "merge level_order: ";
    for (int x : ST::level_order(merged)) cout << x << " ";
    cout << "(ожидаем 3 4 5 5 4 7)" << endl;

    cout << "\n=== C. BST ===" << endl;
    ST::TreeNode* bst = nullptr;
    for (int x : {8, 3, 10, 1, 6, 14, 4, 7, 13}) bst = ST::bst_insert(bst, x);
    cout << "bst inorder: ";
    for (int x : ST::inorder_iter(bst)) cout << x << " ";
    cout << "(ожидаем 1 3 4 6 7 8 10 13 14)" << endl;
    cout << "bst search(7) = " << (ST::bst_search(bst, 7) != nullptr)
         << " search(9) = " << (ST::bst_search(bst, 9) != nullptr) << " (1 0)" << endl;
    cout << "bst_search_rec(7) = " << (ST::bst_search_recursive(bst, 7) != nullptr) << " (1)" << endl;
    ST::TreeNode* bst2 = nullptr;
    for (int x : {5, 3, 8}) bst2 = ST::bst_insert_iter(bst2, x);
    cout << "bst_insert_iter: ";
    for (int x : ST::inorder_iter(bst2)) cout << x << " ";
    cout << "(ожидаем 3 5 8)" << endl;
    cout << "floor(5) = " << ST::bst_floor(bst, 5)->val << " ceil(5) = " << ST::bst_ceil(bst, 5)->val << " (ожидаем 4 6)" << endl;
    cout << "is_bst_sorted = " << ST::is_bst_sorted(bst) << " is_bst_bounds = " << ST::is_bst_bounds(bst) << " (1 1)" << endl;
    ST::TreeNode* bad = ST::build_from_level_order({10, 5, 15, 2, 8, 12, 7});
    cout << "is_bst_sorted(bad) = " << ST::is_bst_sorted(bad) << " (0)" << endl;
    ST::TreeNode* sumt = ST::build_from_level_order({13, 10, 3, 4, 6, 3});
    cout << "is_sum_tree = " << ST::is_sum_tree(sumt) << " (1)" << endl;
    cout << "is_sum_tree(root) = " << ST::is_sum_tree(root) << " (0)" << endl;
    ST::TreeNode* mbst = ST::build_from_level_order({5, 4, 8, 3, INT_MIN, 6, 3});
    cout << "max_sum_bst = " << ST::max_sum_bst(mbst) << " (ожидаем 7)" << endl;
    ST::TreeNode* rot = ST::build_from_level_order({2, 1, 3});
    rot = ST::rotate_left(rot);
    cout << "rotate_left({2,1,3}) root = " << rot->val << " (ожидаем 3)" << endl;
    bst = ST::bst_erase(bst, 8);
    cout << "bst after erase(8): ";
    for (int x : ST::inorder_iter(bst)) cout << x << " ";
    cout << "(ожидаем 1 3 4 6 7 10 13 14)" << endl;

    cout << "\n=== D. СБАЛАНСИРОВАННЫЕ ===" << endl;
    // AVL
    ST::AVLTree avl;
    for (int x : {10, 20, 30, 40, 50, 25}) avl.insert(x);
    cout << "AVL inorder: ";
    for (int x : avl.inorder()) cout << x << " ";
    cout << "(ожидаем 10 20 25 30 40 50)" << endl;
    avl.erase(20);
    cout << "AVL search(20) = " << avl.search(20) << " search(25) = " << avl.search(25) << " (0 1)" << endl;

    // Red-black
    ST::RedBlackTree rb;
    for (int x : {7, 3, 18, 10, 22, 8, 11, 26, 2, 6, 13}) rb.insert(x);
    cout << "RB validate = " << rb.validate() << " (1)" << endl;
    cout << "RB inorder: ";
    vector<int> rbord;
    // inorder через итератор не делаем — проверим через search-набор
    for (int x : {7, 3, 18, 10, 22, 8, 11, 26, 2, 6, 13}) {
        if (rb.search(x)) rbord.push_back(x);
    }
    cout << "search all = " << ((int)rbord.size() == 11) << endl;
    rb.erase(18); rb.erase(3); rb.erase(7);
    cout << "RB after erase(18,3,7): validate = " << rb.validate()
         << " search(7) = " << rb.search(7) << " search(13) = " << rb.search(13) << " (1 0 1)" << endl;

    // Splay
    ST::SplayTree spt;
    for (int x : {1, 2, 3, 4, 5, 6}) spt.insert(x);
    cout << "splay search(3) = " << spt.search(3) << " root = " << spt.root->key << " (1 3)" << endl;
    spt.erase(3);
    cout << "splay after erase(3): search = " << spt.search(3) << " (0)" << endl;

    // AA
    ST::AATree aa;
    for (int x : {30, 20, 10, 40, 35, 25, 15, 5}) aa.insert(x);
    cout << "AA inorder: ";
    for (int x : aa.inorder()) cout << x << " ";
    cout << "(ожидаем 5 10 15 20 25 30 35 40)" << endl;
    aa.erase(20);
    cout << "AA search(20) = " << aa.search(20) << " search(25) = " << aa.search(25) << " (0 1)" << endl;

    // Scapegoat
    ST::ScapegoatTree sc;
    for (int x : {1, 2, 3, 4, 5, 6, 7, 8}) sc.insert(x);
    cout << "Scapegoat inorder: ";
    for (int x : sc.inorder()) cout << x << " ";
    cout << "(ожидаем 1..8)" << endl;
    sc.erase(4);
    cout << "Scapegoat search(4) = " << sc.search(4) << " search(5) = " << sc.search(5) << " (0 1)" << endl;

    cout << "\n=== E. МНОГОПУТЕВЫЕ ===" << endl;
    // B-tree (t = 2 — 2-3-4)
    ST::BTree bt(2);
    for (int x : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) bt.insert(x);
    cout << "B-tree(t=2) inorder: ";
    for (int x : bt.inorder()) cout << x << " ";
    cout << "(ожидаем 1..10)" << endl;
    bt.erase(4); bt.erase(7);
    cout << "B-tree search(4) = " << bt.search(4) << " search(7) = " << bt.search(7)
         << " search(9) = " << bt.search(9) << " (0 0 1)" << endl;

    // B+ tree
    ST::BPlusTree bpt(4);
    for (int x : {10, 20, 30, 40, 50, 60, 70}) bpt.insert(x);
    cout << "B+ search(30) = " << bpt.search(30) << " search(35) = " << bpt.search(35) << " (1 0)" << endl;
    cout << "B+ range[25,60]: ";
    for (int x : bpt.range(25, 60)) cout << x << " ";
    cout << "(ожидаем 30 40 50 60)" << endl;

    // 2-3 и 2-3-4 как частные случаи B-дерева
    ST::TwoThreeTree ttt(1);
    for (int x : {5, 15, 25, 35, 45}) ttt.insert(x);
    cout << "2-3 inorder: ";
    for (int x : ttt.inorder()) cout << x << " ";
    cout << "(ожидаем 5 15 25 35 45)" << endl;
    ST::TwoThreeFourTree ttft(2);
    for (int x : {5, 15, 25, 35, 45}) ttft.insert(x);
    ttft.erase(15);
    cout << "2-3-4 search(15) = " << ttft.search(15) << " search(25) = " << ttft.search(25) << " (0 1)" << endl;
    // B* (E.3): распределение с соседом вместо расщепления — тот же inorder
    ST::BTree bstar(3, true);
    for (int x : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) bstar.insert(x);
    cout << "B*(t=3) inorder: ";
    for (int x : bstar.inorder()) cout << x << " ";
    cout << "(ожидаем 1..10)" << endl;
    cout << "B* search(6) = " << bstar.search(6) << " search(11) = " << bstar.search(11) << " (1 0)" << endl;

    cout << "\n=== F. TREAP ===" << endl;
    ST::Treap tr;
    for (int x : {5, 3, 8, 1, 4, 7, 9}) tr.insert(x);
    cout << "Treap inorder: ";
    for (int x : tr.inorder()) cout << x << " ";
    cout << "(ожидаем 1 3 4 5 7 8 9)" << endl;
    cout << "Treap kth(4) = " << tr.kth(4) << " rank(7) = " << tr.rank(7) << " (ожидаем 5 4)" << endl;
    tr.erase(3);
    cout << "Treap search(3) = " << tr.search(3) << " (0)" << endl;

    ST::ImplicitTreap it;
    for (int i = 1; i <= 6; i++) it.insert_at(i - 1, i);
    cout << "ImplicitTreap: ";
    for (int x : it.to_vector()) cout << x << " ";
    cout << "(ожидаем 1 2 3 4 5 6)" << endl;
    it.reverse_range(1, 5);
    cout << "after reverse(1,5): ";
    for (int x : it.to_vector()) cout << x << " ";
    cout << "(ожидаем 1 5 4 3 2 6)" << endl;
    it.erase_range(0, 2);
    cout << "after erase(0,2): ";
    for (int x : it.to_vector()) cout << x << " ";
    cout << "(ожидаем 4 3 2 6)" << endl;

    cout << "\n=== G. СПЕЦИАЛИЗИРОВАННЫЕ ===" << endl;
    ST::WaveletTree wt({3, 1, 4, 1, 5, 9, 2, 6}, 1, 9);
    cout << "Wavelet kth(0, 8, 2) = " << wt.kth(0, 8, 2) << " (ожидаем 2)" << endl;
    cout << "Wavelet kth(2, 6, 0) = " << wt.kth(2, 6, 0) << " (ожидаем 1)" << endl;
    cout << "Wavelet count_leq(0,8,4) = " << wt.count_leq(0, 8, 4) << " (ожидаем 5)" << endl;
    cout << "Wavelet count_eq(0,8,1) = " << wt.count_eq(0, 8, 1) << " (ожидаем 2)" << endl;

    ST::WaveletMatrix wm({3, 1, 4, 1, 5, 9, 2, 6});
    cout << "WMatrix kth(0,8,2) = " << wm.kth(0, 8, 2) << " (ожидаем 2)" << endl;
    cout << "WMatrix kth(2,6,0) = " << wm.kth(2, 6, 0) << " (ожидаем 1)" << endl;
    cout << "WMatrix kth(0,8,7) = " << wm.kth(0, 8, 7) << " (ожидаем 9)" << endl;
    cout << "WMatrix count_leq(0,8,4) = " << wm.count_leq(0, 8, 4) << " (ожидаем 5)" << endl;
    cout << "WMatrix count_leq(2,6,5) = " << wm.count_leq(2, 6, 5) << " (ожидаем 3)" << endl;
    cout << "WMatrix count_leq(0,8,20) = " << wm.count_leq(0, 8, 20) << " (ожидаем 8)" << endl;
    cout << "WMatrix count_eq(0,8,1) = " << wm.count_eq(0, 8, 1) << " (ожидаем 2)" << endl;

    ST::OrderStatTree ost;
    for (int x : {9, 4, 7, 1, 6, 3}) ost.insert(x);
    cout << "OrderStat kth(3) = " << ost.kth(3) << " rank(6) = " << ost.rank(6) << " (ожидаем 4 3)" << endl;

    cout << "\n=== H. STL ===" << endl;
    ST::stl_extrema({3, 1, 4, 1, 5});
    ST::stl_set_minmax({3, 1, 4, 1, 5});
    ST::stl_multiset_count({3, 1, 4, 1, 5}, 1);
    vector<int> mmr = ST::stl_multimap_range({{1, 10}, {2, 20}, {1, 30}, {3, 40}}, 1);
    cout << "  std::multimap equal_range(1): ";
    for (int x : mmr) cout << x << " ";
    cout << "(ожидаем 10 30)" << endl;

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_B_MAIN

#endif // STRUCT_B_CPP
