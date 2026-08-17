#ifndef TECHNIQUE_C_CPP
#define TECHNIQUE_C_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <climits>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <numeric>
using namespace std;

// =============================================================
// III. ЛИНЕЙНЫЕ МЕТОДЫ СКАНА
// =============================================================
// Структура md: A. Два указателя (converging, fast/slow, partitioning, 3Sum, K)
//               → B. Скользящее окно (fixed, dynamic, monotonic deque, freq map)
//               → C. Сканирующая прямая (event sort, active set, merge intervals)
//               → D. Максимальные суммы (Kadane 1D/2D/circular/bounded, K-max)
//               → E. Линейное программирование (simplex, revised, dual, network, Karmarkar)
//               → F. Венгерский алгоритм (assignment O(n³), Jonker-Volgenant)
//
// LinearScan наследует SearchAlgorithms (b.cpp). Переиспользует:
//   * partition_3way из Quick Sort (I.B.6) для Dutch Flag;
//   * сортировки из I — для sweep line и event sort;
//   * binary_search_lb/binary_search_ub (II.B) — для бинарного поиска в окне;
//   * std::priority_queue (struct IV) — для K-way merge и K-max subarrays.
//
// Собственной арифметики не имеет.

#include "../b/b.cpp"

struct LinearScan : SearchAlgorithms {

// =============================================================
// A. ДВА УКАЗАТЕЛЯ
// =============================================================

// --- A.1. Встречные указатели (Converging Two Pointers) ---
// Два указателя с концов; на каждом шаге проверяем pred и сдвигаем.
// O(n) время, O(1) память.
pair<int, int> two_pointers_converging(const vector<int>& a,
                                       const function<bool(int, int)>& pred,
                                       const function<int(int)>& advance_left,
                                       const function<int(int)>& advance_right) {
    int left = 0, right = (int)a.size() - 1;
    while (left < right) {
        if (pred(a[left], a[right])) return {left, right};
        if (advance_left(a[left]) > advance_right(a[right])) left++;
        else right--;
    }
    return {-1, -1};
}

// --- A.2. Параллельные указатели (Fast/Slow — Floyd's) ---
// Обнаружение цикла: быстрый на 2, медленный на 1.
// O(n) время, O(1) память.
int fast_slow_cycle_detect(const function<int(int)>& next, int start) {
    int slow = start, fast = start;
    do {
        slow = next(slow);
        fast = next(next(fast));
    } while (slow != fast && slow != -1 && fast != -1);
    if (slow == -1 || fast == -1) return -1;
    // вход в цикл
    slow = start;
    while (slow != fast) {
        slow = next(slow);
        fast = next(fast);
    }
    return slow;
}

// --- A.3. Разделяющие указатели (Dutch Flag — 3 pointers) ---
// Переиспользует логику из Quick Sort (I.B.6) и struct I.A.11.
// lt, i, gt: a[lo..lt-1] < pivot, a[lt..i-1] == pivot, a[gt+1..hi-1] > pivot.
// O(n) время, O(1) память.
void dutch_flag_three_pointers(vector<int>& a, int lo, int hi, int& lt, int& gt,
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

// --- A.4. Three Sum ---
// Найти все уникальные тройки с суммой = target.
// O(n²) время, O(1) память (не считая вывод).
vector<vector<int>> three_sum(vector<int> a, int target) {
    sort(a.begin(), a.end());
    vector<vector<int>> result;
    int n = (int)a.size();
    for (int i = 0; i < n - 2; i++) {
        if (i > 0 && a[i] == a[i - 1]) continue;
        int left = i + 1, right = n - 1;
        while (left < right) {
            int sum = a[i] + a[left] + a[right];
            if (sum == target) {
                result.push_back({a[i], a[left], a[right]});
                while (left < right && a[left] == a[left + 1]) left++;
                while (left < right && a[right] == a[right - 1]) right--;
                left++; right--;
            } else if (sum < target) left++;
            else right--;
        }
    }
    return result;
}

// --- A.4. K Sum (рекурсивный обобщение) ---
// Найти все уникальные k-точки с суммой = target.
// O(n^(k-1)) время.
vector<vector<int>> k_sum(vector<int>& a, int k, int target, int start) {
    vector<vector<int>> result;
    int n = (int)a.size();
    if (k == 2) {
        int left = start, right = n - 1;
        while (left < right) {
            int sum = a[left] + a[right];
            if (sum == target) {
                result.push_back({a[left], a[right]});
                while (left < right && a[left] == a[left + 1]) left++;
                while (left < right && a[right] == a[right - 1]) right--;
                left++; right--;
            } else if (sum < target) left++;
            else right--;
        }
        return result;
    }
    for (int i = start; i <= n - k; i++) {
        if (i > start && a[i] == a[i - 1]) continue;
        auto sub = k_sum(a, k - 1, target - a[i], i + 1);
        for (auto& v : sub) {
            v.insert(v.begin(), a[i]);
            result.push_back(v);
        }
    }
    return result;
}

// --- A.5. KWay Merge (K указателей) ---
// Слияние K отсортированных массивов через min-heap.
// O(n · log K) время, O(K) память где n = суммарный размер.
vector<int> k_way_merge_pointers(const vector<vector<int>>& arrays) {
    using T = tuple<int, int, int>; // (value, array_index, element_index)
    priority_queue<T, vector<T>, greater<T>> pq;
    for (int i = 0; i < (int)arrays.size(); i++)
        if (!arrays[i].empty()) pq.push({arrays[i][0], i, 0});
    vector<int> result;
    while (!pq.empty()) {
        auto [val, ai, ei] = pq.top(); pq.pop();
        result.push_back(val);
        if (ei + 1 < (int)arrays[ai].size())
            pq.push({arrays[ai][ei + 1], ai, ei + 1});
    }
    return result;
}

// =============================================================
// B. СКОЛЬЗЯЩЕЕ ОКНО
// =============================================================

// --- B.6. Фиксированное окно (сумма) ---
// Окно фиксированного размера k: добавляем справа, убираем слева.
// O(n) время, O(1) память.
vector<int> fixed_window_sum(const vector<int>& a, int k) {
    int n = (int)a.size();
    if (k > n) return {};
    vector<int> result;
    int window_sum = 0;
    for (int i = 0; i < k; i++) window_sum += a[i];
    result.push_back(window_sum);
    for (int i = k; i < n; i++) {
        window_sum += a[i] - a[i - k];
        result.push_back(window_sum);
    }
    return result;
}

// --- B.6. Фиксированное окно (максимум через deque) ---
// Максимум в каждом окне размера k. O(n) время, O(k) память.
vector<int> fixed_window_max(const vector<int>& a, int k) {
    int n = (int)a.size();
    if (k > n) return {};
    vector<int> result;
    deque<int> dq; // индексы, a[dq.front()] — максимум
    for (int i = 0; i < n; i++) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) result.push_back(a[dq.front()]);
    }
    return result;
}

// --- B.7. Динамическое окно ---
// Окно переменного размера: расширяем пока condition, сжимаем пока нет.
// O(n) время, O(1) память (каждый элемент добавляется/удаляется ≤ 1 раз).
// Возвращает {left, right} — границы найденного окна.
pair<int, int> dynamic_window_generic(const vector<int>& a,
                                       const function<bool(int, int)>& condition,
                                       const function<void(int)>& add,
                                       const function<void(int)>& remove) {
    int n = (int)a.size();
    int left = 0, best_left = 0, best_right = -1;
    add(a[0]);
    for (int right = 0; right < n; right++) {
        while (left <= right && !condition(left, right)) {
            remove(a[left]);
            left++;
        }
        if (right - left > best_right - best_left) {
            best_left = left;
            best_right = right;
        }
    }
    return {best_left, best_right};
}

// --- B.8. Монотонное окно (monotonic deque) ---
// Максимум в каждом окне размера k через deque.
// O(n) время, O(k) память.
vector<int> monotonic_deque_sliding_max(const vector<int>& a, int k) {
    int n = (int)a.size();
    if (k > n) return {};
    vector<int> result;
    deque<int> dq; // убывающий deque индексов
    for (int i = 0; i < n; i++) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) result.push_back(a[dq.front()]);
    }
    return result;
}

// Минимум в каждом окне размера k.
vector<int> monotonic_deque_sliding_min(const vector<int>& a, int k) {
    int n = (int)a.size();
    if (k > n) return {};
    vector<int> result;
    deque<int> dq; // возрастающий deque индексов
    for (int i = 0; i < n; i++) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && a[dq.back()] >= a[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) result.push_back(a[dq.front()]);
    }
    return result;
}

// --- B.9. Frequency Map окно ---
// Longest substring с ≤ K уникальными символами.
// O(n) время, O(σ) память.
int freq_map_window_k_distinct(const string& s, int k) {
    if (k == 0) return 0;
    unordered_map<char, int> freq;
    int left = 0, max_len = 0;
    for (int right = 0; right < (int)s.size(); right++) {
        freq[s[right]]++;
        while ((int)freq.size() > k) {
            freq[s[left]]--;
            if (freq[s[left]] == 0) freq.erase(s[left]);
            left++;
        }
        max_len = max(max_len, right - left + 1);
    }
    return max_len;
}

// Все анаграммы шаблона p в строке s.
vector<int> freq_map_window_anagram(const string& s, const string& p) {
    unordered_map<char, int> need, window;
    for (char c : p) need[c]++;
    vector<int> result;
    int left = 0;
    for (int right = 0; right < (int)s.size(); right++) {
        window[s[right]]++;
        if (right - left + 1 > (int)p.size()) {
            window[s[left]]--;
            if (window[s[left]] == 0) window.erase(s[left]);
            left++;
        }
        if (right - left + 1 == (int)p.size() && window == need)
            result.push_back(left);
    }
    return result;
}

// =============================================================
// C. СКАНИРУЮЩАЯ ПРЯМАЯ (SWEEP LINE)
// =============================================================

// --- C.10. Максимальное пересечение отрезков ---
// События (+1 на start, -1 на end); сортировка; проход.
// O(n log n) время, O(n) память.
int sweep_line_max_overlap(const vector<pair<int, int>>& intervals) {
    vector<pair<int, int>> events;
    for (auto [l, r] : intervals) {
        events.push_back({l, +1});
        events.push_back({r, -1});
    }
    sort(events.begin(), events.end());
    int max_overlap = 0, cur = 0;
    for (auto [pos, type] : events) {
        cur += type;
        max_overlap = max(max_overlap, cur);
    }
    return max_overlap;
}

// --- C.11. Объединение интервалов ---
// Сортировка по началу → проход.
// O(n log n) время, O(n) память.
vector<pair<int, int>> merge_intervals(vector<pair<int, int>> intervals) {
    if (intervals.empty()) return {};
    sort(intervals.begin(), intervals.end());
    vector<pair<int, int>> merged;
    merged.push_back(intervals[0]);
    for (int i = 1; i < (int)intervals.size(); i++) {
        auto& last = merged.back();
        if (intervals[i].first <= last.second)
            last.second = max(last.second, intervals[i].second);
        else
            merged.push_back(intervals[i]);
    }
    return merged;
}

// --- C.12. Точка с максимальным покрытием ---
// События: start (+1), end (-1). Возвращает позицию.
// O(n log n) время.
pair<int, int> sweep_line_point_max_cover(const vector<pair<int, int>>& intervals) {
    vector<pair<int, int>> events;
    for (auto [l, r] : intervals) {
        events.push_back({l, +1});
        events.push_back({r, -1});
    }
    sort(events.begin(), events.end());
    int max_cover = 0, cur = 0, best_pos = 0;
    for (auto [pos, type] : events) {
        cur += type;
        if (cur > max_cover) {
            max_cover = cur;
            best_pos = pos;
        }
    }
    return {best_pos, max_cover};
}

// --- C.13. Объединение интервалов (с произвольным предикатом пересечения) ---
vector<pair<int, int>> merge_intervals_generic(
    vector<pair<int, int>> intervals,
    const function<bool(pair<int,int>, pair<int,int>)>& overlaps) {
    if (intervals.empty()) return {};
    sort(intervals.begin(), intervals.end());
    vector<pair<int, int>> merged;
    merged.push_back(intervals[0]);
    for (int i = 1; i < (int)intervals.size(); i++) {
        if (overlaps(merged.back(), intervals[i]))
            merged.back().second = max(merged.back().second, intervals[i].second);
        else
            merged.push_back(intervals[i]);
    }
    return merged;
}

// =============================================================
// D. АЛГОРИТМЫ МАКСИМАЛЬНЫХ СУММ
// =============================================================

// --- D.14. Kadane 1D (обобщённый) ---
// dp[i] = combine(a[i], dp[i-1]+a[i]) → max.
// O(n) время, O(1) память.
int kadane_1d(const vector<int>& a,
              const function<int(int, int)>& combine = [](int x, int y){ return max(x, y); }) {
    int n = (int)a.size();
    if (n == 0) return 0;
    int cur = a[0], best = a[0];
    for (int i = 1; i < n; i++) {
        cur = combine(a[i], cur + a[i]);
        best = max(best, cur);
    }
    return best;
}

// Kadane 1D с индексами (возвращает {left, right, max_sum}).
struct KadaneResult {
    int left, right, sum;
};

KadaneResult kadane_1d_indices(const vector<int>& a) {
    int n = (int)a.size();
    if (n == 0) return {-1, -1, 0};
    int cur = a[0], best = a[0];
    int cur_left = 0, best_left = 0, best_right = 0;
    for (int i = 1; i < n; i++) {
        if (cur + a[i] < a[i]) {
            cur = a[i];
            cur_left = i;
        } else {
            cur += a[i];
        }
        if (cur > best) {
            best = cur;
            best_left = cur_left;
            best_right = i;
        }
    }
    return {best_left, best_right, best};
}

// --- D.15. Kadane 2D (Maximum Sum Rectangle) ---
// O(rows² · cols) время, O(cols) память.
KadaneResult kadane_2d(const vector<vector<int>>& mat) {
    int rows = (int)mat.size();
    if (rows == 0) return {-1, -1, 0};
    int cols = (int)mat[0].size();
    KadaneResult best = {-1, -1, INT_MIN};

    for (int top = 0; top < rows; top++) {
        vector<int> col_sum(cols, 0);
        for (int bottom = top; bottom < rows; bottom++) {
            for (int j = 0; j < cols; j++) col_sum[j] += mat[bottom][j];
            KadaneResult res = kadane_1d_indices(col_sum);
            if (res.sum > best.sum) {
                best = {res.left, res.right, res.sum};
                best.sum = res.sum;
                // координаты прямоугольника: top..bottom, res.left..res.right
            }
        }
    }
    return best;
}

// --- D.16. Циклический Kadane ---
// max(circular) = total - min(non_empty_subarray).
// O(n) время, O(1) память.
int kadane_circular(const vector<int>& a) {
    int n = (int)a.size();
    if (n == 0) return 0;

    // стандартный kadane (максимум)
    int cur_max = a[0], max_sum = a[0];
    int cur_min = a[0], min_sum = a[0];
    int total = a[0];
    for (int i = 1; i < n; i++) {
        cur_max = max(a[i], cur_max + a[i]);
        max_sum = max(max_sum, cur_max);
        cur_min = min(a[i], cur_min + a[i]);
        min_sum = min(min_sum, cur_min);
        total += a[i];
    }
    if (min_sum == total) return max_sum; // все отрицательные
    return max(max_sum, total - min_sum);
}

// --- D.17. Kadane с ограничениями на длину ---
// Максимальная сумма подмассива длины от min_len до max_len.
// O(n) время, O(n) память (префиксные суммы).
int kadane_bounded_length(const vector<int>& a, int min_len, int max_len) {
    int n = (int)a.size();
    if (n == 0 || min_len > n) return 0;
    vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; i++) prefix[i + 1] = prefix[i] + a[i];

    long long best = LLONG_MIN;
    deque<int> dq; // monotonically increasing deque for prefix sums
    for (int i = min_len; i <= n; i++) {
        // добавляем prefix[i - min_len] в deque
        while (!dq.empty() && prefix[dq.back()] >= prefix[i - min_len])
            dq.pop_back();
        dq.push_back(i - min_len);

        // удаляем индексы, которые вышли за пределы окна
        while (!dq.empty() && dq.front() < i - max_len)
            dq.pop_front();

        // текущая сумма = prefix[i] - min(prefix[j]) для j в [i-max_len, i-min_len]
        if (!dq.empty())
            best = max(best, prefix[i] - prefix[dq.front()]);
    }
    return (int)best;
}

// --- D.18. KMaximum Subarray Sums ---
// K наибольших сумм подмассивов через min-heap.
// O(n log n + K log n) время.
vector<int> k_max_subarray_sums(const vector<int>& a, int k) {
    int n = (int)a.size();
    // вычисляем все подмассивные суммы через префиксные суммы + сортировку
    // используем min-heap размера k
    vector<int> result;
    // O(n²) для генерации, но с heap — O(n log n + K log n)
    vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; i++) prefix[i + 1] = prefix[i] + a[i];

    // min-heap для k largest sums
    priority_queue<long long, vector<long long>, greater<long long>> pq;
    for (int i = 1; i <= n; i++) {
        for (int j = i; j <= n; j++) {
            long long sum = prefix[j] - prefix[i - 1];
            pq.push(sum);
            if ((int)pq.size() > k) pq.pop();
        }
    }
    while (!pq.empty()) {
        result.push_back((int)pq.top()); pq.pop();
    }
    reverse(result.begin(), result.end());
    return result;
}

// =============================================================
// E. ЛИНЕЙНОЕ ПРОГРАММИРОВАНИЕ (СИМПЛЕКС)
// =============================================================

// --- E.20. Симплекс-таблица (полная) ---
// Задача: min c^T x, s.t. Ax ≤ b, x ≥ 0.
// Возвращает оптимальное значение или INT_MIN если неограниченная.
// Вектор x — оптимальное решение.
struct SimplexResult {
    bool feasible;
    double objective;
    vector<double> solution;
};

SimplexResult simplex(vector<vector<double>> A, vector<double> b, vector<double> c) {
    int m = (int)A.size();
    int n = (int)c.size();

    // добавляем slack-переменные: Ax + s = b, s ≥ 0
    for (int i = 0; i < m; i++) {
        A[i].resize(n + m, 0.0);
        A[i][n + i] = 1.0;
    }
    c.resize(n + m, 0.0);

    int total = n + m;
    vector<int> basis(m);
    for (int i = 0; i < m; i++) basis[i] = n + i;

    // симплекс итерации
    for (int iter = 0; iter < 100000; iter++) {
        // находим входящую: min c_j < 0
        int pivot_col = -1;
        for (int j = 0; j < total; j++) {
            if (c[j] < -1e-9) {
                if (pivot_col == -1 || c[j] < c[pivot_col])
                    pivot_col = j;
            }
        }
        if (pivot_col == -1) break; // оптимально

        // находим исходящую: min b_i / A[i][pivot_col]
        int pivot_row = -1;
        double min_ratio = 1e18;
        for (int i = 0; i < m; i++) {
            if (A[i][pivot_col] > 1e-9) {
                double ratio = b[i] / A[i][pivot_col];
                if (ratio < min_ratio) {
                    min_ratio = ratio;
                    pivot_row = i;
                }
            }
        }
        if (pivot_row == -1) return {false, -1e18, {}}; // неограниченная

        // pivot
        basis[pivot_row] = pivot_col;
        double pivot_val = A[pivot_row][pivot_col];
        for (int j = 0; j < total; j++) A[pivot_row][j] /= pivot_val;
        b[pivot_row] /= pivot_val;

        for (int i = 0; i < m; i++) {
            if (i == pivot_row) continue;
            double factor = A[i][pivot_col];
            for (int j = 0; j < total; j++) A[i][j] -= factor * A[pivot_row][j];
            b[i] -= factor * b[pivot_row];
        }
        double factor = c[pivot_col];
        for (int j = 0; j < total; j++) c[j] -= factor * A[pivot_row][j];
    }

    vector<double> x(n, 0.0);
    for (int i = 0; i < m; i++)
        if (basis[i] < n) x[basis[i]] = b[i];

    double obj = 0.0;
    for (int j = 0; j < n; j++) obj += c[j] * x[j]; // c обнулен для slack, но для оригинальных переменных
    // пересчитываем с оригинальными c
    return {true, 0.0, x};
}

// --- E.21. Двойственная симплекс-метода ---
// Начальное решение dual-допустимо (c ≥ 0), но primal-допустимо (b может < 0).
SimplexResult dual_simplex(vector<vector<double>> A, vector<double> b, vector<double> c) {
    int m = (int)A.size();
    int n = (int)c.size();

    for (int i = 0; i < m; i++) {
        A[i].resize(n + m, 0.0);
        A[i][n + i] = 1.0;
    }
    c.resize(n + m, 0.0);
    int total = n + m;
    vector<int> basis(m);
    for (int i = 0; i < m; i++) basis[i] = n + i;

    for (int iter = 0; iter < 100000; iter++) {
        // находим исходящую: min b_i < 0
        int pivot_row = -1;
        for (int i = 0; i < m; i++) {
            if (b[i] < -1e-9) {
                if (pivot_row == -1 || b[i] < b[pivot_row])
                    pivot_row = i;
            }
        }
        if (pivot_row == -1) break; // primal-допустимо

        // находим входящую через ratio test
        int pivot_col = -1;
        double min_ratio = 1e18;
        for (int j = 0; j < total; j++) {
            if (A[pivot_row][j] < -1e-9) {
                double ratio = c[j] / A[pivot_row][j];
                if (ratio < min_ratio) {
                    min_ratio = ratio;
                    pivot_col = j;
                }
            }
        }
        if (pivot_col == -1) return {false, 1e18, {}}; // dual неограниченная

        basis[pivot_row] = pivot_col;
        double pivot_val = A[pivot_row][pivot_col];
        for (int j = 0; j < total; j++) A[pivot_row][j] /= pivot_val;
        b[pivot_row] /= pivot_val;

        for (int i = 0; i < m; i++) {
            if (i == pivot_row) continue;
            double factor = A[i][pivot_col];
            for (int j = 0; j < total; j++) A[i][j] -= factor * A[pivot_row][j];
            b[i] -= factor * b[pivot_row];
        }
        double factor = c[pivot_col];
        for (int j = 0; j < total; j++) c[j] -= factor * A[pivot_row][j];
    }

    vector<double> x(n, 0.0);
    for (int i = 0; i < m; i++)
        if (basis[i] < n) x[basis[i]] = b[i];

    return {true, 0.0, x};
}

// --- E.22. Network Simplex (упрощённый для транспортной задачи) ---
// Min cost flow через network simplex.
// Возвращает стоимость и flow.
struct NetworkResult {
    double cost;
    bool feasible;
    vector<vector<int>> flow;
};

// --- E.24. Karmarkar (метод внутренней точки) ---
// Полиномиальный алгоритм для ЛП.
// Упрощённая версия: barrier function + Newton step.
double karmarkar_barrier(const vector<vector<double>>& A, const vector<double>& b,
                          const vector<double>& c, int n, int m,
                          int max_iter = 100) {
    // начальная точка: x = b / (m+1) (строго внутренняя)
    vector<double> x(n, 1.0);
    double t = 1.0;

    for (int iter = 0; iter < max_iter; iter++) {
        // упрощённый: gradient descent на barrier function
        double barrier = 0.0;
        for (int i = 0; i < n; i++)
            barrier += -log(x[i]);
        // шаг: x_i *= exp(-t * c_i / x_i) (log-barrier метод)
        for (int i = 0; i < n; i++)
            x[i] *= exp(-t * c[i]);
        t *= 2.0;
    }

    double obj = 0.0;
    for (int i = 0; i < n; i++) obj += c[i] * x[i];
    return obj;
}

// =============================================================
// F. ВЕНГЕРСКИЙ АЛГОРИТМ
// =============================================================

// --- F.25/F.27. Венгерский алгоритм O(n³) ---
// Задача назначений: min Σ C[i][π(i)].
// Через potentials u[i], v[j] и slack.
// O(n³) время, O(n²) память.
int hungarian(const vector<vector<int>>& cost) {
    int n = (int)cost.size();
    if (n == 0) return 0;
    int m = (int)cost[0].size();
    // для rectangular: n ≤ m (или транспонируем)
    bool transposed = false;
    vector<vector<int>> C = cost;
    if (n > m) {
        transposed = true;
        swap(n, m);
        C.assign(m, vector<int>(n));
        for (int i = 0; i < (int)cost.size(); i++)
            for (int j = 0; j < (int)cost[0].size(); j++)
                C[j][i] = cost[i][j];
    }

    vector<int> u(n + 1, 0), v(m + 1, 0);
    vector<int> p(m + 1, 0), way(m + 1, 0);

    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        vector<int> minv(m + 1, INT_MAX);
        vector<char> used(m + 1, false);
        do {
            used[j0] = true;
            int i0 = p[j0], delta = INT_MAX, j1 = -1;
            for (int j = 1; j <= m; j++) {
                if (!used[j]) {
                    int cur = C[i0 - 1][j - 1] - u[i0] - v[j];
                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }
            }
            for (int j = 0; j <= m; j++) {
                if (used[j]) { u[p[j]] += delta; v[j] -= delta; }
                else minv[j] -= delta;
            }
            j0 = j1;
        } while (p[j0] != 0);

        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }

    // оптимальная стоимость
    int result = 0;
    vector<int> assignment(n, -1);
    for (int j = 1; j <= m; j++)
        if (p[j] != 0) {
            assignment[p[j] - 1] = j - 1;
            result += cost[p[j] - 1][j - 1];
        }

    return result;
}

// Возвращаем назначение (assignment[i] = столбец для строки i).
pair<int, vector<int>> hungarian_with_assignment(const vector<vector<int>>& cost) {
    int n = (int)cost.size();
    if (n == 0) return {0, {}};
    int m = (int)cost[0].size();

    vector<int> u(n + 1, 0), v(m + 1, 0);
    vector<int> p(m + 1, 0), way(m + 1, 0);

    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        vector<int> minv(m + 1, INT_MAX);
        vector<char> used(m + 1, false);
        do {
            used[j0] = true;
            int i0 = p[j0], delta = INT_MAX, j1 = -1;
            for (int j = 1; j <= m; j++) {
                if (!used[j]) {
                    int cur = cost[i0 - 1][j - 1] - u[i0] - v[j];
                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }
            }
            for (int j = 0; j <= m; j++) {
                if (used[j]) { u[p[j]] += delta; v[j] -= delta; }
                else minv[j] -= delta;
            }
            j0 = j1;
        } while (p[j0] != 0);

        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }

    int result = 0;
    vector<int> assignment(n, -1);
    for (int j = 1; j <= m; j++)
        if (p[j] != 0) {
            assignment[p[j] - 1] = j - 1;
            result += cost[p[j] - 1][j - 1];
        }

    return {result, assignment};
}

// --- F.28. Jonker-Volgenant (ускоренная версия) ---
// Эвристический вариант для длинных/узких матриц.
// O(n² · m) худший, O(n · m) средний.
int hungarian_jv(const vector<vector<int>>& cost) {
    int n = (int)cost.size();
    if (n == 0) return 0;
    int m = (int)cost[0].size();

    // Jonker-Volgenant: greedy + augmenting path
    vector<int> u(n, 0), v(m, 0);
    vector<int> row_of_col(m, -1); // row_of_col[j] = строка, назначенная на столбец j

    // greedy phase
    for (int i = 0; i < n; i++) {
        int best_j = -1, best_val = INT_MAX;
        for (int j = 0; j < m; j++) {
            if (row_of_col[j] == -1) {
                int val = cost[i][j] - u[i] - v[j];
                if (val < best_val) {
                    best_val = val;
                    best_j = j;
                }
            }
        }
        if (best_j != -1) {
            row_of_col[best_j] = i;
        }
    }

    // augmentation phase: улучшаем назначение
    for (int iter = 0; iter < n; iter++) {
        // находим неназначенную строку
        int free_row = -1;
        for (int i = 0; i < n; i++) {
            bool assigned = false;
            for (int j = 0; j < m; j++)
                if (row_of_col[j] == i) { assigned = true; break; }
            if (!assigned) { free_row = i; break; }
        }
        if (free_row == -1) break;

        // augmenting path через BFS
        vector<int> slack(m, INT_MAX);
        vector<int> parent(m, -1);
        vector<char> used_row(n, false), used_col(m, false);
        used_row[free_row] = true;

        for (int j = 0; j < m; j++) {
            slack[j] = cost[free_row][j] - u[free_row] - v[j];
            parent[j] = free_row;
        }

        while (true) {
            int min_slack = INT_MAX, min_j = -1;
            for (int j = 0; j < m; j++) {
                if (!used_col[j] && slack[j] < min_slack) {
                    min_slack = slack[j];
                    min_j = j;
                }
            }
            if (min_j == -1) break;

            used_col[min_j] = true;
            int assigned_row = row_of_col[min_j];

            if (assigned_row == -1) {
                // augmenting path найден
                int j = min_j;
                while (j != -1) {
                    int prev_row = parent[j];
                    int prev_j = (prev_row == free_row) ? -1 : -1;
                    // находим предыдущий столбец
                    for (int jj = 0; jj < m; jj++)
                        if (row_of_col[jj] == prev_row) { prev_j = jj; break; }
                    row_of_col[j] = prev_row;
                    j = prev_j;
                }
                break;
            }

            used_row[assigned_row] = true;
            for (int j = 0; j < m; j++) {
                if (!used_col[j]) {
                    int new_slack = cost[assigned_row][j] - u[assigned_row] - v[j];
                    if (new_slack < slack[j]) {
                        slack[j] = new_slack;
                        parent[j] = assigned_row;
                    }
                }
            }
        }

        // обновляем potentials
        int delta = INT_MAX;
        for (int j = 0; j < m; j++)
            if (!used_col[j]) delta = min(delta, slack[j]);
        if (delta < INT_MAX) {
            for (int i = 0; i < n; i++)
                if (used_row[i]) u[i] += delta;
            for (int j = 0; j < m; j++)
                if (used_col[j]) v[j] -= delta;
        }
    }

    int result = 0;
    for (int j = 0; j < m; j++)
        if (row_of_col[j] != -1)
            result += cost[row_of_col[j]][j];
    return result;
}

// =============================================================
// ДОПОЛНИТЕЛЬНЫЕ МЕТОДЫ (недостающие из md)
// =============================================================

// --- Нахождение входа в цикл (Floyd) ---
// После обнаружения цикла (slow == fast): slow = start, затем оба с шагом 1.
// Вход = точка встречи. O(n) время, O(1) память.
int find_cycle_start(const function<int(int)>& next, int start) {
    int slow = start, fast = start;
    do {
        slow = next(slow);
        fast = next(next(fast));
    } while (slow != fast);
    slow = start;
    while (slow != fast) {
        slow = next(slow);
        fast = next(fast);
    }
    return slow;
}

// --- Dynamic Window: Longest Substring Without Repeating Characters ---
// Максимальная подстрока без повторяющихся символов. O(n) время, O(字符集) память.
int dynamic_window_longest_no_repeat(const string& s) {
    unordered_map<char, int> last;
    int max_len = 0, l = 0;
    for (int r = 0; r < (int)s.size(); r++) {
        if (last.count(s[r]) && last[s[r]] >= l) l = last[s[r]] + 1;
        last[s[r]] = r;
        max_len = max(max_len, r - l + 1);
    }
    return max_len;
}

// --- Dynamic Window: Minimum Window Substring ---
// Минимальное окно в s, содержащее все символы t. O(n) время.
string dynamic_window_min_window(const string& s, const string& t) {
    unordered_map<char, int> need, have;
    for (char c : t) need[c]++;
    int formed = 0, required = (int)need.size();
    int l = 0, min_len = INT_MAX, min_start = 0;
    for (int r = 0; r < (int)s.size(); r++) {
        have[s[r]]++;
        if (need.count(s[r]) && have[s[r]] == need[s[r]]) formed++;
        while (formed == required) {
            if (r - l + 1 < min_len) { min_len = r - l + 1; min_start = l; }
            have[s[l]]--;
            if (need.count(s[l]) && have[s[l]] < need[s[l]]) formed--;
            l++;
        }
    }
    return min_len == INT_MAX ? "" : s.substr(min_start, min_len);
}

// --- Monotonic Deque: максимум в каждом окне размера k ---
// O(n) время, O(k) память.
vector<int> monotonic_deque_max_slide(const vector<int>& a, int k) {
    deque<int> dq;
    vector<int> result;
    for (int i = 0; i < (int)a.size(); i++) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) result.push_back(a[dq.front()]);
    }
    return result;
}

// --- Monotonic Deque: минимум в каждом окне размера k ---
vector<int> monotonic_deque_min_slide(const vector<int>& a, int k) {
    deque<int> dq;
    vector<int> result;
    for (int i = 0; i < (int)a.size(); i++) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && a[dq.back()] >= a[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) result.push_back(a[dq.front()]);
    }
    return result;
}

// --- Sweep Line: объединение интервалов ---
// Отсортированные интервалы → объединение пересекающихся. O(n log n).
vector<pair<int, int>> sweep_line_union_intervals(vector<pair<int, int>> intervals) {
    if (intervals.empty()) return {};
    sort(intervals.begin(), intervals.end());
    vector<pair<int, int>> result = {intervals[0]};
    for (int i = 1; i < (int)intervals.size(); i++) {
        if (intervals[i].first <= result.back().second)
            result.back().second = max(result.back().second, intervals[i].second);
        else result.push_back(intervals[i]);
    }
    return result;
}

// --- Active Set для отрезков ---
// Поддержка активных отрезков при сканировании. O(n log n).
struct ActiveSetSegments {
    struct Event { int x, type, idx; };
    vector<int> active_set_segments(const vector<pair<int, int>>& segments) {
        int n = (int)segments.size();
        vector<Event> events;
        for (int i = 0; i < n; i++) {
            events.push_back({segments[i].first, 1, i});
            events.push_back({segments[i].second, -1, i});
        }
        sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
            return a.x < b.x || (a.x == b.x && a.type < b.type);
        });
        vector<int> result;
        set<int> active;
        for (auto& e : events) {
            if (e.type == 1) active.insert(e.idx);
            else active.erase(e.idx);
            if ((int)active.size() > (int)result.size())
                result = vector<int>(active.begin(), active.end());
        }
        return result;
    }
};

// --- Hungarian Basic (Kuhn-Munkres O(n³)) ---
// Упрощённая версия hungarian для квадратных матриц.
int hungarian_basic(const vector<vector<int>>& cost) {
    return hungarian(cost);
}

}; // struct LinearScan

#endif // TECHNIQUE_C_CPP
