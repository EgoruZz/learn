#ifndef ALGO_ANALYSIS_D_CPP
#define ALGO_ANALYSIS_D_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <chrono>
using namespace std;

// =============================================================
// D. АНАЛИЗ РЕКУРСИВНЫХ АЛГОРИТМОВ
// =============================================================
// Структура md: A. Рекуррентные соотношения
//               → B. Методы решения
//               → C. Мастер-теорема
//               → D. Примеры для олимпиад
//
// RecursionAnalysis — наследует AnalysisMethods (c.cpp).
// Рекуррентности, дерево рекурсии, Мастер-теорема,
// Merge Sort / Quick Sort / бинарный поиск / быстрое возведение.

#ifndef INSIDE_ALGO_ANALYSIS_D
#define INSIDE_ALGO_ANALYSIS_D
#include "../c/c.cpp"
#endif

struct RecursionAnalysis : AnalysisMethods {

// =============================================================
// B. МЕТОДЫ РЕШЕНИЯ
// =============================================================

// --- B.1. Дерево рекурсии: Merge Sort ---
// Возвращает стоимость (число операций) и число уровней.
pair<long long, int> merge_sort_tree(int n) {
    if (n <= 1) return {0, 0};
    long long cost = n;  // слияние: O(n)
    int levels = 1;
    auto [left_cost, left_levels] = merge_sort_tree(n / 2);
    auto [right_cost, right_levels] = merge_sort_tree(n - n / 2);
    cost += left_cost + right_cost;
    levels += max(left_levels, right_levels);
    return {cost, levels};
}

// --- B.2. Итерационное решение: T(n) = T(n-1) + n ---
// Прямое суммирование: T(n) = n(n+1)/2.
long long iterative_sum(int n) {
    long long sum = 0;
    for (int i = 1; i <= n; i++) sum += i;
    return sum;
}

// =============================================================
// C. МАСТЕР-ТЕОРЕМА
// =============================================================

// --- C.1. Проверка случая Мастер-теоремы ---
// T(n) = a*T(n/b) + O(n^d).
// Возвращает номер случая (1, 2, 3) и асимптотику.
string master_theorem(int a, int b, int d) {
    double log_b_a = log(a) / log(b);
    if (d > log_b_a + 0.001) return "Case 1: O(n^" + to_string(d) + ")";
    if (fabs(d - log_b_a) < 0.001) return "Case 2: O(n^" + to_string(d) + " log n)";
    return "Case 3: O(n^" + to_string((int)round(log_b_a * 100) / 100.0) + ")";
}

// =============================================================
// D. ПРИМЕРЫ ДЛЯ ОЛИМПИАД
// =============================================================

// --- D.1. Merge Sort: O(n log n) ---
long long merge_sort_ops = 0;
void merge_ops(vector<int>& a, int lo, int mid, int hi) {
    vector<int> temp;
    int i = lo, j = mid;
    while (i < mid && j < hi) {
        merge_sort_ops++;
        if (a[i] <= a[j]) temp.push_back(a[i++]);
        else temp.push_back(a[j++]);
    }
    while (i < mid) temp.push_back(a[i++]);
    while (j < hi) temp.push_back(a[j++]);
    for (int k = 0; k < (int)temp.size(); k++) a[lo + k] = temp[k];
}

void merge_sort_ops_count(vector<int>& a, int lo, int hi) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_ops_count(a, lo, mid);
    merge_sort_ops_count(a, mid, hi);
    merge_ops(a, lo, mid, hi);
}

// --- D.2. Quick Sort: O(n log n) средний ---
long long quicksort_comparisons = 0;
void quicksort_ops(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return;
    int pivot = a[lo + rand() % (hi - lo)];
    int i = lo, j = hi - 1;
    while (i <= j) {
        while (i <= j && a[i] < pivot) { quicksort_comparisons++; i++; }
        while (i <= j && a[j] > pivot) { quicksort_comparisons++; j--; }
        if (i <= j) { swap(a[i], a[j]); i++; j--; }
    }
    quicksort_ops(a, lo, j + 1);
    quicksort_ops(a, i, hi);
}

// --- D.3. Быстрое возведение в степень: O(log n) ---
long long fast_power_ops = 0;
long long fast_power(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        fast_power_ops++;
        if (exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

// --- D.4. Бинарный поиск: O(log n) ---
int binary_search_ops_count(const vector<int>& a, int target) {
    int lo = 0, hi = (int)a.size() - 1;
    int ops = 0;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        ops++;
        if (a[mid] == target) return ops;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return ops;
}

}; // struct RecursionAnalysis

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_D_MAIN
int main() {
    RecursionAnalysis ra;
    srand(42);

    cout << "=== B. ДЕРЕВО РЕКУРСИИ: MERGE SORT ===" << endl;
    for (int n : {8, 16, 32, 64, 128}) {
        auto [cost, levels] = ra.merge_sort_tree(n);
        cout << "  n=" << n << ": cost=" << cost << " levels=" << levels
             << " (n log n=" << (long long)(n * log2(n)) << ")" << endl;
    }

    cout << "\n=== B. ИТЕРАЦИОННОЕ РЕШЕНИЕ: T(n) = T(n-1) + n ===" << endl;
    for (int n : {5, 10, 100, 1000}) {
        long long iter = ra.iterative_sum(n);
        long long formula = (long long)n * (n + 1) / 2;
        cout << "  n=" << n << ": iter=" << iter << " formula=" << formula << endl;
    }

    cout << "\n=== C. МАСТЕР-ТЕОРЕМА ===" << endl;
    cout << "  " << ra.master_theorem(2, 2, 1) << "  (Merge Sort)" << endl;
    cout << "  " << ra.master_theorem(1, 2, 0) << "  (Binary Search)" << endl;
    cout << "  " << ra.master_theorem(7, 2, 2) << "  (Strassen)" << endl;
    cout << "  " << ra.master_theorem(3, 2, 1) << "  (Karatsuba)" << endl;
    cout << "  " << ra.master_theorem(2, 2, 0) << "  (Counter example)" << endl;

    cout << "\n=== D. ПРИМЕРЫ ДЛЯ ОЛИМПИАД ===" << endl;

    cout << "--- Merge Sort: O(n log n) ---" << endl;
    for (int n : {100, 1000, 10000, 100000}) {
        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = rand();
        ra.merge_sort_ops = 0;
        ra.merge_sort_ops_count(a, 0, n);
        cout << "  n=" << n << ": ops=" << ra.merge_sort_ops
             << " (1.44*n*log2(n)=" << (long long)(1.44 * n * log2(n)) << ")" << endl;
    }

    cout << "\n--- Quick Sort: O(n log n) средний ---" << endl;
    for (int n : {100, 1000, 10000, 100000}) {
        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = rand();
        ra.quicksort_comparisons = 0;
        ra.quicksort_ops(a, 0, n);
        cout << "  n=" << n << ": comparisons=" << ra.quicksort_comparisons
             << " (1.39*n*log2(n)=" << (long long)(1.39 * n * log2(n)) << ")" << endl;
    }

    cout << "\n--- Быстрое возведение в степень: O(log n) ---" << endl;
    for (long long n : {10LL, 100LL, 1000LL, 1000000LL, 1000000000LL}) {
        ra.fast_power_ops = 0;
        ra.fast_power(3, n, 1000000007);
        cout << "  3^" << n << " mod M: ops=" << ra.fast_power_ops
             << " (log₂n=" << (int)ceil(log2((double)n)) << ")" << endl;
    }

    cout << "\n--- Бинарный поиск: O(log n) ---" << endl;
    {
        vector<int> a(100000);
        for (int i = 0; i < 100000; i++) a[i] = i * 2;
        for (int target : {0, 50000, 99998}) {
            int ops = ra.binary_search_ops_count(a, target);
            cout << "  n=100000, target=" << target << ": " << ops
                 << " ops (log₂n=" << (int)log2(100000) << ")" << endl;
        }
    }

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_D_CPP
