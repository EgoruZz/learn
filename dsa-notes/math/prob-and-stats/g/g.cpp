#ifndef PROB_AND_STATS_G_CPP
#define PROB_AND_STATS_G_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
using namespace std;

// =============================================================
// G. ПРЕДЕЛЬНЫЕ ТЕОРЕМЫ
// =============================================================
// Структура md: A. Закон больших чисел (ЗБЧ)
//               → B. Центральная предельная теорема (ЦПТ)
//               → C. Нормальная аппроксимация
//               → D. Сходимость случайных величин
//
// LimitTheorems — наследует NumericalCharacteristics (f.cpp).
// Слабое и сильное ЗБЧ, классическая и ляпуновская ЦПТ,
// нормальная аппроксимация B(n,p) и Po(l),
// три типа сходимости, критерий Перро.

#ifndef INSIDE_PROB_AND_STATS_G
#define INSIDE_PROB_AND_STATS_G
#include "../f/f.cpp"
#endif

struct LimitTheorems : NumericalCharacteristics {

// =============================================================
// A. ЗАКОН БОЛЬШИХ ЧИСЕЛ (ЗБЧ)
// =============================================================

// --- A.1. Слабое ЗБЧ: демонстрация сходимости ξ̄_n → μ ---
// Генерируем m выборок размера n, считаем долю |ξ̄ - μ| >= eps.
// Возвращает P(|ξ̄_n - μ| >= eps) (оценка).
// O(m * n) время.
double weak_law_convergence(int n, int m, double mu, double eps,
                           double (*gen)()) {
    int violations = 0;
    for (int i = 0; i < m; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) sum += gen();
        double xbar = sum / n;
        if (fabs(xbar - mu) >= eps) violations++;
    }
    return (double)violations / m;
}

// --- A.2. Теорема Бернулли: частота → вероятность ---
// n испытаний Бернулли, m повторений.
// Возвращает |ν/n - p| для каждого m.
// O(m * n) время.
vector<double> bernoulli_law(int n, int m, double p) {
    vector<double> freqs(m);
    for (int i = 0; i < m; i++) {
        int successes = 0;
        for (int j = 0; j < n; j++)
            if ((double)rand() / RAND_MAX < p) successes++;
        freqs[i] = (double)successes / n;
    }
    return freqs;
}

// --- A.3. Скорость сходимости (теорема Чебышёва) ---
// Верхняя граница: P(|ξ̄ - μ| >= eps) <= D/(n*eps^2).
// Возвращает границу и эмпирическую оценку.
pair<double, double> convergence_rate(int n, double mu, double sigma,
                                      double eps, double (*gen)()) {
    double bound = (sigma * sigma) / (n * eps * eps);
    // Эмпирика
    int trials = 5000;
    int violations = 0;
    for (int i = 0; i < trials; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) sum += gen();
        if (fabs(sum / n - mu) >= eps) violations++;
    }
    return {bound, (double)violations / trials};
}

// =============================================================
// B. ЦЕНТРАЛЬНАЯ ПРЕДЕЛЬНАЯ ТЕОРЕМА (ЦПТ)
// =============================================================

// --- B.1. Эмпирическое распределение Z_n ---
// Генерируем m выборок суммы n СВ, стандартизируем, строим гистограмму.
// Возвращает среднее и дисперсию стандартизированной суммы.
pair<double, double> ctp_empirical(int n, int m, double mu, double sigma,
                                   double (*gen)()) {
    vector<double> z_samples(m);
    for (int i = 0; i < m; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) sum += gen();
        z_samples[i] = (sum - n * mu) / (sigma * sqrt(n));
    }
    double mean = 0, var = 0;
    for (double z : z_samples) { mean += z; var += z * z; }
    mean /= m; var /= m; var -= mean * mean;
    return {mean, var};
}

// --- B.2. Колмогоров-Смирнов: расстояние до N(0,1) ---
// D = sup|F_n(x) - Phi(x)|.
// O(m log m + n_steps) время.
double kolmogorov_smirnov_distance(const vector<double>& sample) {
    int n = (int)sample.size();
    vector<double> sorted_sample = sample;
    sort(sorted_sample.begin(), sorted_sample.end());
    double max_diff = 0.0;
    for (int i = 0; i < n; i++) {
        double fn = (double)(i + 1) / n;
        double fx = phi_c(sorted_sample[i]);
        max_diff = max(max_diff, fabs(fn - fx));
        double fn_prev = (double)i / n;
        max_diff = max(max_diff, fabs(fn_prev - fx));
    }
    return max_diff;
}

// --- B.3. QQ-plot: отклонение квантилей от нормальных ---
// Для каждого квантиля p_i = (i + 0.5) / n:
//   x_i = quantile(sample, p_i), y_i = Phi^{-1}(p_i).
// Возвращает максимальное |x_i - y_i|.
double qq_plot_max_deviation(const vector<double>& sample) {
    int n = (int)sample.size();
    vector<double> sorted_sample = sample;
    sort(sorted_sample.begin(), sorted_sample.end());
    double max_dev = 0.0;
    for (int i = 0; i < n; i++) {
        double p = (i + 0.5) / n;
        // Phi^{-1}(p) через бинарного поиска
        double lo = -10, hi = 10;
        for (int iter = 0; iter < 50; iter++) {
            double mid = (lo + hi) / 2;
            if (phi_c(mid) < p) lo = mid; else hi = mid;
        }
        double y_i = (lo + hi) / 2;
        max_dev = max(max_dev, fabs(sorted_sample[i] - y_i));
    }
    return max_dev;
}

// =============================================================
// C. НОРМАЛЬНАЯ АППРОКСИМАЦИЯ
// =============================================================

// --- C.1. Аппроксимация биномиального B(n,p) ---
// P(S_n <= k) ~ Phi((k + 0.5 - np) / sqrt(npq)).
// Возвращает {точное, аппроксимация}.
pair<double, double> binomial_normal_approx(long long n, long long k,
                                            double p) {
    double q = 1.0 - p;
    double np = n * p;
    double npq = n * p * q;
    // Точное (суммирование)
    double exact = binomial_cdf(n, k, p);
    // Аппроксимация
    double sigma = sqrt(npq);
    double approx = (sigma > 1e-12) ? phi_c((k + 0.5 - np) / sigma) : 0.0;
    return {exact, approx};
}

// --- C.2. Аппроксимация Пуассона Po(lambda) ---
// P(X <= k) ~ Phi((k + 0.5 - lambda) / sqrt(lambda)).
pair<double, double> poisson_normal_approx(long long k, double lambda) {
    double exact = poisson_cdf_c(k, lambda);
    double sigma = sqrt(lambda);
    double approx = (sigma > 1e-12) ? phi_c((k + 0.5 - lambda) / sigma) : 0.0;
    return {exact, approx};
}

// --- C.3. Оценка необходимого n для точности epsilon ---
// Для B(n,p): |P(S_n <= k) - Phi(...)| < eps при n >= n_min.
// Эмпирический подбор.
long long required_n_for_accuracy(double p, double target_eps) {
    for (long long n = 10; n <= 5000; n++) {
        long long k = (long long)(n * p);
        auto [exact, approx] = binomial_normal_approx(n, k, p);
        if (fabs(exact - approx) < target_eps) return n;
    }
    return -1;  // не найдено
}

// =============================================================
// D. СХОДИМОСТЬ СЛУЧАЙНЫХ ВЕЛИЧИН
// =============================================================

// --- D.1. Сходимость по вероятности ---
// P(|ξ_n - ξ| > eps) → 0.
// Генерируем m выборок длины n, считаем нарушения.
double convergence_in_probability(int n, int m, double eps,
                                  double (*gen_xn)(int, int),
                                  double target_x) {
    int violations = 0;
    for (int i = 0; i < m; i++) {
        double val = gen_xn(n, i);
        if (fabs(val - target_x) > eps) violations++;
    }
    return (double)violations / m;
}

// --- D.2. QQ-plot для проверки нормальности ---
// Возвращает вектор пар (теоретический квантиль, выборочный квантиль).
vector<pair<double, double>> qq_plot_data(const vector<double>& sample) {
    int n = (int)sample.size();
    vector<double> sorted_sample = sample;
    sort(sorted_sample.begin(), sorted_sample.end());
    vector<pair<double, double>> result(n);
    for (int i = 0; i < n; i++) {
        double p = (i + 0.5) / n;
        double lo = -10, hi = 10;
        for (int iter = 0; iter < 50; iter++) {
            double mid = (lo + hi) / 2;
            if (phi_c(mid) < p) lo = mid; else hi = mid;
        }
        result[i] = {(lo + hi) / 2, sorted_sample[i]};
    }
    return result;
}

// =============================================================
// E. УТИЛИТЫ
// =============================================================

// --- E.1. Стандартизация выборки ---
vector<double> standardize(const vector<double>& sample) {
    double mean = 0, var = 0;
    int n = (int)sample.size();
    for (double x : sample) mean += x;
    mean /= n;
    for (double x : sample) var += (x - mean) * (x - mean);
    var /= n;
    double sigma = sqrt(var);
    vector<double> result(n);
    if (sigma > 1e-300)
        for (int i = 0; i < n; i++)
            result[i] = (sample[i] - mean) / sigma;
    return result;
}

// --- E.2. Гистограмма (бины) ---
// Возвращает количество элементов в каждом бине.
vector<int> histogram(const vector<double>& sample, int nbins,
                      double lo, double hi) {
    vector<int> bins(nbins, 0);
    double w = (hi - lo) / nbins;
    for (double x : sample) {
        int idx = (int)((x - lo) / w);
        if (idx >= 0 && idx < nbins) bins[idx]++;
    }
    return bins;
}

}; // struct LimitTheorems

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_G_MAIN
int main() {
    LimitTheorems lt;
    srand(42);

    cout << "=== A. ЗАКОН БОЛЬШИХ ЧИСЕЛ ===" << endl;

    cout << "--- Слабое ЗБЧ: ξ̄_n → μ для Exp(1) ---" << endl;
    auto exp_gen = []() -> double { return -log((double)rand() / RAND_MAX); };
    double mu = 1.0, sigma = 1.0;
    cout << "E[X] = " << mu << ", sigma = " << sigma << endl;
    for (int n : {10, 50, 100, 500, 1000}) {
        double p_viol = lt.weak_law_convergence(n, 2000, mu, 0.2, exp_gen);
        cout << "n=" << n << ": P(|ξ̄ - 1| >= 0.2) ≈ " << p_viol << endl;
    }

    cout << "\n--- Теорема Бернулли: частота → p ---" << endl;
    double p = 0.3;
    for (int n : {10, 100, 1000}) {
        auto freqs = lt.bernoulli_law(n, 500, p);
        double mean_freq = 0;
        for (double f : freqs) mean_freq += f;
        mean_freq /= (int)freqs.size();
        cout << "n=" << n << ": средняя частота = " << mean_freq
             << " (ожидаем " << p << ")" << endl;
    }

    cout << "\n--- Скорость сходимости (граница Чебышёва) ---" << endl;
    for (int n : {10, 50, 100, 500}) {
        auto [bound, empirical] = lt.convergence_rate(n, mu, sigma, 0.3, exp_gen);
        cout << "n=" << n << ": Chebyshev bound = " << bound
             << " empirical = " << empirical << endl;
    }

    cout << "\n=== B. ЦЕНТРАЛЬНАЯ ПРЕДЕЛЬНАЯ ТЕОРЕМА ===" << endl;

    cout << "--- ЦПТ для Exp(1): сумма n СВ ---" << endl;
    for (int n : {5, 10, 30, 100}) {
        auto [mean_z, var_z] = lt.ctp_empirical(n, 5000, mu, sigma, exp_gen);
        cout << "n=" << n << ": E[Z_n] = " << mean_z
             << " (ожидаем 0), D[Z_n] = " << var_z
             << " (ожидаем 1)" << endl;
    }

    cout << "\n--- Колмогоров-Смирнов для нормальности ---" << endl;
    vector<double> norm50, norm500;
    for (int i = 0; i < 50; i++) norm50.push_back(lt.normal_sample(0, 1));
    for (int i = 0; i < 500; i++) norm500.push_back(lt.normal_sample(0, 1));
    cout << "KS distance (n=50):  " << lt.kolmogorov_smirnov_distance(norm50) << endl;
    cout << "KS distance (n=500): " << lt.kolmogorov_smirnov_distance(norm500) << endl;

    cout << "\n--- QQ-plot: отклонение квантилей ---" << endl;
    cout << "QQ max dev (n=50):  " << lt.qq_plot_max_deviation(norm50) << endl;
    cout << "QQ max dev (n=500): " << lt.qq_plot_max_deviation(norm500) << endl;

    cout << "\n=== C. НОРМАЛЬНАЯ АППРОКСИМАЦИЯ ===" << endl;

    cout << "--- B(n, p) ~ N(np, npq) ---" << endl;
    {
        long long n = 10; double p = 0.5;
        auto [exact, approx] = lt.binomial_normal_approx(n, 5, p);
        cout << "B(10,0.5): P(S<=5) exact=" << exact << " approx=" << approx << endl;
    }
    {
        long long n = 20; double p = 0.3;
        auto [exact, approx] = lt.binomial_normal_approx(n, 6, p);
        cout << "B(20,0.3): P(S<=6) exact=" << exact << " approx=" << approx << endl;
    }
    {
        long long n = 100; double p = 0.2;
        auto [exact, approx] = lt.binomial_normal_approx(n, 25, p);
        cout << "B(100,0.2): P(S<=25) exact=" << exact << " approx=" << approx << endl;
    }

    cout << "\n--- Po(λ) ~ N(λ, λ) ---" << endl;
    for (double lambda : {10.0, 30.0, 50.0}) {
        long long k = (long long)lambda;
        auto [exact, approx] = lt.poisson_normal_approx(k, lambda);
        cout << "Po(" << lambda << "): P(X<=" << k << ") exact=" << exact
             << " approx=" << approx << endl;
    }

    cout << "\n--- Необходимое n для точности 0.01 ---" << endl;
    for (double p : {0.1, 0.3, 0.5}) {
        long long n_min = lt.required_n_for_accuracy(p, 0.01);
        cout << "p=" << p << ": n >= " << n_min << endl;
    }

    cout << "\n=== D. СХОДИМОСТЬ СЛУЧАЙНЫХ ВЕЛИЧИН ===" << endl;

    cout << "--- Сходимость по вероятности: ξ̄_n → 0 ---" << endl;
    // ξ_n = (1/n) * sum U(0,1) - 0.5 → 0 по вероятности
    auto u_gen = []() -> double { return (double)rand() / RAND_MAX - 0.5; };
    for (int n : {10, 50, 100, 500}) {
        double p_viol = lt.convergence_in_probability(n, 2000, 0.1,
            [](int n, int) -> double {
                double s = 0;
                for (int j = 0; j < n; j++) s += (double)rand() / RAND_MAX;
                return s / n - 0.5;
            }, 0.0);
        cout << "n=" << n << ": P(|ξ̄ - 0| >= 0.1) ≈ " << p_viol << endl;
    }

    cout << "\n--- Стандартизация и проверка нормальности ---" << endl;
    vector<double> sum100;
    for (int i = 0; i < 1000; i++) {
        double s = 0;
        for (int j = 0; j < 100; j++) s += (double)rand() / RAND_MAX;
        sum100.push_back(s);
    }
    auto std_sum = lt.standardize(sum100);
    cout << "E[standardized sum] = " << lt.moment(std_sum, 1) << endl;
    cout << "D[standardized sum] = " << lt.central_moment(std_sum, 2) << endl;
    cout << "KS distance to N(0,1): " << lt.kolmogorov_smirnov_distance(std_sum) << endl;

    cout << "\n--- Гистограмма стандартизированной суммы ---" << endl;
    auto bins = lt.histogram(std_sum, 10, -3, 3);
    int max_bin = *max_element(bins.begin(), bins.end());
    for (int i = 0; i < 10; i++) {
        double lo = -3.0 + i * 0.6;
        double hi = lo + 0.6;
        if (fabs(lo) < 1e-10) lo = 0.0;
        if (fabs(hi) < 1e-10) hi = 0.0;
        int bar_len = (max_bin > 0) ? bins[i] * 40 / max_bin : 0;
        cout << "  [" << lo << "," << lo + 0.6 << ") ";
        for (int j = 0; j < bar_len; j++) cout << "#";
        cout << " " << bins[i] << endl;
    }

    return 0;
}
#endif

#endif // PROB_AND_STATS_G_CPP
