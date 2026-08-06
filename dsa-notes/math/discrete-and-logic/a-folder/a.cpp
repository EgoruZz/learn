#ifndef DISCRETE_LOGIC_A_CPP
#define DISCRETE_LOGIC_A_CPP

#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <algorithm>
using namespace std;

// =============================================================
// A. МНОЖЕСТВА И ОТНОШЕНИЯ
// =============================================================
// Структура md: A. Основы теории множеств (1. Множества, 2. Операции,
//               3. Алгебра, 4. Мощность, 5. Включения-исключения,
//               6. Булеан и SOS DP) → B. Отношения на множествах
//               (1. Декартово произведение, 2. Бинарные отношения,
//               3. Операции, 4. Свойства, 5. Эквивалентность,
//               6. Порядок, 7. Замыкания, 8. Соответствия и функции)
//
// Базовый класс ветки discrete-and-logic: не наследует ничего — теория
// множеств является фундаментом, из которого строится остальное
// (b.cpp — логика, c.cpp — булевы функции, d.cpp — автоматы). Все
// методы реализованы для произвольных n, m: констант в алгоритмах нет.
//
// Соглашение о представлении:
//   * множество A ⊆ U, |U| ≤ 63 — битовая маска unsigned long long:
//     бит x = 1 ⟺ x ∈ A;
//   * отношение R ⊆ A × B (|A| = n, |B| = m) — булева матрица
//     vector<vector<int>> размера n×m: R[i][j] = 1 ⟺ (aᵢ, bⱼ) ∈ R;
//   * функция f: [0, n) → [0, m) — vector<int> размера n: f[i] = образ i.
//
// Содержит:
//   A. Множества: set_belongs, is_subset, set_equal, set_union,
//      set_intersection, set_difference, set_symmetric_difference,
//      set_complement, set_cardinality, power_set_size, all_subsets,
//      submasks_of, inc_excl_union, inc_excl_exact,
//      zeta_transform, mobius_transform, superset_zeta_transform,
//      superset_mobius_transform
//   B. Отношения: cartesian_product, relation_from_pairs,
//      relation_union, relation_intersection, relation_difference,
//      relation_complement, relation_inverse, relation_compose,
//      relation_power, is_reflexive, is_irreflexive, is_symmetric,
//      is_antisymmetric, is_transitive, is_equivalence,
//      is_partial_order, is_total_order, equivalence_classes,
//      reflexive_closure, symmetric_closure, transitive_closure,
//      topological_sort, count_topological_orders, is_function,
//      is_injective, is_surjective, is_bijective, function_inverse,
//      function_compose, function_cycle_length, functional_graph_analyze

struct SetsAndRelations {

// =============================================================
// A. ОСНОВЫ ТЕОРИИ МНОЖЕСТВ
// =============================================================

// --- A.1. Принадлежность и подмножества (битовые маски) ---
// Множество A ⊆ U кодируется маской: бит x = 1 ⟺ x ∈ A.
// |U| ≤ 63, иначе 1ULL << x выходит за разрядную сетку.
bool set_belongs(unsigned long long mask, int x) {
    return (mask >> x) & 1ULL;
}

// A ⊆ B: каждый бит A есть в B.
bool is_subset(unsigned long long a, unsigned long long b) {
    return (a & b) == a;
}

// A = B ⟺ A ⊆ B и B ⊆ A — проверка по маскам тривиальна.
bool set_equal(unsigned long long a, unsigned long long b) {
    return a == b;
}

// --- A.2. Операции над множествами ---
// Каждая операция — одна инструкция процессора.
unsigned long long set_union(unsigned long long a, unsigned long long b) {
    return a | b;
}

unsigned long long set_intersection(unsigned long long a, unsigned long long b) {
    return a & b;
}

// A \ B: бит остаётся, если он есть в A и нет в B.
unsigned long long set_difference(unsigned long long a, unsigned long long b) {
    return a & ~b;
}

// A △ B = (A \ B) ∪ (B \ A) — исключающее ИЛИ принадлежностей.
unsigned long long set_symmetric_difference(unsigned long long a, unsigned long long b) {
    return a ^ b;
}

// A̅ = U \ A: n — мощность универсума U = {0, 1, ..., n−1}, n ≤ 63.
unsigned long long set_complement(unsigned long long a, int n) {
    return (~a) & ((1ULL << n) - 1);
}

// --- A.4. Мощность множества ---
// |A| = число единичных битов маски (popcount).
int set_cardinality(unsigned long long mask) {
    return __builtin_popcountll(mask);
}

// |2^A| = 2^|A| — число всех подмножеств (n ≤ 63).
unsigned long long power_set_size(int n) {
    return 1ULL << n;
}

// --- A.6. Все подмножества универсума U (|U| = n) ---
// Каждая маска от 0 до 2ⁿ−1 — ровно одно подмножество. O(2ⁿ).
vector<unsigned long long> all_subsets(int n) {
    vector<unsigned long long> res;
    for (unsigned long long mask = 0; mask < (1ULL << n); mask++)
        res.push_back(mask);
    return res;
}

// --- A.6. Все подмножества заданного множества mask ---
// Классический цикл: s = (s−1) & mask перебирает подмаски в порядке
// убывания, пока не дойдёт до нуля (после нуля следующая итерация
// дала бы снова mask, поэтому после вывода 0 — остановка).
// Суммарно по всем маскам перебор даёт O(3ⁿ) пар (маска, подмаска).
vector<unsigned long long> submasks_of(unsigned long long mask) {
    vector<unsigned long long> res;
    for (unsigned long long s = mask;; s = (s - 1) & mask) {
        res.push_back(s);
        if (s == 0) break;
    }
    return res;
}

// --- A.5. Формула включений-исключений: |∪Aᵢ| ---
// f(S) = |∩_{i∈S} Aᵢ| — мощность пересечения множеств из подмножества S;
// вызывающий код предподсчитывает f(S) для всех S ⊆ [0, n), n ≤ 20.
// Ответ: Σ_{∅≠S} (−1)^(|S|+1) · f(S). O(2ⁿ).
long long inc_excl_union(int n, const function<long long(unsigned long long)>& f) {
    long long res = 0;
    for (unsigned long long s = 1; s < (1ULL << n); s++) {
        int bits = __builtin_popcountll(s);
        if (bits % 2 == 1) res += f(s);
        else res -= f(s);
    }
    return res;
}

// --- A.5. Элементы, лежащие ровно в k множествах ---
// f(S) = |∩_{i∈S} Aᵢ|; f(0) = |U| — мощность универсума.
// g[j] = Σ_{|S|=j} f(S); каждый элемент, лежащий в t множествах,
// посчитан в g[j] с кратностью C(t, j), поэтому биномиальная инверсия:
// a[k] = Σ_{j≥k} (−1)^{j−k} · C(j,k) · g[j].
// Возвращает вектор a[0..n]: a[k] — число элементов ровно в k множествах.
vector<long long> inc_excl_exact(int n, const function<long long(unsigned long long)>& f) {
    vector<long long> g(n + 1, 0);
    for (unsigned long long s = 0; s < (1ULL << n); s++)
        g[__builtin_popcountll(s)] += f(s);
    vector<long long> res(n + 1, 0);
    for (int k = 0; k <= n; k++)
        for (int j = k; j <= n; j++) {
            long long c = 1;
            for (int i = 0; i < k; i++) c = c * (j - i) / (i + 1);
            if ((j - k) % 2 == 0) res[k] += c * g[j];
            else res[k] -= c * g[j];
        }
    return res;
}

// --- A.6. Zeta-преобразование (SOS DP): f[mask] = Σ_{sub ⊆ mask} a[sub] ---
// DP по битам in-place: после обработки бита i в f[mask] лежит сумма
// по всем подмаскам, отличающимся от mask только в битах 0..i−1.
// O(n·2ⁿ) вместо наивного O(3ⁿ).
void zeta_transform(vector<long long>& f) {
    int n = (int)f.size();
    for (int i = 1; i < n; i <<= 1)
        for (int mask = 0; mask < n; mask++)
            if (mask & i) f[mask] += f[mask ^ i];
}

// --- A.6. Обратное преобразование Мёбиуса ---
// Восстанавливает a по f (тот же цикл, знак минус).
void mobius_transform(vector<long long>& f) {
    int n = (int)f.size();
    for (int i = 1; i < n; i <<= 1)
        for (int mask = 0; mask < n; mask++)
            if (mask & i) f[mask] -= f[mask ^ i];
}

// --- A.6. Zeta по надмножествам: f[mask] = Σ_{sup ⊇ mask} a[sup] ---
// Симметричный вариант: накапливаем бит, если он НЕ установлен.
void superset_zeta_transform(vector<long long>& f) {
    int n = (int)f.size();
    for (int i = 1; i < n; i <<= 1)
        for (int mask = 0; mask < n; mask++)
            if (!(mask & i)) f[mask] += f[mask | i];
}

// --- A.6. Обратное по надмножествам ---
void superset_mobius_transform(vector<long long>& f) {
    int n = (int)f.size();
    for (int i = 1; i < n; i <<= 1)
        for (int mask = 0; mask < n; mask++)
            if (!(mask & i)) f[mask] -= f[mask | i];
}

// =============================================================
// B. ОТНОШЕНИЯ НА МНОЖЕСТВАХ
// =============================================================

// --- B.1. Декартово произведение A × B ---
// Все упорядоченные пары (a, b), |A × B| = |A|·|B| пар.
vector<pair<int, int>> cartesian_product(const vector<int>& a, const vector<int>& b) {
    vector<pair<int, int>> res;
    for (int x : a)
        for (int y : b)
            res.push_back({x, y});
    return res;
}

// --- B.2. Булева матрица отношения из списка пар ---
// R[i][j] = 1 ⟺ (i, j) ∈ R; n = |A|, m = |B|.
vector<vector<int>> relation_from_pairs(int n, int m, const vector<pair<int, int>>& pairs) {
    vector<vector<int>> r(n, vector<int>(m, 0));
    for (auto [x, y] : pairs) r[x][y] = 1;
    return r;
}

// --- B.3. Теоретико-множественные операции над отношениями ---
// Отношение — множество пар, поэтому ∪, ∩, \ — поэлементно.
vector<vector<int>> relation_union(int n, int m,
                                   const vector<vector<int>>& a,
                                   const vector<vector<int>>& b) {
    vector<vector<int>> r(n, vector<int>(m, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            r[i][j] = a[i][j] || b[i][j];
    return r;
}

vector<vector<int>> relation_intersection(int n, int m,
                                          const vector<vector<int>>& a,
                                          const vector<vector<int>>& b) {
    vector<vector<int>> r(n, vector<int>(m, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            r[i][j] = a[i][j] && b[i][j];
    return r;
}

vector<vector<int>> relation_difference(int n, int m,
                                        const vector<vector<int>>& a,
                                        const vector<vector<int>>& b) {
    vector<vector<int>> r(n, vector<int>(m, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            r[i][j] = a[i][j] && !b[i][j];
    return r;
}

// Дополнение R̅ = (A × B) \ R — все пары, которых нет в R.
vector<vector<int>> relation_complement(int n, int m, const vector<vector<int>>& a) {
    vector<vector<int>> r(n, vector<int>(m, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            r[i][j] = !a[i][j];
    return r;
}

// --- B.3. Обратное отношение R⁻¹ ---
// (y, x) ∈ R⁻¹ ⟺ (x, y) ∈ R — транспонирование матрицы m×n.
vector<vector<int>> relation_inverse(int m, int n, const vector<vector<int>>& a) {
    vector<vector<int>> r(m, vector<int>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            r[j][i] = a[i][j];
    return r;
}

// --- B.3. Композиция отношений R ∘ S ---
// (i, k) ∈ R∘S ⟺ ∃j: (i, j) ∈ R и (j, k) ∈ S — булево умножение
// матриц n×m на m×p: сумма → ИЛИ, произведение → И.
// Путь длины 2: i → j → k. Ассоциативна.
vector<vector<int>> relation_compose(int n, int m, int p,
                                     const vector<vector<int>>& a,
                                     const vector<vector<int>>& b) {
    vector<vector<int>> r(n, vector<int>(p, 0));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < p; k++)
            for (int j = 0; j < m; j++)
                if (a[i][j] && b[j][k]) { r[i][k] = 1; break; }
    return r;
}

// --- B.3. Степень отношения R^k ---
// R⁰ = id (диагональ), R^k = R ∘ R^{k−1} — существование пути длины k.
// R¹ = R. O(k·n³) наивно.
vector<vector<int>> relation_power(int n, const vector<vector<int>>& r, int k) {
    vector<vector<int>> res(n, vector<int>(n, 0));
    if (k == 0) {
        for (int i = 0; i < n; i++) res[i][i] = 1;
        return res;
    }
    res = r;
    for (int e = 1; e < k; e++) res = relation_compose(n, n, n, res, r);
    return res;
}

// --- B.4. Свойства бинарных отношений (n×n) ---

// Рефлексивность: ∀i: (i, i) ∈ R — вся диагональ единичная.
bool is_reflexive(int n, const vector<vector<int>>& r) {
    for (int i = 0; i < n; i++)
        if (!r[i][i]) return false;
    return true;
}

// Антирефлексивность: ∀i: (i, i) ∉ R — диагональ нулевая.
bool is_irreflexive(int n, const vector<vector<int>>& r) {
    for (int i = 0; i < n; i++)
        if (r[i][i]) return false;
    return true;
}

// Симметричность: R = R⁻¹ — матрица симметрична.
bool is_symmetric(int n, const vector<vector<int>>& r) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (r[i][j] != r[j][i]) return false;
    return true;
}

// Антисимметричность: (a,b) ∈ R и (b,a) ∈ R ⇒ a = b —
// нет взаимных стрелок между разными вершинами.
bool is_antisymmetric(int n, const vector<vector<int>>& r) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j && r[i][j] && r[j][i]) return false;
    return true;
}

// Транзитивность: R ∘ R ⊆ R — для всех троек
// (i, j) ∈ R и (j, k) ∈ R ⇒ (i, k) ∈ R. O(n³).
bool is_transitive(int n, const vector<vector<int>>& r) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (r[i][j])
                for (int k = 0; k < n; k++)
                    if (r[j][k] && !r[i][k]) return false;
    return true;
}

// --- B.5. Отношение эквивалентности: рефлексивно + симметрично + транзитивно ---
bool is_equivalence(int n, const vector<vector<int>>& r) {
    return is_reflexive(n, r) && is_symmetric(n, r) && is_transitive(n, r);
}

// --- B.6. Частичный порядок: рефлексивно + антисимметрично + транзитивно ---
bool is_partial_order(int n, const vector<vector<int>>& r) {
    return is_reflexive(n, r) && is_antisymmetric(n, r) && is_transitive(n, r);
}

// --- B.6. Линейный (полный) порядок: частичный порядок + сравнимость ---
// Любые два разных элемента сравнимы: r[i][j] или r[j][i].
bool is_total_order(int n, const vector<vector<int>>& r) {
    if (!is_partial_order(n, r)) return false;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (!r[i][j] && !r[j][i]) return false;
    return true;
}

// --- B.5. Классы эквивалентности ---
// Классы = компоненты связности графа отношения: обход BFS по матрице.
// Возвращает cls[i] — номер класса элемента i; O(n²).
vector<int> equivalence_classes(int n, const vector<vector<int>>& r) {
    vector<int> cls(n, -1);
    int cid = 0;
    for (int s = 0; s < n; s++) {
        if (cls[s] != -1) continue;
        queue<int> q;
        q.push(s);
        cls[s] = cid;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v = 0; v < n; v++)
                if (r[u][v] && cls[v] == -1) {
                    cls[v] = cid;
                    q.push(v);
                }
        }
        cid++;
    }
    return cls;
}

// --- B.7. Рефлексивное замыкание: R ∪ id ---
// Наименьшее рефлексивное отношение, содержащее R.
vector<vector<int>> reflexive_closure(int n, const vector<vector<int>>& r) {
    vector<vector<int>> c = r;
    for (int i = 0; i < n; i++) c[i][i] = 1;
    return c;
}

// --- B.7. Симметричное замыкание: R ∪ R⁻¹ ---
// Наименьшее симметричное отношение, содержащее R.
vector<vector<int>> symmetric_closure(int n, const vector<vector<int>>& r) {
    vector<vector<int>> c = r;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (r[i][j]) c[j][i] = 1;
    return c;
}

// --- B.7. Транзитивное замыкание R⁺ (алгоритм Уоршалла) ---
// (i, j) ∈ R⁺ ⟺ из i в j есть путь ненулевой длины. Идея: после
// обработки промежуточной вершины k все пути используют только вершины
// 0..k−1 как промежуточные; если достижимы i→k и k→j, достижимо i→j.
// O(n³), память O(n²).
vector<vector<int>> transitive_closure(int n, const vector<vector<int>>& r) {
    vector<vector<int>> c = r;
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            if (c[i][k])
                for (int j = 0; j < n; j++)
                    if (c[k][j]) c[i][j] = 1;
    return c;
}

// --- B.6. Топологическая сортировка (алгоритм Кана) ---
// Порядок, в котором (a, b) ∈ R ⇒ a раньше b. Берём вершину без
// входящих рёбер, кладём в ответ, убираем её исходящие рёбра.
// O(n²) на матрице. Если есть цикл — возвращает пустой вектор.
vector<int> topological_sort(int n, const vector<vector<int>>& r) {
    vector<int> indeg(n, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            indeg[j] += r[i][j];
    queue<int> q;
    for (int i = 0; i < n; i++)
        if (indeg[i] == 0) q.push(i);
    vector<int> res;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        res.push_back(u);
        for (int v = 0; v < n; v++)
            if (r[u][v] && --indeg[v] == 0) q.push(v);
    }
    if ((int)res.size() != n) res.clear();
    return res;
}

// --- B.6. Количество топологических сортировок ---
// DP по подмножествам: cnt[mask] — число корректных порядков элементов
// из mask. К mask можно добавить v, если все предшественники v уже в
// mask: (pred[v] & mask) == pred[v]. O(n·2ⁿ), n ≤ 20.
long long count_topological_orders(int n, const vector<vector<int>>& r) {
    vector<unsigned long long> pred(n, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (r[i][j]) pred[j] |= 1ULL << i;
    int full = 1 << n;
    vector<long long> cnt(full, 0);
    cnt[0] = 1;
    for (int mask = 0; mask < full; mask++) {
        if (cnt[mask] == 0) continue;
        for (int v = 0; v < n; v++) {
            if (mask & (1 << v)) continue;
            if ((pred[v] & mask) == pred[v])
                cnt[mask | (1 << v)] += cnt[mask];
        }
    }
    return cnt[full - 1];
}

// --- B.8. Функция: в каждой строке матрицы ровно одна единица ---
// Соответствие f ⊆ A × B — функция, если ∀a ∈ A ∃!b: (a, b) ∈ f.
bool is_function(int n, int m, const vector<vector<int>>& r) {
    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int j = 0; j < m; j++) cnt += r[i][j];
        if (cnt != 1) return false;
    }
    return true;
}

// --- B.8. Инъекция: f(a₁) = f(a₂) ⇒ a₁ = a₂ ---
// Все образы различны; f[i] — образ i.
bool is_injective(const vector<int>& f) {
    vector<bool> seen(f.size(), false);
    for (int x : f) {
        if (seen[x]) return false;
        seen[x] = true;
    }
    return true;
}

// --- B.8. Сюръекция: ∀b ∈ [0, m) ∃a: f(a) = b ---
// Образы покрывают весь кодомен.
bool is_surjective(const vector<int>& f, int m) {
    vector<bool> hit(m, false);
    for (int x : f) hit[x] = true;
    for (int b = 0; b < m; b++)
        if (!hit[b]) return false;
    return true;
}

// --- B.8. Биекция: инъекция + сюръекция ---
bool is_bijective(const vector<int>& f, int m) {
    return is_injective(f) && is_surjective(f, m);
}

// --- B.8. Обратная функция f⁻¹ ---
// Существует как функция ⟺ f биективна. Возвращает g: g[f[i]] = i;
// не задействованные образы — −1.
vector<int> function_inverse(const vector<int>& f, int m) {
    vector<int> g(m, -1);
    for (int i = 0; i < (int)f.size(); i++) g[f[i]] = i;
    return g;
}

// --- B.8. Композиция функций: h = g ∘ f, h(x) = g(f(x)) ---
// Сначала f, потом g. Ассоциативна: (h∘g)∘f = h∘(g∘f).
vector<int> function_compose(const vector<int>& g, const vector<int>& f) {
    vector<int> h(f.size());
    for (int i = 0; i < (int)f.size(); i++) h[i] = g[f[i]];
    return h;
}

// --- B.8. Длина цикла функционального графа из вершины x ---
// «Заяц и черепаха»: медленный x → f(x), быстрый x → f(f(x));
// когда встретились — идём по кругу до возврата. O(длина цикла).
int function_cycle_length(const vector<int>& f, int x) {
    int slow = x, fast = x;
    do {
        slow = f[slow];
        fast = f[f[fast]];
    } while (slow != fast);
    int len = 1;
    for (int cur = f[slow]; cur != slow; cur = f[cur]) len++;
    return len;
}

// --- B.8. Анализ функционального графа (снятие листьев) ---
// Каждая компонента = «хвост» + цикл. Вершины с нулевой входящей
// степенью не лежат в циклах: снимаем их, понижая входящие степени
// следующих; оставшиеся — ровно вершины циклов. dist[i] — расстояние
// до цикла (0 для вершин цикла), cyc[i] — номер цикла. O(n).
pair<vector<int>, vector<int>> functional_graph_analyze(const vector<int>& f) {
    int n = (int)f.size();
    vector<int> indeg(n, 0);
    for (int x = 0; x < n; x++) indeg[f[x]]++;
    queue<int> q;
    for (int x = 0; x < n; x++)
        if (indeg[x] == 0) q.push(x);
    vector<bool> in_cycle(n, true);
    vector<int> order;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        in_cycle[u] = false;
        order.push_back(u);
        if (--indeg[f[u]] == 0) q.push(f[u]);
    }
    vector<int> cyc(n, -1), dist(n, 0);
    int cid = 0;
    for (int x = 0; x < n; x++) {
        if (!in_cycle[x] || cyc[x] != -1) continue;
        int cur = x;
        do {
            cyc[cur] = cid;
            dist[cur] = 0;
            cur = f[cur];
        } while (cur != x);
        cid++;
    }
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        dist[u] = dist[f[u]] + 1;
        cyc[u] = cyc[f[u]];
    }
    return {dist, cyc};
}

}; // конец struct SetsAndRelations

#ifndef SETS_RELATIONS_MAIN
signed main() {
    SetsAndRelations s;

    cout << "=== A. Операции над множествами (маски) ===" << endl;
    // U = {0..5}, A = {1,2,3}, B = {3,4,5}
    unsigned long long A = 0b1110, B = 0b111000;
    cout << "A = {1,2,3}, B = {3,4,5}: |A| = " << s.set_cardinality(A)
         << ", |B| = " << s.set_cardinality(B) << endl;
    cout << "A ∪ B = " << s.set_cardinality(s.set_union(A, B)) << " элементов (ожидаем 5)" << endl;
    cout << "A ∩ B = " << s.set_cardinality(s.set_intersection(A, B)) << " элементов (ожидаем 1)" << endl;
    cout << "A \\ B = " << s.set_cardinality(s.set_difference(A, B)) << " элементов (ожидаем 2)" << endl;
    cout << "A △ B = " << s.set_cardinality(s.set_symmetric_difference(A, B)) << " элементов (ожидаем 4)" << endl;
    cout << "A̅ (в U из 6 элементов) = " << s.set_cardinality(s.set_complement(A, 6)) << " элементов (ожидаем 3)" << endl;
    cout << "3 ∈ A: " << s.set_belongs(A, 3) << ", 4 ∈ A: " << s.set_belongs(A, 4) << endl;
    cout << "{1,2} ⊆ A: " << s.is_subset(0b110, A) << ", A ⊆ B: " << s.is_subset(A, B) << endl;
    cout << "|2^A| = " << s.power_set_size(3) << " (ожидаем 8)" << endl;

    cout << "\n=== A. Перебор подмножеств ===" << endl;
    auto subs = s.all_subsets(3);
    cout << "Все подмножества U={0,1,2}: " << subs.size() << " шт." << endl;
    auto submasks = s.submasks_of(0b1011);
    cout << "Подмаски 1011: ";
    for (auto m : submasks) cout << m << " ";
    cout << "(" << submasks.size() << " шт., ожидаем 8)" << endl;

    cout << "\n=== A. Включения-исключения ===" << endl;
    // Три множества на универсуме {0,1,2}: A={0,1}, B={0,2}, C={1,2}
    auto inter_fn = [](unsigned long long s) -> long long {
        long long inter[8] = {3, 2, 2, 1, 2, 1, 1, 0};
        return inter[s];
    };
    cout << "|A ∪ B ∪ C| = " << s.inc_excl_union(3, inter_fn) << " (ожидаем 3)" << endl;
    auto exact = s.inc_excl_exact(3, inter_fn);
    cout << "ровно в 1 множестве: " << exact[1] << ", в 2: " << exact[2]
         << ", в 3: " << exact[3] << " (ожидаем 0, 3, 0)" << endl;

    cout << "\n=== A. SOS DP (zeta / mobius) ===" << endl;
    vector<long long> z = {1, 2, 4, 8};
    s.zeta_transform(z);
    cout << "zeta([1,2,4,8]) = [" << z[0] << "," << z[1] << "," << z[2] << "," << z[3] << "] (ожидаем 1,3,5,15)" << endl;
    s.mobius_transform(z);
    cout << "mobius обратно = [" << z[0] << "," << z[1] << "," << z[2] << "," << z[3] << "] (ожидаем 1,2,4,8)" << endl;
    vector<long long> sz = {1, 2, 4, 8};
    s.superset_zeta_transform(sz);
    cout << "superset_zeta([1,2,4,8]) = [" << sz[0] << "," << sz[1] << "," << sz[2] << "," << sz[3]
         << "] (ожидаем 15,10,12,8)" << endl;

    cout << "\n=== B. Декартово произведение ===" << endl;
    auto cp = s.cartesian_product({1, 2}, {3, 4, 5});
    cout << "{1,2} × {3,4,5}: " << cp.size() << " пар (ожидаем 6): ";
    for (auto [x, y] : cp) cout << "(" << x << "," << y << ") ";
    cout << endl;

    cout << "\n=== B. Отношения: операции ===" << endl;
    // R = {(0,1),(1,2)}, S = {(1,0),(2,1)} на {0,1,2}
    auto R = s.relation_from_pairs(3, 3, {{0, 1}, {1, 2}});
    auto S = s.relation_from_pairs(3, 3, {{1, 0}, {2, 1}});
    auto comp = s.relation_compose(3, 3, 3, R, S);
    cout << "R∘S: ";
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (comp[i][j]) cout << "(" << i << "," << j << ") ";
    cout << " (ожидаем (0,0) (1,1))" << endl;
    auto Rinv = s.relation_inverse(3, 3, R);
    cout << "R⁻¹: ";
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (Rinv[i][j]) cout << "(" << i << "," << j << ") ";
    cout << " (ожидаем (1,0) (2,1))" << endl;
    auto swap2 = s.relation_from_pairs(2, 2, {{0, 1}, {1, 0}});
    auto p2 = s.relation_power(2, swap2, 2);
    cout << "swap²: ";
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            if (p2[i][j]) cout << "(" << i << "," << j << ") ";
    cout << " (ожидаем тождественное (0,0) (1,1))" << endl;

    cout << "\n=== B. Свойства отношений ===" << endl;
    // T = {(0,0),(0,1),(1,1)} — частичный порядок 0 ≤ 1
    auto T = s.relation_from_pairs(2, 2, {{0, 0}, {0, 1}, {1, 1}});
    cout << "T рефлексивно: " << s.is_reflexive(2, T)
         << ", симметрично: " << s.is_symmetric(2, T)
         << ", антисимметрично: " << s.is_antisymmetric(2, T)
         << ", транзитивно: " << s.is_transitive(2, T) << endl;
    cout << "T — эквивалентность: " << s.is_equivalence(2, T)
         << ", частичный порядок: " << s.is_partial_order(2, T)
         << ", линейный порядок: " << s.is_total_order(2, T) << endl;

    cout << "\n=== B. Эквивалентность и классы ===" << endl;
    // Классы {0,2}, {1,3}: все пары внутри классов + петли
    auto E = s.relation_from_pairs(4, 4, {{0, 0}, {0, 2}, {2, 0}, {2, 2},
                                          {1, 1}, {1, 3}, {3, 1}, {3, 3}});
    cout << "E — эквивалентность: " << s.is_equivalence(4, E) << endl;
    auto cls = s.equivalence_classes(4, E);
    cout << "Классы: ";
    for (int i = 0; i < 4; i++) cout << i << "→" << cls[i] << "  ";
    cout << "(ожидаем 0→0 1→1 2→0 3→1)" << endl;

    cout << "\n=== B. Замыкания ===" << endl;
    // Цепь 0→1→2→3
    auto chain = s.relation_from_pairs(4, 4, {{0, 1}, {1, 2}, {2, 3}});
    auto tc = s.transitive_closure(4, chain);
    int ones = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) ones += tc[i][j];
    cout << "Транзитивное замыкание цепи 0→1→2→3: " << ones
         << " единиц (ожидаем 6), транзитивно: " << s.is_transitive(4, tc) << endl;
    auto rc = s.reflexive_closure(4, chain);
    cout << "Рефлексивное замыкание рефлексивно: " << s.is_reflexive(4, rc) << endl;
    auto sc = s.symmetric_closure(4, chain);
    cout << "Симметричное замыкание симметрично: " << s.is_symmetric(4, sc) << endl;

    cout << "\n=== B. Порядок: топологическая сортировка ===" << endl;
    // DAG: 0→1, 0→2, 1→3, 2→3
    auto dag = s.relation_from_pairs(4, 4, {{0, 1}, {0, 2}, {1, 3}, {2, 3}});
    auto ord = s.topological_sort(4, dag);
    cout << "Порядок: ";
    for (int x : ord) cout << x << " ";
    cout << "(ожидаем 0, затем 1 и 2, затем 3)" << endl;
    cout << "Количество топологических сортировок: "
         << s.count_topological_orders(4, dag) << " (ожидаем 2)" << endl;
    auto cyc_dag = s.relation_from_pairs(3, 3, {{0, 1}, {1, 2}, {2, 0}});
    cout << "Цикл 0→1→2→0: сортировка пуста (есть цикл): "
         << s.topological_sort(3, cyc_dag).empty() << endl;

    cout << "\n=== B. Функции ===" << endl;
    // f = 0→1, 1→2, 2→0 — биекция на {0,1,2}
    vector<int> f = {1, 2, 0};
    cout << "f инъективна: " << s.is_injective(f)
         << ", сюръективна: " << s.is_surjective(f, 3)
         << ", биективна: " << s.is_bijective(f, 3) << endl;
    auto inv = s.function_inverse(f, 3);
    cout << "f⁻¹ = {";
    for (int i = 0; i < 3; i++) cout << inv[i] << (i < 2 ? ", " : "");
    cout << "} (ожидаем {2, 0, 1})" << endl;
    vector<int> g = {0, 2, 1};
    auto h = s.function_compose(g, f);
    cout << "g∘f = {";
    for (int i = 0; i < 3; i++) cout << h[i] << (i < 2 ? ", " : "");
    cout << "} (ожидаем {2, 1, 0})" << endl;
    auto fn_mat = s.relation_from_pairs(3, 3, {{0, 1}, {1, 2}, {2, 0}});
    auto not_fn = s.relation_from_pairs(3, 3, {{0, 1}, {0, 2}, {1, 0}, {2, 0}});
    cout << "Матрица с одной единицей в строке — функция: "
         << s.is_function(3, 3, fn_mat) << "; с двумя в строке: "
         << s.is_function(3, 3, not_fn) << endl;

    cout << "\n=== B. Функциональные графы ===" << endl;
    // fg: 0→1, 1→2, 2→0 (цикл), 3→2, 4→3 (хвосты)
    vector<int> fg = {1, 2, 0, 2, 3};
    auto [dist, cyc] = s.functional_graph_analyze(fg);
    cout << "dist = {";
    for (int i = 0; i < 5; i++) cout << dist[i] << (i < 4 ? ", " : "");
    cout << "} (ожидаем {0, 0, 0, 1, 2}), cyc = {";
    for (int i = 0; i < 5; i++) cout << cyc[i] << (i < 4 ? ", " : "");
    cout << "} (ожидаем все 0)" << endl;
    cout << "Длина цикла из 4: " << s.function_cycle_length(fg, 4)
         << " (ожидаем 3)" << endl;
}
#endif // SETS_RELATIONS_MAIN
#endif // DISCRETE_LOGIC_A_CPP
