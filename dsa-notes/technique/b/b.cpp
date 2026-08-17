#ifndef TECHNIQUE_B_CPP
#define TECHNIQUE_B_CPP

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
#include <numeric>
using namespace std;

// =============================================================
// II. ПОИСК
// =============================================================
// Структура md: A. Линейный и последовательный
//               → B. Двоичный поиск
//               → C. Промежуточные (Jump, Interpolation, Fibonacci, Exponential)
//               → D. Троичный поиск
//               → E. Выбор медианы и k-й статистики
//               → F. Метаэвристики (Backtracking, Hill Climbing, SA, A*)
//               → G. Стриминговый поиск (Reservoir, Heavy Hitters)
//               → H. Параметрический и многокритериальный
//
// SearchAlgorithms наследует Sortings (a.cpp). Переиспользует:
//   * partition_3way из Quick Sort (I.B.6) для QuickSelect;
//   * сортировки из I — для предобработки перед поиском;
//   * std::priority_queue (struct IV) для HeapSelect и A*.
//
// Собственной арифметики не имеет.

#include "../a/a.cpp"

struct SearchAlgorithms : Sortings {

// =============================================================
// A. ЛИНЕЙНЫЙ И ПОСЛЕДОВАТЕЛЬНЫЙ ПОИСК
// =============================================================

// --- A.1. Sequential Search ---
// Проход по всем элементам. O(n) время, O(1) память.
// Возвращает индекс первого вхождения или -1.
int linear_search(const vector<int>& a, int target, int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    for (int i = lo; i < hi; i++)
        if (a[i] == target) return i;
    return -1;
}

// С предикатом: возвращает индекс первого элемента, для которого pred == true.
int linear_search_if(const vector<int>& a, const function<bool(int)>& pred,
                     int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    for (int i = lo; i < hi; i++)
        if (pred(a[i])) return i;
    return -1;
}

// --- A.2. Sentinel Search ---
// Искомый элемент в конце: убирает проверку границ.
int sentinel_search(vector<int> a, int target) {
    int n = (int)a.size();
    if (n == 0) return -1;
    int last = a[n - 1];
    a[n - 1] = target;
    int i = 0;
    while (a[i] != target) i++;
    a[n - 1] = last;
    if (i < n - 1 || a[n - 1] == target) return i;
    return -1;
}

// --- A.3. Double-ended Search ---
// Два указателя с концов; каждый ищет свою «половину».
// Возвращает пару индексов или {-1, -1}.
pair<int, int> double_ended_search(const vector<int>& a, int target) {
    int lo = 0, hi = (int)a.size() - 1;
    while (lo <= hi) {
        if (a[lo] == target && a[hi] == target) return {lo, hi};
        if (a[lo] == target) lo++;
        else if (a[hi] == target) hi--;
        else { lo++; hi--; }
    }
    return {-1, -1};
}

// --- A.2. Особые случаи: все вхождения ---
// Возвращает вектор индексов всех вхождений target.
vector<int> linear_search_all(const vector<int>& a, int target) {
    vector<int> result;
    for (int i = 0; i < (int)a.size(); i++)
        if (a[i] == target) result.push_back(i);
    return result;
}

// --- A.2. Особые случаи: подсчёт количества ---
// Возвращает количество вхождений target.
int linear_search_count(const vector<int>& a, int target) {
    int cnt = 0;
    for (int x : a)
        if (x == target) cnt++;
    return cnt;
}

// =============================================================
// B. ДВОИЧНЫЙ ПОИСК
// =============================================================

// --- B.4. Lower Bound: первый элемент ≥ target ---
// Инвариант: ответ ∈ [lo, hi]; при a[mid] < target: lo = mid + 1.
int binary_search_lb(const vector<int>& a, int target,
                     int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

// --- B.4. Upper Bound: первый элемент > target ---
int binary_search_ub(const vector<int>& a, int target,
                     int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] <= target) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

// --- B.4. Equal Range: [lower, upper) ---
pair<int, int> binary_search_eq(const vector<int>& a, int target,
                                int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    int lb = binary_search_lb(a, target, lo, hi);
    int ub = binary_search_ub(a, target, lb, hi);
    return {lb, ub};
}

// --- B.4. Бинарный поиск с произвольным компаратором ---
// cmp(a, b) — «a строго меньше b». Возвращает позицию вставки.
int binary_search_cmp(const vector<int>& a, int target,
                      const function<bool(int, int)>& cmp,
                      int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (cmp(a[mid], target)) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

// --- B.5. Рекурсивный бинарный поиск ---
// Divide & conquer: база lo == hi, рекурсия в одной половине.
// O(log n) время, O(log n) память (стек вызовов).
int binary_search_recursive_rec(const vector<int>& a, int target,
                                int lo, int hi) {
    if (lo >= hi) return -1;
    int mid = lo + (hi - lo) / 2;
    if (a[mid] == target) return mid;
    if (a[mid] < target) return binary_search_recursive_rec(a, target, mid + 1, hi);
    return binary_search_recursive_rec(a, target, lo, mid);
}

int binary_search_recursive(const vector<int>& a, int target) {
    return binary_search_recursive_rec(a, target, 0, (int)a.size());
}

// --- B.5. Бинарный поиск по ответу ---
// Предикат P(x) монотонен. Возвращает минимальный x из [lo_val, hi_val] такой, что P(x).
// Параметр check: функция проверки условия.
int binary_search_answer(int lo_val, int hi_val,
                         const function<bool(int)>& check) {
    while (lo_val < hi_val) {
        int mid = lo_val + (hi_val - lo_val) / 2;
        if (check(mid)) hi_val = mid;
        else lo_val = mid + 1;
    }
    return lo_val;
}

// Вещественная версия: возвращает x с точностью eps.
double binary_search_answer_real(double lo_val, double hi_val, double eps,
                                 const function<bool(double)>& check) {
    for (int iter = 0; iter < 200 && hi_val - lo_val > eps; iter++) {
        double mid = lo_val + (hi_val - lo_val) / 2.0;
        if (check(mid)) hi_val = mid;
        else lo_val = mid;
    }
    return lo_val;
}

// --- B.6. Вещественный бинарный поиск (поиск корня) ---
// f(lo) · f(hi) < 0 → корень на [lo, hi].
double binary_search_real(double lo, double hi, double eps,
                          const function<double(double)>& f) {
    double fl = f(lo);
    for (int iter = 0; iter < 200 && hi - lo > eps; iter++) {
        double mid = lo + (hi - lo) / 2.0;
        double fm = f(mid);
        if (fm == 0.0) return mid;
        if (fl * fm < 0) { hi = mid; }
        else { lo = mid; fl = fm; }
    }
    return lo + (hi - lo) / 2.0;
}

// --- B.7. Бинарный поиск в rotated массиве ---
// Массив повёрнут; одна половина гарантированно отсортирована.
int binary_search_rotated(const vector<int>& a, int target) {
    int lo = 0, hi = (int)a.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == target) return mid;
        if (a[lo] <= a[mid]) { // левая половина отсортирована
            if (a[lo] <= target && target < a[mid]) hi = mid - 1;
            else lo = mid + 1;
        } else { // правая половина отсортирована
            if (a[mid] < target && target <= a[hi]) lo = mid + 1;
            else hi = mid - 1;
        }
    }
    return -1;
}

// --- B.8. Бинарный поиск в 2D матрице ---
// Ступенчатый поиск: O(m + n).
bool search_matrix_sorted(const vector<vector<int>>& mat, int target) {
    if (mat.empty() || mat[0].empty()) return false;
    int r = 0, c = (int)mat[0].size() - 1;
    while (r < (int)mat.size() && c >= 0) {
        if (mat[r][c] == target) return true;
        if (mat[r][c] < target) r++;
        else c--;
    }
    return false;
}

// =============================================================
// C. ПРОМЕЖУТОЧНЫЕ АЛГОРИТМЫ ПОИСКА
// =============================================================

// --- C.9. Jump Search ---
// Прыжки √n, затем линейный поиск. O(√n) время, O(1) память.
int jump_search(const vector<int>& a, int target) {
    int n = (int)a.size();
    int step = (int)sqrt(n);
    int prev = 0;
    while (a[min(step, n) - 1] < target) {
        prev = step;
        step += (int)sqrt(n);
        if (prev >= n) return -1;
    }
    while (prev < min(step, n)) {
        if (a[prev] == target) return prev;
        prev++;
    }
    return -1;
}

// --- C.10. Interpolation Search ---
// Предсказание через интерполяцию. O(log log n) среднее, O(n) худшее.
int interpolation_search(const vector<int>& a, int target) {
    int lo = 0, hi = (int)a.size() - 1;
    while (lo <= hi && target >= a[lo] && target <= a[hi]) {
        if (lo == hi) return (a[lo] == target) ? lo : -1;
        int pos = lo + (int)(((double)(hi - lo) / (a[hi] - a[lo])) * (target - a[lo]));
        if (a[pos] == target) return pos;
        if (a[pos] < target) lo = pos + 1;
        else hi = pos - 1;
    }
    return -1;
}

// --- C.11. Fibonacci Search ---
// Деление через числа Фибоначчи. O(log n), O(1) память.
int fibonacci_search(const vector<int>& a, int target) {
    int n = (int)a.size();
    if (n == 0) return -1;
    int f2 = 0, f1 = 1, f = f1 + f2;
    while (f < n) { f2 = f1; f1 = f; f = f1 + f2; }
    int offset = -1;
    while (f > 1) {
        int i = min(offset + f2, n - 1);
        if (a[i] < target) { f = f1; f1 = f2; f2 = f - f1; offset = i; }
        else if (a[i] > target) { f = f2; f1 = f1 - f2; f2 = f - f1; }
        else return i;
    }
    if (f1 && offset + 1 < n && a[offset + 1] == target) return offset + 1;
    return -1;
}

// --- C.12. Exponential Search ---
// Экспоненциальный поиск границы + бинарный поиск. O(log pos).
int exponential_search(const vector<int>& a, int target) {
    int n = (int)a.size();
    if (n == 0) return -1;
    if (a[0] == target) return 0;
    int i = 1;
    while (i < n && a[i] <= target) i *= 2;
    // бинарный поиск в [i/2, min(i, n-1)]
    int lo = i / 2, hi = min(i, n - 1);
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == target) return mid;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// =============================================================
// D. ТРОИЧНЫЙ ПОИСК
// =============================================================

// --- D.13. Ternary Search (целочисленный) ---
// Для унимодальной функции: отбрасываем треть на каждом шаге.
// Возвращает позицию максимума.
int ternary_search_int(const vector<int>& a) {
    int lo = 0, hi = (int)a.size() - 1;
    while (hi - lo > 2) {
        int m1 = lo + (hi - lo) / 3;
        int m2 = hi - (hi - lo) / 3;
        if (a[m1] < a[m2]) lo = m1 + 1;
        else hi = m2 - 1;
    }
    int best = lo;
    for (int i = lo + 1; i <= hi; i++)
        if (a[i] > a[best]) best = i;
    return best;
}

// --- D.13. Ternary Search (вещественный) ---
double ternary_search_real(double lo, double hi, double eps,
                           const function<double(double)>& f) {
    while (hi - lo > eps) {
        double m1 = lo + (hi - lo) / 3.0;
        double m2 = hi - (hi - lo) / 3.0;
        if (f(m1) < f(m2)) lo = m1;
        else hi = m2;
    }
    return (lo + hi) / 2.0;
}

// --- D.14. Golden Section Search ---
// Оптимальное деление: φ = (1+√5)/2; одна новая оценка на итерацию.
double golden_section_search(double lo, double hi, double eps,
                             const function<double(double)>& f) {
    const double phi = (1.0 + sqrt(5.0)) / 2.0;
    double x1 = hi - (hi - lo) / phi;
    double x2 = lo + (hi - lo) / phi;
    double f1 = f(x1), f2 = f(x2);
    while (hi - lo > eps) {
        if (f1 < f2) {
            hi = x2;
            x2 = x1; f2 = f1;
            x1 = hi - (hi - lo) / phi;
            f1 = f(x1);
        } else {
            lo = x1;
            x1 = x2; f1 = f2;
            x2 = lo + (hi - lo) / phi;
            f2 = f(x2);
        }
    }
    return (lo + hi) / 2.0;
}

// =============================================================
// E. ВЫБОР МЕДИАНЫ И k-Й СТАТИСТИКИ
// =============================================================

// --- E.15. QuickSelect (randomized) ---
// O(n) ожидаемо, O(n²) худший. Переиспользует partition из Quick Sort.
int quickselect(vector<int>& a, int k, int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    while (lo < hi) {
        int pivot = a[lo + rand() % (hi - lo)];
        int lt = lo, i = lo, gt = hi - 1;
        while (i <= gt) {
            if (a[i] < pivot) swap(a[lt++], a[i++]);
            else if (a[i] > pivot) swap(a[i], a[gt--]);
            else i++;
        }
        if (k < lt) hi = lt;
        else if (k >= gt + 1) lo = gt + 1;
        else return a[k];
    }
    return a[lo];
}

// --- E.16. Median of Medians (BFPRT) ---
// Гарантированный O(n). Группы по 5 → медиана каждой → рекурсия.
int median_of_medians(vector<int>& a, int k, int lo = -1, int hi = -1) {
    if (lo == -1) lo = 0;
    if (hi == -1) hi = (int)a.size();
    if (hi - lo <= 5) {
        sort(a.begin() + lo, a.begin() + hi);
        return a[lo + k];
    }
    vector<int> medians;
    for (int i = lo; i < hi; i += 5) {
        int end = min(i + 5, hi);
        sort(a.begin() + i, a.begin() + end);
        medians.push_back(a[(i + end - 1) / 2]);
    }
    int pivot = median_of_medians(medians, (int)medians.size() / 2);

    // partition вокруг pivot
    int lt = lo, i = lo, gt = hi - 1;
    while (i <= gt) {
        if (a[i] < pivot) swap(a[lt++], a[i++]);
        else if (a[i] > pivot) swap(a[i], a[gt--]);
        else i++;
    }
    if (k < lt) return median_of_medians(a, k, lo, lt);
    if (k >= gt + 1) return median_of_medians(a, k - gt - 1, gt + 1, hi);
    return pivot;
}

// --- E.17. IntroSelect ---
// QuickSelect с переключением на Median of Medians при глубине > c·log n.
int intro_select_rec(vector<int>& a, int k, int lo, int hi, int depth_limit) {
    while (hi - lo > 1) {
        if (depth_limit == 0) {
            // fallback на median of medians
            return median_of_medians(a, k, lo, hi);
        }
        int pivot = a[lo + rand() % (hi - lo)];
        int lt = lo, i = lo, gt = hi - 1;
        while (i <= gt) {
            if (a[i] < pivot) swap(a[lt++], a[i++]);
            else if (a[i] > pivot) swap(a[i], a[gt--]);
            else i++;
        }
        if (k < lt) { hi = lt; depth_limit--; }
        else if (k >= gt + 1) { lo = gt + 1; depth_limit--; }
        else return a[k];
    }
    return a[lo];
}

int intro_select(vector<int>& a, int k) {
    return intro_select_rec(a, k, 0, (int)a.size(), 2 * (int)log2(a.size()));
}

// --- E.18. HeapSelect ---
// k-й элемент через min-heap размера k. O(n log k), O(k) памяти.
int heap_select(const vector<int>& a, int k) {
    priority_queue<int, vector<int>, greater<int>> pq;
    for (int x : a) {
        pq.push(x);
        if ((int)pq.size() > k) pq.pop();
    }
    return pq.top();
}

// --- E.19. Running Median (два кучи) ---
// max-heap для меньшей половины, min-heap для большей.
struct RunningMedian {
    priority_queue<int> lo;                     // max-heap
    priority_queue<int, vector<int>, greater<int>> hi;  // min-heap

    void add(int num) {
        lo.push(num);
        hi.push(lo.top()); lo.pop();
        if ((int)hi.size() > (int)lo.size()) {
            lo.push(hi.top()); hi.pop();
        }
    }

    double median() const {
        if (lo.size() > hi.size()) return lo.top();
        return (lo.top() + hi.top()) / 2.0;
    }
};

// --- E.20. Counting Select ---
// Выбор k-го элемента, когда значения ∈ [0, range-1].
// O(n + range) время, O(range) память.
int counting_select(const vector<int>& a, int k, int range) {
    vector<int> freq(range, 0);
    for (int x : a) freq[x]++;
    int acc = 0;
    for (int i = 0; i < range; i++) {
        acc += freq[i];
        if (acc > k) return i;
    }
    return range - 1;
}

// --- E.21. Медиана двух отсортированных массивов ---
// Бинарный поиск по разбиению; O(log min(m, n)) время, O(1) память.
double median_two_sorted(const vector<int>& a, const vector<int>& b) {
    if (a.size() > b.size()) return median_two_sorted(b, a);
    int m = (int)a.size(), n = (int)b.size();
    int lo = 0, hi = m;
    while (lo <= hi) {
        int i = lo + (hi - lo) / 2;
        int j = (m + n + 1) / 2 - i;
        int aleft  = (i > 0) ? a[i - 1] : INT_MIN;
        int aright = (i < m) ? a[i]     : INT_MAX;
        int bleft  = (j > 0) ? b[j - 1] : INT_MIN;
        int bright = (j < n) ? b[j]     : INT_MAX;
        if (aleft <= bright && bleft <= aright) {
            if ((m + n) % 2 == 1) return max(aleft, bleft);
            return (max(aleft, bleft) + min(aright, bright)) / 2.0;
        }
        if (aleft > bright) hi = i - 1;
        else lo = i + 1;
    }
    return 0.0;
}

// =============================================================
// F. МЕТАЭВРИСТИКИ И ЛОКАЛЬНЫЙ ПОИСК
// =============================================================

// --- F.20. Backtracking (обобщённый шаблон) ---
// Параметры: current state, check validity, generate next candidates.
// Пример: N-Queens — расстановка ферзей с проверкой диагоналей.
int n_queens_count(int n) {
    int count = 0;
    vector<int> board(n, -1); // board[i] = столбец ферзя в строке i

    auto is_safe = [&](int row, int col) -> bool {
        for (int r = 0; r < row; r++) {
            int c = board[r];
            if (c == col || abs(r - row) == abs(c - col)) return false;
        }
        return true;
    };

    function<void(int)> backtrack = [&](int row) {
        if (row == n) { count++; return; }
        for (int col = 0; col < n; col++) {
            if (is_safe(row, col)) {
                board[row] = col;
                backtrack(row + 1);
                board[row] = -1;
            }
        }
    };

    backtrack(0);
    return count;
}

// --- F.22. Simulated Annealing ---
// Параметры: начальная T, alpha (охлаждение), eps (остановка).
double simulated_annealing(double initial_temp, double alpha, double eps,
                           function<double(vector<double>&)> energy,
                           function<void(vector<double>&)> neighbor,
                           vector<double> state) {
    mt19937 rng(random_device{}());
    uniform_real_distribution<double> uniform(0.0, 1.0);

    double cur_energy = energy(state);
    double best_energy = cur_energy;
    vector<double> best_state = state;
    double T = initial_temp;

    while (T > eps) {
        vector<double> next_state = state;
        neighbor(next_state);
        double next_energy = energy(next_state);
        double delta = next_energy - cur_energy;

        if (delta < 0 || uniform(rng) < exp(-delta / T)) {
            state = next_state;
            cur_energy = next_energy;
            if (cur_energy < best_energy) {
                best_energy = cur_energy;
                best_state = state;
            }
        }
        T *= alpha;
    }
    state = best_state;
    return best_energy;
}

// --- F.25. A* Search (на графе) ---
// Возвращает кратчайший путь от start до goal или пустой вектор.
struct AStarNode {
    int node;
    double f, g;
    bool operator>(const AStarNode& other) const { return f > other.f; }
};

vector<int> a_star_search(const vector<vector<pair<int, int>>>& adj,
                          int start, int goal,
                          const function<double(int, int)>& heuristic) {
    int n = (int)adj.size();
    vector<double> g_score(n, 1e18);
    vector<int> parent(n, -1);
    priority_queue<AStarNode, vector<AStarNode>, greater<AStarNode>> pq;

    g_score[start] = 0;
    pq.push({start, heuristic(start, goal), 0});

    while (!pq.empty()) {
        auto [u, f, g] = pq.top(); pq.pop();
        if (u == goal) break;
        if (g > g_score[u]) continue;
        for (auto [v, w] : adj[u]) {
            double new_g = g_score[u] + w;
            if (new_g < g_score[v]) {
                g_score[v] = new_g;
                parent[v] = u;
                pq.push({v, new_g + heuristic(v, goal), new_g});
            }
        }
    }

    if (g_score[goal] >= 1e18) return {};
    vector<int> path;
    for (int v = goal; v != -1; v = parent[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

// --- F.26. IDA* ---
// Итеративное углубление с порогом f(n) ≤ limit.
vector<int> ida_star_search(const vector<vector<pair<int, int>>>& adj,
                            int start, int goal,
                            const function<double(int, int)>& heuristic) {
    int n = (int)adj.size();
    vector<int> path = {start};
    vector<char> visited(n, 0);
    visited[start] = 1;

    double threshold = heuristic(start, goal);

    function<double()> dfs = [&]() -> double {
        int u = path.back();
        double f = (path.size() > 1 ? 0 : 0) + heuristic(u, goal);
        // g = сумма весов по path
        double g = 0;
        for (int i = 1; i < (int)path.size(); i++) {
            for (auto [v, w] : adj[path[i - 1]])
                if (v == path[i]) { g += w; break; }
        }
        f = g + heuristic(u, goal);

        if (u == goal) return -1; // найден
        double min_over = 1e18;
        for (auto [v, w] : adj[u]) {
            if (!visited[v]) {
                path.push_back(v);
                visited[v] = 1;
                double t = dfs();
                if (t == -1) return -1;
                if (t < min_over) min_over = t;
                visited[v] = 0;
                path.pop_back();
            }
        }
        return min_over;
    };

    while (threshold < 1e18) {
        visited.assign(n, 0);
        visited[start] = 1;
        path = {start};
        double t = dfs();
        if (t == -1) return path;
        if (t >= 1e18) return {};
        threshold = t;
    }
    return {};
}

// =============================================================
// G. СТРИМИНГОВЫЙ И ОНЛАЙН ПОИСК
// =============================================================

// --- G.28. Reservoir Sampling ---
// Выбор k случайных элементов из потока неизвестного размера.
vector<int> reservoir_sampling(int k, const function<int()>& next_element,
                               int n) {
    vector<int> reservoir(k);
    for (int i = 0; i < k; i++) reservoir[i] = next_element();
    for (int i = k; i < n; i++) {
        int x = next_element();
        int j = rand() % (i + 1);
        if (j < k) reservoir[j] = x;
    }
    return reservoir;
}

// --- G.29. Misra-Gries Heavy Hitters ---
// Элементы > n/k раз. O(n) время, O(k) память.
vector<pair<int, int>> misra_gries(const vector<int>& a, int k) {
    unordered_map<int, int> count;
    for (int x : a) {
        if (count.size() < k - 1) { count[x]++; continue; }
        if (count.count(x)) { count[x]++; continue; }
        // уменьшаем все на 1 и удаляем нулевые
        vector<int> to_erase;
        for (auto& [key, cnt] : count) {
            if (--cnt == 0) to_erase.push_back(key);
        }
        for (int key : to_erase) count.erase(key);
    }
    // проверяем кандидатов
    vector<pair<int, int>> result;
    for (auto& [key, cnt] : count) {
        int real_cnt = 0;
        for (int x : a) if (x == key) real_cnt++;
        if (real_cnt > (int)a.size() / k) result.push_back({key, real_cnt});
    }
    return result;
}

// =============================================================
// H. ПАРАМЕТРИЧЕСКИЙ И МНОГОКРИТЕРИАЛЬНЫЙ
// =============================================================

// --- H.32. Pareto Front ---
// Множество недоминированных решений. O(n log n).
vector<int> pareto_front(const vector<pair<int, int>>& points) {
    int n = (int)points.size();
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return points[a].first < points[b].first ||
               (points[a].first == points[b].first && points[a].second > points[b].second);
    });
    vector<int> front;
    int max_y = INT_MIN;
    for (int i : idx) {
        if (points[i].second > max_y) {
            front.push_back(i);
            max_y = points[i].second;
        }
    }
    return front;
}

// =============================================================
// ДОПОЛНИТЕЛЬНЫЕ МЕТОДЫ (недостающие из md)
// =============================================================

// --- Hill Climbing ---
// Локальный поиск: начальное решение → moves в окрестности → лучший邻居 → повтор.
// Параметры: начальное состояние, функция邻居 (возвращает вектор соседей), функция оценки.
template<typename State>
State hill_climbing(State initial,
                    function<vector<State>(const State&)> neighbors,
                    function<double(const State&)> evaluate) {
    State current = initial;
    double cur_val = evaluate(current);
    while (true) {
        auto nbrs = neighbors(current);
        if (nbrs.empty()) break;
        State best_nbr = nbrs[0];
        double best_val = evaluate(best_nbr);
        for (int i = 1; i < (int)nbrs.size(); i++) {
            double val = evaluate(nbrs[i]);
            if (val > best_val) { best_val = val; best_nbr = nbrs[i]; }
        }
        if (best_val <= cur_val) break;
        current = best_nbr;
        cur_val = best_val;
    }
    return current;
}

// --- Tabu Search ---
// Локальный поиск с памятью (tabu list): запрещённые недавние ходы.
template<typename State>
State tabu_search(State initial, int max_iter, int tabu_size,
                  function<vector<State>(const State&)> neighbors,
                  function<double(const State&)> evaluate) {
    State current = initial;
    double best_val = evaluate(current);
    State best_state = current;
    deque<State> tabu;
    for (int iter = 0; iter < max_iter; iter++) {
        auto nbrs = neighbors(current);
        State best_nbr = current;
        double best_nbr_val = -1e18;
        bool found = false;
        for (auto& nbr : nbrs) {
            bool is_tabu = false;
            for (auto& t : tabu) if (t == nbr) { is_tabu = true; break; }
            double val = evaluate(nbr);
            if (!is_tabu || val > best_val) {
                if (val > best_nbr_val) { best_nbr_val = val; best_nbr = nbr; found = true; }
            }
        }
        if (!found) break;
        current = best_nbr;
        tabu.push_back(current);
        if ((int)tabu.size() > tabu_size) tabu.pop_front();
        if (best_nbr_val > best_val) { best_val = best_nbr_val; best_state = current; }
    }
    return best_state;
}

// --- Genetic Algorithm ---
// Эволюционный подход: популяция → selection → crossover → mutation → повтор.
template<typename Gene>
struct GeneticAlgorithm {
    struct Individual { vector<Gene> genes; double fitness; };

    function<double(const vector<Gene>&)> fitness_fn;
    function<vector<Gene>()> random_individual;
    function<vector<Gene>(const vector<Gene>&, const vector<Gene>&)> crossover;
    function<void(vector<Gene>&)> mutate;
    int pop_size;
    double mutation_rate;
    int generations;

    Individual evolve() {
        vector<Individual> pop(pop_size);
        for (auto& ind : pop) {
            ind.genes = random_individual();
            ind.fitness = fitness_fn(ind.genes);
        }
        for (int g = 0; g < generations; g++) {
            sort(pop.begin(), pop.end(), [](const Individual& a, const Individual& b) {
                return a.fitness > b.fitness;
            });
            vector<Individual> new_pop;
            // элита
            new_pop.push_back(pop[0]);
            new_pop.push_back(pop[1]);
            while ((int)new_pop.size() < pop_size) {
                // tournament selection
                auto select = [&]() -> const Individual& {
                    int i = rand() % pop_size, j = rand() % pop_size;
                    return pop[i].fitness > pop[j].fitness ? pop[i] : pop[j];
                };
                auto& p1 = select();
                auto& p2 = select();
                auto child_genes = crossover(p1.genes, p2.genes);
                if ((double)rand() / RAND_MAX < mutation_rate) mutate(child_genes);
                new_pop.push_back({child_genes, fitness_fn(child_genes)});
            }
            pop = new_pop;
        }
        return *max_element(pop.begin(), pop.end(), [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
        });
    }
};

// --- Beam Search ---
// Ограниченный BFS: сохранять только k лучших узлов на каждом уровне.
template<typename State>
State beam_search(State initial, int beam_width, int max_depth,
                  function<vector<pair<State, double>>(const State&)> expand,
                  function<bool(const State&)> is_goal) {
    vector<pair<State, double>> current = {{initial, 0.0}};
    for (int depth = 0; depth < max_depth && !current.empty(); depth++) {
        vector<pair<State, double>> candidates;
        for (auto& [state, score] : current) {
            auto expanded = expand(state);
            for (auto& [next_state, step_score] : expanded)
                candidates.push_back({next_state, score + step_score});
        }
        sort(candidates.begin(), candidates.end(),
             [](auto& a, auto& b) { return a.second > b.second; });
        if ((int)candidates.size() > beam_width)
            candidates.resize(beam_width);
        current = candidates;
        for (auto& [state, score] : current)
            if (is_goal(state)) return state;
    }
    return current.empty() ? initial : current[0].first;
}

// --- Greenwald-Khanna (упрощённый) ---
// Приближённые квантили с гарантированной ошибкой ε.
struct GreenwaldKhanna {
    double epsilon;
    vector<pair<double, int>> tuples; // (value, delta)

    GreenwaldKhanna(double eps) : epsilon(eps) {}

    void insert(double val) {
        int s = (int)ceil(1.0 / epsilon);
        tuples.push_back({val, 0});
        if ((int)tuples.size() >= 2 * s) {
            // merge
            sort(tuples.begin(), tuples.end());
            vector<pair<double, int>> merged;
            merged.push_back(tuples[0]);
            for (int i = 1; i < (int)tuples.size(); i++) {
                if (i % (s / 2) == 0) merged.push_back(tuples[i]);
                else merged.back().second += tuples[i].second + 1;
            }
            tuples = merged;
        }
    }

    double quantile(double q) const {
        if (tuples.empty()) return 0;
        int n = 0;
        for (auto& [val, delta] : tuples) n += delta + 1;
        int target = (int)(q * n);
        int cum = 0;
        for (auto& [val, delta] : tuples) {
            cum += delta + 1;
            if (cum >= target) return val;
        }
        return tuples.back().first;
    }
};

// --- Binary Search в 2D матрице (бинарный поиск по строкам) ---
bool search_matrix_bs(const vector<vector<int>>& mat, int target) {
    if (mat.empty() || mat[0].empty()) return false;
    int rows = (int)mat.size(), cols = (int)mat[0].size();
    int lo = 0, hi = rows * cols - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = mat[mid / cols][mid % cols];
        if (val == target) return true;
        if (val < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return false;
}

// --- Running Median ---
// Два кучи: max-heap для меньшей половины, min-heap для большей.
// O(log n) на добавление, O(1) на запрос.

}; // struct SearchAlgorithms

#endif // TECHNIQUE_B_CPP
