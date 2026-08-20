#ifndef ALGO_ANALYSIS_A_CPP
#define ALGO_ANALYSIS_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <cmath>
using namespace std;

// =============================================================
// A. ВВЕДЕНИЕ В ТЕОРИЮ СЛОЖНОСТИ
// =============================================================
// Структура md: A. Основные понятия
//               → B. Модели вычислений
//               → C. Метрики эффективности
//
// ComplexityBasics — базовый класс всей ветки algo-analysis.
// Замер времени, подсчёт операций, анализ памяти.

struct ComplexityBasics {

// =============================================================
// C. МЕТРИКИ ЭФФЕКТИВНОСТИ
// =============================================================

// --- C.1. Замер времени выполнения функции ---
// Запускает func(n) и возвращает время в микросекундах.
double measure_time(function<void(int)> func, int n, int iterations = 1) {
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) func(n);
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::microseconds>(end - start).count() / (double)iterations;
}

// --- C.2. Подсчёт операций (пример: bubble sort) ---
// Возвращает {сравнения, обмены}.
pair<long long, long long> bubble_sort_counted(vector<int> a) {
    long long comparisons = 0, swaps = 0;
    int n = (int)a.size();
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++) {
            comparisons++;
            if (a[j] > a[j + 1]) {
                swap(a[j], a[j + 1]);
                swaps++;
            }
        }
    return {comparisons, swaps};
}

// --- C.3. Подсчёт операций (пример: insertion sort) ---
pair<long long, long long> insertion_sort_counted(vector<int> a) {
    long long comparisons = 0, shifts = 0;
    int n = (int)a.size();
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            comparisons++;
            a[j + 1] = a[j];
            shifts++;
            j--;
        }
        if (j >= 0) comparisons++;  // последнее сравнение (неудачное)
        a[j + 1] = key;
    }
    return {comparisons, shifts};
}

// --- C.4. Подсчёт операций (пример: linear search) ---
// Возвращает число сравнений.
long long linear_search_counted(const vector<int>& a, int target) {
    long long comparisons = 0;
    for (int i = 0; i < (int)a.size(); i++) {
        comparisons++;
        if (a[i] == target) return comparisons;
    }
    return comparisons;
}

// --- C.5. Подсчёт операций (пример: binary search) ---
// Возвращает число сравнений.
long long binary_search_counted(const vector<int>& a, int target) {
    long long comparisons = 0;
    int lo = 0, hi = (int)a.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        comparisons++;
        if (a[mid] == target) return comparisons;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return comparisons;
}

// --- C.6. Анализ пространственной сложности ---
// Подсчёт дополнительной памяти (в элементах).
int merge_sort_memory(int n) {
    // Merge Sort: O(n) доп. памяти для временного массива
    return n;  // simplified
}

// --- C.7. Демонстрация O(n²) vs O(n log n) ---
// Возвращает {время O(n²), время O(n log n)} для данных размеров.
pair<double, double> compare_quadratic_vs_linearithmic(int n) {
    // O(n²): bubble sort на случайных данных
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = rand();
    auto start1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n - 1; j++)
            if (a[j] > a[j + 1]) swap(a[j], a[j + 1]);
    auto end1 = chrono::high_resolution_clock::now();

    // O(n log n): через std::sort
    vector<int> b(n);
    for (int i = 0; i < n; i++) b[i] = rand();
    auto start2 = chrono::high_resolution_clock::now();
    sort(b.begin(), b.end());
    auto end2 = chrono::high_resolution_clock::now();

    double t1 = chrono::duration_cast<chrono::microseconds>(end1 - start1).count();
    double t2 = chrono::duration_cast<chrono::microseconds>(end2 - start2).count();
    return {t1, t2};
}

}; // struct ComplexityBasics

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_A_MAIN
int main() {
    ComplexityBasics cb;
    srand(42);

    cout << "=== A. ОСНОВНЫЕ ПОНЯТИЯ ===" << endl;

    cout << "--- Замер времени: O(n²) vs O(n log n) ---" << endl;
    for (int n : {100, 500, 1000}) {
        auto [t_quad, t_linlog] = cb.compare_quadratic_vs_linearithmic(n);
        cout << "  n=" << n << ": O(n²)=" << t_quad << "μs, O(n log n)=" << t_linlog << "μs, ratio=" << (t_linlog > 0 ? t_quad/t_linlog : 0) << "x" << endl;
    }

    cout << "\n=== C. ПОДСЧЁТ ОПЕРАЦИЙ ===" << endl;

    cout << "--- Bubble Sort ---" << endl;
    {
        vector<int> a = {5, 3, 8, 1, 9, 2, 7, 4, 6};
        auto [comps, swaps] = cb.bubble_sort_counted(a);
        cout << "  {5,3,8,1,9,2,7,4,6}: comparisons=" << comps << ", swaps=" << swaps << endl;
        // Теория: n(n-1)/2 comparisons, O(n²) swaps
        cout << "  Теория: n(n-1)/2 = " << 9 * 8 / 2 << " comparisons" << endl;
    }

    cout << "\n--- Insertion Sort ---" << endl;
    {
        vector<int> a = {5, 3, 8, 1, 9, 2, 7, 4, 6};
        auto [comps, shifts] = cb.insertion_sort_counted(a);
        cout << "  {5,3,8,1,9,2,7,4,6}: comparisons=" << comps << ", shifts=" << shifts << endl;
        // Лучший (отсортированный): n-1 comparisons, 0 shifts
        vector<int> sorted = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto [c2, s2] = cb.insertion_sort_counted(sorted);
        cout << "  Отсортированный: comparisons=" << c2 << ", shifts=" << s2 << endl;
    }

    cout << "\n--- Поиск: Linear vs Binary ---" << endl;
    {
        vector<int> a(10000);
        for (int i = 0; i < 10000; i++) a[i] = i * 2;  // чётные
        // Линейный поиск: O(n)
        long long lc = cb.linear_search_counted(a, 9998);
        cout << "  Linear search(9998): " << lc << " comparisons (O(n))" << endl;
        // Бинарный поиск: O(log n)
        long long bc = cb.binary_search_counted(a, 9998);
        cout << "  Binary search(9998): " << bc << " comparisons (O(log n))" << endl;
        cout << "  Speedup: " << (double)lc / bc << "x" << endl;
    }

    cout << "\n--- Пространственная сложность ---" << endl;
    cout << "  Merge Sort доп. память (n=1000): " << cb.merge_sort_memory(1000) << " элементов" << endl;
    cout << "  Quick Sort доп. память: O(log n) = " << (int)log2(1000) << " (стек)" << endl;

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_A_CPP
