#ifndef PROB_AND_STATS_I_CPP
#define PROB_AND_STATS_I_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <set>
using namespace std;

// =============================================================
// I. ВЕРОЯТНОСТНЫЕ АЛГОРИТМЫ И МЕТОДЫ
// =============================================================
// Структура md: A. Методы Монте-Карло
//               -> B. Рандомизированные алгоритмы
//               -> C. Случайные блуждания
//               -> D. Эвристические методы оптимизации
//               -> E. Вероятностный метод в комбинаторике
//               -> F. Приложения в Data Science
//
// ProbabilisticAlgorithms -- наследует MathematicalStatistics (h.cpp).
// MC-оценки (pi, интегралы), рандомизированные алгоритмы,
// случайные блуждания, PageRank, имитация отжига,
// бутстрап, A/B тестирование.

#ifndef INSIDE_PROB_AND_STATS_I
#define INSIDE_PROB_AND_STATS_I
#include "../h/h.cpp"
#endif

struct ProbabilisticAlgorithms : MathematicalStatistics {

// =============================================================
// A. МЕТОДЫ МОНТЕ-КАРЛО
// =============================================================

// --- A.1. Оценка числа pi ---
// n случайных точек в [0,1]^2; pi ~ 4 * (внутри круга) / n.
// O(n) время.
double monte_carlo_pi(int n) {
    int inside = 0;
    for (int i = 0; i < n; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        if (x * x + y * y <= 1.0) inside++;
    }
    return 4.0 * inside / n;
}

// --- A.2. Оценка интеграла Монте-Карло ---
// I = integral_a^b f(x) dx ~ (b-a)/n * sum f(X_i), X_i ~ U(a,b).
// O(n) время.
double monte_carlo_integral(double (*f)(double), double a, double b, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        double x = a + (b - a) * ((double)rand() / RAND_MAX);
        sum += f(x);
    }
    return (b - a) * sum / n;
}

// --- A.3. Importance Sampling ---
// I = E[f(X)/g(X)] при X ~ g вместо U(a,b).
// g_pdf -- плотность, g_sample -- генератор.
// O(n) время.
double importance_sampling(double (*f)(double), double (*g_pdf)(double),
                           double (*g_sample)(), int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        double x = g_sample();
        sum += f(x) / g_pdf(x);
    }
    return sum / n;
}

// --- A.4. Antithetic Variables ---
// Используем пары (U, 1-U) вместо двух независимых U.
// Дисперсия снижается за счёт отрицательной корреляции.
// O(n) время.
double antithetic_integral(double (*f)(double), double a, double b, int n) {
    double sum = 0;
    for (int i = 0; i < n / 2; i++) {
        double u = (double)rand() / RAND_MAX;
        double x1 = a + (b - a) * u;
        double x2 = a + (b - a) * (1.0 - u);
        sum += f(x1) + f(x2);
    }
    return (b - a) * sum / n;
}

// =============================================================
// B. РАНДОМИЗИРОВАННЫЕ АЛГОРИТМЫ
// =============================================================

// --- B.1. Рандомизированный QuickSelect (поиск k-го элемента) ---
// Las Vegas: всегда правильный ответ, E[T] = O(n).
// O(n) время в среднем.
int quickselect_random(vector<int>& arr, int k, int lo, int hi) {
    if (lo == hi) return arr[lo];
    int pivot_idx = lo + rand() % (hi - lo + 1);
    swap(arr[pivot_idx], arr[hi]);
    int pivot = arr[hi];
    int i = lo;
    for (int j = lo; j < hi; j++) {
        if (arr[j] < pivot) { swap(arr[i], arr[j]); i++; }
    }
    swap(arr[i], arr[hi]);
    if (k == i) return arr[i];
    else if (k < i) return quickselect_random(arr, k, lo, i - 1);
    else return quickselect_random(arr, k, i + 1, hi);
}

// --- B.2. Тест Миллера-Рабина (проверка простоты) ---
// Monte Carlo: фиксированное время, вероятность ошибки <= 4^{-k}.
// composite -> true с вероятностью >= 1 - 4^{-k}.
bool miller_rabin_test(long long n, int k) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;
    // Разложение n-1 = 2^r * d
    long long d = n - 1;
    int r = 0;
    while (d % 2 == 0) { d /= 2; r++; }
    for (int i = 0; i < k; i++) {
        long long a = 2 + rand() % (n - 3);
        long long x = 1;
        long long base = a % n;
        long long exp = d;
        while (exp > 0) {
            if (exp & 1) x = (__int128)x * base % n;
            base = (__int128)base * base % n;
            exp >>= 1;
        }
        if (x == 1 || x == n - 1) continue;
        bool found = false;
        for (int j = 0; j < r - 1; j++) {
            x = (__int128)x * x % n;
            if (x == n - 1) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

// =============================================================
// C. СЛУЧАЙНЫЕ БЛУЖДАНИЯ
// =============================================================

// --- C.1. Случайное блуждание на Z ---
// n шагов: +1 с вероятностью p, -1 с q=1-p.
// Возвращает позицию после n шагов.
int random_walk_1d(int n, double p) {
    int pos = 0;
    for (int i = 0; i < n; i++)
        pos += ((double)rand() / RAND_MAX < p) ? 1 : -1;
    return pos;
}

// --- C.2. Число возвратов в 0 ---
// Симуляция: запускаем n шагов, считаем возвраты.
// O(n * trials) время.
int count_returns(int n, int trials) {
    int returns = 0;
    for (int t = 0; t < trials; t++) {
        int pos = 0;
        for (int i = 0; i < n; i++) {
            pos += ((double)rand() / RAND_MAX < 0.5) ? 1 : -1;
            if (pos == 0) { returns++; break; }
        }
    }
    return returns;
}

// --- C.3. PageRank ---
// power method: iterates PR = (1-d)/n + d * M * PR.
// O(max_iters * (n + m)) время.
vector<double> pagerank(const vector<vector<int>>& adj, double d = 0.85,
                        int max_iters = 100, double eps = 1e-8) {
    int n = (int)adj.size();
    vector<double> pr(n, 1.0 / n);
    vector<int> out_degree(n, 0);
    for (int i = 0; i < n; i++)
        out_degree[i] = (int)adj[i].size();
    for (int iter = 0; iter < max_iters; iter++) {
        vector<double> pr_new(n, (1.0 - d) / n);
        for (int v = 0; v < n; v++) {
            if (out_degree[v] == 0) {
                // Дangling node: распределяем pr[v] поровну
                for (int u = 0; u < n; u++)
                    pr_new[u] += d * pr[v] / n;
            } else {
                for (int u : adj[v])
                    pr_new[u] += d * pr[v] / out_degree[v];
            }
        }
        double diff = 0;
        for (int i = 0; i < n; i++)
            diff += fabs(pr_new[i] - pr[i]);
        pr = pr_new;
        if (diff < eps) break;
    }
    return pr;
}

// =============================================================
// D. ЭВРИСТИЧЕСКИЕ МЕТОДЫ ОПТИМИЗАЦИИ
// =============================================================

// --- D.1. Имитация отжига (Simulated Annealing) ---
// minimize f(x) на [lo, hi].
// temp_schedule: T(k) = T0 / (1 + k).
// O(max_iters) время.
double simulated_annealing(double (*f)(double), double lo, double hi,
                           double T0 = 100.0, int max_iters = 10000) {
    double x = lo + (hi - lo) * ((double)rand() / RAND_MAX);
    double fx = f(x);
    double best_x = x, best_fx = fx;
    for (int k = 0; k < max_iters; k++) {
        double T = T0 / (1.0 + k);
        double x_new = x + (2.0 * ((double)rand() / RAND_MAX) - 1.0) * T * 0.1;
        if (x_new < lo) x_new = lo;
        if (x_new > hi) x_new = hi;
        double fx_new = f(x_new);
        double delta = fx_new - fx;
        if (delta < 0 || (double)rand() / RAND_MAX < exp(-delta / T)) {
            x = x_new;
            fx = fx_new;
        }
        if (fx < best_fx) { best_x = x; best_fx = fx; }
    }
    return best_x;
}

// =============================================================
// E. ВЕРОЯТНОСТНЫЙ МЕТОД В КОМБИНАТОРИКЕ
// =============================================================

// --- E.1. Метод ожидания: existence proof ---
// Генерируем m случайных объектов, считаем число с нужным свойством.
// Если E[X] > 0 -> существует хотя бы один.
// O(m * cost_per_object) время.
int existence_proof(int m, bool (*has_property)(const vector<int>&)) {
    int found = 0;
    for (int i = 0; i < m; i++) {
        vector<int> obj = {(int)(rand() % 10), (int)(rand() % 10),
                           (int)(rand() % 10)};
        if (has_property(obj)) found++;
    }
    return found;
}

// =============================================================
// F. ПРИЛОЖЕНИЯ В DATA SCIENCE
// =============================================================

// --- F.1. Бутстрап (Bootstrap) ---
// Оценка стандартной ошибки статистики theta через бутстрап.
// B бутстрап-выборок, для каждой theta*_b.
// Возвращает {среднее theta*, стандартную ошибку}.
pair<double, double> bootstrap_se(const vector<double>& sample, int B,
                                  double (*statistic)(const vector<double>&)) {
    int n = (int)sample.size();
    vector<double> theta_star(B);
    for (int b = 0; b < B; b++) {
        vector<double> boot_sample(n);
        for (int i = 0; i < n; i++)
            boot_sample[i] = sample[rand() % n];
        theta_star[b] = statistic(boot_sample);
    }
    double mean = 0;
    for (double t : theta_star) mean += t;
    mean /= B;
    double var = 0;
    for (double t : theta_star) var += (t - mean) * (t - mean);
    var /= (B - 1);
    return {mean, sqrt(var)};
}

// --- F.2. A/B тест (Z-тест) ---
// H0: mu_A = mu_B. Возвращает {z-stat, p-value}.
pair<double, double> ab_test(const vector<double>& a, const vector<double>& b) {
    int na = (int)a.size(), nb = (int)b.size();
    double ma = sample_mean(a), mb = sample_mean(b);
    double sa2 = sample_variance(a), sb2 = sample_variance(b);
    double se = sqrt(sa2 / na + sb2 / nb);
    double z = (ma - mb) / se;
    double p = 2.0 * (1.0 - phi_c(fabs(z)));
    return {z, p};
}

// --- F.3. Размер выборки для A/B теста ---
// n = (z_{alpha/2} + z_beta)^2 * 2*sigma^2 / delta^2.
int ab_sample_size(double alpha, double power, double sigma, double delta) {
    double z_alpha = 1.96;
    if (fabs(alpha - 0.01) < 0.001) z_alpha = 2.576;
    double z_beta = 0.84;  // ~80% power
    if (fabs(power - 0.90) < 0.01) z_beta = 1.28;
    return (int)ceil(pow(z_alpha + z_beta, 2) * 2.0 * sigma * sigma / (delta * delta));
}

// --- F.4. Логистическая регрессия (1 шаг градиентного спуска) ---
// sigmoid(w.x) = 1 / (1 + exp(-w.x)).
// Loss = -sum [y*log(p) + (1-y)*log(1-p)].
// Возвращает new w после одного шага.
vector<double> logistic_regression_step(const vector<vector<double>>& X,
                                        const vector<int>& y,
                                        const vector<double>& w,
                                        double lr) {
    int n = (int)X.size();
    int d = (int)w.size();
    vector<double> grad(d, 0.0);
    for (int i = 0; i < n; i++) {
        double dot = 0;
        for (int j = 0; j < d; j++) dot += X[i][j] * w[j];
        double p = 1.0 / (1.0 + exp(-dot));
        double err = p - y[i];
        for (int j = 0; j < d; j++)
            grad[j] += err * X[i][j];
    }
    vector<double> w_new(d);
    for (int j = 0; j < d; j++)
        w_new[j] = w[j] - lr * grad[j] / n;
    return w_new;
}

}; // struct ProbabilisticAlgorithms

#endif // PROB_AND_STATS_I_CPP

// =============================================================
// MAIN
// =============================================================
#ifdef PROB_AND_STATS_I_MAIN
int main() {
    ProbabilisticAlgorithms pa;
    srand(42);

    cout << "=== A. МОНТЕ-КАРЛО ===" << endl;
    cout << "--- Оценка pi ---" << endl;
    for (int n : {1000, 10000, 100000, 1000000}) {
        double pi_hat = pa.monte_carlo_pi(n);
        cout << "n=" << n << ": pi_hat=" << pi_hat
             << " |error|=" << fabs(pi_hat - M_PI) << endl;
    }

    cout << "\n--- Интегрирование MC: integral_0^1 x^2 dx = 1/3 ---" << endl;
    auto x2 = [](double x) -> double { return x * x; };
    for (int n : {1000, 10000, 100000}) {
        double I = pa.monte_carlo_integral(x2, 0, 1, n);
        cout << "n=" << n << ": I_hat=" << I << " |error|=" << fabs(I - 1.0/3) << endl;
    }

    cout << "\n--- Antithetic variables ---" << endl;
    for (int n : {1000, 10000, 100000}) {
        double I = pa.antithetic_integral(x2, 0, 1, n);
        cout << "n=" << n << ": I_hat=" << I << " |error|=" << fabs(I - 1.0/3) << endl;
    }

    cout << "\n=== B. РАНДОМИЗИРОВАННЫЕ АЛГОРИТМЫ ===" << endl;
    cout << "--- QuickSelect: медиана ---" << endl;
    vector<int> arr = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int n_arr = (int)arr.size();
    int med = pa.quickselect_random(arr, n_arr / 2, 0, n_arr - 1);
    cout << "Медиана: " << med << endl;

    cout << "\n--- Miller-Rabin: проверка простоты ---" << endl;
    for (long long n : {17LL, 97LL, 100LL, 127LL, 1000003LL, 1000000007LL}) {
        bool prime = pa.miller_rabin_test(n, 10);
        cout << n << ": " << (prime ? "PRIME" : "composite") << endl;
    }

    cout << "\n=== C. СЛУЧАЙНЫЕ БЛУЖДАНИЯ ===" << endl;
    cout << "--- 1D random walk (p=0.5, 100 шагов) ---" << endl;
    for (int i = 0; i < 5; i++)
        cout << "  Позиция: " << pa.random_walk_1d(100, 0.5) << endl;

    cout << "\n--- Возвраты в 0 (1000 шагов, 100 симуляций) ---" << endl;
    int ret = pa.count_returns(1000, 100);
    cout << "Число симуляций, вернувшихся в 0: " << ret << "/100" << endl;

    cout << "\n--- PageRank на треугольнике + хвосте ---" << endl;
    // 0-1-2 треугольник, 2-3 хвост
    vector<vector<int>> adj = {{1, 2}, {0, 2}, {0, 1, 3}, {2}};
    auto pr = pa.pagerank(adj);
    cout << "PR = [";
    for (int i = 0; i < 4; i++) {
        if (i > 0) cout << ", ";
        cout << pr[i];
    }
    cout << "]" << endl;

    cout << "\n=== D. ИМИТАЦИЯ ОТЖИГА ===" << endl;
    // Минимум f(x) = (x-3)^2 + 1 на [0, 10]
    auto f = [](double x) -> double { return (x - 3) * (x - 3) + 1; };
    double x_opt = pa.simulated_annealing(f, 0, 10);
    cout << "f(x) = (x-3)^2 + 1: минимум при x = " << x_opt
         << " (ожидаем 3)" << endl;

    cout << "\n=== E. ВЕРОЯТНОСТНЫЙ МЕТОД ===" << endl;
    auto has_prop = [](const vector<int>& v) -> bool {
        return v[0] + v[1] + v[2] > 15;
    };
    int found = pa.existence_proof(1000, has_prop);
    cout << "Объектов со свойством (сумма > 15): " << found << "/1000"
         << " (E[X] > 0 -> существует)" << endl;

    cout << "\n=== F. DATA SCIENCE ===" << endl;
    cout << "--- Bootstrap SE для среднего ---" << endl;
    vector<double> sample_data;
    for (int i = 0; i < 50; i++)
        sample_data.push_back(pa.normal_sample(10, 2));
    auto mean_stat = [](const vector<double>& s) -> double {
        double sum = 0; for (double x : s) sum += x; return sum / (int)s.size();
    };
    auto [boot_mean, boot_se] = pa.bootstrap_se(sample_data, 1000, mean_stat);
    cout << "Выборочное среднее: " << pa.sample_mean(sample_data) << endl;
    cout << "Bootstrap mean: " << boot_mean << ", SE: " << boot_se << endl;

    cout << "\n--- A/B тест ---" << endl;
    vector<double> group_a, group_b;
    for (int i = 0; i < 50; i++) {
        group_a.push_back(pa.normal_sample(10, 2));
        group_b.push_back(pa.normal_sample(10.5, 2));
    }
    auto [z, p_val] = pa.ab_test(group_a, group_b);
    cout << "A: mean=" << pa.sample_mean(group_a)
         << ", B: mean=" << pa.sample_mean(group_b) << endl;
    cout << "Z = " << z << ", p-value = " << p_val
         << (p_val < 0.05 ? " (significant)" : " (not significant)") << endl;

    cout << "\n--- Размер выборки для A/B теста ---" << endl;
    int n_req = pa.ab_sample_size(0.05, 0.80, 2.0, 0.5);
    cout << "alpha=0.05, power=0.80, sigma=2, delta=0.5: n = " << n_req << endl;

    cout << "\n--- Логистическая регрессия (1 шаг) ---" << endl;
    vector<vector<double>> X = {{1, 0}, {1, 1}, {1, 2}, {1, 3}};
    vector<int> y = {0, 0, 1, 1};
    vector<double> w = {0, 0};
    auto w_new = pa.logistic_regression_step(X, y, w, 0.1);
    cout << "w до: [" << w[0] << ", " << w[1] << "]" << endl;
    cout << "w после: [" << w_new[0] << ", " << w_new[1] << "]" << endl;

    return 0;
}
#endif
