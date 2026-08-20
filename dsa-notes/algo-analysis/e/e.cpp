#ifndef ALGO_ANALYSIS_E_CPP
#define ALGO_ANALYSIS_E_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <chrono>
#include <random>
using namespace std;

// =============================================================
// E. ПРАКТИЧЕСКИЙ АНАЛИЗ
// =============================================================
// Структура md: A. Эмпирический анализ
//               → B. Статистические методы
//
// PracticalAnalysis — наследует RecursionAnalysis (d.cpp).
// Профилирование, бенчмаркинг, кросс-алгоритмический анализ,
// регрессионный анализ, доверительные интервалы.

#ifndef INSIDE_ALGO_ANALYSIS_E
#define INSIDE_ALGO_ANALYSIS_E
#include "../d/d.cpp"
#endif

struct PracticalAnalysis : RecursionAnalysis {

// =============================================================
// A. ЭМПИРИЧЕСКИЙ АНАЛИЗ
// =============================================================

// --- A.1. Бенчмаркинг: замер времени функции ---
// Запускает func(n) m раз, возвращает медиану времени (микросекунды).
double benchmark(function<void(int)> func, int n, int m = 20) {
    vector<double> times(m);
    for (int i = 0; i < m; i++) {
        auto start = chrono::high_resolution_clock::now();
        func(n);
        auto end = chrono::high_resolution_clock::now();
        times[i] = chrono::duration_cast<chrono::microseconds>(end - start).count();
    }
    sort(times.begin(), times.end());
    return times[m / 2];  // медиана
}

// --- A.2. Генерация данных: типы ---
vector<int> generate_random(int n) {
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = rand();
    return a;
}

vector<int> generate_sorted(int n) {
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = i;
    return a;
}

vector<int> generate_reverse(int n) {
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = n - i;
    return a;
}

vector<int> generate_partially_sorted(int n, int swaps = 10) {
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = i;
    for (int i = 0; i < swaps; i++) {
        int i1 = rand() % n, i2 = rand() % n;
        swap(a[i1], a[i2]);
    }
    return a;
}

// --- A.3. Статистика выборки ---
struct Stats {
    double mean, median, stddev, min_val, max_val;
};

Stats compute_stats(vector<double> v) {
    sort(v.begin(), v.end());
    int n = (int)v.size();
    Stats s;
    s.min_val = v[0];
    s.max_val = v[n - 1];
    s.median = (n % 2 == 1) ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2.0;
    double sum = 0;
    for (double x : v) sum += x;
    s.mean = sum / n;
    double var = 0;
    for (double x : v) var += (x - s.mean) * (x - s.mean);
    s.stddev = sqrt(var / (n - 1));
    return s;
}

// --- A.4. Кросс-алгоритмический анализ ---
// Возвращает crossover point (примерное n, где t1 == t2).
int find_crossover(function<double(int)> t1, function<double(int)> t2,
                   int lo = 2, int hi = 10000) {
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (t1(mid) < t2(mid)) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

// =============================================================
// B. СТАТИСТИЧЕСКИЕ МЕТОДЫ
// =============================================================

// --- B.1. Линейная регрессия (МНК) ---
// Данные: (x[i], y[i]). Модель: y = a + b*x.
// Возвращает {a, b, R²}.
struct RegressionResult {
    double a, b, R2;
};

RegressionResult linear_regression(const vector<double>& x, const vector<double>& y) {
    int n = (int)x.size();
    double sx = 0, sy = 0, sxy = 0, sxx = 0;
    for (int i = 0; i < n; i++) {
        sx += x[i]; sy += y[i];
        sxy += x[i] * y[i]; sxx += x[i] * x[i];
    }
    double x_mean = sx / n, y_mean = sy / n;
    double b = (sxy - n * x_mean * y_mean) / (sxx - n * x_mean * x_mean);
    double a = y_mean - b * x_mean;
    // R²
    double ss_tot = 0, ss_res = 0;
    for (int i = 0; i < n; i++) {
        ss_tot += (y[i] - y_mean) * (y[i] - y_mean);
        ss_res += (y[i] - (a + b * x[i])) * (y[i] - (a + b * x[i]));
    }
    double R2 = (ss_tot > 1e-300) ? 1.0 - ss_res / ss_tot : 1.0;
    return {a, b, R2};
}

// --- B.2. Регрессия для определения степени полинома ---
// Логарифмирование: log T(n) = log a + b * log n.
// Возвращает оценку степени b.
double estimate_polynomial_degree(const vector<int>& sizes,
                                   const vector<double>& times) {
    vector<double> log_n, log_t;
    for (int i = 0; i < (int)sizes.size(); i++) {
        if (sizes[i] > 0 && times[i] > 0) {
            log_n.push_back(log(sizes[i]));
            log_t.push_back(log(times[i]));
        }
    }
    auto [a, b, R2] = linear_regression(log_n, log_t);
    return b;
}

// --- B.3. 95% доверительный интервал ---
// Возвращает {нижняя, верхняя} границы.
pair<double, double> confidence_interval_95(const vector<double>& sample) {
    int n = (int)sample.size();
    double mean = 0;
    for (double x : sample) mean += x;
    mean /= n;
    double var = 0;
    for (double x : sample) var += (x - mean) * (x - mean);
    double se = sqrt(var / n) / sqrt((double)n);
    // Используем z=1.96 для 95% (при больших n)
    double margin = 1.96 * se;
    return {mean - margin, mean + margin};
}

// --- B.4. Bootstrap: оценка стандартной ошибки медианы ---
double bootstrap_se_median(vector<double> sample, int B = 1000) {
    int n = (int)sample.size();
    vector<double> medians(B);
    for (int b = 0; b < B; b++) {
        vector<double> boot(n);
        for (int i = 0; i < n; i++) boot[i] = sample[rand() % n];
        sort(boot.begin(), boot.end());
        medians[b] = (n % 2 == 1) ? boot[n / 2] : (boot[n / 2 - 1] + boot[n / 2]) / 2.0;
    }
    double mean = 0;
    for (double m : medians) mean += m;
    mean /= B;
    double var = 0;
    for (double m : medians) var += (m - mean) * (m - mean);
    return sqrt(var / (B - 1));
}

}; // struct PracticalAnalysis

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_E_MAIN
int main() {
    PracticalAnalysis pa;
    srand(42);

    cout << "=== A. ЭМПИРИЧЕСКИЙ АНАЛИЗ ===" << endl;

    cout << "--- Бенчмаркинг: разные типы данных ---" << endl;
    {
        for (int n : {1000, 10000, 100000}) {
            auto rand_data = pa.generate_random(n);
            auto sorted_data = pa.generate_sorted(n);
            auto rev_data = pa.generate_reverse(n);
            auto partial_data = pa.generate_partially_sorted(n, n / 10);

            auto t_rand = pa.benchmark([&](int) { sort(rand_data.begin(), rand_data.end()); }, n, 5);
            auto t_sorted = pa.benchmark([&](int) { sort(sorted_data.begin(), sorted_data.end()); }, n, 5);
            auto t_rev = pa.benchmark([&](int) { sort(rev_data.begin(), rev_data.end()); }, n, 5);
            auto t_part = pa.benchmark([&](int) { sort(partial_data.begin(), partial_data.end()); }, n, 5);

            cout << "  n=" << n << ": random=" << t_rand << "μs sorted=" << t_sorted
                 << "μs reverse=" << t_rev << "μs partial=" << t_part << "μs" << endl;
        }
    }

    cout << "\n--- Статистика замеров ---" << endl;
    {
        vector<double> times;
        auto data = pa.generate_random(10000);
        for (int i = 0; i < 20; i++) {
            auto start = chrono::high_resolution_clock::now();
            sort(data.begin(), data.end());
            auto end = chrono::high_resolution_clock::now();
            times.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
            shuffle(data.begin(), data.end(), default_random_engine(42 + i));
        }
        auto s = pa.compute_stats(times);
        cout << "  20 замеров sort(n=10000):" << endl;
        cout << "    mean=" << s.mean << " median=" << s.median
             << " stddev=" << s.stddev << endl;
        cout << "    min=" << s.min_val << " max=" << s.max_val << endl;
    }

    cout << "\n--- Кросс-алгоритмический анализ ---" << endl;
    {
        // QuickSort vs Insertion Sort
        auto qs_time = [&](int n) -> double {
            vector<int> a(n);
            for (int i = 0; i < n; i++) a[i] = rand();
            auto start = chrono::high_resolution_clock::now();
            sort(a.begin(), a.end());
            auto end = chrono::high_resolution_clock::now();
            return chrono::duration_cast<chrono::microseconds>(end - start).count();
        };
        auto is_time = [&](int n) -> double {
            vector<int> a(n);
            for (int i = 0; i < n; i++) a[i] = n - i;
            auto start = chrono::high_resolution_clock::now();
            // Insertion sort
            for (int i = 1; i < n; i++) {
                int key = a[i], j = i - 1;
                while (j >= 0 && a[j] > key) { a[j + 1] = a[j]; j--; }
                a[j + 1] = key;
            }
            auto end = chrono::high_resolution_clock::now();
            return chrono::duration_cast<chrono::microseconds>(end - start).count();
        };
        int crossover = pa.find_crossover(is_time, qs_time, 2, 500);
        cout << "  Crossover (Insertion vs QuickSort): n* ≈ " << crossover << endl;
    }

    cout << "\n=== B. СТАТИСТИЧЕСКИЕ МЕТОДЫ ===" << endl;

    cout << "--- Линейная регрессия: определение степени ---" << endl;
    {
        vector<int> sizes = {100, 500, 1000, 5000, 10000};
        vector<double> times;
        for (int n : sizes) {
            vector<int> a(n);
            for (int i = 0; i < n; i++) a[i] = rand();
            auto start = chrono::high_resolution_clock::now();
            sort(a.begin(), a.end());
            auto end = chrono::high_resolution_clock::now();
            times.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }
        double degree = pa.estimate_polynomial_degree(sizes, times);
        cout << "  Оценка степени: " << degree << " (ожидаем ~1 для O(n log n))" << endl;

        // Линейная регрессия log-log
        vector<double> log_n, log_t;
        for (int i = 0; i < (int)sizes.size(); i++) {
            log_n.push_back(log(sizes[i]));
            log_t.push_back(log(times[i]));
        }
        auto [a, b, R2] = pa.linear_regression(log_n, log_t);
        cout << "  log(T) = " << a << " + " << b << " * log(n)" << endl;
        cout << "  R² = " << R2 << endl;
    }

    cout << "\n--- 95% доверительный интервал ---" << endl;
    {
        vector<double> sample;
        for (int i = 0; i < 30; i++) {
            vector<int> a(10000);
            for (int j = 0; j < 10000; j++) a[j] = rand();
            auto start = chrono::high_resolution_clock::now();
            sort(a.begin(), a.end());
            auto end = chrono::high_resolution_clock::now();
            sample.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }
        auto [lo, hi] = pa.confidence_interval_95(sample);
        auto s = pa.compute_stats(sample);
        cout << "  30 замеров sort(n=10000):" << endl;
        cout << "    mean=" << s.mean << " 95% CI=[" << lo << ", " << hi << "]" << endl;
    }

    cout << "\n--- Bootstrap SE медианы ---" << endl;
    {
        vector<double> sample;
        for (int i = 0; i < 50; i++) {
            vector<int> a(10000);
            for (int j = 0; j < 10000; j++) a[j] = rand();
            auto start = chrono::high_resolution_clock::now();
            sort(a.begin(), a.end());
            auto end = chrono::high_resolution_clock::now();
            sample.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }
        double se = pa.bootstrap_se_median(sample);
        auto s = pa.compute_stats(sample);
        cout << "  Bootstrap SE(median) = " << se << " (median=" << s.median << ")" << endl;
    }

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_E_CPP
