#ifndef ALGO_ANALYSIS_F_CPP
#define ALGO_ANALYSIS_F_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdlib>
using namespace std;

// =============================================================
// F. ПРОСТРАНСТВЕННАЯ СЛОЖНОСТЬ
// =============================================================
// Структура md: A. Модели памяти
//               → B. Анализ использования памяти
//               → C. Cache-Oblivious алгоритмы
//
// SpaceComplexity — наследует PracticalAnalysis (e.cpp).
// Анализ памяти, in-place алгоритмы, cache-oblivious.

#ifndef INSIDE_ALGO_ANALYSIS_F
#define INSIDE_ALGO_ANALYSIS_F
#include "../e/e.cpp"
#endif

struct SpaceComplexity : PracticalAnalysis {

// =============================================================
// B. АНАЛИЗ ИСПОЛЬЗОВАНИЯ ПАМЯТИ
// =============================================================

// --- B.1. Подсчёт памяти: in-place vs out-of-place ---
// Возвращает память в элементах (доп. к входным данным).

// In-place: Bubble Sort — O(1) доп. памяти
void bubble_sort_inplace(vector<int>& a) {
    int n = (int)a.size();
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (a[j] > a[j + 1]) swap(a[j], a[j + 1]);
}
// Доп. память: O(1) — только временные переменные i, j, temp

// Out-of-place: Merge Sort — O(n) доп. памяти
void merge(vector<int>& a, int lo, int mid, int hi) {
    vector<int> temp(hi - lo);  // O(n) доп. памяти!
    int i = lo, j = mid, k = 0;
    while (i < mid && j < hi) {
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while (i < mid) temp[k++] = a[i++];
    while (j < hi) temp[k++] = a[j++];
    for (int k = 0; k < hi - lo; k++) a[lo + k] = temp[k];
}

void merge_sort_inplace(vector<int>& a, int lo, int hi) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_inplace(a, lo, mid);
    merge_sort_inplace(a, mid, hi);
    merge(a, lo, mid, hi);
}

// In-place Quick Sort — O(log n) доп. памяти (стек рекурсии)
void quick_sort_inplace(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return;
    int pivot = a[lo + rand() % (hi - lo)];
    int i = lo, j = hi - 1;
    while (i <= j) {
        while (i <= j && a[i] < pivot) i++;
        while (i <= j && a[j] > pivot) j--;
        if (i <= j) { swap(a[i], a[j]); i++; j--; }
    }
    quick_sort_inplace(a, lo, j + 1);
    quick_sort_inplace(a, i, hi);
}
// Доп. память: O(log n) — стек рекурсии

// --- B.2. Сравнение памяти разных сортировок ---
struct SpaceComparison {
    string name;
    int extra_memory;  // в элементах
    string description;
};

vector<SpaceComparison> compare_memory() {
    return {
        {"Bubble Sort", 1, "O(1) — in-place"},
        {"Selection Sort", 1, "O(1) — in-place"},
        {"Insertion Sort", 1, "O(1) — in-place"},
        {"Quick Sort", -1, "O(log n) — стек рекурсии"},
        {"Merge Sort", -1, "O(n) — временный массив"},
        {"Heap Sort", 1, "O(1) — in-place"},
        {"Counting Sort", -2, "O(k) — массив счётчиков"},
        {"Radix Sort", -2, "O(n + k) — вспомогательный массив"}
    };
}

// =============================================================
// C. CACHE-OBLIVIOUS АЛГОРИТМЫ
// =============================================================

// --- C.1. Кэш-промахи: демонстрация ---
// Последовательный vs случайный обход массива.
// Подсчёт «кэш-промахов» (условно: каждый переход > 64 байт = промах).

long long cache_misses_sequential(const vector<int>& a, int cache_line = 16) {
    // cache_line — число int в одной кэш-линии (64 байта / 4 байта = 16)
    long long misses = 0;
    for (int i = 0; i < (int)a.size(); i += cache_line) {
        misses++;  // каждый блок = 1 промах
    }
    return misses;
}

long long cache_misses_random(const vector<int>& a, int iterations = 10000) {
    long long misses = 0;
    int n = (int)a.size();
    for (int i = 0; i < iterations; i++) {
        int idx = rand() % n;
        misses++;  // каждый random access = потенциальный промах
    }
    return misses;
}

// --- C.2. Tiled Matrix Multiplication ---
// Блочное умножение: O(n³) арифметика, но лучше cache behavior.
void matrix_multiply_naive(const vector<vector<double>>& A,
                            const vector<vector<double>>& B,
                            vector<vector<double>>& C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++)
                C[i][j] += A[i][k] * B[k][j];
        }
}

void matrix_multiply_tiled(const vector<vector<double>>& A,
                            const vector<vector<double>>& B,
                            vector<vector<double>>& C, int n, int tile) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = 0;
    for (int ii = 0; ii < n; ii += tile)
        for (int jj = 0; jj < n; jj += tile)
            for (int kk = 0; kk < n; kk += tile) {
                int i_end = min(ii + tile, n);
                int j_end = min(jj + tile, n);
                int k_end = min(kk + tile, n);
                for (int i = ii; i < i_end; i++)
                    for (int j = jj; j < j_end; j++)
                        for (int k = kk; k < k_end; k++)
                            C[i][j] += A[i][k] * B[k][j];
            }
}

// --- C.3. Cache complexity ---
// Оценка I/O операций.
long long io_estimate(int N, int B, int M) {
    // Внешняя сортировка: O((N/B) * log_{M/B}(N/M))
    if (M <= B) return N;  // degenerate
    int runs = (N + M - 1) / M;  // число проходов
    int merge_factor = M / B;    // k-way merge
    long long io = 0;
    int current = N;
    while (current > M) {
        io += current / B;  // чтение + запись
        current = (current + merge_factor - 1) / merge_factor;
    }
    io += N / B;  // финальное чтение
    return io;
}

}; // struct SpaceComplexity

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_F_MAIN
int main() {
    SpaceComplexity sc;
    srand(42);

    cout << "=== B. АНАЛИЗ ПАМЯТИ ===" << endl;

    cout << "--- Сравнение памяти сортировок ---" << endl;
    {
        auto comps = sc.compare_memory();
        for (auto& c : comps) {
            string mem = (c.extra_memory == 1) ? "O(1)" :
                         (c.extra_memory == -1) ? "O(log n)" :
                         (c.extra_memory == -2) ? "O(n+k)" : "O(?)";
            cout << "  " << c.name << ": " << mem << " — " << c.description << endl;
        }
    }

    cout << "\n--- In-place vs Out-of-place: время и память ---" << endl;
    {
        int n = 100000;
        vector<int> a(n), b(n);
        for (int i = 0; i < n; i++) { a[i] = rand(); b[i] = a[i]; }

        // Bubble Sort (in-place, O(n²))
        auto start1 = chrono::high_resolution_clock::now();
        sc.bubble_sort_inplace(a);
        auto end1 = chrono::high_resolution_clock::now();
        auto t1 = chrono::duration_cast<chrono::milliseconds>(end1 - start1).count();

        // Quick Sort (in-place, O(n log n))
        auto start2 = chrono::high_resolution_clock::now();
        sc.quick_sort_inplace(b, 0, n);
        auto end2 = chrono::high_resolution_clock::now();
        auto t2 = chrono::duration_cast<chrono::milliseconds>(end2 - start2).count();

        cout << "  n=" << n << ":" << endl;
        cout << "    Bubble Sort (in-place O(1)): " << t1 << " ms" << endl;
        cout << "    Quick Sort (in-place O(log n)): " << t2 << " ms" << endl;
    }

    cout << "\n--- Память рекурсии: Merge Sort vs Quick Sort ---" << endl;
    {
        int n = 1000000;
        cout << "  n=" << n << ":" << endl;
        cout << "    Merge Sort доп. память: O(n) = " << n << " элементов" << endl;
        cout << "    Quick Sort доп. память: O(log n) = " << (int)log2(n) << " элементов" << endl;
    }

    cout << "\n=== C. CACHE-OBLIVIOUS ===" << endl;

    cout << "--- Кэш-промахи: последовательный vs случайный ---" << endl;
    {
        int n = 1000000;
        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = i;
        long long seq_misses = sc.cache_misses_sequential(a);
        long long rand_misses = sc.cache_misses_random(a, 100000);
        cout << "  n=" << n << ", cache_line=16 int:" << endl;
        cout << "    Последовательный: " << seq_misses << " промахов" << endl;
        cout << "    Случайный (100k accesses): " << rand_misses << " промахов" << endl;
        cout << "    Ratio: " << (double)rand_misses / seq_misses << "x" << endl;
    }

    cout << "\n--- Tiled Matrix Multiply: naive vs tiled ---" << endl;
    {
        int n = 256;
        vector<vector<double>> A(n, vector<double>(n)), B(n, vector<double>(n));
        vector<vector<double>> C_naive(n, vector<double>(n)), C_tiled(n, vector<double>(n));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                A[i][j] = rand() % 10;
                B[i][j] = rand() % 10;
            }

        auto start1 = chrono::high_resolution_clock::now();
        sc.matrix_multiply_naive(A, B, C_naive, n);
        auto end1 = chrono::high_resolution_clock::now();
        auto t1 = chrono::duration_cast<chrono::microseconds>(end1 - start1).count();

        int tile = 32;
        auto start2 = chrono::high_resolution_clock::now();
        sc.matrix_multiply_tiled(A, B, C_tiled, n, tile);
        auto end2 = chrono::high_resolution_clock::now();
        auto t2 = chrono::duration_cast<chrono::microseconds>(end2 - start2).count();

        // Проверка корректности
        bool correct = true;
        for (int i = 0; i < n && correct; i++)
            for (int j = 0; j < n && correct; j++)
                if (fabs(C_naive[i][j] - C_tiled[i][j]) > 1e-6) correct = false;

        cout << "  " << n << "x" << n << " matrix, tile=" << tile << ":" << endl;
        cout << "    Naive: " << t1 << " μs" << endl;
        cout << "    Tiled: " << t2 << " μs" << endl;
        cout << "    Speedup: " << (t1 > 0 ? (double)t1/t2 : 0) << "x" << endl;
        cout << "    Correct: " << (correct ? "YES" : "NO") << endl;
    }

    cout << "\n--- I/O оценка (внешняя сортировка) ---" << endl;
    {
        int N = 1000000, B = 256, M = 10000;
        long long io = sc.io_estimate(N, B, M);
        cout << "  N=" << N << " B=" << B << " M=" << M << ": ~" << io << " I/O операций" << endl;
    }

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_F_CPP
