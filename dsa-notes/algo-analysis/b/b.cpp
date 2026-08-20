#ifndef ALGO_ANALYSIS_B_CPP
#define ALGO_ANALYSIS_B_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <chrono>
#include <bitset>
using namespace std;

// =============================================================
// B. АСИМПТОТИЧЕСКИЙ АНАЛИЗ
// =============================================================
// Структура md: A. Асимптотические обозначения
//               → B. Свойства
//               → C. Типовые классы сложности
//               → D. Битовая сложность
//
// AsymptoticAnalysis — наследует ComplexityBasics (a.cpp).
// O/Ω/Θ/o/ω, типовые классы, битовые маски, перебор подмножеств.

#ifndef INSIDE_ALGO_ANALYSIS_B
#define INSIDE_ALGO_ANALYSIS_B
#include "../a/a.cpp"
#endif

struct AsymptoticAnalysis : ComplexityBasics {

// =============================================================
// C. ТИПОВЫЕ КЛАССЫ СЛОЖНОСТИ (демонстрация)
// =============================================================

// --- C.1. O(1): доступ по индексу ---
int access_by_index(const vector<int>& a, int i) { return a[i]; }

// --- C.2. O(log n): бинарный поиск ---
int binary_search_demo(const vector<int>& a, int target) {
    int lo = 0, hi = (int)a.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == target) return mid;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// --- C.3. O(n): линейный обход ---
long long sum_array(const vector<int>& a) {
    long long sum = 0;
    for (int x : a) sum += x;
    return sum;
}

// --- C.4. O(n log n): сортировка ---
void sort_demo(vector<int>& a) { sort(a.begin(), a.end()); }

// --- C.5. O(n²): умножение матриц ---
vector<vector<int>> matrix_multiply_naive(const vector<vector<int>>& A,
                                           const vector<vector<int>>& B) {
    int n = (int)A.size();
    vector<vector<int>> C(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

// --- C.6. O(2ⁿ): перебор подмножеств ---
// Возвращает сумму всех элементов, входящих в подмножества чётного размера.
long long subset_sum_even(int n, const vector<int>& a) {
    long long total = 0;
    for (int mask = 0; mask < (1 << n); mask++) {
        if (__builtin_popcount(mask) % 2 == 0) {
            for (int i = 0; i < n; i++)
                if (mask & (1 << i)) total += a[i];
        }
    }
    return total;
}

// --- C.7. O(n!): генерация всех перестановок ---
int factorial_count = 0;
void generate_permutations(vector<int>& perm, int pos) {
    if (pos == (int)perm.size()) {
        factorial_count++;
        return;
    }
    for (int i = pos; i < (int)perm.size(); i++) {
        swap(perm[pos], perm[i]);
        generate_permutations(perm, pos + 1);
        swap(perm[pos], perm[i]);
    }
}

// =============================================================
// D. БИТОВАЯ СЛОЖНОСТЬ
// =============================================================

// --- D.1. POPCOUNT через lookup ---
// Число единичных бит в числе x.
int popcount_lookup(int x) {
    int count = 0;
    while (x) { count += x & 1; x >>= 1; }
    return count;
}

// --- D.2. Перебор всех подмножеств с подсчётом суммы ---
long long subset_enumeration_sum(int n, const vector<int>& a) {
    long long total = 0;
    for (int mask = 0; mask < (1 << n); mask++) {
        long long subset_sum = 0;
        for (int i = 0; i < n; i++)
            if (mask & (1 << i)) subset_sum += a[i];
        total += subset_sum;
    }
    return total;
}

// --- D.3. Перебор пар подмножеств (O(3ⁿ)) ---
// Для каждого элемента: 4 варианта (ни в A, ни в B; только A; только B; оба).
long long pair_subset_count(int n) {
    long long count = 0;
    for (int A = 0; A < (1 << n); A++) {
        for (int B = A; B < (1 << n); B++) {
            count++;
        }
    }
    return count;
}

// --- D.4. Битовые трюки ---
// Проверка: является ли n степенью двойки.
bool is_power_of_two(int n) { return n > 0 && (n & (n - 1)) == 0; }

// Последний установленный бит (lowbit).
int lowbit(int x) { return x & (-x); }

// Подсчёт бит между двумя позициями.
int bits_between(int x, int lo, int hi) {
    int mask = ((1 << (hi + 1)) - 1) & ~((1 << lo) - 1);
    return __builtin_popcount(x & mask);
}

}; // struct AsymptoticAnalysis

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_B_MAIN
int main() {
    AsymptoticAnalysis aa;
    srand(42);

    cout << "=== C. ТИПОВЫЕ КЛАССЫ СЛОЖНОСТИ ===" << endl;

    cout << "--- Замеры времени по классам ---" << endl;
    for (int n : {100, 1000, 10000}) {
        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = rand();

        auto t1 = aa.measure_time([&](int) { volatile int x = aa.access_by_index(a, n/2); }, 1, 1000);
        auto t2 = aa.measure_time([&](int) { volatile int x = aa.binary_search_demo(a, a[n/2]); }, 1, 100);
        auto t3 = aa.measure_time([&](int) { volatile long long s = aa.sum_array(a); }, 1, 10);
        vector<int> a_copy = a;
        auto t4 = aa.measure_time([&](int) { sort(a_copy.begin(), a_copy.end()); }, 1, 1);

        cout << "  n=" << n << ":" << endl;
        cout << "    O(1)     access: " << t1 << " μs" << endl;
        cout << "    O(log n) binary: " << t2 << " μs" << endl;
        cout << "    O(n)     sum:    " << t3 << " μs" << endl;
        cout << "    O(n log n) sort: " << t4 << " μs" << endl;
    }

    cout << "\n--- O(n²): умножение матриц ---" << endl;
    {
        int n = 100;
        vector<vector<int>> A(n, vector<int>(n, 1)), B(n, vector<int>(n, 1));
        auto start = chrono::high_resolution_clock::now();
        auto C = aa.matrix_multiply_naive(A, B);
        auto end = chrono::high_resolution_clock::now();
        auto t = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << "  100x100 matrix multiply: " << t << " ms" << endl;
    }

    cout << "\n--- O(2ⁿ): перебор подмножеств ---" << endl;
    for (int n : {10, 15, 20}) {
        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = i + 1;
        auto start = chrono::high_resolution_clock::now();
        long long result = aa.subset_sum_even(n, a);
        auto end = chrono::high_resolution_clock::now();
        auto t = chrono::duration_cast<chrono::microseconds>(end - start).count();
        cout << "  n=" << n << ": result=" << result << ", time=" << t << " μs" << endl;
    }

    cout << "\n--- O(n!): перестановки ---" << endl;
    for (int n : {3, 4, 5, 6, 7, 8, 9, 10}) {
        vector<int> perm(n);
        for (int i = 0; i < n; i++) perm[i] = i;
        aa.factorial_count = 0;
        auto start = chrono::high_resolution_clock::now();
        aa.generate_permutations(perm, 0);
        auto end = chrono::high_resolution_clock::now();
        auto t = chrono::duration_cast<chrono::microseconds>(end - start).count();
        cout << "  n=" << n << ": " << aa.factorial_count << " permutations, " << t << " μs" << endl;
    }

    cout << "\n=== D. БИТОВАЯ СЛОЖНОСТЬ ===" << endl;

    cout << "--- Битовые трюки ---" << endl;
    for (int x : {0, 1, 2, 3, 4, 7, 8, 15, 16}) {
        cout << "  " << x << ": popcount=" << aa.popcount_lookup(x)
             << " is_power_of_2=" << (aa.is_power_of_two(x) ? "Y" : "N")
             << " lowbit=" << aa.lowbit(x) << endl;
    }

    cout << "\n--- Перебор подмножеств: сумма по чётным размерам ---" << endl;
    {
        vector<int> a = {1, 2, 3, 4};
        long long result = aa.subset_sum_even(4, a);
        // Подмножества чётного размера: {}, {1,2}, {1,3}, {1,4}, {2,3}, {2,4}, {3,4}, {1,2,3,4}
        // Суммы: 0+3+4+5+5+6+7+10 = 36
        cout << "  {1,2,3,4}: " << result << " (ожидаем 36)" << endl;
    }

    cout << "\n--- Перебор пар подмножеств (O(3ⁿ)) ---" << endl;
    for (int n : {4, 8, 12, 16}) {
        auto start = chrono::high_resolution_clock::now();
        long long count = aa.pair_subset_count(n);
        auto end = chrono::high_resolution_clock::now();
        auto t = chrono::duration_cast<chrono::microseconds>(end - start).count();
        cout << "  n=" << n << ": " << count << " пар, " << t << " μs" << endl;
        // Теория: 3^n
        cout << "    3^" << n << " = " << (long long)pow(3, n) << endl;
    }

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_B_CPP
