#ifndef ALGO_ANALYSIS_C_CPP
#define ALGO_ANALYSIS_C_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>
#include <chrono>
using namespace std;

// =============================================================
// C. МЕТОДЫ АНАЛИЗА АЛГОРИТМОВ
// =============================================================
// Структура md: A. Анализ по случаям
//               → B. Анализ итеративных алгоритмов
//               → C. Метод подсчёта операций
//
// AnalysisMethods — наследует AsymptoticAnalysis (b.cpp).
// Худший/лучший/средний случай, суммирование рядов,
// подсчёт операций в сортировках.

#ifndef INSIDE_ALGO_ANALYSIS_C
#define INSIDE_ALGO_ANALYSIS_C
#include "../b/b.cpp"
#endif

struct AnalysisMethods : AsymptoticAnalysis {

// =============================================================
// A. АНАЛИЗ ПО СЛУЧАЯМ
// =============================================================

// --- A.1. Анализ Quick Sort по случаям ---
// Возвращает число сравнений для худшего, лучшего и среднего случая.
struct QuickSortAnalysis {
    long long worst_comparisons(int n) {
        // Худший: T(n) = T(n-1) + O(n) → n(n-1)/2
        return (long long)n * (n - 1) / 2;
    }
    long long best_comparisons(int n) {
        // Лучший: T(n) = 2T(n/2) + O(n) → n log n
        if (n <= 1) return 0;
        return n + 2 * best_comparisons(n / 2);
    }
    long long avg_comparisons(int n) {
        // Средний: ~1.39 n log n (2n ln n)
        if (n <= 1) return 0;
        return (long long)(2.0 * n * log(n) / log(2));
    }
};

// --- A.2. Анализ Binary Search по случаям ---
long long binary_search_comparisons(int n, int target_pos) {
    int lo = 0, hi = n - 1;
    long long comps = 0;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        comps++;
        if (mid == target_pos) return comps;
        if (mid < target_pos) lo = mid + 1;
        else hi = mid - 1;
    }
    return comps;
}

// =============================================================
// B. АНАЛИЗ ИТЕРАТИВНЫХ АЛГОРИТМОВ
// =============================================================

// --- B.1. Суммирование: арифметический ряд ---
long long arithmetic_sum(long long n) { return n * (n + 1) / 2; }

// --- B.2. Суммирование: геометрический ряд ---
double geometric_sum(double r, int n) {
    if (fabs(r - 1.0) < 1e-12) return n + 1;
    return (pow(r, n + 1) - 1.0) / (r - 1.0);
}

// --- B.3. Суммирование: гармонический ряд ---
double harmonic_sum(int n) {
    double sum = 0;
    for (int i = 1; i <= n; i++) sum += 1.0 / i;
    return sum;
}

// --- B.4. Суммирование: произведение факториалов ---
double log_factorial(int n) {
    double sum = 0;
    for (int i = 1; i <= n; i++) sum += log(i);
    return sum;
}

// --- B.5. Анализ вложенных циклов ---
// Типичные паттерны и их сложность.
long long nested_loop_analysis(int n) {
    long long ops = 0;
    // O(n²): два вложенных цикла
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            ops++;
    return ops;
}

long long nested_loop_upper_triangle(int n) {
    long long ops = 0;
    // O(n²/2): верхний треугольник
    for (int i = 0; i < n; i++)
        for (int j = i; j < n; j++)
            ops++;
    return ops;
}

long long nested_loop_log(int n) {
    long long ops = 0;
    // O(n log n): внешний цикл n, внутренний log n
    for (int i = 0; i < n; i++)
        for (int j = 1; j < n; j *= 2)
            ops++;
    return ops;
}

// =============================================================
// C. МЕТОД ПОДСЧЁТА ОПЕРАЦИЙ
// =============================================================

// --- C.1. Bubble Sort: подсчёт сравнений и обменов ---
pair<long long, long long> bubble_sort_analysis(int n) {
    long long comparisons = 0, swaps = 0;
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = n - i;  // обратный порядок (худший)
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++) {
            comparisons++;
            if (a[j] > a[j + 1]) { swap(a[j], a[j + 1]); swaps++; }
        }
    return {comparisons, swaps};
}

// --- C.2. Insertion Sort: подсчёт операций ---
pair<long long, long long> insertion_sort_analysis(int n) {
    long long comparisons = 0, shifts = 0;
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = n - i;  // худший
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) { comparisons++; a[j + 1] = a[j]; shifts++; j--; }
        if (j >= 0) comparisons++;
        a[j + 1] = key;
    }
    return {comparisons, shifts};
}

// --- C.3. Merge Sort: подсчёт сравнений ---
long long merge_sort_comparisons = 0;
void merge_count(vector<int>& a, int lo, int mid, int hi) {
    vector<int> temp;
    int i = lo, j = mid;
    while (i < mid && j < hi) {
        merge_sort_comparisons++;
        if (a[i] <= a[j]) temp.push_back(a[i++]);
        else temp.push_back(a[j++]);
    }
    while (i < mid) temp.push_back(a[i++]);
    while (j < hi) temp.push_back(a[j++]);
    for (int k = 0; k < (int)temp.size(); k++) a[lo + k] = temp[k];
}

void merge_sort_count(vector<int>& a, int lo, int hi) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_count(a, lo, mid);
    merge_sort_count(a, mid, hi);
    merge_count(a, lo, mid, hi);
}

// --- C.4. Нижняя граница: дерево решений ---
// Для n! листьев: высота ≥ log₂(n!).
double lower_bound_comparisons(int n) {
    double log_factorial = 0;
    for (int i = 2; i <= n; i++) log_factorial += log2(i);
    return log_factorial;
}

}; // struct AnalysisMethods

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_C_MAIN
int main() {
    AnalysisMethods am;
    srand(42);

    cout << "=== A. АНАЛИЗ ПО СЛУЧАЯМ ===" << endl;

    cout << "--- Quick Sort: худший/лучший/средний ---" << endl;
    {
        AnalysisMethods::QuickSortAnalysis qsa;
        for (int n : {10, 100, 1000, 10000}) {
            cout << "  n=" << n
                 << ": worst=" << qsa.worst_comparisons(n)
                 << " best=" << qsa.best_comparisons(n)
                 << " avg=" << qsa.avg_comparisons(n) << endl;
        }
    }

    cout << "\n--- Binary Search: число сравнений по позиции ---" << endl;
    {
        int n = 1024;
        for (int pos : {0, 256, 512, 768, 1023}) {
            long long comps = am.binary_search_comparisons(n, pos);
            cout << "  n=" << n << ", pos=" << pos
                 << ": " << comps << " comparisons (log₂n=" << (int)log2(n) << ")" << endl;
        }
    }

    cout << "\n=== B. СУММИРОВАНИЕ РЯДОВ ===" << endl;

    cout << "--- Арифметический ряд ---" << endl;
    for (int n : {10, 100, 1000, 10000}) {
        cout << "  Σ1..n = " << am.arithmetic_sum(n) << " = n(n+1)/2" << endl;
    }

    cout << "\n--- Геометрический ряд ---" << endl;
    for (double r : {0.5, 0.9, 2.0}) {
        cout << "  Σ r^i, i=0..10, r=" << r << ": " << am.geometric_sum(r, 10) << endl;
    }
    cout << "  Σ (1/2)^i, i=0..∞ = " << am.geometric_sum(0.5, 100) << " (ожидаем 2.0)" << endl;

    cout << "\n--- Гармонический ряд vs ln(n) ---" << endl;
    for (int n : {10, 100, 1000, 10000, 100000}) {
        double H = am.harmonic_sum(n);
        double ln_n = log(n);
        cout << "  n=" << n << ": H(n)=" << H << " ln(n)=" << ln_n
             << " разница=" << H - ln_n << endl;
    }

    cout << "\n--- Факториал (через логарифмы) ---" << endl;
    for (int n : {5, 10, 20, 50}) {
        double log_fact = am.log_factorial(n);
        double approx = n * log(n) - n;  // стёрлинг
        cout << "  ln(" << n << "!)=" << log_fact << " approx=" << approx << endl;
    }

    cout << "\n=== C. АНАЛИЗ ВЛОЖЕННЫХ ЦИКЛОВ ===" << endl;
    for (int n : {10, 100, 1000}) {
        cout << "  n=" << n << ":" << endl;
        cout << "    O(n²) full:     " << am.nested_loop_analysis(n) << " ops (n²=" << (long long)n*n << ")" << endl;
        cout << "    O(n²/2) upper:  " << am.nested_loop_upper_triangle(n) << " ops (n(n-1)/2=" << (long long)n*(n-1)/2 << ")" << endl;
        cout << "    O(n log n):     " << am.nested_loop_log(n) << " ops (n*log₂n=" << (long long)(n*log2(n)) << ")" << endl;
    }

    cout << "\n=== C. ПОДСЧЁТ ОПЕРАЦИЙ В СОРТИРОВКАХ ===" << endl;

    cout << "--- Bubble Sort (худший) ---" << endl;
    for (int n : {10, 50, 100}) {
        auto [comps, swaps] = am.bubble_sort_analysis(n);
        cout << "  n=" << n << ": comparisons=" << comps << " (n(n-1)/2=" << (long long)n*(n-1)/2
             << "), swaps=" << swaps << endl;
    }

    cout << "\n--- Insertion Sort (худший) ---" << endl;
    for (int n : {10, 50, 100}) {
        auto [comps, shifts] = am.insertion_sort_analysis(n);
        cout << "  n=" << n << ": comparisons=" << comps << " shifts=" << shifts << endl;
    }

    cout << "\n--- Merge Sort: число сравнений ---" << endl;
    for (int n : {10, 100, 1000, 10000}) {
        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = rand();
        am.merge_sort_comparisons = 0;
        am.merge_sort_count(a, 0, n);
        cout << "  n=" << n << ": comparisons=" << am.merge_sort_comparisons
             << " (теория ~1.44 n log n=" << (long long)(1.44 * n * log2(n)) << ")" << endl;
    }

    cout << "\n--- Нижняя граница: log₂(n!) ---" << endl;
    for (int n : {5, 10, 20, 50, 100}) {
        double lb = am.lower_bound_comparisons(n);
        cout << "  n=" << n << ": log₂(n!)=" << lb
             << " = Ω(n log n), n log₂n=" << n * log2(n) << endl;
    }

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_C_CPP
