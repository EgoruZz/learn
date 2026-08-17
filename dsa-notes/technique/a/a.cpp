#ifndef TECHNIQUE_A_CPP
#define TECHNIQUE_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <climits>
#include <random>
using namespace std;

// =============================================================
// I. СОРТИРОВКИ
// =============================================================
// Структура md: A. Квадратичные (Bubble, Selection, Insertion)
//               → B. O(n log n) (Merge, Heap, Quick)
//               → C. Линейные (Counting, Radix, Bucket)
//               → D. Дополнительно (Shell, Tim, Intro, ...)
//
// Sortings — базовый класс ветки technique. Собственной арифметики
// не имеет. Закладывает фундамент: сортировка — база для поиска (II),
// жадных (V) и планирования (VII). Все методы параметризуются
// компаратором cmp и диапазоном [lo, hi).
//
// Переиспользование из других веток (не переписываем заново):
//   * HeapIndexedTree — из struct/II.A (куча-индексация массивом)
//   * Binary Insertion Sort использует бинарный поиск ( technique II.B )

struct Sortings {

// =============================================================
// A. КВАДРАТИЧНЫЕ СОРТИРОВКИ O(n²)
// =============================================================

// --- A.1. Bubble Sort ---
// Проходы по массиву: обмен соседних при нарушении порядка.
// Флаг раннего выхода: если за проход не было обменов — массив отсортирован.
// O(n²) худший/средний, O(n) лучший (с флагом), O(1) памяти, стабильная.
// Параметр: cmp(a, b) — строгое отношение «a < b».
void bubble_sort(vector<int>& a,
                 const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                 int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    for (int i = lo; i < hi - 1; i++) {
        bool swapped = false;
        for (int j = lo; j < hi - 1 - (i - lo); j++) {
            if (!cmp(a[j], a[j + 1]) && a[j] != a[j + 1]) {
                swap(a[j], a[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// --- A.2. Selection Sort ---
// На каждом шаге: минимум в [i, n) → обмен с a[i].
// O(n²) сравнений, O(n) обменов, O(1) памяти, нестабильная.
// Оптимизация Two-Phase: за один проход и min, и max.
void selection_sort(vector<int>& a,
                    const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                    int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    for (int i = lo; i < (lo + hi) / 2 + 1; i++) {
        int mn = i, mx = i;
        for (int j = i + 1; j < hi - (i - lo); j++) {
            if (cmp(a[j], a[mn])) mn = j;
            if (cmp(a[mx], a[j])) mx = j;
        }
        swap(a[i], a[mn]);
        if (mx == i) mx = mn;
        swap(a[hi - 1 - (i - lo)], a[mx]);
    }
}

// --- A.3. Insertion Sort ---
// Каждый элемент вставляется в отсортированный префикс (сдвиги вправо).
// O(n²) худший, O(n + d) почти отсортированный (d — число инверсий),
// O(1) памяти, стабильная. База для Tim Sort.
void insertion_sort(vector<int>& a,
                    const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                    int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    for (int i = lo + 1; i < hi; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= lo && !cmp(a[j], key) && a[j] != key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

// --- A.3. Binary Insertion Sort ---
// Вставка через бинарный поиск: O(n log n) сравнений, O(n²) сдвигов.
// Стабильная.
void binary_insertion_sort(vector<int>& a,
                           const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                           int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    for (int i = lo + 1; i < hi; i++) {
        int key = a[i];
        // бинарный поиск позиции для вставки
        int l = lo, r = i;
        while (l < r) {
            int m = l + (r - l) / 2;
            if (!cmp(key, a[m]) && key != a[m]) l = m + 1;
            else r = m;
        }
        for (int j = i; j > l; j--) a[j] = a[j - 1];
        a[l] = key;
    }
}

// --- A.3. Recursive Insertion Sort ---
void recursive_insertion_sort(vector<int>& a, int n,
                              const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; }) {
    if (n <= 1) return;
    recursive_insertion_sort(a, n - 1, cmp);
    int key = a[n - 1];
    int j = n - 2;
    while (j >= 0 && !cmp(a[j], key) && a[j] != key) {
        a[j + 1] = a[j];
        j--;
    }
    a[j + 1] = key;
}

// =============================================================
// B. ЭФФЕКТИВНЫЕ СОРТИРОВКИ O(n log n)
// =============================================================

// --- B.4. Merge Sort (рекурсивный) ---
// Divide & conquer: T(n) = 2T(n/2) + O(n) → O(n log n).
// Стабильная, O(n) доп. памяти.
void merge(vector<int>& a, int lo, int mid, int hi,
           const function<bool(int, int)>& cmp) {
    vector<int> tmp(a.begin() + lo, a.begin() + hi);
    int i = 0, j = mid - lo, k = lo;
    while (i < mid - lo && j < hi - lo) {
        if (!cmp(tmp[j], tmp[i]) && tmp[j] != tmp[i]) a[k++] = tmp[i++];
        else a[k++] = tmp[j++];
    }
    while (i < mid - lo) a[k++] = tmp[i++];
    while (j < hi - lo) a[k++] = tmp[j++];
}

void merge_sort_rec(vector<int>& a, int lo, int hi,
                    const function<bool(int, int)>& cmp) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_rec(a, lo, mid, cmp);
    merge_sort_rec(a, mid, hi, cmp);
    merge(a, lo, mid, hi, cmp);
}

void merge_sort(vector<int>& a,
                const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    merge_sort_rec(a, lo, hi, cmp);
}

// --- B.4. Merge Sort (итеративный, bottom-up) ---
void merge_sort_iter(vector<int>& a,
                     const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                     int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int n = hi - lo;
    for (int sz = 1; sz < n; sz *= 2) {
        for (int l = lo; l < hi; l += 2 * sz) {
            int m = min(l + sz, hi);
            int r = min(l + 2 * sz, hi);
            merge(a, l, m, r, cmp);
        }
    }
}

// --- B.5. Heap Sort ---
// Построение max-heap за O(n) (снизу вверх) + n extract-max за O(log n).
// O(n log n) всегда, O(1) памяти, нестабильная.
void sift_down(vector<int>& a, int n, int i,
               const function<bool(int, int)>& cmp) {
    while (true) {
        int largest = i;
        int l = 2 * i + 1, r = 2 * i + 2;
        if (l < n && !cmp(a[l], a[largest]) && a[l] != a[largest]) largest = l;
        if (r < n && !cmp(a[r], a[largest]) && a[r] != a[largest]) largest = r;
        if (largest == i) break;
        swap(a[i], a[largest]);
        i = largest;
    }
}

void build_heap(vector<int>& a, int n,
                const function<bool(int, int)>& cmp) {
    for (int i = n / 2 - 1; i >= 0; i--) sift_down(a, n, i, cmp);
}

void heap_sort(vector<int>& a,
               const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
               int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int n = hi - lo;
    // строим min-heap для компаратора cmp (по умолчанию: a < b)
    // для max-heap инвертируем cmp
    auto heap_cmp = [&cmp](int x, int y) { return cmp(y, x); };
    for (int i = n / 2 - 1; i >= 0; i--) sift_down(a, n, i, heap_cmp);
    for (int i = n - 1; i > 0; i--) {
        swap(a[lo], a[lo + i]);
        sift_down(a, i, 0, heap_cmp);
    }
}

// --- B.6. Quick Sort ---
// Выбор pivot → partition → рекурсия. O(n log n) ожидаемо, O(n²) худший.
// Randomized и Median-of-three для отсечения худшего случая.

// Partition (Lomuto): элементы < pivot слева, ≥ pivot справа.
int partition_lomuto(vector<int>& a, int lo, int hi,
                     const function<bool(int, int)>& cmp) {
    int pivot = a[hi - 1];
    int i = lo;
    for (int j = lo; j < hi - 1; j++) {
        if (cmp(a[j], pivot) || a[j] == pivot) {
            swap(a[i], a[j]);
            i++;
        }
    }
    swap(a[i], a[hi - 1]);
    return i;
}

// 3-way partition (Dutch National Flag): < pivot | == pivot | > pivot.
// Переиспользует логику из struct I.A.11.
void partition_3way(vector<int>& a, int lo, int hi, int& lt, int& gt,
                    const function<bool(int, int)>& cmp) {
    int pivot = a[lo];
    lt = lo; gt = hi - 1;
    int i = lo;
    while (i <= gt) {
        if (cmp(a[i], pivot)) swap(a[lt++], a[i++]);
        else if (!cmp(a[i], pivot) && a[i] != pivot) swap(a[i], a[gt--]);
        else i++;
    }
}

void quick_sort_rec(vector<int>& a, int lo, int hi,
                    const function<bool(int, int)>& cmp) {
    if (hi - lo <= 1) return;
    // Median-of-three: pivot = median(a[lo], a[mid], a[hi-1])
    int mid = lo + (hi - lo) / 2;
    if (cmp(a[mid], a[lo])) swap(a[lo], a[mid]);
    if (cmp(a[hi - 1], a[lo])) swap(a[lo], a[hi - 1]);
    if (cmp(a[hi - 1], a[mid])) swap(a[mid], a[hi - 1]);
    swap(a[mid], a[hi - 2]);

    int lt, gt;
    partition_3way(a, lo, hi, lt, gt, cmp);
    quick_sort_rec(a, lo, lt, cmp);
    quick_sort_rec(a, gt + 1, hi, cmp);
}

void quick_sort(vector<int>& a,
                const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    quick_sort_rec(a, lo, hi, cmp);
}

// =============================================================
// C. ЛИНЕЙНЫЕ СОРТИРОВКИ O(n + k)
// =============================================================

// --- C.7. Counting Sort ---
// Для целых ключей из [lo_key, hi_key]: подсчёт частот → префиксы → расстановка.
// O(n + k) времени, O(n + k) памяти, стабильная.
// Параметр: функция получения ключа key_fn(element).
void counting_sort(vector<int>& a,
                   const function<int(int)>& key_fn = [](int x){ return x; },
                   int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    if (lo >= hi) return;

    int min_key = key_fn(a[lo]), max_key = key_fn(a[lo]);
    for (int i = lo + 1; i < hi; i++) {
        min_key = min(min_key, key_fn(a[i]));
        max_key = max(max_key, key_fn(a[i]));
    }
    int range = max_key - min_key + 1;
    vector<int> count(range, 0);
    for (int i = lo; i < hi; i++) count[key_fn(a[i]) - min_key]++;
    for (int i = 1; i < range; i++) count[i] += count[i - 1];
    vector<int> output(hi - lo);
    for (int i = hi - 1; i >= lo; i--) {
        int k = key_fn(a[i]) - min_key;
        output[--count[k]] = a[i];
    }
    for (int i = lo; i < hi; i++) a[i] = output[i - lo];
}

// --- C.8. Radix Sort (LSD) ---
// Сортировка по разрядам от младшего к старшему через Counting Sort.
// O(d·(n + b)) времени, стабильная. b — основание (radix).
void radix_sort_lsd(vector<int>& a, int radix = 256,
                    int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    if (lo >= hi) return;

    // определяем максимум по модулю
    int max_val = 0;
    for (int i = lo; i < hi; i++) max_val = max(max_val, abs(a[i]));

    vector<int> tmp(hi - lo);
    for (int exp = 1; max_val / exp > 0; exp *= radix) {
        vector<int> count(radix, 0);
        for (int i = lo; i < hi; i++) count[(a[i] / exp) % radix]++;
        for (int i = 1; i < radix; i++) count[i] += count[i - 1];
        for (int i = hi - 1; i >= lo; i--) {
            int d = (a[i] / exp) % radix;
            tmp[--count[d]] = a[i];
        }
        for (int i = lo; i < hi; i++) a[i] = tmp[i - lo];
    }
}

// --- C.9. Bucket Sort ---
// Разбиение на n бакетов → сортировка внутри каждого → слияние.
// O(n) среднее (равномерное распределение), O(n²) худшее.
void bucket_sort(vector<int>& a,
                 const function<int(int)>& bucket_fn = [](int x){ return x; },
                 int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    if (lo >= hi) return;

    int min_val = a[lo], max_val = a[lo];
    for (int i = lo + 1; i < hi; i++) {
        min_val = min(min_val, a[i]);
        max_val = max(max_val, a[i]);
    }
    int range = max_val - min_val + 1;
    int num_buckets = max(1, range / max(1, hi - lo));
    vector<vector<int>> buckets(num_buckets);
    for (int i = lo; i < hi; i++) {
        int idx = (a[i] - min_val) / max(1, range / num_buckets);
        idx = min(idx, num_buckets - 1);
        buckets[idx].push_back(a[i]);
    }
    int pos = lo;
    for (auto& b : buckets) {
        sort(b.begin(), b.end());
        for (int x : b) a[pos++] = x;
    }
}

// =============================================================
// D. ДОПОЛНИТЕЛЬНЫЕ СОРТИРОВКИ
// =============================================================

// --- D.10. Shell Sort ---
// Insertion Sort с убывающими gap. O(n^{1+})取决于 gap sequence.
void shell_sort(vector<int>& a,
                const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int n = hi - lo;
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = lo + gap; i < hi; i++) {
            int tmp = a[i];
            int j = i;
            while (j >= lo + gap && cmp(tmp, a[j - gap])) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = tmp;
        }
    }
}

// --- D.11. Cocktail Shaker Sort ---
// Bidirectional Bubble Sort. O(n²), стабильная.
void cocktail_shaker_sort(vector<int>& a,
                          const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                          int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    bool swapped = true;
    while (swapped) {
        swapped = false;
        for (int i = lo; i < hi - 1; i++) {
            if (!cmp(a[i], a[i + 1]) && a[i] != a[i + 1]) {
                swap(a[i], a[i + 1]);
                swapped = true;
            }
        }
        hi--;
        if (!swapped) break;
        swapped = false;
        for (int i = hi - 1; i > lo; i--) {
            if (!cmp(a[i - 1], a[i]) && a[i - 1] != a[i]) {
                swap(a[i - 1], a[i]);
                swapped = true;
            }
        }
        lo++;
    }
}

// --- D.12. Comb Sort ---
// Bubble Sort с gap = n/1.3. O(n²) худший, O(n log n) средний.
void comb_sort(vector<int>& a,
               const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
               int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int n = hi - lo;
    int gap = n;
    double shrink = 1.3;
    bool sorted = false;
    while (!sorted) {
        gap = max(1, (int)(gap / shrink));
        sorted = (gap == 1);
        for (int i = lo; i + gap < hi; i++) {
            if (!cmp(a[i], a[i + gap]) && a[i] != a[i + gap]) {
                swap(a[i], a[i + gap]);
                sorted = false;
            }
        }
    }
}

// --- D.13. Cycle Sort ---
// Минимум записей в память: каждый элемент → напрямую в позицию.
// O(n²), O(1) памяти, нестабильная. Оптимальна по числу записей.
void cycle_sort(vector<int>& a,
                const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    for (int s = lo; s < hi - 1; s++) {
        int item = a[s];
        int pos = s;
        for (int i = s + 1; i < hi; i++)
            if (cmp(a[i], item) || (!cmp(item, a[i]) && a[i] != item)) pos++;
        if (pos == s) continue;
        while (item == a[pos]) pos++;
        swap(item, a[pos]);
        while (pos != s) {
            pos = s;
            for (int i = s + 1; i < hi; i++)
                if (cmp(a[i], item) || (!cmp(item, a[i]) && a[i] != item)) pos++;
            while (item == a[pos]) pos++;
            swap(item, a[pos]);
        }
    }
}

// --- D.14. Strand Sort ---
// Вытягивание отсортированных подпоследовательностей + слияние.
// O(n²) худший, O(n log n) средний.
void strand_sort(vector<int>& a,
                 const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                 int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    vector<int> result;
    vector<int> remaining(a.begin() + lo, a.begin() + hi);
    while (!remaining.empty()) {
        vector<int> strand = {remaining[0]};
        remaining.erase(remaining.begin());
        auto it = remaining.begin();
        while (it != remaining.end()) {
            if (cmp(strand.back(), *it) || strand.back() == *it) {
                strand.push_back(*it);
                it = remaining.erase(it);
            } else it++;
        }
        // merge strand into result
        vector<int> merged;
        merged.reserve(result.size() + strand.size());
        size_t i = 0, j = 0;
        while (i < result.size() && j < strand.size()) {
            if (cmp(strand[j], result[i]) || (!cmp(result[i], strand[j]) && result[i] == strand[j]))
                merged.push_back(strand[j++]);
            else merged.push_back(result[i++]);
        }
        while (i < result.size()) merged.push_back(result[i++]);
        while (j < strand.size()) merged.push_back(strand[j++]);
        result = merged;
    }
    for (int i = lo; i < hi; i++) a[i] = result[i - lo];
}

// --- D.15. Patience Sort ---
// Раскладка по стопкам (бинарный поиск верхушки) + n-ways merge.
// O(n log n) время, O(n) памяти.
void patience_sort(vector<int>& a,
                   const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                   int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    vector<vector<int>> piles;
    for (int i = lo; i < hi; i++) {
        int x = a[i];
        // бинарный поиск: левая стопка с верхушкой >= x
        int l = 0, r = (int)piles.size();
        while (l < r) {
            int m = l + (r - l) / 2;
            if (cmp(piles[m].back(), x)) l = m + 1;
            else r = m;
        }
        if (l == (int)piles.size()) piles.push_back({});
        piles[l].push_back(x);
    }
    // min-heap: (value, pile_index, index_in_pile)
    using T = tuple<int, int, int>;
    priority_queue<T, vector<T>, greater<T>> pq;
    for (int i = 0; i < (int)piles.size(); i++)
        pq.push({piles[i][0], i, 0});
    int idx = lo;
    while (!pq.empty()) {
        auto [val, pi, si] = pq.top(); pq.pop();
        a[idx++] = val;
        if (si + 1 < (int)piles[pi].size())
            pq.push({piles[pi][si + 1], pi, si + 1});
    }
}

// --- D.16. Tim Sort ---
// Гибрид: runs (возрастающие/убывающие подмассивы) → Insertion Sort → Merge.
// O(n log n) худший, O(n) лучший, стабильная.
static const int MIN_RUN = 32;

void tim_sort(vector<int>& a,
              const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
              int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int n = hi - lo;
    for (int s = lo; s < hi; s += MIN_RUN) {
        int e = min(s + MIN_RUN, hi);
        binary_insertion_sort(a, cmp, s, e);
    }
    for (int sz = MIN_RUN; sz < n; sz *= 2) {
        for (int l = lo; l < hi; l += 2 * sz) {
            int m = min(l + sz, hi);
            int r = min(l + 2 * sz, hi);
            merge(a, l, m, r, cmp);
        }
    }
}

// --- D.17. Intro Sort ---
// Quick Sort с переключением на Heap Sort при глубине > c·log n;
// Insertion Sort для малых подмассивов.
void intro_sort_rec(vector<int>& a, int lo, int hi, int depth_limit,
                    const function<bool(int, int)>& cmp) {
    if (hi - lo <= 1) return;
    if (hi - lo <= MIN_RUN) {
        binary_insertion_sort(a, cmp, lo, hi);
        return;
    }
    if (depth_limit == 0) {
        heap_sort(a, cmp, lo, hi);
        return;
    }
    int lt, gt;
    partition_3way(a, lo, hi, lt, gt, cmp);
    intro_sort_rec(a, lo, lt, depth_limit - 1, cmp);
    intro_sort_rec(a, gt + 1, hi, depth_limit - 1, cmp);
}

void intro_sort(vector<int>& a,
                const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int depth_limit = 2 * (int)log2(hi - lo);
    intro_sort_rec(a, lo, hi, depth_limit, cmp);
}

// --- D.18. Gnome Sort ---
// Insertion Sort через обмены соседних элементов. O(n²), O(1) памяти.
void gnome_sort(vector<int>& a,
                const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int i = lo;
    while (i < hi) {
        if (i == lo || !cmp(a[i], a[i - 1]) && a[i] == a[i - 1]) i++;
        else { swap(a[i], a[i - 1]); i--; }
    }
}

// --- D.19. Odd-Even Sort ---
// Параллельная: чередующиеся фазы odd/even сравнений. O(n²).
void odd_even_sort(vector<int>& a,
                   const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                   int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int n = hi - lo;
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        for (int i = lo + 1; i + 1 < hi; i += 2) {
            if (!cmp(a[i], a[i + 1]) && a[i] != a[i + 1]) {
                swap(a[i], a[i + 1]);
                sorted = false;
            }
        }
        for (int i = lo; i + 1 < hi; i += 2) {
            if (!cmp(a[i], a[i + 1]) && a[i] != a[i + 1]) {
                swap(a[i], a[i + 1]);
                sorted = false;
            }
        }
    }
}

// --- D.20. Bitonic Sort ---
// Сеть comparator'ов для битонной последовательности.
// O(n log²n) comparisons, O(log²n) глубина.
void bitonic_merge(vector<int>& a, int lo, int cnt, bool dir,
                   const function<bool(int, int)>& cmp) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = lo; i < lo + k; i++) {
            bool should_swap = dir ? !cmp(a[i], a[i + k]) : cmp(a[i], a[i + k]);
            if (should_swap && a[i] != a[i + k]) swap(a[i], a[i + k]);
        }
        bitonic_merge(a, lo, k, dir, cmp);
        bitonic_merge(a, lo + k, k, dir, cmp);
    }
}

void bitonic_sort_rec(vector<int>& a, int lo, int cnt, bool dir,
                      const function<bool(int, int)>& cmp) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonic_sort_rec(a, lo, k, true, cmp);
        bitonic_sort_rec(a, lo + k, k, false, cmp);
        bitonic_merge(a, lo, cnt, dir, cmp);
    }
}

void bitonic_sort(vector<int>& a,
                  const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                  int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    bitonic_sort_rec(a, lo, hi - lo, true, cmp);
}

// --- D.22. Topological Sort (Kahn — BFS) ---
// DAG → линейная упорядоченность. O(V + E).
// Возвращает пустой вектор, если граф не является DAG.
vector<int> topological_sort_kahn(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<int> in_degree(n, 0);
    for (int u = 0; u < n; u++)
        for (int v : adj[u]) in_degree[v]++;
    queue<int> q;
    for (int i = 0; i < n; i++)
        if (in_degree[i] == 0) q.push(i);
    vector<int> order;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (int v : adj[u])
            if (--in_degree[v] == 0) q.push(v);
    }
    if ((int)order.size() != n) return {};
    return order;
}

// --- D.22. Topological Sort (DFS — reverse postorder) ---
void dfs_topo(const vector<vector<int>>& adj, int u, vector<char>& vis,
              vector<int>& order) {
    vis[u] = 1;
    for (int v : adj[u])
        if (!vis[v]) dfs_topo(adj, v, vis, order);
    order.push_back(u);
}

vector<int> topological_sort_dfs(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<char> vis(n, 0);
    vector<int> order;
    for (int i = 0; i < n; i++)
        if (!vis[i]) dfs_topo(adj, i, vis, order);
    reverse(order.begin(), order.end());
    return order;
}

// --- D.24. External Merge Sort ---
// Сортировка данных, не помещающихся в память: блоки → сортировка → k-merge.
// Параметр max_in_memory — максимальный размер блока в памяти.
vector<int> external_merge_sort(vector<int> a, int max_in_memory) {
    int n = (int)a.size();
    if (n <= max_in_memory) { sort(a.begin(), a.end()); return a; }

    // разбиваем на блоки, сортируем каждый
    vector<vector<int>> blocks;
    for (int i = 0; i < n; i += max_in_memory) {
        int end = min(i + max_in_memory, n);
        vector<int> block(a.begin() + i, a.begin() + end);
        sort(block.begin(), block.end());
        blocks.push_back(block);
    }

    // k-ways merge через min-heap
    using T = tuple<int, int, int>; // (value, block_index, element_index)
    priority_queue<T, vector<T>, greater<T>> pq;
    for (int i = 0; i < (int)blocks.size(); i++)
        if (!blocks[i].empty()) pq.push({blocks[i][0], i, 0});

    vector<int> result;
    while (!pq.empty()) {
        auto [val, bi, ei] = pq.top(); pq.pop();
        result.push_back(val);
        if (ei + 1 < (int)blocks[bi].size())
            pq.push({blocks[bi][ei + 1], bi, ei + 1});
    }
    return result;
}

// --- Radix Sort (MSD) ---
// Рекурсивный: сначала по старшему разряду, затем рекурсия внутри каждого бакета.
// Стабильная. Параметр radix — основание.
void radix_sort_msd_rec(vector<int>& a, int lo, int hi, int exp, int radix) {
    if (hi - lo <= 1 || exp <= 0) return;
    vector<int> count(radix, 0);
    for (int i = lo; i < hi; i++) count[(a[i] / exp) % radix]++;
    for (int i = 1; i < radix; i++) count[i] += count[i - 1];
    vector<int> tmp(hi - lo);
    for (int i = hi - 1; i >= lo; i--) {
        int d = (a[i] / exp) % radix;
        tmp[--count[d]] = a[i];
    }
    for (int i = lo; i < hi; i++) a[i] = tmp[i - lo];
    // рекурсия внутри бакетов
    for (int i = 0; i < radix; i++) {
        int start = (i == 0) ? lo : lo + count[i - 1];
        int end = lo + count[i];
        if (end - start > 1)
            radix_sort_msd_rec(a, start, end, exp / radix, radix);
    }
}

void radix_sort_msd(vector<int>& a, int radix = 256,
                    int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    if (lo >= hi) return;
    int max_val = 0;
    for (int i = lo; i < hi; i++) max_val = max(max_val, abs(a[i]));
    int exp = 1;
    while (max_val / exp >= radix) exp *= radix;
    radix_sort_msd_rec(a, lo, hi, exp, radix);
}

// =============================================================
// D. ДОПОЛНИТЕЛЬНЫЕ СОРТИРОВКИ (продолжение)
// =============================================================

// --- D.23. Bead Sort (Gravity Sort) ---
// Имитация гравитации: каждый элемент — столбик бусин.
// Для неотрицательных целых. O(n · max_val) время, O(n · max_val) памяти.
vector<int> bead_sort(vector<int> a) {
    if (a.empty()) return a;
    int n = (int)a.size();
    int max_val = *max_element(a.begin(), a.end());
    vector<vector<char>> grid(n, vector<char>(max_val, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < a[i]; j++) grid[i][j] = 1;
    for (int j = 0; j < max_val; j++) {
        int sum = 0;
        for (int i = 0; i < n; i++) { sum += grid[i][j]; grid[i][j] = 0; }
        for (int i = n - sum; i < n; i++) grid[i][j] = 1;
    }
    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int j = 0; j < max_val; j++) cnt += grid[i][j];
        a[i] = cnt;
    }
    return a;
}

// --- D.24. Bogo Sort ---
// Случайная перестановка + проверка. O((n+1)!) ожидаемо.
void bogo_sort(vector<int>& a,
               const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; }) {
    auto is_sorted = [&]() -> bool {
        for (int i = 1; i < (int)a.size(); i++)
            if (cmp(a[i], a[i - 1])) return false;
        return true;
    };
    mt19937 rng(random_device{}());
    while (!is_sorted()) shuffle(a.begin(), a.end(), rng);
}

// --- D.25. Circle Sort ---
// Рекурсивное сравнение и обмен крайних элементов по кругу.
bool circle_sort_rec(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return false;
    bool swapped = false;
    int l = lo, r = hi;
    while (l < r) {
        if (a[l] > a[r]) { swap(a[l], a[r]); swapped = true; }
        l++; r--;
    }
    if (lo == hi) return swapped;
    bool left = circle_sort_rec(a, lo, (lo + hi) / 2);
    bool right = circle_sort_rec(a, (lo + hi) / 2 + 1, hi);
    return swapped || left || right;
}

void circle_sort(vector<int>& a,
                 const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; }) {
    do { } while (circle_sort_rec(a, 0, (int)a.size() - 1));
}

// --- D.26. Cyclic Sort ---
// Для массива [1..n]: каждый элемент ставится на позицию a[i]−1.
// O(n) время, O(1) память.
void cyclic_sort(vector<int>& a) {
    int i = 0;
    while (i < (int)a.size()) {
        int correct = a[i] - 1;
        if (a[i] != a[correct]) swap(a[i], a[correct]);
        else i++;
    }
}

// --- D.27. Double Sort ---
// Сортировка двух массивов одновременно по ключу первого.
void double_sort(vector<int>& a, vector<int>& b) {
    int n = (int)a.size();
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (a[j] > a[j + 1]) {
                swap(a[j], a[j + 1]);
                swap(b[j], b[j + 1]);
            }
}

// --- D.28. Exchange Sort ---
// Обмен каждого элемента с каждым следующим. O(n²).
void exchange_sort(vector<int>& a,
                   const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; }) {
    int n = (int)a.size();
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (cmp(a[j], a[i])) swap(a[i], a[j]);
}

// --- D.29. Natural Sort ---
// Merge Sort, находит уже отсортированные runs и сливает их.
void natural_sort(vector<int>& a,
                  const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; }) {
    int n = (int)a.size();
    for (int sz = 1; sz < n; sz *= 2) {
        for (int l = 0; l < n; l += 2 * sz) {
            int m = min(l + sz, n);
            int r = min(l + 2 * sz, n);
            merge(a, l, m, r, cmp);
        }
    }
}

// --- D.30. Pancake Sort ---
// Только операция reverse(a[0..k]). O(n²) время, O(1) память.
void pancake_sort(vector<int>& a,
                  const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; }) {
    for (int sz = (int)a.size(); sz > 1; sz--) {
        int mi = 0;
        for (int i = 1; i < sz; i++)
            if (cmp(a[mi], a[i])) mi = i;
        if (mi == sz - 1) continue;
        reverse(a.begin(), a.begin() + mi + 1);
        reverse(a.begin(), a.begin() + sz);
    }
}

// --- D.31. Pigeonhole Sort ---
// Частный случай Counting Sort: число бакетов = n.
void pigeonhole_sort(vector<int>& a) {
    if (a.empty()) return;
    int min_val = *min_element(a.begin(), a.end());
    int max_val = *max_element(a.begin(), a.end());
    int range = max_val - min_val + 1;
    vector<vector<int>> holes(range);
    for (int x : a) holes[x - min_val].push_back(x);
    int idx = 0;
    for (auto& h : holes)
        for (int x : h) a[idx++] = x;
}

// --- D.32. Shrink Shell Sort ---
// Shell Sort с последовательностью gap = gap * 2/3 + 1.
void shrink_shell_sort(vector<int>& a,
                       const function<bool(int, int)>& cmp = [](int x, int y){ return x < y; },
                       int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int n = hi - lo;
    int gap = 1;
    while (gap < n / 3) gap = gap * 3 + 1;
    for (; gap >= 1; gap /= 3) {
        for (int i = lo + gap; i < hi; i++) {
            int tmp = a[i];
            int j = i;
            while (j >= lo + gap && cmp(tmp, a[j - gap])) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = tmp;
        }
    }
}

// --- D.33. Slowsort ---
// Divide & conquer: сортируем обе половины, затем сравниваем и сдвигаем.
// O(n^{log n}) — намеренно медленная.
void slowsort_rec(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    slowsort_rec(a, lo, mid);
    slowsort_rec(a, mid + 1, hi);
    if (a[mid] > a[hi]) swap(a[mid], a[hi]);
    slowsort_rec(a, lo, hi - 1);
}

void slowsort(vector<int>& a) {
    if (!a.empty()) slowsort_rec(a, 0, (int)a.size() - 1);
}

// --- D.34. Stooge Sort ---
// Рекурсия на 2/3 начала, 2/3 конца, снова 2/3 начала. O(n^{2.709}).
void stooge_sort_rec(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return;
    if (a[lo] > a[hi]) swap(a[lo], a[hi]);
    if (hi - lo + 1 > 2) {
        int t = (hi - lo + 1) / 3;
        stooge_sort_rec(a, lo, hi - t);
        stooge_sort_rec(a, lo + t, hi);
        stooge_sort_rec(a, lo, hi - t);
    }
}

void stooge_sort(vector<int>& a) {
    if (!a.empty()) stooge_sort_rec(a, 0, (int)a.size() - 1);
}

// --- D.35. Tree Sort ---
// Вставка всех элементов в BST, затем inorder-обход.
// O(n log n) средний, O(n²) худший.
void tree_sort(vector<int>& a) {
    struct TreeNode { int val; TreeNode *l, *r; TreeNode(int v) : val(v), l(nullptr), r(nullptr) {} };
    function<TreeNode*(TreeNode*, int)> insert = [&](TreeNode* t, int v) -> TreeNode* {
        if (!t) return new TreeNode(v);
        if (v < t->val) t->l = insert(t->l, v);
        else t->r = insert(t->r, v);
        return t;
    };
    function<void(TreeNode*, vector<int>&)> inorder = [&](TreeNode* t, vector<int>& res) {
        if (!t) return;
        inorder(t->l, res);
        res.push_back(t->val);
        inorder(t->r, res);
    };
    function<void(TreeNode*)> free_tree = [&](TreeNode* t) {
        if (!t) return;
        free_tree(t->l); free_tree(t->r); delete t;
    };
    TreeNode* root = nullptr;
    for (int x : a) root = insert(root, x);
    vector<int> sorted;
    inorder(root, sorted);
    for (int i = 0; i < (int)a.size(); i++) a[i] = sorted[i];
    free_tree(root);
}

// --- D.36. Wiggle Sort ---
// a[0] ≤ a[1] ≥ a[2] ≤ a[3] ≥ ... — один проход swap'ами.
void wiggle_sort(vector<int>& a) {
    for (int i = 1; i < (int)a.size(); i++) {
        if ((i % 2 == 1 && a[i - 1] > a[i]) || (i % 2 == 0 && a[i - 1] < a[i]))
            swap(a[i - 1], a[i]);
    }
}

}; // struct Sortings

#endif // TECHNIQUE_A_CPP
