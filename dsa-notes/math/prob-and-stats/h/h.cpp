#ifndef PROB_AND_STATS_H_CPP
#define PROB_AND_STATS_H_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <map>
using namespace std;

// =============================================================
// H. МАТЕМАТИЧЕСКАЯ СТАТИСТИКА
// =============================================================
// Структура md: A. Описательная статистика
//               → B. Статистические оценки
//               → C. Доверительные интервалы
//               → D. Проверка статистических гипотез
//               → E. Статистические меры связи
//
// MathematicalStatistics — наследует LimitTheorems (g.cpp).
// Описательная статистика, метод моментов, MLE,
// доверительные интервалы, Z/t/chi2 критерии,
// корреляция Спирмена, энтропия, взаимная информация.

#ifndef INSIDE_PROB_AND_STATS_H
#define INSIDE_PROB_AND_STATS_H
#include "../g/g.cpp"
#endif

struct MathematicalStatistics : LimitTheorems {

// =============================================================
// A. ОПИСАТЕЛЬНАЯ СТАТИСТИКА
// =============================================================

// --- A.1. Выборочное среднее ---
double sample_mean(const vector<double>& x) {
    double sum = 0;
    for (double v : x) sum += v;
    return sum / (int)x.size();
}

// --- A.2. Выборочная дисперсия (несмещённая) ---
double sample_variance(const vector<double>& x) {
    int n = (int)x.size();
    if (n <= 1) return 0;
    double mean = sample_mean(x);
    double var = 0;
    for (double v : x) var += (v - mean) * (v - mean);
    return var / (n - 1);
}

// --- A.3. Стандартное отклонение ---
double sample_std(const vector<double>& x) {
    return sqrt(sample_variance(x));
}

// --- A.4. Выборочная медиана ---
double sample_median(vector<double> x) {
    int n = (int)x.size();
    sort(x.begin(), x.end());
    if (n % 2 == 1) return x[n / 2];
    return (x[n / 2 - 1] + x[n / 2]) / 2.0;
}

// --- A.5. Мода ---
double sample_mode(const vector<double>& x) {
    map<double, int> freq;
    for (double v : x) freq[v]++;
    int max_count = 0;
    double mode_val = x[0];
    for (auto& [val, cnt] : freq) {
        if (cnt > max_count) { max_count = cnt; mode_val = val; }
    }
    return mode_val;
}

// --- A.6. Квантиль ---
double sample_quantile(vector<double> x, double p) {
    int n = (int)x.size();
    sort(x.begin(), x.end());
    double idx = p * (n - 1);
    int lo = (int)floor(idx);
    int hi = (int)ceil(idx);
    if (lo == hi) return x[lo];
    double frac = idx - lo;
    return x[lo] * (1.0 - frac) + x[hi] * frac;
}

// --- A.7. Межквартильный размах ---
double sample_iqr(const vector<double>& x) {
    return sample_quantile(x, 0.75) - sample_quantile(x, 0.25);
}

// --- A.8. Коэффициент вариации ---
double sample_cv(const vector<double>& x) {
    double s = sample_std(x);
    double m = sample_mean(x);
    return (fabs(m) > 1e-300) ? s / fabs(m) : 0.0;
}

// --- A.9. Среднее геометрическое ---
double geometric_mean(const vector<double>& x) {
    double log_sum = 0;
    for (double v : x) log_sum += log(v);
    return exp(log_sum / (int)x.size());
}

// --- A.10. Среднее гармоническое ---
double harmonic_mean(const vector<double>& x) {
    double inv_sum = 0;
    for (double v : x) inv_sum += 1.0 / v;
    return (int)x.size() / inv_sum;
}

// --- A.11. Среднее абсолютное отклонение (MAD) ---
double mean_absolute_deviation(const vector<double>& x) {
    double m = sample_mean(x);
    double mad = 0;
    for (double v : x) mad += fabs(v - m);
    return mad / (int)x.size();
}

// =============================================================
// B. СТАТИСТИЧЕСКИЕ ОЦЕНКИ
// =============================================================

// --- B.1. Метод моментов: оценка λ для Exp(λ) ---
// E[X] = 1/lambda => lambda_hat = 1/X_bar.
double mm_estimate_exp(const vector<double>& x) {
    return 1.0 / sample_mean(x);
}

// --- B.2. MLE: оценка λ для Exp(λ) ---
// lambda_hat = n / sum(x_i) = 1/X_bar (совпадает с MM).
double mle_estimate_exp(const vector<double>& x) {
    double sum = 0;
    for (double v : x) sum += v;
    return (int)x.size() / sum;
}

// --- B.3. MLE: оценка mu, sigma для N(mu, sigma^2) ---
pair<double, double> mle_estimate_normal(const vector<double>& x) {
    int n = (int)x.size();
    double mu = sample_mean(x);
    double sigma2 = 0;
    for (double v : x) sigma2 += (v - mu) * (v - mu);
    sigma2 /= n;  // MLE использует деление на n, не n-1
    return {mu, sqrt(sigma2)};
}

// --- B.4. Информация Фишера для Exp(λ) ---
// I(lambda) = n / lambda^2.
double fisher_information_exp(int n, double lambda) {
    return (double)n / (lambda * lambda);
}

// --- B.5. Проверка несмещённости MLE ---
// Генерируем m выборок, считаем E[theta_hat].
pair<double, double> check_bias_exp(int n, int m, double true_lambda) {
    double sum_est = 0;
    for (int i = 0; i < m; i++) {
        vector<double> sample(n);
        for (int j = 0; j < n; j++)
            sample[j] = -log((double)rand() / RAND_MAX) / true_lambda;
        sum_est += mle_estimate_exp(sample);
    }
    double mean_est = sum_est / m;
    return {mean_est, fabs(mean_est - true_lambda)};
}

// =============================================================
// C. ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ
// =============================================================

// --- C.1. Доверительный интервал для среднего (Z) ---
// X_bar ± z_{alpha/2} * sigma / sqrt(n).
pair<double, double> ci_mean_known_sigma(const vector<double>& x,
                                         double sigma, double alpha) {
    double xbar = sample_mean(x);
    int n = (int)x.size();
    // z_{0.025} = 1.96 для alpha = 0.05
    double z = 1.96;  // приблизительно; можно вычислить точнее
    if (fabs(alpha - 0.01) < 0.001) z = 2.576;
    else if (fabs(alpha - 0.10) < 0.001) z = 1.645;
    double margin = z * sigma / sqrt((double)n);
    return {xbar - margin, xbar + margin};
}

// --- C.2. Доверительный интервал для среднего (t) ---
// X_bar ± t_{alpha/2, n-1} * S / sqrt(n).
// Приближение t-квантили через норму для больших n.
pair<double, double> ci_mean_unknown_sigma(const vector<double>& x,
                                           double alpha) {
    double xbar = sample_mean(x);
    double s = sample_std(x);
    int n = (int)x.size();
    // Приближение: для n > 30 используем z, иначе t
    double t_crit;
    if (n > 30) {
        t_crit = 1.96;
        if (fabs(alpha - 0.01) < 0.001) t_crit = 2.576;
    } else {
        // Грубое приближение t-квантили
        t_crit = 1.96 + 1.96 / (4.0 * (n - 1));
        if (fabs(alpha - 0.01) < 0.001) t_crit = 2.576 + 2.576 / (4.0 * (n - 1));
    }
    double margin = t_crit * s / sqrt((double)n);
    return {xbar - margin, xbar + margin};
}

// --- C.3. Доверительный интервал для дисперсии (chi^2) ---
// [(n-1)*S^2 / chi2_upper, (n-1)*S^2 / chi2_lower].
pair<double, double> ci_variance(const vector<double>& x, double alpha) {
    int n = (int)x.size();
    double s2 = sample_variance(x);
    // Приближение chi^2 квантили: chi^2_k ~ k * (1 - 2/(9k) + z*sqrt(2/(9k)))^3
    auto chi2_approx = [](int k, double p) -> double {
        // Wilson-Hilferty: chi2_k ~ k * (1 - 2/(9k) + z*sqrt(2/(9k)))^3
        // z = Phi^{-1}(p): z>0 для p>0.5, z<0 для p<0.5
        double z;
        if (fabs(p - 0.025) < 0.01) z = -1.96;
        else if (fabs(p - 0.975) < 0.01) z = 1.96;
        else if (fabs(p - 0.005) < 0.01) z = -2.576;
        else if (fabs(p - 0.995) < 0.01) z = 2.576;
        else z = 1.96;  // fallback
        double a = 2.0 / (9.0 * k);
        double term = 1.0 - a + z * sqrt(a);
        return k * term * term * term;
    };
    int df = n - 1;
    double chi2_upper = chi2_approx(df, 1.0 - alpha / 2);
    double chi2_lower = chi2_approx(df, alpha / 2);
    double lower = df * s2 / chi2_upper;
    double upper = df * s2 / chi2_lower;
    return {lower, upper};
}

// --- C.4. Доверительный интервал для пропорции ---
// p_hat ± z_{alpha/2} * sqrt(p_hat*(1-p_hat)/n).
pair<double, double> ci_proportion(int successes, int n, double alpha) {
    double phat = (double)successes / n;
    double z = 1.96;
    if (fabs(alpha - 0.01) < 0.001) z = 2.576;
    double margin = z * sqrt(phat * (1.0 - phat) / n);
    return {phat - margin, phat + margin};
}

// =============================================================
// D. ПРОВЕРКА СТАТИСТИЧЕСКИХ ГИПОТЕЗ
// =============================================================

// --- D.1. Z-критерий для среднего ---
// H0: mu = mu0. Возвращает {z-stat, p-value}.
pair<double, double> z_test_mean(const vector<double>& x, double mu0,
                                 double sigma) {
    double xbar = sample_mean(x);
    int n = (int)x.size();
    double z = (xbar - mu0) / (sigma / sqrt((double)n));
    double p = 2.0 * (1.0 - phi_c(fabs(z)));
    return {z, p};
}

// --- D.2. t-критерий для среднего ---
// H0: mu = mu0. Возвращает {t-stat, p-value}.
pair<double, double> t_test_mean(const vector<double>& x, double mu0) {
    double xbar = sample_mean(x);
    double s = sample_std(x);
    int n = (int)x.size();
    double t = (xbar - mu0) / (s / sqrt((double)n));
    // p-value через t-распределение (приближение)
    // Для больших n t ~ N(0,1)
    double p = 2.0 * (1.0 - phi_c(fabs(t)));
    return {t, p};
}

// --- D.3. χ²-критерий согласия ---
// H0: выборка из распределения F.
// observed[i] — число в бине i, expected[i] — ожидаемое.
// Возвращает {chi2-stat, df, p-value}.
struct Chi2Result { double stat; int df; double p_value; };
Chi2Result chi2_goodness_of_fit(const vector<int>& observed,
                                const vector<double>& expected,
                                int num_estimated_params) {
    double chi2 = 0;
    int k = (int)observed.size();
    for (int i = 0; i < k; i++) {
        if (expected[i] > 1e-12)
            chi2 += (observed[i] - expected[i]) * (observed[i] - expected[i]) / expected[i];
    }
    int df = k - 1 - num_estimated_params;
    // p-value через gamma-функцию (приближение)
    double p = 1.0;  // упрощённо
    if (df > 0) {
        // Используем приближение: chi2 ~ df * (1 + z*sqrt(2/df))^3
        // инвертируем для p-value
        double x = chi2 / df;
        double z_approx = (pow(x, 1.0/3.0) - (1.0 - 2.0/(9.0*df))) / sqrt(2.0/(9.0*df));
        p = 2.0 * (1.0 - phi_c(fabs(z_approx)));
    }
    return {chi2, df, p};
}

// --- D.4. χ²-критерий независимости ---
// table[i][j] — наблюдаемые частоты.
// Возвращает {chi2-stat, df, p-value}.
Chi2Result chi2_independence(const vector<vector<int>>& table) {
    int r = (int)table.size();
    int c = (int)table[0].size();
    int N = 0;
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            N += table[i][j];
    vector<int> row_sum(r, 0), col_sum(c, 0);
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++) {
            row_sum[i] += table[i][j];
            col_sum[j] += table[i][j];
        }
    double chi2 = 0;
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            double expected = (double)row_sum[i] * col_sum[j] / N;
            if (expected > 1e-12)
                chi2 += (table[i][j] - expected) * (table[i][j] - expected) / expected;
        }
    }
    int df = (r - 1) * (c - 1);
    double x = chi2 / df;
    double z_approx = (pow(x, 1.0/3.0) - (1.0 - 2.0/(9.0*df))) / sqrt(2.0/(9.0*df));
    double p = 2.0 * (1.0 - phi_c(fabs(z_approx)));
    return {chi2, df, p};
}

// =============================================================
// E. СТАТИСТИЧЕСКИЕ МЕРЫ СВЯЗИ
// =============================================================

// --- E.1. Ранговая корреляция Спирмена ---
double spearman_correlation(const vector<double>& x,
                            const vector<double>& y) {
    int n = (int)x.size();
    // Преобразуем в ранги
    auto to_ranks = [](vector<double> v) -> vector<double> {
        int n = (int)v.size();
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b) { return v[a] < v[b]; });
        vector<double> ranks(n);
        for (int i = 0; i < n; i++) ranks[idx[i]] = i + 1;
        return ranks;
    };
    vector<double> rx = to_ranks(x);
    vector<double> ry = to_ranks(y);
    double d2 = 0;
    for (int i = 0; i < n; i++)
        d2 += (rx[i] - ry[i]) * (rx[i] - ry[i]);
    return 1.0 - 6.0 * d2 / (n * (n * n - 1));
}

// --- E.2. Коэффициент Жаккара ---
// Для бинарных векторов (0/1).
double jaccard_similarity(const vector<int>& a, const vector<int>& b) {
    int intersection = 0, union_size = 0;
    for (int i = 0; i < (int)a.size(); i++) {
        if (a[i] == 1 || b[i] == 1) union_size++;
        if (a[i] == 1 && b[i] == 1) intersection++;
    }
    return (union_size > 0) ? (double)intersection / union_size : 0.0;
}

// --- E.3. Энтропия Шеннона ---
// probabilities[i] = P(X = i). Логарифм по основанию 2.
double shannon_entropy(const vector<double>& probabilities) {
    double H = 0;
    for (double p : probabilities)
        if (p > 1e-12) H -= p * log2(p);
    return H;
}

// --- E.4. Совместная энтропия ---
double joint_entropy(const vector<vector<double>>& joint_probs) {
    double H = 0;
    for (auto& row : joint_probs)
        for (double p : row)
            if (p > 1e-12) H -= p * log2(p);
    return H;
}

// --- E.5. Взаимная информация ---
// I(X; Y) = H(X) + H(Y) - H(X, Y).
double mutual_information(const vector<double>& px,
                          const vector<double>& py,
                          const vector<vector<double>>& pxy) {
    return shannon_entropy(px) + shannon_entropy(py) - joint_entropy(pxy);
}

// --- E.6. Условная энтропия ---
// H(X|Y) = H(X, Y) - H(Y).
double conditional_entropy(const vector<double>& py,
                           const vector<vector<double>>& pxy) {
    return joint_entropy(pxy) - shannon_entropy(py);
}

}; // struct MathematicalStatistics

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_H_MAIN
int main() {
    MathematicalStatistics ms;
    srand(42);

    cout << "=== A. ОПИСАТЕЛЬНАЯ СТАТИСТИКА ===" << endl;
    vector<double> data = {4.2, 5.1, 3.8, 6.0, 4.9, 5.5, 3.5, 7.2, 4.8, 5.0};
    cout << "Данные: ";
    for (double v : data) cout << v << " ";
    cout << endl;
    cout << "Среднее: " << ms.sample_mean(data) << endl;
    cout << "Дисперсия (S^2): " << ms.sample_variance(data) << endl;
    cout << "Стд. отклонение: " << ms.sample_std(data) << endl;
    cout << "Медиана: " << ms.sample_median(data) << endl;
    cout << "Мода: " << ms.sample_mode(data) << endl;
    cout << "Q1: " << ms.sample_quantile(data, 0.25) << endl;
    cout << "Q3: " << ms.sample_quantile(data, 0.75) << endl;
    cout << "IQR: " << ms.sample_iqr(data) << endl;
    cout << "CV: " << ms.sample_cv(data) << endl;
    cout << "Среднее геом.: " << ms.geometric_mean(data) << endl;
    cout << "Среднее гарм.: " << ms.harmonic_mean(data) << endl;
    cout << "MAD: " << ms.mean_absolute_deviation(data) << endl;
    cout << "Цепочка неравенств: H <= G <= X̄: "
         << ms.harmonic_mean(data) << " <= " << ms.geometric_mean(data)
         << " <= " << ms.sample_mean(data) << endl;

    cout << "\n=== B. СТАТИСТИЧЕСКИЕ ОЦЕНКИ ===" << endl;
    cout << "--- Exp(lambda=2) ---" << endl;
    double true_lambda = 2.0;
    int n = 50, m = 5000;
    auto [mean_est, bias] = ms.check_bias_exp(n, m, true_lambda);
    cout << "Истинное lambda = " << true_lambda << endl;
    cout << "E[lambda_hat] = " << mean_est << " (смещение = " << bias << ")" << endl;
    cout << "Информация Фишера I(lambda) = " << ms.fisher_information_exp(n, true_lambda) << endl;

    cout << "\n--- MLE для N(mu, sigma^2) ---" << endl;
    vector<double> norm_data;
    for (int i = 0; i < 100; i++) norm_data.push_back(ms.normal_sample(5, 2));
    auto [mu_hat, sigma_hat] = ms.mle_estimate_normal(norm_data);
    cout << "Истинные: mu=5, sigma=2" << endl;
    cout << "MLE: mu_hat=" << mu_hat << ", sigma_hat=" << sigma_hat << endl;

    cout << "\n=== C. ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ ===" << endl;
    vector<double> sample50;
    for (int i = 0; i < 50; i++) sample50.push_back(ms.normal_sample(10, 3));

    cout << "--- DI для среднего (sigma=3 известна, alpha=0.05) ---" << endl;
    auto ci_z = ms.ci_mean_known_sigma(sample50, 3.0, 0.05);
    cout << "95% DI (Z): [" << ci_z.first << ", " << ci_z.second << "]" << endl;

    cout << "--- DI для среднего (sigma неизвестна, alpha=0.05) ---" << endl;
    auto ci_t = ms.ci_mean_unknown_sigma(sample50, 0.05);
    cout << "95% DI (t): [" << ci_t.first << ", " << ci_t.second << "]" << endl;

    cout << "--- DI для дисперсии (alpha=0.05) ---" << endl;
    auto ci_var = ms.ci_variance(sample50, 0.05);
    cout << "95% DI для sigma^2: [" << ci_var.first << ", " << ci_var.second << "]" << endl;

    cout << "--- DI для пропорции (30 успехов из 100, alpha=0.05) ---" << endl;
    auto ci_prop = ms.ci_proportion(30, 100, 0.05);
    cout << "95% DI для p: [" << ci_prop.first << ", " << ci_prop.second << "]" << endl;

    cout << "\n=== D. ПРОВЕРКА ГИПОТЕЗ ===" << endl;

    cout << "--- Z-критерий: H0: mu=10 ---" << endl;
    auto [z_stat, z_p] = ms.z_test_mean(sample50, 10.0, 3.0);
    cout << "Z = " << z_stat << ", p-value = " << z_p
         << (z_p < 0.05 ? " (отвергаем H0)" : " (не отвергаем H0)") << endl;

    cout << "--- t-критерий: H0: mu=10 ---" << endl;
    auto [t_stat, t_p] = ms.t_test_mean(sample50, 10.0);
    cout << "T = " << t_stat << ", p-value = " << t_p
         << (t_p < 0.05 ? " (отвергаем H0)" : " (не отвергаем H0)") << endl;

    cout << "--- χ² согласия: Exp(1) vs Exp(1) ---" << endl;
    vector<double> exp_data;
    for (int i = 0; i < 200; i++) exp_data.push_back(ms.exponential_sample(1.0));
    // Бины: [0,0.5), [0.5,1), [1,1.5), [1.5,2), [2,inf)
    vector<int> obs = {0, 0, 0, 0, 0};
    for (double v : exp_data) {
        int bin = (int)(v / 0.5);
        if (bin >= 5) bin = 4;
        obs[bin]++;
    }
    vector<double> exp_exp = {0, 0, 0, 0, 0};
    for (int i = 0; i < 4; i++)
        exp_exp[i] = 200.0 * (exp(-i * 0.5) - exp(-(i + 1) * 0.5));
    exp_exp[4] = 200.0 * exp(-2.0);
    auto chi2_res = ms.chi2_goodness_of_fit(obs, exp_exp, 1);
    cout << "chi2 = " << chi2_res.stat << ", df = " << chi2_res.df
         << ", p-value = " << chi2_res.p_value << endl;

    cout << "--- χ² независимости: пол 2x2 ---" << endl;
    vector<vector<int>> table = {{50, 30}, {20, 40}};
    auto chi2_ind = ms.chi2_independence(table);
    cout << "Таблица: [[50,30],[20,40]]" << endl;
    cout << "chi2 = " << chi2_ind.stat << ", df = " << chi2_ind.df
         << ", p-value = " << chi2_ind.p_value << endl;

    cout << "\n=== E. МЕРЫ СВЯЗИ ===" << endl;

    cout << "--- Спирмен ---" << endl;
    vector<double> sx = {1, 2, 3, 4, 5, 6, 7, 8};
    vector<double> sy = {2, 4, 5, 4, 8, 7, 8, 9};
    cout << "rho_s = " << ms.spearman_correlation(sx, sy) << endl;

    cout << "--- Жаккар (бинарные множества) ---" << endl;
    vector<int> a = {1, 1, 0, 1, 0, 1};
    vector<int> b = {1, 0, 0, 1, 1, 1};
    cout << "J(A,B) = " << ms.jaccard_similarity(a, b) << endl;

    cout << "--- Энтропия и взаимная информация ---" << endl;
    // Бернуллиевская СВ: p=0.3
    vector<double> px = {0.3, 0.7};
    cout << "H(X) = " << ms.shannon_entropy(px) << " бит" << endl;

    // Совместное распределение X,Y
    vector<vector<double>> pxy = {{0.2, 0.1}, {0.1, 0.6}};
    vector<double> py = {0.3, 0.7};
    cout << "H(X,Y) = " << ms.joint_entropy(pxy) << " бит" << endl;
    cout << "I(X;Y) = " << ms.mutual_information(px, py, pxy) << " бит" << endl;
    cout << "H(X|Y) = " << ms.conditional_entropy(py, pxy) << " бит" << endl;

    return 0;
}
#endif

#endif // PROB_AND_STATS_H_CPP
