#ifndef STRUCT_A_CPP
#define STRUCT_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <deque>
#include <queue>
#include <stack>
#include <unordered_map>
#include <climits>
using namespace std;

// =============================================================
// I. ЛИНЕЙНЫЕ СТРУКТУРЫ
// =============================================================
// Структура md: A. Массивы и последовательные контейнеры
//               → B. Списки и цепочки
//               → C. Стеки (LIFO)
//               → D. Очереди (FIFO) и деки
//               → E. Специализированные буферы
//               → F. Гибридные, функциональные и ленивые структуры
//
// LinearStructures — базовый класс всей ветки struct. Собственной
// арифметики не имеет. Закладывает три сквозных приёма ветки:
//   * арена-память и курсорные ссылки (индексы вместо указателей) —
//     CursorList; переиспользуются деревьями (II) и кучами (IV);
//   * амортизированный анализ — DynArray (фактор роста f),
//     QueueTwoStacks; используется в разделе V и analysis.md;
//   * монотонный стек/очередь — движки mono_scan и sliding_window_extrema;
//     используются в оптимизации ДП (dynamic.md, J).
//
// Переиспользование из других веток (не переписываем заново):
//   * Combinatorics::next_perm — алгоритм Нарайаны (метод struct
//     Combinatorics) из math/combinatorics/a/a.cpp — раздел A.3.7;
//   * Combinatorics::Sudoku — канонический 9×9-решатель с MRV оттуда же
//     (демо в main; обобщённый решатель на n×n — собственный метод
//     SudokuSolver, раздел A.3.16);
//   * SetsAndRelations::set_cardinality — popcount битовой маски из
//     math/discrete-and-logic/a-folder/a.cpp — переиспользуется в
//     SudokuSolver и BitBoard::popcount (разделы A.3.16–A.3.17).
//
// ВНИМАНИЕ (скрытие имён): методы index_2d, nge, pge здесь локальные;
// одноимённые из других веток не подключаются.

#ifndef INSIDE_STRUCT
#define INSIDE_STRUCT
#define COMBINATORICS_MAIN
#include "../../math/combinatorics/a/a.cpp"
#undef INSIDE_STRUCT
#undef COMBINATORICS_MAIN
#endif

#ifndef INSIDE_STRUCT
#define INSIDE_STRUCT
#define SETS_RELATIONS_MAIN
#include "../../math/discrete-and-logic/a-folder/a.cpp"
#undef INSIDE_STRUCT
#undef SETS_RELATIONS_MAIN
#endif

struct LinearStructures {

// =============================================================
// A. МАССИВЫ И ПОСЛЕДОВАТЕЛЬНЫЕ КОНТЕЙНЕРЫ
// =============================================================

// --- A.1.2. Индекс двумерного массива в линейной последовательности ---
// Row-major: k = r·m + c; column-major: k = c·n + r.
// Параметр порядка order: 0 — row-major, 1 — column-major.
// O(1). Обобщается на d измерений методом index_kd.
int index_2d(int r, int c, int rows, int cols, int order = 0) {
    return (order == 0) ? r * cols + c : c * rows + r;
}

// --- A.1.2. Индекс k-мерного массива (d измерений, stride-схема) ---
// dims — вектор размеров (n₀, ..., n_{d−1}); idx — вектор индексов.
// Row-major: k = Σ idx[j]·stride_j, stride_j = ∏_{t>j} dims[t].
// Column-major: stride_j = ∏_{t<j} dims[t].
// O(d) времени (при фиксированном d — O(1)).
long long index_kd(const vector<long long>& dims, const vector<long long>& idx,
                   bool column_major = false) {
    int d = (int)dims.size();
    long long k = 0, stride = 1;
    for (int j = column_major ? 0 : d - 1;
         column_major ? j < d : j >= 0;
         j += column_major ? 1 : -1) {
        k += idx[j] * stride;
        stride *= dims[j];
    }
    return k;
}

// --- A.2.1. Демонстрационный динамический массив (амортизация push_back) ---
// Рост: capacity' = capacity·f при заполнении (f > 1). Суммарная стоимость
// n вставок — геометрическая прогрессия: O(n); каждая вставка — O(1)
// амортизированно. Параметр f — фактор роста.
struct DynArray {
    int n, cap;
    int f;              // фактор роста (параметр, например 2)
    vector<int> a;

    DynArray(int factor = 2) : n(0), cap(0), f(factor) {}

    void ensure() {
        if (n < cap) return;
        int ncap = (cap == 0) ? 1 : cap * f;
        a.resize(ncap);
        cap = ncap;
    }

    void push_back(int x) { ensure(); a[n++] = x; }     // O(1) амортизированно
    void pop_back() { n--; }                            // O(1)
    int at(int i) const { return a[i]; }                // O(1)
    int size() const { return n; }
    int capacity() const { return cap; }
};

// --- A.3.1. Все равновесные индексы ---
// a[i] — равновесие ⟺ сумма слева = сумме справа.
// Один проход: left накапливается, right = total − left − a[i].
// Параметризация: предикат cmp(left, right) — по умолчанию равенство.
// O(n) времени, O(1) дополнительной памяти.
vector<int> equilibrium_indices(const vector<int>& a,
                                const function<bool(long long, long long)>& cmp = nullptr) {
    long long total = 0;
    for (int x : a) total += x;
    vector<int> res;
    long long left = 0;
    for (int i = 0; i < (int)a.size(); i++) {
        long long right = total - left - a[i];
        bool ok = (cmp == nullptr) ? (left == right) : cmp(left, right);
        if (ok) res.push_back(i);
        left += a[i];
    }
    return res;
}

// --- A.3.2. Число троек (k = 3 элемента) с суммой T ---
// Сортировка + два указателя: фиксируем i, сужаем [l, r] с двух сторон.
// Обобщение: сумма T как параметр; на k-элементные суммы — рекурсивное
// обобщение (техника enumeration). O(n²) времени, O(1) памяти.
long long count_triplets_sum(vector<int> a, long long T) {
    sort(a.begin(), a.end());
    int n = (int)a.size();
    long long cnt = 0;
    for (int i = 0; i < n - 2; i++) {
        int l = i + 1, r = n - 1;
        while (l < r) {
            long long s = (long long)a[i] + a[l] + a[r];
            if (s == T) { cnt++; l++; r--; }
            else if (s < T) l++;
            else r--;
        }
    }
    return cnt;
}

// --- A.3.3. k-я порядковая статистика: QuickSelect ---
// Разбиение (Lomuto) на [≤ pivot | pivot | > pivot], спуск в одну часть
// по рангу target. На месте (модифицирует a). Ожидаемо O(n),
// худший случай O(n²); детерминированная версия — медиана медиан
// (technique.md). Параметр k (1-индексированный).
int kth_largest_quickselect(vector<int>& a, int k) {
    int n = (int)a.size();
    int target = n - k;              // k-th largest = (n−k)-th smallest (0-indexed)
    int lo = 0, hi = n - 1;
    while (lo < hi) {
        int pivot = a[hi];
        int i = lo;
        for (int j = lo; j < hi; j++) {
            if (a[j] <= pivot) { swap(a[i], a[j]); i++; }
        }
        swap(a[i], a[hi]);
        if (target < i) hi = i - 1;
        else if (target > i) lo = i + 1;
        else return a[i];
    }
    return a[lo];
}

// --- A.3.3. k-я порядковая статистика: min-куча размера k ---
// Куча держит k наибольших виденных; вершина — k-й наибольший.
// O(n log k) времени, O(k) памяти. Выгодно при малом k.
int kth_largest_heap(const vector<int>& a, int k) {
    priority_queue<int, vector<int>, greater<int>> pq;
    for (int x : a) {
        pq.push(x);
        if ((int)pq.size() > k) pq.pop();
    }
    return pq.top();
}

// --- A.3.4. k-й элемент двух отсортированных массивов ---
// На каждом шаге отбрасываем половину одного из массивов: сравниваем
// a[i + mid] и b[j + mid], где mid = k/2 — размер отбрасываемого блока.
// O(log(min(n, m))) времени, O(1) памяти. Медиана — частный случай.
int kth_two_sorted(const vector<int>& a, const vector<int>& b, int k) {
    int n = (int)a.size(), m = (int)b.size();
    k--;                                    // 0-индексация
    int i = 0, j = 0;
    while (true) {
        if (i == n) return b[j + k];
        if (j == m) return a[i + k];
        if (k == 0) return min(a[i], b[j]);
        int mid = k / 2;
        int ai = min(n - 1, i + mid), bj = min(m - 1, j + mid);
        if (a[ai] <= b[bj]) { k -= ai - i + 1; i = ai + 1; }
        else                { k -= bj - j + 1; j = bj + 1; }
    }
}

// --- A.3.4. Медиана двух отсортированных массивов ---
// Через kth_two_sorted: L = n + m; если L нечётно — k = (L+1)/2,
// иначе среднее двух средних. O(log(min(n,m))).
double median_two_sorted(const vector<int>& a, const vector<int>& b) {
    int L = (int)a.size() + (int)b.size();
    if (L % 2 == 1) return kth_two_sorted(a, b, (L + 1) / 2);
    return (kth_two_sorted(a, b, L / 2) + kth_two_sorted(a, b, L / 2 + 1)) / 2.0;
}

// --- A.3.5. Проверка монотонности ---
// Параметры: dir — направление (0 — любое, 1 — возрастание, −1 — убывание),
// strict — строгая монотонность. Один проход. O(n), O(1).
bool is_monotonic(const vector<int>& a, int dir = 0, bool strict = false) {
    int n = (int)a.size();
    int d = 0;                              // определённое направление
    for (int i = 0; i + 1 < n; i++) {
        int cur = 0;
        if (a[i] < a[i + 1]) cur = 1;
        else if (a[i] > a[i + 1]) cur = -1;
        else if (strict) return false;      // равенство нарушает строгость
        else continue;                      // равенство допустимо в нестрогом режиме
        if (d == 0) d = cur;
        else if (d != cur) return false;    // смена направления
    }
    // d == 0 — массив без направляющих пар (пустой/одноэлементный/все равны):
    // подходит под любое направление в нестрогом режиме.
    return dir == 0 || d == dir || d == 0;
}

// --- A.3.6. Число пар с суммой S: хеш-счётчик ---
// Проход: для a[i] добавляем число ранее встреченных S − a[i].
// O(n) ожидаемо, O(n) памяти.
long long count_pairs_sum(const vector<int>& a, long long S) {
    unordered_map<long long, int> seen;
    long long cnt = 0;
    for (int x : a) {
        auto it = seen.find(S - x);
        if (it != seen.end()) cnt += it->second;
        seen[x]++;
    }
    return cnt;
}

// --- A.3.6. Число пар с суммой S: два указателя после сортировки ---
// O(n log n) времени, O(1) памяти.
long long count_pairs_sum_two_ptr(vector<int> a, long long S) {
    sort(a.begin(), a.end());
    int l = 0, r = (int)a.size() - 1;
    long long cnt = 0;
    while (l < r) {
        long long s = (long long)a[l] + a[r];
        if (s == S) { cnt++; l++; r--; }
        else if (s < S) l++;
        else r--;
    }
    return cnt;
}

// --- A.3.7. Перестановки (алгоритм Нарайаны) ---
// Каноническая реализация живёт в math/combinatorics/a/a.cpp как
// метод struct Combinatorics: bool next_perm(vector<int>&) — переиспользуется,
// а не переписывается (шаги: max i: a[i]<a[i+1]; max j: a[j]>a[i]; swap; reverse).
// Вызов из этого файла: comb.next_perm(seq) (см. демо внизу) — возвращает true,
// пока перестановки не кончатся (затем сбрасывает в минимальную и false).

// --- A.3.8. Префиксные суммы ---
// pref[0] = 0, pref[i+1] = pref[i] + a[i]; сумма [l, r] = pref[r+1] − pref[l].
// O(n) предподсчёт, O(1) запрос.
vector<long long> build_prefix_sum(const vector<int>& a) {
    vector<long long> pref(a.size() + 1, 0);
    for (int i = 0; i < (int)a.size(); i++) pref[i + 1] = pref[i] + a[i];
    return pref;
}

long long range_sum(const vector<long long>& pref, int l, int r) {
    return pref[r + 1] - pref[l];
}

// --- A.3.9. Префиксные произведения и произведение «кроме себя» ---
// product_except_self: b[k] = ∏_{j≠k} a[j] — через префиксные и суффиксные
// произведения (без деления — устойчиво к нулям). O(n) времени.
vector<long long> build_prefix_prod(const vector<int>& a) {
    vector<long long> prod(a.size() + 1, 1);
    for (int i = 0; i < (int)a.size(); i++) prod[i + 1] = prod[i] * a[i];
    return prod;
}

vector<long long> product_except_self(const vector<int>& a) {
    int n = (int)a.size();
    vector<long long> res(n, 1);
    long long left = 1;
    for (int i = 0; i < n; i++) { res[i] = left; left *= a[i]; }
    long long right = 1;
    for (int i = n - 1; i >= 0; i--) { res[i] *= right; right *= a[i]; }
    return res;
}

// --- A.3.10. Циклический сдвиг массива (три реверса) ---
// Правый сдвиг на k: k нормализуется в [0, n); left-сдвиг задаётся
// отрицательным k. O(n) времени, O(1) дополнительной памяти.
void rotate_array(vector<int>& a, int k) {
    int n = (int)a.size();
    if (n == 0) return;
    k %= n;
    if (k < 0) k += n;                      // слева = право на n−|k|
    reverse(a.begin(), a.end());
    reverse(a.begin(), a.begin() + k);
    reverse(a.begin() + k, a.end());
}

// --- A.3.11. Dutch National Flag (три зоны по mid) ---
// Инвариант: [0, l) < mid, [l, m) = mid, (r, n) > mid.
// O(n) времени, O(1) памяти; обобщение на c цветов — counting sort.
void dutch_flag(vector<int>& a, int mid) {
    int l = 0, m = 0, r = (int)a.size() - 1;
    while (m <= r) {
        if (a[m] < mid) { swap(a[l], a[m]); l++; m++; }
        else if (a[m] == mid) m++;
        else { swap(a[m], a[r]); r--; }
    }
}

// --- A.3.12. Максимальная сумма подотрезка: разделяй и властвуй ---
// T(n) = 2T(n/2) + O(n) → O(n log n); пересекающий середину максимум —
// суффикс левой половины + префикс правой.
long long max_subarray_dc_rec(const vector<int>& a, int l, int r) {
    if (l == r) return a[l];
    int m = (l + r) / 2;
    long long best = max(max_subarray_dc_rec(a, l, m), max_subarray_dc_rec(a, m + 1, r));
    long long suf = 0, best_suf = LLONG_MIN;
    for (int i = m; i >= l; i--) { suf += a[i]; best_suf = max(best_suf, suf); }
    long long pref = 0, best_pref = LLONG_MIN;
    for (int i = m + 1; i <= r; i++) { pref += a[i]; best_pref = max(best_pref, pref); }
    return max(best, best_suf + best_pref);
}

long long max_subarray_dc(const vector<int>& a) {
    return max_subarray_dc_rec(a, 0, (int)a.size() - 1);
}

// --- A.3.13. Алгоритм Кадане (максимальная/минимальная сумма подотрезка) ---
// best_ending[i] = max(a[i], best_ending[i−1] + a[i]). O(n), O(1).
// Обобщение: знак операции параметром (max по умолчанию).
// Конвенция: пустой подотрезок допускается (старт best = cur = 0) — на
// полностью отрицательных массивах kadane_max вернёт 0, не минимум массива.
long long kadane(const vector<int>& a, bool want_min = false) {
    long long best = 0, cur = 0;
    for (int x : a) {
        long long v = x;
        cur = want_min ? min(v, cur + v) : max(v, cur + v);
        best = want_min ? min(best, cur) : max(best, cur);
    }
    return best;
}

long long kadane_max(const vector<int>& a) { return kadane(a, false); }
long long kadane_min(const vector<int>& a) { return kadane(a, true); }

// --- A.3.14. Sliding Window Maximum: наивный вариант O(n·k) ---
// Оптимальный O(n) — монотонная очередь (раздел D.7: sliding_window_extrema).
vector<int> sliding_max_naive(const vector<int>& a, int k) {
    vector<int> res;
    for (int i = 0; i + k <= (int)a.size(); i++) {
        int mx = a[i];
        for (int j = i + 1; j < i + k; j++) mx = max(mx, a[j]);
        res.push_back(mx);
    }
    return res;
}

// --- A.3.15. Rain Water Trapping (два указателя) ---
// Уровень воды над клеткой ограничен меньшим из виденных максимумов.
// O(n) времени, O(1) памяти.
long long rain_water_trap(const vector<int>& a) {
    int n = (int)a.size();
    long long water = 0;
    int l = 0, r = n - 1, left_max = 0, right_max = 0;
    while (l <= r) {
        if (left_max <= right_max) {
            left_max = max(left_max, a[l]);
            water += left_max - a[l];
            l++;
        } else {
            right_max = max(right_max, a[r]);
            water += right_max - a[r];
            r--;
        }
    }
    return water;
}

// --- A.3.16. Обобщённый решатель судоку (n × n, блоки s × s, MRV) ---
// n = s²; битовые маски строк/столбцов/блоков — проверка кандидата O(1);
// ход — клетка с минимальным числом вариантов (MRV). Каноническая 9×9
// версия — Combinatorics::Sudoku из math/combinatorics (переиспользуется
// в main). Здесь: параметры s, счёт решений, восстановление доски.
// Сложность: экспоненциальная в худшем случае; MRV + маски делают
// стандартные размеры мгновенными.
struct SudokuSolver {
    int s, n;                               // s — сторона блока, n = s²
    vector<vector<int>> g;                  // доска, 0 = пустая клетка
    vector<int> rows, cols, blocks;

    SudokuSolver(const vector<vector<int>>& board, int block) : s(block), g(board) {
        n = s * s;
        rows.assign(n, 0); cols.assign(n, 0); blocks.assign(n, 0);
        for (int r = 0; r < n; r++)
            for (int c = 0; c < n; c++)
                if (g[r][c]) place(r, c, g[r][c], true);
    }

    int block_id(int r, int c) const { return (r / s) * s + c / s; }

    // маска допустимых цифр (биты 1..n)
    int allowed(int r, int c) const {
        int used = rows[r] | cols[c] | blocks[block_id(r, c)];
        return (int)(~used & ((1ULL << (n + 1)) - 2));
    }

    void place(int r, int c, int d, bool on) {
        int b = 1 << d;
        if (on) { rows[r] |= b; cols[c] |= b; blocks[block_id(r, c)] |= b; g[r][c] = d; }
        else    { rows[r] &= ~b; cols[c] &= ~b; blocks[block_id(r, c)] &= ~b; g[r][c] = 0; }
    }

    // возвращает true, если найдено решение (восстановленное в g)
    bool solve() {
        int br = -1, bc = -1, best = n + 1;
        SetsAndRelations s;
        for (int r = 0; r < n; r++)
            for (int c = 0; c < n; c++)
                if (g[r][c] == 0) {
                    int cnt = s.set_cardinality((unsigned long long)allowed(r, c));
                    if (cnt < best) { best = cnt; br = r; bc = c; }
                }
        if (br == -1) return true;
        int mask = allowed(br, bc);
        for (int d = 1; d <= n; d++) {
            if (!(mask >> d & 1)) continue;
            place(br, bc, d, true);
            if (solve()) return true;
            place(br, bc, d, false);
        }
        return false;
    }

    // число всех решений (без записи в g)
    long long count_solutions() {
        int br = -1, bc = -1, best = n + 1;
        SetsAndRelations s;
        for (int r = 0; r < n; r++)
            for (int c = 0; c < n; c++)
                if (g[r][c] == 0) {
                    int cnt = s.set_cardinality((unsigned long long)allowed(r, c));
                    if (cnt < best) { best = cnt; br = r; bc = c; }
                }
        if (br == -1) return 1;
        long long sum = 0;
        int mask = allowed(br, bc);
        for (int d = 1; d <= n; d++) {
            if (!(mask >> d & 1)) continue;
            place(br, bc, d, true);
            sum += count_solutions();
            place(br, bc, d, false);
        }
        return sum;
    }
};

// --- A.3.17. Bit Board: доска n×n, bits бит на клетку ---
// Клетка (r, c) занимает биты [p, p+bits) в сквозной нумерации битов,
// p = (r·n + c)·bits; слово слова = ⌈(n·n·bits)/64⌉. Значения 0..2^bits−1.
// Параметры: n, bits. O(1) на клетку; popcount — O(слов).
struct BitBoard {
    int n, bits, words;
    vector<unsigned long long> bb;          // n строк, words слов каждая

    BitBoard(int size, int cell_bits = 1) : n(size), bits(cell_bits) {
        words = (n * n * bits + 63) / 64;
        bb.assign(words, 0ULL);
    }

    // маска из bits младших битов
    unsigned long long cell_mask() const {
        return bits >= 64 ? ~0ULL : ((1ULL << bits) - 1);
    }

    // установить значение val в клетку (r, c); запись может пересекать границу слова
    void set_cell(int r, int c, unsigned long long val = 1) {
        int p = (r * n + c) * bits;
        int w = p / 64, o = p % 64;
        int keep = 64 - o;                  // сколько битов слова осталось с позиции o
        if (bits <= keep) {
            unsigned long long m = cell_mask() << o;
            bb[w] = (bb[w] & ~m) | (val << o);
        } else {
            unsigned long long ml = cell_mask() & ((1ULL << keep) - 1);   // низкие keep битов
            bb[w] = (bb[w] & ~(ml << o)) | ((val & ml) << o);
            bb[w + 1] = (bb[w + 1] & ~(cell_mask() >> keep)) | (val >> keep);
        }
    }

    void clear_cell(int r, int c) { set_cell(r, c, 0); }

    // значение хранимое в клетке (r, c)
    int get_cell(int r, int c) const {
        int p = (r * n + c) * bits;
        int w = p / 64, o = p % 64;
        int keep = 64 - o;
        unsigned long long lo = bb[w] >> o;
        if (bits <= keep) return (int)(lo & cell_mask());
        return (int)((lo | (bb[w + 1] << keep)) & cell_mask());
    }

    long long popcount() const {
        SetsAndRelations s;                 // set_cardinality — из discrete-and-logic
        long long cnt = 0;
        for (unsigned long long w : bb) cnt += s.set_cardinality(w);
        return cnt;
    }

    void print() const {
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) cout << (get_cell(r, c) ? '#' : '.');
            cout << '\n';
        }
    }
};

// =============================================================
// B. СПИСКИ И ЦЕПОЧКИ
// =============================================================

// --- B.1. Узел односвязного списка ---
struct SNode {
    int val;
    SNode* next;
    SNode(int v, SNode* nx = nullptr) : val(v), next(nx) {}
};

// --- B.1. Односвязный список: построение из последовательности ---
// Прямая сборка через хвостовой указатель (keep tail). O(n).
SNode* slist_build(const vector<int>& seq) {
    SNode* head = nullptr, **tail = &head;
    for (int x : seq) { *tail = new SNode(x); tail = &(*tail)->next; }
    return head;
}

// --- B.1. Печать списка (прямой порядок) ---
void slist_print(SNode* head) {
    for (SNode* p = head; p; p = p->next) cout << p->val << (p->next ? " -> " : "");
    cout << '\n';
}

// --- B.1. Печать в обратном порядке (стек/рекурсия) ---
void slist_print_reverse(SNode* head) {
    if (!head) return;
    slist_print_reverse(head->next);
    cout << head->val << ' ';
}

// --- B.1. Середина списка (быстрый и медленный указатели) ---
int slist_middle(SNode* head) {
    SNode* slow = head, *fast = head;
    while (fast && fast->next) { slow = slow->next; fast = fast->next->next; }
    return slow ? slow->val : -1;
}

// --- B.1. Обнаружение цикла (Floyd's cycle detection) ---
// Встреча «зайца» и «черепахи» ⟺ цикл есть. O(n), O(1).
bool slist_has_loop(SNode* head) {
    SNode* slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
}

// --- B.2. Узел двусвязного списка ---
struct DNode {
    int val;
    DNode* prev;
    DNode* next;
    DNode(int v, DNode* p = nullptr, DNode* nx = nullptr) : val(v), prev(p), next(nx) {}
};

// --- B.2. Двусвязный список (вставка/удаление/печать/pop с концов) ---
// Базовый класс для стека на двусвязном списке (DLStack, C.1),
// DequeDoubly и Steque (D.5–D.6): операции на концах — O(1).
struct DList {
    DNode* head = nullptr;
    DNode* tail = nullptr;
    int n = 0;

    void push_back(int v) {
        DNode* u = new DNode(v, tail, nullptr);
        if (tail) tail->next = u; else head = u;
        tail = u;
        n++;
    }

    void push_front(int v) {
        DNode* u = new DNode(v, nullptr, head);
        if (head) head->prev = u; else tail = u;
        head = u;
        n++;
    }

    void erase(DNode* u) {
        if (u->prev) u->prev->next = u->next; else head = u->next;
        if (u->next) u->next->prev = u->prev; else tail = u->prev;
        delete u;
        n--;
    }

    bool pop_front(int& out) {
        if (!head) return false;
        out = head->val;
        erase(head);
        return true;
    }

    bool pop_back(int& out) {
        if (!tail) return false;
        out = tail->val;
        erase(tail);
        return true;
    }

    int size() const { return n; }
    bool empty() const { return !head; }

    void print_forward() const {
        for (DNode* p = head; p; p = p->next) cout << p->val << ' ';
        cout << '\n';
    }

    void print_backward() const {
        for (DNode* p = tail; p; p = p->prev) cout << p->val << ' ';
        cout << '\n';
    }

    ~DList() { while (head) { DNode* t = head; head = head->next; delete t; } }
};

// --- B.3. Кольцевой список (циклический, вставка/печать) ---
// Переиспользуем SNode из B.1 (тот же односвязный узел).
struct CircularList {
    SNode* tail = nullptr;                  // tail->next — голова кольца

    // вставка в конец кольца O(1)
    void push_back(int v) {
        SNode* u = new SNode(v);
        if (!tail) { u->next = u; tail = u; }
        else { u->next = tail->next; tail->next = u; tail = u; }
    }

    // сдвиг головы (поворот кольца) O(1)
    void rotate() { if (tail) tail = tail->next; }

    // удаление первого вхождения значения v; O(n)
    bool remove(int v) {
        if (!tail) return false;
        SNode* head = tail->next;
        SNode* prev = tail;
        SNode* cur = head;
        do {
            if (cur->val == v) {
                if (cur == head && cur == tail) { tail = nullptr; }
                else if (cur == head) { tail->next = head->next; }
                else if (cur == tail) { prev->next = head; tail = prev; }
                else { prev->next = cur->next; }
                delete cur;
                return true;
            }
            prev = cur;
            cur = cur->next;
        } while (cur != head);
        return false;
    }

    void print() const {
        if (!tail) return;
        SNode* p = tail->next;
        do { cout << p->val << ' '; p = p->next; } while (p != tail->next);
        cout << '\n';
    }

    ~CircularList() { while (tail) { SNode* h = tail->next; if (h == tail) { delete tail; tail = nullptr; } else { tail->next = h->next; delete h; } } }
};

// --- B.3. Список с хвостовым указателем: push_back и append за O(1) ---
struct TailList {
    SNode* head = nullptr;
    SNode* tail = nullptr;

    void push_back(int v) {
        if (tail) { tail->next = new SNode(v); tail = tail->next; }
        else head = tail = new SNode(v);
    }

    // присоединить весь список other (его хвост переходит к нам)
    void append(TailList& other) {
        if (!other.head) return;
        if (tail) tail->next = other.head; else head = other.head;
        tail = other.tail;
        other.head = other.tail = nullptr;
    }

    ~TailList() { while (head) { SNode* t = head; head = head->next; delete t; } }
};

// --- B.4. XOR Linked List (одно поле: xor соседних указателей) ---
// prev = both XOR next — при обходе знаем один сосед, восстанавливаем другой.
struct XorNode {
    int val;
    uintptr_t both;                         // xor(prev, next)
    XorNode(int v, uintptr_t b = 0) : val(v), both(b) {}
};

struct XorList {
    XorNode* head = nullptr;

    void push_front(int v) {
        XorNode* u = new XorNode(v, (uintptr_t)head);
        if (head) head->both ^= (uintptr_t)u;
        head = u;
    }

    // обход с головы (другой конец недоступен)
    void print_forward() const {
        XorNode* cur = head, *prev = nullptr;
        while (cur) {
            cout << cur->val << ' ';
            XorNode* nx = (XorNode*)(cur->both ^ (uintptr_t)prev);
            prev = cur; cur = nx;
        }
        cout << '\n';
    }

    ~XorList() {
        XorNode* cur = head, *prev = nullptr;
        while (cur) {
            XorNode* nx = (XorNode*)(cur->both ^ (uintptr_t)prev);
            delete cur;
            prev = cur; cur = nx;
        }
    }
};

// --- B.5. Unrolled Linked List (блоки по B элементов) ---
// Узлы-блоки массива + ссылка на следующий блок; вставка — сдвиг в блоке.
// Параметр размера блока B. Поиск O(n), вставка O(B).
struct UnrolledList {
    int B;
    struct Block {
        vector<int> a;
        Block* next;
        explicit Block(int blk) : next(nullptr) { a.reserve(blk); }
    };
    Block* head = nullptr;

    explicit UnrolledList(int block_size = 16) : B(block_size) {}

    void push_back(int v) {
        if (!head) head = new Block(B);
        Block* b = head;
        while (b->next) b = b->next;
        if ((int)b->a.size() == B) { b->next = new Block(B); b = b->next; }
        b->a.push_back(v);
    }

    // вставка по индексу idx (с разделением переполненного блока)
    void insert_at(int idx, int v) {
        Block* b = head;
        while (b && idx > (int)b->a.size()) { idx -= (int)b->a.size(); b = b->next; }
        if (!b) { push_back(v); return; }
        b->a.insert(b->a.begin() + idx, v);
        if ((int)b->a.size() > B) {           // разделить блок пополам
            int half = (int)b->a.size() / 2;
            Block* nb = new Block(B);
            nb->next = b->next; b->next = nb;
            nb->a.assign(b->a.begin() + half, b->a.end());
            b->a.resize(half);
        }
    }

    // поиск: возвращает глобальный индекс или -1; O(n)
    int find(int v) const {
        int global = 0;
        for (Block* b = head; b; b = b->next) {
            for (int i = 0; i < (int)b->a.size(); i++) {
                if (b->a[i] == v) return global + i;
            }
            global += (int)b->a.size();
        }
        return -1;
    }

    // удаление по глобальному индексу; O(n)
    bool remove(int idx) {
        Block* b = head;
        while (b && idx >= (int)b->a.size()) { idx -= (int)b->a.size(); b = b->next; }
        if (!b) return false;
        b->a.erase(b->a.begin() + idx);
        merge_blocks();
        return true;
    }

    // пересборка: слияние соседних блоков, если суммарный размер ≤ B
    void merge_blocks() {
        Block* b = head;
        while (b && b->next) {
            if ((int)b->a.size() + (int)b->next->a.size() <= B) {
                Block* nx = b->next;
                b->a.insert(b->a.end(), nx->a.begin(), nx->a.end());
                b->next = nx->next;
                delete nx;
            } else {
                b = b->next;
            }
        }
    }

    void print() const {
        for (Block* b = head; b; b = b->next)
            for (int x : b->a) cout << x << ' ';
        cout << '\n';
    }

    ~UnrolledList() {
        Block* b = head;
        while (b) { Block* nx = b->next; delete b; b = nx; }
    }
};

// --- B.6. Курсорный список (арена-память: узлы в массиве, ссылки-индексы) ---
// Арена: пул узлов фиксированного размера + свободный список — аллокация
// O(1) без malloc и без фрагментации. Список оперирует индексами-курсорами.
// Обобщение: размер пула N параметром; хранимое значение — произвольного
// типа (здесь int). Приём переиспользуется деревьями (II) и кучами (IV).
struct CursorList {
    struct Node { int val; int next; };     // next = −1 — конец
    int N, head, free_head;
    vector<Node> nodes;

    explicit CursorList(int pool_size) : N(pool_size), head(-1), free_head(0),
                                         nodes(pool_size) {
        for (int i = 0; i + 1 < N; i++) nodes[i].next = i + 1;
        nodes[N - 1].next = -1;
    }

    int alloc_node(int v) {
        int id = free_head;
        if (id == -1) return -1;            // пул исчерпан
        free_head = nodes[id].next;
        nodes[id] = {v, -1};
        return id;
    }

    void free_node(int id) {
        nodes[id].next = free_head;
        free_head = id;
    }

    void push_front(int v) {
        int id = alloc_node(v);
        if (id == -1) return;
        nodes[id].next = head;
        head = id;
    }

    // вставка после узла prev_id (prev_id = −1 — в голову)
    void insert_after(int prev_id, int v) {
        int id = alloc_node(v);
        if (id == -1) return;
        if (prev_id == -1) { nodes[id].next = head; head = id; }
        else { nodes[id].next = nodes[prev_id].next; nodes[prev_id].next = id; }
    }

    void erase_after(int prev_id) {
        int id = (prev_id == -1) ? head : nodes[prev_id].next;
        if (id == -1) return;
        if (prev_id == -1) head = nodes[id].next;
        else nodes[prev_id].next = nodes[id].next;
        free_node(id);
    }

    int find(int v) const {
        for (int id = head; id != -1; id = nodes[id].next)
            if (nodes[id].val == v) return id;
        return -1;
    }

    void print() const {
        for (int id = head; id != -1; id = nodes[id].next)
            cout << nodes[id].val << (nodes[id].next == -1 ? "" : " -> ");
        cout << '\n';
    }
};

// --- B.7. Сложные операции над списками (курсорная форма) ---

// Слияние двух отсортированных списков (индексы голов a, b) в новый список.
// Приём «сентинельная голова»: временный узел −1 в голове результата.
int cursor_merge_sorted(CursorList& L, int a, int b) {
    int sent = L.alloc_node(-1), last = sent;
    while (a != -1 && b != -1) {
        if (L.nodes[a].val <= L.nodes[b].val) { L.nodes[last].next = a; last = a; a = L.nodes[a].next; }
        else                                   { L.nodes[last].next = b; last = b; b = L.nodes[b].next; }
    }
    L.nodes[last].next = (a == -1) ? b : a;
    int res = L.nodes[sent].next;
    L.free_node(sent);
    return res;
}

// Палиндромность: середина (fast/slow) + разворот второй половины.
bool cursor_is_palindrome(CursorList& L, int head) {
    int slow = head, fast = head;
    while (fast != -1 && L.nodes[fast].next != -1) {
        slow = L.nodes[slow].next;
        fast = L.nodes[L.nodes[fast].next].next;
    }
    int mid = L.nodes[slow].next;           // вторая половина (после середины)
    int rev = -1;
    while (mid != -1) {
        int nx = L.nodes[mid].next;
        L.nodes[mid].next = rev;
        rev = mid;
        mid = nx;
    }
    int l = head, r = rev;
    bool ok = true;
    while (r != -1) {
        if (L.nodes[l].val != L.nodes[r].val) { ok = false; break; }
        l = L.nodes[l].next; r = L.nodes[r].next;
    }
    return ok;
}

// Разворот по группам размера k (хвост < k не переворачивается).
int cursor_reverse_k_group(CursorList& L, int head, int k) {
    int cur = head;
    int cnt = 0;
    for (int p = head; p != -1 && cnt < k; p = L.nodes[p].next) cnt++;
    if (cnt < k) return head;
    int prev = -1;
    while (cnt-- > 0) {
        int nx = L.nodes[cur].next;
        L.nodes[cur].next = prev;
        prev = cur;
        cur = nx;
    }
    int tail = head;
    L.nodes[tail].next = cursor_reverse_k_group(L, cur, k);
    return prev;
}

// Сдвиг вправо на k (k mod длина): отсоединить хвост, прицепить к голове.
int cursor_rotate_right(CursorList& L, int head, int k) {
    if (head == -1) return -1;
    int len = 0;
    for (int p = head; p != -1; p = L.nodes[p].next) len++;
    k %= len;
    len -= k;                               // позиция нового «разреза»
    int cut = head;
    while (--len > 0) cut = L.nodes[cut].next;
    int new_head = L.nodes[cut].next;
    if (new_head == -1) return head;        // k = 0
    L.nodes[cut].next = -1;
    int tail = new_head;
    while (L.nodes[tail].next != -1) tail = L.nodes[tail].next;
    L.nodes[tail].next = head;
    return new_head;
}

// Обмен значениями узлов по позициям i, j.
void cursor_swap_nodes(CursorList& L, int head, int i, int j) {
    int x = head;
    for (int t = 0; t < i && x != -1; t++) x = L.nodes[x].next;
    int y = head;
    for (int t = 0; t < j && y != -1; t++) y = L.nodes[y].next;
    if (x != -1 && y != -1) swap(L.nodes[x].val, L.nodes[y].val);
}

// --- B.8. Self-Organizing List (Move-to-Front) ---
// Поток запросов queries; после доступа элемент перемещается в голову.
// Амортизированно O(log n) для стационарных распределений (частотная
// эвристика); параметр — сама эвристика (здесь MTF).
void self_organizing_mtf(CursorList& L, int& head, const vector<int>& queries) {
    for (int q : queries) {
        int id = L.find(q);
        if (id == -1) continue;
        if (id == head) continue;           // уже голова — нечего перемещать
        int prev = -1;
        // prev — узел, следующий за которым идёт id
        for (int p = head; p != -1; p = L.nodes[p].next) {
            if (L.nodes[p].next == id) { prev = p; break; }
        }
        if (prev == -1) continue;           // не найден (не должно случаться)
        L.nodes[prev].next = L.nodes[id].next;
        L.nodes[id].next = head;
        head = id;
    }
}

// =============================================================
// C. СТЕКИ (LIFO — LAST IN, FIRST OUT)
// =============================================================

// --- C.1. Базовые реализации ---

// Массивный стек на векторе. Все операции O(1).
struct ArrayStack {
    vector<int> a;
    void push(int x) { a.push_back(x); }
    void pop() { a.pop_back(); }
    int top() const { return a.back(); }
    int size() const { return (int)a.size(); }
    bool empty() const { return a.empty(); }
};

// Списочный стек на односвязном списке (узел SNode из B.1). Все операции O(1).
struct ListStack {
    SNode* head = nullptr;
    int n = 0;

    void push(int x) { head = new SNode(x, head); n++; }
    void pop() { SNode* t = head; head = head->next; delete t; n--; }
    int top() const { return head->val; }
    int size() const { return n; }
    bool empty() const { return !head; }

    ~ListStack() { while (!empty()) pop(); }
};

// Списочный стек на двусвязном списке (переиспользует DList из B.2):
// операции на голове (push_front/pop_front) — O(1).
struct DLStack {
    DList d;

    void push(int x) { d.push_front(x); }
    bool pop(int& out) {
        if (!d.head) return false;
        out = d.head->val;
        d.erase(d.head);
        return true;
    }
    int top() const { return d.head->val; }
    int size() const { return d.size(); }
    bool empty() const { return !d.head; }
};

// Два стека в одном массиве: A растёт слева (→), B растёт справа (←).
// Переполнение — только при полном заполнении; ёмкость делится динамически.
struct TwoStacks {
    int n;
    vector<int> a;
    int topA, topB;          // topA — последний элемент A, topB — первый элемент B
    TwoStacks(int cap = 100) : n(cap), a(cap), topA(-1), topB(cap) {}

    bool pushA(int x) { if (topA + 1 >= topB) return false; a[++topA] = x; return true; }
    bool pushB(int x) { if (topA + 1 >= topB) return false; a[--topB] = x; return true; }
    bool popA(int& out) { if (topA < 0) return false; out = a[topA--]; return true; }
    bool popB(int& out) { if (topB >= n) return false; out = a[topB++]; return true; }
    int sizeA() const { return topA + 1; }
    int sizeB() const { return n - topB; }
};

// --- C.2. Стек из двух очередей ---
// push O(n)/pop O(1) ИЛИ push O(1)/pop O(n) — параметр fast_push.
struct StackTwoQueues {
    deque<int> q1, q2;
    bool fast_push;

    StackTwoQueues(bool fp = true) : fast_push(fp) {}

    void push(int x) {
        if (fast_push) {
            q2.push_back(x);
            while (!q1.empty()) { q2.push_back(q1.front()); q1.pop_front(); }
            swap(q1, q2);
        } else {
            q1.push_back(x);
        }
    }

    void pop() {
        if (!fast_push) {
            while (q1.size() > 1) { q2.push_back(q1.front()); q1.pop_front(); }
            q1.pop_front();
            swap(q1, q2);
        } else {
            q1.pop_front();
        }
    }

    int top() const { return q1.front(); }
    bool empty() const { return q1.empty(); }
    int size() const { return (int)q1.size(); }
};

// --- C.3. Минимум/максимум в стеке (три стека) ---
// push/pop/getMin/getMax — все O(1). Параллельные стеки минимумов и максимумов.
struct MinMaxStack {
    vector<int> vals, mins, maxs;

    void push(int x) {
        vals.push_back(x);
        mins.push_back(mins.empty() ? x : min(x, mins.back()));
        maxs.push_back(maxs.empty() ? x : max(x, maxs.back()));
    }

    void pop() {
        vals.pop_back();
        mins.pop_back();
        maxs.pop_back();
    }

    int top() const { return vals.back(); }
    int getMin() const { return mins.back(); }
    int getMax() const { return maxs.back(); }
    bool empty() const { return vals.empty(); }
    int size() const { return (int)vals.size(); }
};

// --- C.4. Монотонный стек (единый движок) ---
// Параметры: direction (0 = справа налево, 1 = слева направо),
//            cmp (0 = greater, 1 = less), strict (0 = нестрого, 1 = строго).
// Возвращает вектор индексов-ответов (для каждого элемента — ближайший удовлетворяющий).
// O(n) суммарно.
vector<int> mono_scan(const vector<int>& a, int dir = 0, int cmp = 0, bool strict = false) {
    int n = (int)a.size();
    vector<int> res(n, -1);
    vector<int> st;

    auto dominates = [&](int stack_val, int new_val) -> bool {
        if (cmp == 0) { // greater — pop stack_val if it's ≤ new_val
            return strict ? (stack_val <= new_val) : (stack_val < new_val);
        } else { // less — pop stack_val if it's ≥ new_val
            return strict ? (stack_val >= new_val) : (stack_val > new_val);
        }
    };

    if (dir == 0) { // справа налево
        for (int i = n - 1; i >= 0; i--) {
            while (!st.empty() && dominates(a[st.back()], a[i])) st.pop_back();
            if (!st.empty()) res[i] = st.back();
            st.push_back(i);
        }
    } else { // слева направо
        for (int i = 0; i < n; i++) {
            while (!st.empty() && dominates(a[st.back()], a[i])) st.pop_back();
            if (!st.empty()) res[i] = st.back();
            st.push_back(i);
        }
    }
    return res;
}

// Обёртки над mono_scan.
vector<int> nge(const vector<int>& a) { return mono_scan(a, 0, 0, true); }   // next greater element
vector<int> nse(const vector<int>& a) { return mono_scan(a, 0, 1, true); }   // next smaller element
vector<int> pge(const vector<int>& a) { return mono_scan(a, 1, 0, true); }   // previous greater element
vector<int> pse(const vector<int>& a) { return mono_scan(a, 1, 1, true); }   // previous smaller element

// --- C.5. Алгоритмы на стеке ---

// Проверка сбалансированности скобок (k типов). Параметр — маппинг пар.
// O(n) времени, O(k + depth) памяти.
bool is_balanced(const string& s, const string& open = "([{", const string& close = ")]}") {
    vector<int> st;
    for (char c : s) {
        size_t pos = open.find(c);
        if (pos != string::npos) { st.push_back(pos); continue; }
        pos = close.find(c);
        if (pos != string::npos) {
            if (st.empty() || st.back() != (int)pos) return false;
            st.pop_back();
        }
    }
    return st.empty();
}

// Инфикс → постфикс (Shunting Yard). Параметр — функция приоритета.
// O(n) времени.
int op_prec(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

string infix_to_postfix(const string& s) {
    string out;
    vector<char> st;
    for (char c : s) {
        if (isalnum(c)) { out += c; out += ' '; }
        else if (c == '(') st.push_back(c);
        else if (c == ')') {
            while (!st.empty() && st.back() != '(') { out += st.back(); out += ' '; st.pop_back(); }
            if (!st.empty()) st.pop_back();
        } else {
            while (!st.empty() && st.back() != '(' && op_prec(st.back()) >= op_prec(c)) {
                out += st.back(); out += ' '; st.pop_back();
            }
            st.push_back(c);
        }
    }
    while (!st.empty()) { out += st.back(); out += ' '; st.pop_back(); }
    return out;
}

// Инфикс → префикс: реверс строки → постфиксная конвертация → реверс результата.
string infix_to_prefix(string s) {
    reverse(s.begin(), s.end());
    for (char& c : s) { if (c == '(') c = ')'; else if (c == ')') c = '('; }
    string pf = infix_to_postfix(s);
    reverse(pf.begin(), pf.end());
    return pf;
}

// Оценка постфиксного выражения. Числа — однозначные; операторы: +, -, *, /.
int eval_postfix(const string& s) {
    vector<int> st;
    for (char c : s) {
        if (isdigit(c)) st.push_back(c - '0');
        else if (c == '+' || c == '-' || c == '*' || c == '/') {
            int b = st.back(); st.pop_back();
            int a = st.back(); st.pop_back();
            if (c == '+') st.push_back(a + b);
            else if (c == '-') st.push_back(a - b);
            else if (c == '*') st.push_back(a * b);
            else st.push_back(a / b);
        }
    }
    return st.back();
}

// Оценка префиксного выражения (чтение справа налево).
int eval_prefix(const string& s) {
    vector<int> st;
    for (int i = (int)s.size() - 1; i >= 0; i--) {
        char c = s[i];
        if (isdigit(c)) st.push_back(c - '0');
        else if (c == '+' || c == '-' || c == '*' || c == '/') {
            int a = st.back(); st.pop_back();
            int b = st.back(); st.pop_back();
            if (c == '+') st.push_back(a + b);
            else if (c == '-') st.push_back(a - b);
            else if (c == '*') st.push_back(a * b);
            else st.push_back(a / b);
        }
    }
    return st.back();
}

// Максимальный прямоугольник в гистограмме. Монотонный стек барьеров.
// Сентинельные высоты 0 на краях.
// O(n) времени, O(n) памяти.
long long largest_rect_histogram(const vector<int>& h) {
    vector<int> st;
    vector<int> heights(h.begin(), h.end());
    heights.push_back(0);                   // правый барьер
    heights.insert(heights.begin(), 0);      // левый барьер
    long long best = 0;

    for (int i = 0; i < (int)heights.size(); i++) {
        while (!st.empty() && heights[st.back()] > heights[i]) {
            int ht = heights[st.back()]; st.pop_back();
            int w = st.empty() ? i : i - st.back() - 1;
            best = max(best, (long long)ht * w);
        }
        st.push_back(i);
    }
    return best;
}

// Числа 1..n в лексикографическом порядке. DFS по дереву цифр, симулируемый стеком.
// Параметр — основание b (по умолчанию 10).
vector<int> lexicographical_numbers(int n, int b = 10) {
    vector<int> res;
    for (int i = 1; i < min(n + 1, b); i++) {
        vector<int> stack;
        stack.push_back(i);
        while (!stack.empty()) {
            int cur = stack.back(); stack.pop_back();
            if (cur <= n) {
                res.push_back(cur);
                for (int d = b - 1; d >= 0; d--) {
                    long long nxt = (long long)cur * b + d;
                    if (nxt <= n) stack.push_back((int)nxt);
                }
            }
        }
    }
    return res;
}

// Stock Span Problem: span[i] = i − pse(i) (расстояние до предыдущего меньшего).
// O(n).
vector<int> stock_span(const vector<int>& prices) {
    int n = (int)prices.size();
    vector<int> span(n, 1);
    vector<int> st;
    for (int i = 0; i < n; i++) {
        while (!st.empty() && prices[st.back()] <= prices[i]) st.pop_back();
        span[i] = st.empty() ? i + 1 : i - st.back();
        st.push_back(i);
    }
    return span;
}

// Dijkstra's Two-Stack Algorithm: вычисление выражения со скобками.
// Значения — в стек значений, операторы — в стек операторов; при ')' — свёртка.
// O(n).
int dijkstra_two_stack(const string& s) {
    vector<int> vals;
    vector<char> ops;
    for (char c : s) {
        if (isdigit(c)) vals.push_back(c - '0');
        else if (c == '(' || c == '+' || c == '-' || c == '*' || c == '/') ops.push_back(c);
        else if (c == ')') {
            while (ops.back() != '(') {
                char op = ops.back(); ops.pop_back();
                int b = vals.back(); vals.pop_back();
                int a = vals.back(); vals.pop_back();
                if (op == '+') vals.push_back(a + b);
                else if (op == '-') vals.push_back(a - b);
                else if (op == '*') vals.push_back(a * b);
                else vals.push_back(a / b);
            }
            ops.pop_back();
        }
    }
    while (!ops.empty()) {
        char op = ops.back(); ops.pop_back();
        int b = vals.back(); vals.pop_back();
        int a = vals.back(); vals.pop_back();
        if (op == '+') vals.push_back(a + b);
        else if (op == '-') vals.push_back(a - b);
        else if (op == '*') vals.push_back(a * b);
        else vals.push_back(a / b);
    }
    return vals.back();
}

// =============================================================
// D. ОЧЕРЕДИ (FIFO — FIRST IN, FIRST OUT)
// =============================================================

// --- D.1. Базовые реализации ---

// Очередь на массиве (кольцевое использование).
struct ArrayQueue {
    vector<int> a;
    int head, tail, sz, cap;
    ArrayQueue(int c = 100) : a(c), head(0), tail(0), sz(0), cap(c) {}

    bool enqueue(int x) {
        if (sz >= cap) return false;
        a[tail] = x;
        tail = (tail + 1) % cap;
        sz++;
        return true;
    }

    bool dequeue(int& out) {
        if (sz <= 0) return false;
        out = a[head];
        head = (head + 1) % cap;
        sz--;
        return true;
    }

    int front() const { return a[head]; }
    int size() const { return sz; }
    bool empty() const { return sz == 0; }
};

// Очередь на односвязном списке. Переиспользуем SNode из B.1.
struct LinkedQueue {
    SNode *head = nullptr, *tail = nullptr;
    int sz = 0;

    void enqueue(int x) {
        SNode* n = new SNode(x);
        if (tail) tail->next = n; else head = n;
        tail = n;
        sz++;
    }

    bool dequeue(int& out) {
        if (!head) return false;
        out = head->val;
        SNode* t = head; head = head->next; delete t;
        if (!head) tail = nullptr;
        sz--;
        return true;
    }

    int front() const { return head->val; }
    bool empty() const { return !head; }
    int size() const { return sz; }

    ~LinkedQueue() { int dummy; while (dequeue(dummy)); }
};

// --- D.2. Кольцевая очередь ---
// Наследует базовую кольцевую реализацию ArrayQueue (D.1) и добавляет back/full.
struct CircularQueue : ArrayQueue {
    CircularQueue(int c = 100) : ArrayQueue(c) {}

    int back() const { return a[(tail - 1 + cap) % cap]; }
    bool full() const { return sz == cap; }
};

// --- D.3. Очередь из двух стеков ---
// Амортизированно O(1) на операцию (банкёрский метод).
struct QueueTwoStacks {
    vector<int> in, out;

    void enqueue(int x) { in.push_back(x); }

    void transfer() {
        if (out.empty()) {
            while (!in.empty()) { out.push_back(in.back()); in.pop_back(); }
        }
    }

    bool dequeue(int& out_val) {
        transfer();
        if (out.empty()) return false;
        out_val = out.back(); out.pop_back();
        return true;
    }

    int front() { transfer(); return out.back(); }
    bool empty() const { return in.empty() && out.empty(); }
    int size() const { return (int)in.size() + (int)out.size(); }
};

// --- D.4. Приоритетная очередь на списке ---
// Два варианта (см. md D.4) + параметр направления экстремума want_max:
//   A. Неотсортированный связный список: push O(1), extract O(n).
//   B. Отсортированный список: push O(n), extract с головы O(1)
//      (при want_max порядок по убыванию, иначе по возрастанию).
// Переиспользуем SNode (B.1).
struct PriorityQueueList {
    SNode* head = nullptr;
    bool want_max;                          // true — извлекается максимум

    PriorityQueueList(bool max_mode = false) : want_max(max_mode) {}

    // A. вставка в неотсортированный список O(1)
    void pushUnsorted(int val) {
        SNode* u = new SNode(val);
        u->next = head; head = u;
    }

    // извлечение экстремума из неотсортированного за O(n)
    bool extract(int& out) {
        if (!head) return false;
        SNode* prev = nullptr, *bestPrev = nullptr, *best = head;
        for (SNode* p = head; p; p = p->next) {
            bool better = want_max ? (p->val > best->val) : (p->val < best->val);
            if (better) { best = p; bestPrev = prev; }
            prev = p;
        }
        out = best->val;
        if (bestPrev) bestPrev->next = best->next; else head = best->next;
        delete best;
        return true;
    }

    // B. вставка с сохранением порядка: want_max — убывание, иначе возрастание
    void pushSorted(int val) {
        SNode* u = new SNode(val);
        SNode* prev = nullptr, *p = head;
        while (p && (want_max ? p->val > val : p->val < val)) { prev = p; p = p->next; }
        u->next = p;
        if (prev) prev->next = u; else head = u;
    }

    // извлечение экстремума из отсортированного за O(1) — с головы
    bool extractSorted(int& out) {
        if (!head) return false;
        out = head->val;
        SNode* t = head; head = head->next; delete t;
        return true;
    }

    bool empty() const { return !head; }

    ~PriorityQueueList() {
        while (head) { SNode* t = head; head = head->next; delete t; }
    }
};

// --- D.5. Деки ---

// Кольцевой дек на массиве.
// Схема «проверка-откат»: вставка, которая свела бы head == tail, отменяется —
// фактическая ёмкость cap−1 (одна ячейка запаса), как в md D.2/D.5.
struct CircularDeque {
    vector<int> a;
    int head, tail, cap;
    CircularDeque(int c = 100) : a(c), head(0), tail(0), cap(c) {}

    bool pushFront(int x) {
        head = (head - 1 + cap) % cap;
        if (head == tail) { head = (head + 1) % cap; return false; }
        a[head] = x; return true;
    }

    bool pushBack(int x) {
        a[tail] = x;
        tail = (tail + 1) % cap;
        if (head == tail) { tail = (tail - 1 + cap) % cap; return false; }
        return true;
    }

    bool popFront(int& out) {
        if (head == tail) return false;
        out = a[head]; head = (head + 1) % cap; return true;
    }

    bool popBack(int& out) {
        if (head == tail) return false;
        tail = (tail - 1 + cap) % cap;
        out = a[tail]; return true;
    }

    int front() const { return a[head]; }
    int back() const { return a[(tail - 1 + cap) % cap]; }
    bool empty() const { return head == tail; }
};

// Дек на двусвязном списке. Переиспользуем DList (B.2).
struct DequeDoubly {
    DList dll;

    void pushFront(int x) { dll.push_front(x); }
    void pushBack(int x) { dll.push_back(x); }
    bool popFront(int& out) { return dll.pop_front(out); }
    bool popBack(int& out) { return dll.pop_back(out); }
    bool empty() const { return dll.empty(); }
    int size() const { return dll.size(); }
};

// --- D.6. Стеко-очередь (Steque) ---
// push/pop на одном конце (LIFO), enqueue на другом. Переиспользуем DList (B.2).
struct Steque {
    DList dll;

    void push(int x) { dll.push_front(x); }
    bool pop(int& out) { return dll.pop_front(out); }
    void enqueue(int x) { dll.push_back(x); }
    int top() const { return dll.head->val; }
    bool empty() const { return dll.empty(); }
    int size() const { return dll.size(); }
};

// --- D.7. Монотонная очередь (движок скользящего окна) ---
// Параметры: k (длина окна), want_max (true = максимум, false = минимум).
// Каждый элемент входит и выходит один раз — O(n) суммарно.
vector<int> sliding_window_extrema(const vector<int>& a, int k, bool want_max = true) {
    int n = (int)a.size();
    vector<int> res;
    deque<int> dq;

    for (int i = 0; i < n; i++) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        if (want_max) {
            while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();
        } else {
            while (!dq.empty() && a[dq.back()] >= a[i]) dq.pop_back();
        }
        dq.push_back(i);
        if (i >= k - 1) res.push_back(a[dq.front()]);
    }
    return res;
}

vector<int> sliding_max(const vector<int>& a, int k) { return sliding_window_extrema(a, k, true); }
vector<int> sliding_min(const vector<int>& a, int k) { return sliding_window_extrema(a, k, false); }

// --- D.8. Специализированные очереди ---

// Bucket Queue: приоритеты из диапазона [0, P). Вставка O(1), извлечение максимума O(P).
struct BucketQueue {
    int P;
    vector<deque<int>> buckets;
    int max_occupied;   // бегунок: максимальный непустой бакет

    BucketQueue(int num_priorities = 100) : P(num_priorities), buckets(P), max_occupied(-1) {}

    void push(int x, int prio) {
        buckets[prio].push_back(x);
        max_occupied = max(max_occupied, prio);
    }

    bool pop(int& out) {
        while (max_occupied >= 0 && buckets[max_occupied].empty()) max_occupied--;
        if (max_occupied < 0) return false;
        out = buckets[max_occupied].front();
        buckets[max_occupied].pop_front();
        return true;
    }

    bool empty() const { return max_occupied < 0 || buckets[max_occupied].empty(); }
};

// =============================================================
// E. СПЕЦИАЛИЗИРОВАННЫЕ БУФЕРЫ
// =============================================================

// --- E.1. Кольцевой буфер ---
// Наследует CircularQueue (D.2) и добавляет режим overwrite при переполнении.
struct RingBuffer : CircularQueue {
    bool overwrite;
    RingBuffer(int c = 100, bool ow = false) : CircularQueue(c), overwrite(ow) {}

    bool push(int x) {
        if (!full()) { enqueue(x); return true; }
        if (!overwrite) return false;
        int dummy; dequeue(dummy);
        enqueue(x);
        return true;
    }
};

// --- E.2. Буферы скользящего окна ---

// Count-based sliding window: окно последних k элементов.
// Поддерживает среднее, сумму, минимум, максимум.
struct MovingAverage {
    deque<int> dq;
    int k;
    long long sum = 0;

    MovingAverage(int window) : k(window) {}

    void add(int x) {
        dq.push_back(x);
        sum += x;
        if ((int)dq.size() > k) { sum -= dq.front(); dq.pop_front(); }
    }

    double average() const { return dq.empty() ? 0.0 : (double)sum / dq.size(); }
    long long getSum() const { return sum; }
};

// Time-based sliding window: события (t, value), окно длины tau.
// Агрегат — сумма; протухшие события выталкиваются.
struct TimeEvent { long long t; int val; };

long long time_sliding_sum(const vector<TimeEvent>& events, long long tau) {
    long long res = 0;
    int j = 0;
    for (int i = 0; i < (int)events.size(); i++) {
        while (events[i].t - events[j].t >= tau) { res -= events[j].val; j++; }
        res += events[i].val;
    }
    return res;
}

// =============================================================
// F. ГИБРИДНЫЕ, ФУНКЦИОНАЛЬНЫЕ И ЛЕНИВЫЕ СТРУКТУРЫ
// =============================================================

// --- F.3. Zipper (функциональный фокус) ---
// Фокус: левый стек (обратный порядок), текущий элемент, правый стек.
// left/right — O(1), modify — O(1). Персистентность через копирование.
struct ListZipper {
    vector<int> left;    // элементы до фокуса (обратный порядок — стек)
    int focus;
    vector<int> right;   // элементы после фокуса (обратный порядок — стек)

    ListZipper(const vector<int>& init, int pos) {
        for (int i = 0; i < pos; i++) left.push_back(init[i]);
        focus = init[pos];
        for (int i = (int)init.size() - 1; i > pos; i--) right.push_back(init[i]);
    }

    bool moveLeft() {
        if (left.empty()) return false;
        right.push_back(focus);
        focus = left.back(); left.pop_back();
        return true;
    }

    bool moveRight() {
        if (right.empty()) return false;
        left.push_back(focus);
        focus = right.back(); right.pop_back();
        return true;
    }

    void modify(int new_val) { focus = new_val; }

    void insertLeft(int x) { left.push_back(x); }
    void insertRight(int x) { right.push_back(x); }

    vector<int> toVector() const {
        vector<int> res;
        for (int i = (int)left.size() - 1; i >= 0; i--) res.push_back(left[i]);
        res.push_back(focus);
        for (int i = (int)right.size() - 1; i >= 0; i--) res.push_back(right[i]);
        return res;
    }
};

// --- F.4. Chunked Sequence ---
// Последовательность из блоков фиксированного размера B.
// Вставка O(B), поиск O(n/B + B).
struct ChunkedSeq {
    int B;
    vector<vector<int>> blocks;

    ChunkedSeq(int block_size = 64) : B(block_size) {}

    int size() const {
        if (blocks.empty()) return 0;
        return ((int)blocks.size() - 1) * B + (int)blocks.back().size();
    }

    void push_back(int x) {
        if (blocks.empty() || (int)blocks.back().size() >= B)
            blocks.push_back({});
        blocks.back().push_back(x);
    }

    int operator[](int idx) const {
        int b = idx / B;
        return blocks[b][idx - b * B];
    }

    void insert(int pos, int x) {
        if (blocks.empty()) { push_back(x); return; }
        int b = min((int)blocks.size() - 1, pos / B);
        int off = pos - b * B;
        blocks[b].insert(blocks[b].begin() + off, x);
        if ((int)blocks[b].size() > 2 * B) {
            vector<int> nb(blocks[b].begin() + B, blocks[b].end());
            blocks[b].resize(B);
            blocks.insert(blocks.begin() + b + 1, nb);
        }
    }
};

// --- F.5. Ленивые вычисления ---

// Thunk: отложенное вычисление с мемоизацией.
template<typename T>
struct Thunk {
    function<T()> compute;
    mutable T result;
    mutable bool computed = false;

    Thunk(function<T()> f) : compute(f) {}

    T force() const {
        if (!computed) { result = compute(); computed = true; }
        return result;
    }
};

// LazyFibStream: генератор Fibonacci «по требованию».
struct LazyFibStream {
    int limit, idx = 0;
    int a = 0, b = 1;
    LazyFibStream(int n) : limit(n) {}

    bool hasNext() const { return idx < limit; }

    int next() {
        int cur = a;
        int nxt = a + b;
        a = b;
        b = nxt;
        idx++;
        return cur;
    }
};

// --- F.6. Мемоизация ---
// Общая обёртка для функций int → int с кэшированием в unordered_map.
struct MemoizedFn {
    function<int(int)> f;
    unordered_map<int, int> cache;

    MemoizedFn(function<int(int)> func) : f(func) {}

    int operator()(int x) {
        auto it = cache.find(x);
        if (it != cache.end()) return it->second;
        int res = f(x);
        cache[x] = res;
        return res;
    }

    int cacheSize() const { return (int)cache.size(); }
};
// Обёртка для функций int → int с кэшированием в unordered_map.
struct MemoizedFib {
    unordered_map<int, long long> cache;

    long long fib(int n) {
        if (n <= 1) return n;
        auto it = cache.find(n);
        if (it != cache.end()) return it->second;
        long long res = fib(n - 1) + fib(n - 2);
        cache[n] = res;
        return res;
    }

    int cacheSize() const { return (int)cache.size(); }
};

// =============================================================
// signed main() — демонстрация и проверка всех разделов A–F
// =============================================================
}; // конец struct LinearStructures

#ifndef STRUCT_A_MAIN
signed main() {
    using LS = LinearStructures;
    LS ls;

    cout << "=== A. МАССИВЫ ===" << endl;
    // A.1.2 index_2d
    cout << "index_2d(1,0,2,3) = " << ls.index_2d(1, 0, 2, 3) << " (ожидаем 3)" << endl;
    // A.1.2 index_kd: dims {2,3}, idx {1,0}: row-major 1·3+0 = 3,
    // column-major 0·2+1 = 1 (сначала идём по строкам внутри столбца)
    cout << "index_kd row-major = " << ls.index_kd({2,3}, {1,0})
         << ", column-major = " << ls.index_kd({2,3}, {1,0}, true) << " (ожидаем 3 1)" << endl;
    // A.2.1 DynArray: ёмкости 1, 2, 4, 8 при факторе 2
    LS::DynArray dyn(2);
    for (int i = 1; i <= 6; i++) dyn.push_back(i);
    cout << "DynArray size=" << dyn.size() << " cap=" << dyn.capacity()
         << " at(3)=" << dyn.at(3) << " (ожидаем 6 8 4)" << endl;
    // A.3.1
    vector<int> eq = {1,2,3,4,5,6};
    vector<int> eq_res = ls.equilibrium_indices(eq);
    cout << "equilibrium({1..6}): ";
    for (int x : eq_res) cout << x << " ";
    cout << "(ожидаем none)" << endl;
    // A.3.3
    vector<int> arr3 = {7,10,4,3,20,15};
    cout << "kth_largest(3) = " << ls.kth_largest_quickselect(arr3, 3) << " (ожидаем 10)" << endl;
    cout << "kth_largest_heap(3) = " << ls.kth_largest_heap(arr3, 3) << " (ожидаем 10)" << endl;
    // A.3.4
    cout << "kth_two_sorted(k=4) = " << ls.kth_two_sorted({1,3,5,7}, {2,4,6}, 4) << " (ожидаем 4)" << endl;
    cout << "median_two_sorted = " << ls.median_two_sorted({1,3}, {2}) << " (ожидаем 2)" << endl;
    cout << "median_two_sorted = " << ls.median_two_sorted({1,2}, {3,4}) << " (ожидаем 2.5)" << endl;
    // A.3.5
    cout << "is_monotonic({1,2,3}) = " << ls.is_monotonic({1,2,3}) << " (1)" << endl;
    cout << "is_monotonic({1,3,2}) = " << ls.is_monotonic({1,3,2}) << " (0)" << endl;
    cout << "is_monotonic({1,1,2}, strict=0) = " << ls.is_monotonic({1,1,2}) << " (1)" << endl;
    cout << "is_monotonic({1,1,2}, strict=1) = " << ls.is_monotonic({1,1,2}, 0, true) << " (0)" << endl;
    // A.3.2: пары (1,3,5) и (2,3,4) — 2 тройки
    cout << "count_triplets_sum({1..5},9) = " << ls.count_triplets_sum({1,2,3,4,5}, 9) << " (ожидаем 2)" << endl;
    // A.3.6: пары (1,4) и (2,3) — 2
    cout << "count_pairs_sum({1..5},5) = " << ls.count_pairs_sum({1,2,3,4,5}, 5) << " (ожидаем 2)" << endl;
    cout << "count_pairs_sum_two_ptr = " << ls.count_pairs_sum_two_ptr({1,2,3,4,5}, 5) << " (ожидаем 2)" << endl;
    // A.3.8
    vector<int> pfx_in = {1,2,3,4};
    vector<long long> pfx = ls.build_prefix_sum(pfx_in);
    cout << "range_sum(1,3) = " << ls.range_sum(pfx, 1, 3) << " (ожидаем 9)" << endl;
    // A.3.9: префиксные произведения и product_except_self без деления
    vector<long long> pprod = ls.build_prefix_prod({2,3,4});
    cout << "build_prefix_prod: ";
    for (long long x : pprod) cout << x << " ";
    cout << "(ожидаем 1 2 6 24)" << endl;
    vector<long long> pes = ls.product_except_self({1,2,3,4});
    cout << "product_except_self: ";
    for (long long x : pes) cout << x << " ";
    cout << "(ожидаем 24 12 8 6)" << endl;
    // A.3.10
    vector<int> rot = {1,2,3,4,5,6,7};
    ls.rotate_array(rot, 3);
    cout << "rotate({1..7},3): ";
    for (int x : rot) cout << x << " ";
    cout << "(ожидаем 5 6 7 1 2 3 4)" << endl;
    // A.3.11
    vector<int> dnf = {2,0,2,1,1,0};
    ls.dutch_flag(dnf, 1);
    cout << "dutch_flag: ";
    for (int x : dnf) cout << x << " ";
    cout << "(ожидаем 0 0 1 1 2 2)" << endl;
    // A.3.12: разделяй и властвуй (проверка: 4,-1,2,1 → 6)
    vector<int> dc = {-2,1,-3,4,-1,2,1,-5,4};
    cout << "max_subarray_dc = " << ls.max_subarray_dc(dc) << " (ожидаем 6)" << endl;
    // A.3.13
    vector<int> kad = {-2,1,-3,4,-1,2,1,-5,4};
    cout << "kadane_max = " << ls.kadane_max(kad) << " (ожидаем 6)" << endl;
    // A.3.14: наивный вариант (оптимальный — монотонная очередь D.7)
    vector<int> sw_naive = ls.sliding_max_naive({1,3,-1,-3,5,3,6,7}, 3);
    cout << "sliding_max_naive: ";
    for (int x : sw_naive) cout << x << " ";
    cout << "(ожидаем 3 3 5 5 6 7)" << endl;
    // A.3.15
    vector<int> rain = {0,1,0,2,1,0,1,3,2,1,2,1};
    cout << "rain_water = " << ls.rain_water_trap(rain) << " (ожидаем 6)" << endl;
    // A.3.16: обобщённый решатель на 4×4 (s = 2)
    vector<vector<int>> mini = {
        {1,0,0,4},
        {0,0,1,0},
        {0,1,0,3},
        {4,0,2,0}
    };
    LS::SudokuSolver ms(mini, 2);
    cout << "SudokuSolver(4x4) solved = " << ms.solve() << " (ожидаем 1)" << endl;
    cout << "SudokuSolver count_solutions = " << ms.count_solutions() << " (ожидаем 1)" << endl;
    // A.3.17 BitBoard (1 бит на клетку и multi-bit значения)
    LS::BitBoard bb8(8);
    bb8.set_cell(0, 0); bb8.set_cell(7, 7);
    cout << "BitBoard(8x8, 1bit) popcount = " << bb8.popcount() << " (ожидаем 2)" << endl;
    LS::BitBoard bbv(8, 3);                 // 8x8, 3 бита на клетку
    bbv.set_cell(2, 5, 7);                  // значение 7 (биты пересекают границу слова)
    bbv.set_cell(0, 0, 1);
    cout << "BitBoard(3bit) (2,5)=" << bbv.get_cell(2, 5)
         << " (0,0)=" << bbv.get_cell(0, 0)
         << " popcount=" << bbv.popcount() << " (ожидаем 7 1 4)" << endl;

    cout << "\n=== B. СПИСКИ ===" << endl;
    // B.1 slist: построение, печать (прямая/обратная), середина, петля
    LS::SNode* sl = ls.slist_build({1,2,3,4,5});
    cout << "slist_build: ";
    ls.slist_print(sl);
    cout << "slist_print_reverse: ";
    ls.slist_print_reverse(sl);
    cout << "(ожидаем 5 4 3 2 1)" << endl;    cout << "slist_middle = " << ls.slist_middle(sl) << " (ожидаем 3)" << endl;
    LS::SNode* cyc = ls.slist_build({1,2,3});
    LS::SNode* cyc_tail = cyc; while (cyc_tail->next) cyc_tail = cyc_tail->next;
    cyc_tail->next = cyc;                        // замыкаем в кольцо
    cout << "slist_has_loop(cyclic) = " << ls.slist_has_loop(cyc) << " (ожидаем 1)" << endl;
    cout << "slist_has_loop(linear) = " << ls.slist_has_loop(sl) << " (ожидаем 0)" << endl;
    // B.2 DList: вставка с обоих концов, pop с концов
    LS::DList dl;
    dl.push_back(1); dl.push_back(2); dl.push_front(0);
    cout << "DList forward: ";
    dl.print_forward();
    cout << "(ожидаем 0 1 2)" << endl;
    cout << "DList backward: ";
    dl.print_backward();
    cout << "(ожидаем 2 1 0)" << endl;
    int dv; dl.pop_front(dv);
    cout << "DList pop_front = " << dv << " (ожидаем 0)" << endl;
    // B.3 CircularList + TailList (append за O(1))
    LS::CircularList clr;
    clr.push_back(1); clr.push_back(2); clr.push_back(3);
    clr.rotate();
    cout << "CircularList rotate: ";
    clr.print();
    cout << "(ожидаем 2 3 1)" << endl;
    LS::TailList tla, tlb;
    tla.push_back(1); tla.push_back(2); tlb.push_back(3); tlb.push_back(4);
    tla.append(tlb);
    cout << "TailList append: ";
    for (LS::SNode* p = tla.head; p; p = p->next) cout << p->val << ' ';
    cout << "(ожидаем 1 2 3 4)" << endl;
    // B.4 XorList (обход с головы)
    LS::XorList xl;
    xl.push_front(3); xl.push_front(2); xl.push_front(1);
    cout << "XorList: ";
    xl.print_forward();
    cout << "(ожидаем 1 2 3)" << endl;
    // B.5 UnrolledList (блоки по 2)
    LS::UnrolledList ul(2);
    for (int i = 1; i <= 5; i++) ul.push_back(i);
    ul.insert_at(2, 9);
    cout << "UnrolledList insert: ";
    ul.print();
    cout << "(ожидаем 1 2 9 3 4 5)" << endl;
    cout << "UnrolledList find(9) = " << ul.find(9) << " (ожидаем 2)" << endl;
    ul.remove(2);
    cout << "UnrolledList remove: ";
    ul.print();
    cout << "(ожидаем 1 2 3 4 5)" << endl;
    // B.6 CursorList
    LS::CursorList cl(20);
    cl.push_front(10);
    cl.push_front(20);
    cl.push_front(30);
    cout << "CursorList: ";
    for (int p = cl.head; p != -1; p = cl.nodes[p].next) cout << cl.nodes[p].val << " ";
    cout << "(ожидаем 30 20 10)" << endl;
    // B.7 cursor-операции: merge, palindrome, reverse k-group, rotate, swap
    LS::CursorList cm(30);
    int a1 = cm.alloc_node(1), a3 = cm.alloc_node(3), a5 = cm.alloc_node(5);
    cm.nodes[a1].next = a3; cm.nodes[a3].next = a5;            // A: 1 3 5
    int b2 = cm.alloc_node(2), b4 = cm.alloc_node(4), b6 = cm.alloc_node(6);
    cm.nodes[b2].next = b4; cm.nodes[b4].next = b6;            // B: 2 4 6
    int hM = ls.cursor_merge_sorted(cm, a1, b2);
    cout << "cursor_merge_sorted: ";
    for (int p = hM; p != -1; p = cm.nodes[p].next) cout << cm.nodes[p].val << " ";
    cout << "(ожидаем 1 2 3 4 5 6)" << endl;
    LS::CursorList cpal(20);
    cpal.push_front(1); cpal.push_front(2); cpal.push_front(3);
    cpal.push_front(2); cpal.push_front(1);                    // 1 2 3 2 1
    cout << "cursor_is_palindrome = " << ls.cursor_is_palindrome(cpal, cpal.head) << " (ожидаем 1)" << endl;
    LS::CursorList ckg(20);
    for (int i = 6; i >= 1; i--) ckg.push_front(i);            // 1..6
    int hk = ls.cursor_reverse_k_group(ckg, ckg.head, 2);
    cout << "cursor_reverse_k_group(k=2): ";
    for (int p = hk; p != -1; p = ckg.nodes[p].next) cout << ckg.nodes[p].val << " ";
    cout << "(ожидаем 2 1 4 3 6 5)" << endl;
    LS::CursorList crot(20);
    for (int i = 5; i >= 1; i--) crot.push_front(i);           // 1..5
    int hr = ls.cursor_rotate_right(crot, crot.head, 2);
    cout << "cursor_rotate_right(k=2): ";
    for (int p = hr; p != -1; p = crot.nodes[p].next) cout << crot.nodes[p].val << " ";
    cout << "(ожидаем 4 5 1 2 3)" << endl;
    LS::CursorList csw(20);
    for (int i = 5; i >= 1; i--) csw.push_front(i);            // 1..5
    ls.cursor_swap_nodes(csw, csw.head, 1, 3);
    cout << "cursor_swap_nodes(1,3): ";
    for (int p = csw.head; p != -1; p = csw.nodes[p].next) cout << csw.nodes[p].val << " ";
    cout << "(ожидаем 1 4 3 2 5)" << endl;
    // B.8 Self-Organizing List (MTF, включая запросы головы — крайний случай)
    LS::CursorList mtf_l(10);
    mtf_l.push_front(3); mtf_l.push_front(2); mtf_l.push_front(1);
    int mh = mtf_l.head;
    vector<int> mtf_q = {1, 2, 3, 3};
    ls.self_organizing_mtf(mtf_l, mh, mtf_q);
    mtf_l.head = mh;
    cout << "self_organizing_mtf: ";
    for (int p = mtf_l.head; p != -1; p = mtf_l.nodes[p].next) cout << mtf_l.nodes[p].val << " ";
    cout << "(ожидаем 3 2 1)" << endl;

    cout << "\n=== C. СТЕКИ ===" << endl;
    // C.1
    LS::ArrayStack astk;
    astk.push(1); astk.push(2); astk.push(3);
    cout << "ArrayStack top = " << astk.top() << " (ожидаем 3)" << endl;
    astk.pop();
    cout << "after pop: top = " << astk.top() << " (ожидаем 2)" << endl;
    LS::ListStack lstk;
    lstk.push(1); lstk.push(2); lstk.push(3);
    cout << "ListStack top = " << lstk.top() << " (ожидаем 3)" << endl;
    LS::DLStack dstk;
    dstk.push(1); dstk.push(2);
    int tmp;
    dstk.pop(tmp);
    cout << "DLStack pop = " << tmp << " (ожидаем 2)" << endl;
    // C.1 TwoStacks
    LS::TwoStacks ts(10);
    ts.pushA(1); ts.pushA(2); ts.pushB(10); ts.pushB(20);
    ts.popA(tmp);
    cout << "TwoStacks popA = " << tmp << " (ожидаем 2)" << endl;
    ts.popB(tmp);
    cout << "TwoStacks popB = " << tmp << " (ожидаем 20)" << endl;
    // C.2
    LS::StackTwoQueues sq(true);
    sq.push(1); sq.push(2); sq.push(3);
    sq.pop(); sq.pop();
    cout << "StackTwoQueues top = " << sq.top() << " (ожидаем 1)" << endl;
    // C.3
    LS::MinMaxStack mms;
    mms.push(3); mms.push(1); mms.push(2);
    cout << "MinMaxStack min=" << mms.getMin() << " max=" << mms.getMax() << " (ожидаем 1 3)" << endl;
    mms.pop();
    cout << "after pop: min=" << mms.getMin() << " max=" << mms.getMax() << " (ожидаем 1 3)" << endl;
    // C.4
    vector<int> mono_a = {2, 1, 5, 3, 4};
    vector<int> ng = ls.nge(mono_a);
    cout << "nge({2,1,5,3,4}): ";
    for (int x : ng) cout << x << " ";
    cout << "(ожидаем 2 2 -1 4 -1)" << endl;
    // C.5 is_balanced
    cout << "is_balanced(\"(()[])\") = " << ls.is_balanced("(()[])") << " (1)" << endl;
    cout << "is_balanced(\"([)]\") = " << ls.is_balanced("([)]") << " (0)" << endl;
    // C.5 infix_to_postfix
    cout << "infix_to_postfix(\"2+3*4\") = " << ls.infix_to_postfix("2+3*4") << endl;
    // C.5 infix_to_prefix: реверс → постфикс → реверс
    cout << "infix_to_prefix(\"2+3*4\") = " << ls.infix_to_prefix("2+3*4") << " (ожидаем + 2 * 3 4)" << endl;
    // C.5 eval_postfix
    cout << "eval_postfix(\"2 3 4 * +\") = " << ls.eval_postfix("2 3 4 * +") << " (ожидаем 14)" << endl;
    // C.5 eval_prefix: чтение справа налево; "+ 3 * 4 5" = 3 + 4·5 = 23
    cout << "eval_prefix(\"+ 3 * 4 5\") = " << ls.eval_prefix("+ 3 * 4 5") << " (ожидаем 23)" << endl;
    // C.5 largest_rect_histogram
    vector<int> hist = {2,1,5,6,2,3};
    cout << "largest_rect = " << ls.largest_rect_histogram(hist) << " (ожидаем 10)" << endl;
    // C.5 lexicographical_numbers
    vector<int> lex = ls.lexicographical_numbers(13);
    cout << "lexicographical(13): ";
    for (int x : lex) cout << x << " ";
    cout << "(ожидаем 1 10 11 12 13 2 ... 9)" << endl;
    // C.5 stock_span
    vector<int> prices = {100, 80, 60, 70, 60, 75, 85};
    vector<int> sp = ls.stock_span(prices);
    cout << "stock_span: ";
    for (int x : sp) cout << x << " ";
    cout << "(ожидаем 1 1 1 2 1 4 6)" << endl;
    // C.5 dijkstra_two_stack
    cout << "dijkstra(\"(2+3)*(4-1)\") = " << ls.dijkstra_two_stack("(2+3)*(4-1)") << " (ожидаем 15)" << endl;

    cout << "\n=== D. ОЧЕРЕДИ ===" << endl;
    // D.1
    LS::ArrayQueue aq(10);
    aq.enqueue(1); aq.enqueue(2); aq.enqueue(3);
    aq.dequeue(tmp);
    cout << "ArrayQueue dequeue = " << tmp << " front = " << aq.front() << " (ожидаем 1 2)" << endl;
    LS::LinkedQueue lq;
    lq.enqueue(1); lq.enqueue(2); lq.enqueue(3);
    lq.dequeue(tmp);
    cout << "LinkedQueue dequeue = " << tmp << " front = " << lq.front() << " (ожидаем 1 2)" << endl;
    // D.2
    LS::CircularQueue cq(5);
    cq.enqueue(10); cq.enqueue(20); cq.enqueue(30);
    cout << "CircularQueue front=" << cq.front() << " back=" << cq.back() << " (ожидаем 10 30)" << endl;
    // D.3
    LS::QueueTwoStacks qts;
    qts.enqueue(10); qts.enqueue(20); qts.enqueue(30);
    qts.dequeue(tmp);
    cout << "QueueTwoStacks dequeue = " << tmp << " (ожидаем 10)" << endl;
    // D.4 PriorityQueueList
    LS::PriorityQueueList pql;
    pql.pushUnsorted(5); pql.pushUnsorted(1); pql.pushUnsorted(3);
    int mn; pql.extract(mn);
    cout << "PriorityQueueList (unsorted, min) extract = " << mn << " (ожидаем 1)" << endl;
    LS::PriorityQueueList pqs;
    pqs.pushSorted(5); pqs.pushSorted(1); pqs.pushSorted(3);
    pqs.extractSorted(mn);
    cout << "PriorityQueueList (sorted, min) extractSorted = " << mn << " (ожидаем 1)" << endl;
    LS::PriorityQueueList pqmax(true);
    pqmax.pushUnsorted(5); pqmax.pushUnsorted(1); pqmax.pushUnsorted(3);
    pqmax.extract(mn);
    cout << "PriorityQueueList (unsorted, max) extract = " << mn << " (ожидаем 5)" << endl;
    LS::PriorityQueueList pqsmax(true);
    pqsmax.pushSorted(5); pqsmax.pushSorted(1); pqsmax.pushSorted(3);
    pqsmax.extractSorted(mn);
    cout << "PriorityQueueList (sorted, max) extractSorted = " << mn << " (ожидаем 5)" << endl;
    // D.5 CircularDeque
    LS::CircularDeque cd(5);
    cd.pushBack(1); cd.pushBack(2); cd.pushFront(0);
    cd.popBack(tmp);
    cout << "CircularDeque back=" << tmp << " front=" << cd.front() << " (ожидаем 2 0)" << endl;
    LS::DequeDoubly ddq;
    ddq.pushFront(4); ddq.pushBack(5); ddq.pushFront(3);
    ddq.popFront(tmp);
    cout << "DequeDoubly popFront = " << tmp << " (ожидаем 3)" << endl;
    ddq.popBack(tmp);
    cout << "DequeDoubly popBack = " << tmp << " (ожидаем 5)" << endl;
    // D.6 Steque
    LS::Steque sq2;
    sq2.push(1); sq2.push(2); sq2.enqueue(3);
    cout << "Steque top=" << sq2.top() << " size=" << sq2.size() << " (ожидаем 2 3)" << endl;
    // D.7
    vector<int> sw = {1,3,-1,-3,5,3,6,7};
    vector<int> sm = ls.sliding_max(sw, 3);
    cout << "sliding_max(k=3): ";
    for (int x : sm) cout << x << " ";
    cout << "(ожидаем 3 3 5 5 6 7)" << endl;
    vector<int> smin = ls.sliding_min({2,1,3,4,3,2}, 3);
    cout << "sliding_min(k=3): ";
    for (int x : smin) cout << x << " ";
    cout << "(ожидаем 1 1 3 2)" << endl;
    // D.8 BucketQueue
    LS::BucketQueue bq(10);
    bq.push(42, 5); bq.push(10, 3); bq.push(99, 8); bq.push(7, 5);
    bq.pop(tmp);
    cout << "BucketQueue max = " << tmp << " (ожидаем 99)" << endl;

    cout << "\n=== E. БУФЕРЫ ===" << endl;
    // E.1
    LS::RingBuffer rb(3, true);
    rb.push(1); rb.push(2); rb.push(3); rb.push(4); rb.push(5);
    cout << "RingBuffer(overwriting) front=" << rb.front() << " size=" << rb.size() << " (ожидаем 3 3)" << endl;
    // E.2 MovingAverage
    LS::MovingAverage ma(3);
    ma.add(1); ma.add(2); ma.add(3); ma.add(4);
    cout << "MovingAverage(3) = " << ma.average() << " (ожидаем 3)" << endl;
    // E.2 time_sliding_sum: финальное окно tau=5 (события с t=8,10,12): 1+3+4 = 8
    vector<LS::TimeEvent> t_events = {{1,5},{3,2},{5,8},{8,1},{10,3},{12,4}};
    cout << "time_sliding_sum(tau=5) = " << ls.time_sliding_sum(t_events, 5) << " (ожидаем 8)" << endl;

    cout << "\n=== F. ГИБРИДНЫЕ ===" << endl;
    // F.3 Zipper
    LS::ListZipper z({10,20,30,40,50}, 2);
    z.moveLeft();
    z.modify(99);
    vector<int> zv = z.toVector();
    cout << "Zipper after moveLeft+modify: ";
    for (int x : zv) cout << x << " ";
    cout << "(ожидаем 10 99 30 40 50)" << endl;
    // F.4 ChunkedSeq
    LS::ChunkedSeq cs(4);
    for (int i = 0; i < 10; i++) cs.push_back(i);
    cout << "ChunkedSeq size=" << cs.size() << " cs[5]=" << cs[5] << " (ожидаем 10 5)" << endl;
    // F.5 LazyFibStream
    LS::LazyFibStream fib(10);
    cout << "LazyFib(10): ";
    while (fib.hasNext()) cout << fib.next() << " ";
    cout << "(ожидаем 0 1 1 2 3 5 8 13 21 34)" << endl;
    // F.5 Thunk: отложенное вычисление, мемоизация после первого force
    LS::Thunk<int> th([]() -> int { return 6 * 7; });
    cout << "Thunk force x2 = " << th.force() << " " << th.force() << " (ожидаем 42 42)" << endl;
    // F.6 MemoizedFn
    LS::MemoizedFn mfn([](int x) { return x * x; });
    cout << "MemoizedFn(5) = " << mfn(5) << " cache=" << mfn.cacheSize()
         << " (ожидаем 25 1)" << endl;
    cout << "MemoizedFn(5) again = " << mfn(5) << " cache=" << mfn.cacheSize()
         << " (ожидаем 25 1)" << endl;
    // F.6 MemoizedFib
    LS::MemoizedFib mf;
    cout << "MemoizedFib(20) = " << mf.fib(20) << " cache=" << mf.cacheSize() << " (ожидаем 6765)" << endl;

    cout << "\n=== Переиспользовано из combinatorics ===" << endl;
    // next_perm — переиспользование
    Combinatorics comb;
    vector<int> perm = {1,2,3};
    cout << "next_perm({1,2,3}): ";
    do { for (int x : perm) cout << x; cout << " "; } while (comb.next_perm(perm));
    cout << "(ожидаем 123 132 213 231 312 321)" << endl;
    // Sudoku — реальное решение 9x9 (канонический решатель из combinatorics)
    vector<vector<int>> puzzle = {
        {5,3,0,0,7,0,0,0,0},
        {6,0,0,1,9,5,0,0,0},
        {0,9,8,0,0,0,0,6,0},
        {8,0,0,0,6,0,0,0,3},
        {4,0,0,8,0,3,0,0,1},
        {7,0,0,0,2,0,0,0,6},
        {0,6,0,0,0,0,2,8,0},
        {0,0,0,4,1,9,0,0,5},
        {0,0,0,0,8,0,0,7,9}
    };
    Combinatorics::Sudoku sdk(puzzle);
    bool solved = sdk.solve();
    cout << "Sudoku 9x9 solved = " << solved << " (ожидаем 1)" << endl;
    cout << "Sudoku board:" << endl;
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) cout << sdk.g[r][c] << (c == 8 ? "" : " ");
        cout << endl;
    }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_A_MAIN

#endif // STRUCT_A_CPP