#ifndef PROB_AND_STATS_C_CPP
#define PROB_AND_STATS_C_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
using namespace std;

// =============================================================
// C. СЛУЧАЙНЫЕ ВЕЛИЧИНЫ И ИХ РАСПРЕДЕЛЕНИЯ
// =============================================================
// Структура md: A. Определение случайной величины
//               → B. Действия над случайными величинами
//               → C. Функция распределения
//               → D. Классификация вероятностных мер
//               → E. Дискретные распределения
//               → F. Непрерывные распределения
//               → G. Метод преобразования
//               → H. Дополнительные свойства и связи
//
// RandomVariables — наследует MeasureTheoreticProb (b.cpp).
// Вводит: случайные величины, функцию распределения,
// основные дискретные и непрерывные распределения,
// метод преобразования (инверсия, acceptance-rejection),
// моментную порождающую функцию, характеристическую функцию.

#ifndef INSIDE_PROB_AND_STATS_C
#define INSIDE_PROB_AND_STATS_C
#include "../b/b.cpp"
#undef INSIDE_PROB_AND_STATS_C
#endif

struct RandomVariables : MeasureTheoreticProb {

// =============================================================
// C. ФУНКЦИЯ РАСПРЕДЕЛЕНИЯ
// =============================================================

// --- C.1. Функция распределения дискретной СВ ---
double discrete_cdf(double x, const vector<double>& values,
                    const vector<double>& probs) {
    double result = 0.0;
    for (int i = 0; i < (int)values.size(); i++)
        if (values[i] <= x)
            result += probs[i];
    return result;
}

// --- C.2. Точечная вероятность P(X = a) для дискретной СВ ---
double discrete_point_mass(double a, const vector<double>& values,
                           const vector<double>& probs) {
    double result = 0.0;
    for (int i = 0; i < (int)values.size(); i++)
        if (fabs(values[i] - a) < 1e-12)
            result += probs[i];
    return result;
}

// =============================================================
// E. ДИСКРЕТНЫЕ РАСПРЕДЕЛЕНИЯ
// =============================================================

// --- E.1. Распределение Бернулли Ber(p) ---
int bernoulli_sample(double p) {
    double u = (double)rand() / RAND_MAX;
    return (u < p) ? 1 : 0;
}

double bernoulli_pmf(int k, double p) {
    if (k == 1) return p;
    if (k == 0) return 1.0 - p;
    return 0.0;
}

// --- E.2. Биномиальное распределение B(n, p) ---
double binomial_pmf_c(long long n, long long k, double p) {
    if (k < 0 || k > n) return 0.0;
    long long c = 1;
    long long kk = min(k, n - k);
    for (long long i = 0; i < kk; i++)
        c = c * (n - i) / (i + 1);
    return (double)c * pow(p, k) * pow(1.0 - p, n - k);
}

double binomial_cdf_c(long long n, long long k, double p) {
    double result = 0.0;
    for (long long i = 0; i <= k; i++)
        result += binomial_pmf_c(n, i, p);
    return result;
}

int binomial_sample(long long n, double p) {
    int count = 0;
    for (long long i = 0; i < n; i++)
        count += bernoulli_sample(p);
    return count;
}

// --- E.3. Распределение Пуассона Po(lambda) ---
double poisson_pmf_c(long long k, double lambda) {
    if (lambda <= 0.0) return (k == 0) ? 1.0 : 0.0;
    double log_result = -lambda + k * log(lambda);
    for (long long i = 2; i <= k; i++)
        log_result -= log((double)i);
    return exp(log_result);
}

double poisson_cdf_c(long long k, double lambda) {
    double result = 0.0;
    double term = exp(-lambda);
    result += term;
    for (long long i = 1; i <= k; i++) {
        term *= lambda / (double)i;
        result += term;
    }
    return result;
}

long long poisson_sample(double lambda) {
    double L = exp(-lambda);
    long long k = 0;
    double p = 1.0;
    do {
        k++;
        p *= (double)rand() / RAND_MAX;
    } while (p > L);
    return k - 1;
}

// --- E.4. Геометрическое распределение Geom(p) ---
long long geometric_sample(double p) {
    long long k = 0;
    double u;
    do {
        k++;
        u = (double)rand() / RAND_MAX;
    } while (u >= p);
    return k;
}

double geometric_pmf(long long k, double p) {
    if (k < 1) return 0.0;
    return pow(1.0 - p, k - 1) * p;
}

double geometric_cdf(long long k, double p) {
    if (k < 1) return 0.0;
    return 1.0 - pow(1.0 - p, k);
}

// --- E.5. Гипергеометрическое распределение Hg(N, K, n) ---
double hypergeometric_pmf(long long k, long long N, long long K, long long n) {
    if (k < max(0LL, n - (N - K)) || k > min(n, K)) return 0.0;
    auto comb = [](long long a, long long b) -> long long {
        if (b < 0 || b > a) return 0;
        if (b == 0 || b == a) return 1;
        b = min(b, a - b);
        long long result = 1;
        for (long long i = 0; i < b; i++)
            result = result * (a - i) / (i + 1);
        return result;
    };
    return (double)comb(K, k) * comb(N - K, n - k) / (double)comb(N, n);
}

// =============================================================
// F. НЕПРЕРЫВНЫЕ РАСПРЕДЕЛЕНИЯ
// =============================================================

// --- F.1. Равномерное распределение U(a, b) ---
double uniform_pdf(double x, double a, double b) {
    if (x < a || x > b) return 0.0;
    return 1.0 / (b - a);
}

double uniform_cdf(double x, double a, double b) {
    if (x < a) return 0.0;
    if (x > b) return 1.0;
    return (x - a) / (b - a);
}

double uniform_mean(double a, double b) { return (a + b) / 2.0; }
double uniform_var(double a, double b) { return (b - a) * (b - a) / 12.0; }

double uniform_sample(double a, double b) {
    return a + (b - a) * ((double)rand() / RAND_MAX);
}

// --- F.2. Нормальное распределение N(mu, sigma^2) ---
double normal_pdf(double x, double mu, double sigma) {
    double z = (x - mu) / sigma;
    return exp(-z * z / 2.0) / (sigma * sqrt(2.0 * M_PI));
}

// Функция Лапласа Phi(x) — через互补 ошибки.
// Phi(x) = 0.5 * erfc(-x / sqrt(2)).
double phi_c(double x) {
    return 0.5 * erfc(-x / sqrt(2.0));
}

double normal_cdf(double x, double mu, double sigma) {
    return phi_c((x - mu) / sigma);
}

// Метод Бокса-Мюллера: генерирует пару N(0,1).
pair<double, double> box_muller() {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    double r = sqrt(-2.0 * log(u1));
    return {r * cos(2.0 * M_PI * u2), r * sin(2.0 * M_PI * u2)};
}

double normal_sample(double mu, double sigma) {
    auto [z1, z2] = box_muller();
    return mu + sigma * z1;
}

// --- F.3. Экспоненциальное распределение Exp(lambda) ---
double exponential_pdf(double x, double lambda) {
    if (x < 0.0) return 0.0;
    return lambda * exp(-lambda * x);
}

double exponential_cdf(double x, double lambda) {
    if (x < 0.0) return 0.0;
    return 1.0 - exp(-lambda * x);
}

double exponential_mean(double lambda) { return 1.0 / lambda; }
double exponential_var(double lambda) { return 1.0 / (lambda * lambda); }

double exponential_sample(double lambda) {
    double u = (double)rand() / RAND_MAX;
    return -log(u) / lambda;
}

// --- F.4. Распределение хи-квадрат chi^2(k) ---
double chi2_pdf(double x, int k) {
    if (x < 0.0) return 0.0;
    double half_k = k / 2.0;
    auto lgamma_approx = [](double z) -> double {
        if (z < 0.5) return log(M_PI / sin(M_PI * z)) - lgamma(1.0 - z);
        z -= 1.0;
        double g = 7.0;
        double c[9] = {
            0.99999999999980993, 676.5203681218851, -1259.1392167224028,
            771.32342877765313, -176.61502916214059, 12.507343278686905,
            -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7
        };
        double x_ = c[0];
        for (int i = 1; i < (int)g + 2; i++) x_ += c[i] / (z + i);
        double t = z + g + 0.5;
        return 0.5 * log(2.0 * M_PI) + (z + 0.5) * log(t) - t + log(x_);
    };
    double log_result = (half_k - 1.0) * log(x) - x / 2.0;
    log_result -= half_k * log(2.0);
    log_result -= lgamma_approx(half_k);
    return exp(log_result);
}

double chi2_cdf(double x, int k) {
    if (x < 0.0) return 0.0;
    int n_steps = 1000;
    double h = x / n_steps;
    double sum = chi2_pdf(0.0 + 1e-12, k) + chi2_pdf(x, k);
    for (int i = 1; i < n_steps; i++) {
        double xi = i * h;
        sum += chi2_pdf(xi, k) * ((i % 2 == 0) ? 2.0 : 4.0);
    }
    return sum * h / 3.0;
}

double chi2_sample(int k) {
    double sum = 0.0;
    for (int i = 0; i < k; i++) {
        auto [z1, z2] = box_muller();
        sum += z1 * z1;
    }
    return sum;
}

// --- F.5. Распределение Стьюдента t(k) ---
double students_t_pdf(double x, int k) {
    double half_k = (k + 1.0) / 2.0;
    double log_result = lgamma(half_k) - lgamma(k / 2.0);
    log_result -= 0.5 * log(k * M_PI);
    log_result -= half_k * log(1.0 + x * x / k);
    return exp(log_result);
}

double students_t_sample(int k) {
    double z = box_muller().first;
    double chi2 = 0.0;
    for (int i = 0; i < k; i++) {
        double n = box_muller().first;
        chi2 += n * n;
    }
    return z / sqrt(chi2 / k);
}

// --- F.6. Распределение Фишера F(k1, k2) ---
double fisher_f_sample(int k1, int k2) {
    double chi1 = 0.0, chi2 = 0.0;
    for (int i = 0; i < k1; i++) {
        double z = box_muller().first;
        chi1 += z * z;
    }
    for (int i = 0; i < k2; i++) {
        double z = box_muller().first;
        chi2 += z * z;
    }
    return (chi1 / k1) / (chi2 / k2);
}

// =============================================================
// G. МЕТОД ПРЕОБРАЗОВАНИЯ
// =============================================================

// --- G.1. Линейное преобразование ---
double linear_transform_cdf(double y, double a, double b,
                            double (*cdf)(double)) {
    if (a > 0) return cdf((y - b) / a);
    return 1.0 - cdf((y - b) / a);
}

// --- G.2. Метод квантилей (инверсия) ---
double inverse_cdf_sample(double (*cdf)(double), double lo, double hi,
                          double eps = 1e-8) {
    double u = (double)rand() / RAND_MAX;
    while (hi - lo > eps) {
        double mid = (lo + hi) / 2.0;
        if (cdf(mid) < u)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) / 2.0;
}

// --- G.3. Метод рёбер (acceptance-rejection) ---
double acceptance_rejection(double (*f_pdf)(double),
                            double (*g_pdf)(double),
                            double (*g_sample)(),
                            double M) {
    while (true) {
        double x = g_sample();
        double u = (double)rand() / RAND_MAX;
        if (u < f_pdf(x) / (M * g_pdf(x)))
            return x;
    }
}

// =============================================================
// H. ДОПОЛНИТЕЛЬНЫЕ СВОЙСТВА
// =============================================================

// --- H.1. Моментная порождающая функция (MGF) ---
double mgf_bernoulli(double t, double p) {
    return (1.0 - p) + p * exp(t);
}

double mgf_binomial(double t, long long n, double p) {
    return pow((1.0 - p) + p * exp(t), n);
}

double mgf_poisson(double t, double lambda) {
    return exp(lambda * (exp(t) - 1.0));
}

double mgf_normal(double t, double mu, double sigma) {
    return exp(mu * t + sigma * sigma * t * t / 2.0);
}

double mgf_exponential(double t, double lambda) {
    if (t >= lambda) return 1e300;
    return lambda / (lambda - t);
}

// --- H.2. Характеристическая функция ---
pair<double, double> cf_normal(double t, double mu, double sigma) {
    double re = exp(-sigma * sigma * t * t / 2.0) * cos(mu * t);
    double im = exp(-sigma * sigma * t * t / 2.0) * sin(mu * t);
    return {re, im};
}

pair<double, double> cf_poisson(double t, double lambda) {
    double cos_t = cos(t);
    double sin_t = sin(t);
    double re_exp = exp(lambda * (cos_t - 1.0));
    double phase = lambda * sin_t;
    return {re_exp * cos(phase), re_exp * sin(phase)};
}

// --- H.3. Статистические проверки ---

// Kolmogorov-Smirnov: D = sup|F_n(x) - F(x)|.
// cdf — свободная функция (не метод структуры).
double ks_statistic(const vector<double>& sample,
                    double (*cdf)(double)) {
    int n = (int)sample.size();
    vector<double> sorted_sample = sample;
    sort(sorted_sample.begin(), sorted_sample.end());
    double max_diff = 0.0;
    for (int i = 0; i < n; i++) {
        double fn = (double)(i + 1) / n;
        double fx = cdf(sorted_sample[i]);
        double diff = fabs(fn - fx);
        if (diff > max_diff) max_diff = diff;
        double diff_prev = fabs((double)i / n - fx);
        if (diff_prev > max_diff) max_diff = diff_prev;
    }
    return max_diff;
}

}; // struct RandomVariables

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_C_MAIN
int main() {
    RandomVariables rv;
    srand(42);

    cout << "=== E. ДИСКРЕТНЫЕ РАСПРЕДЕЛЕНИЯ ===" << endl;

    cout << "--- Бернулли Ber(0.7) ---" << endl;
    cout << "P(X=0)=" << rv.bernoulli_pmf(0, 0.7)
         << " P(X=1)=" << rv.bernoulli_pmf(1, 0.7) << endl;
    cout << "10 сэмплов: ";
    for (int i = 0; i < 10; i++) cout << rv.bernoulli_sample(0.7) << " ";
    cout << endl;

    cout << "\n--- Биномиальное B(20, 0.4) ---" << endl;
    cout << "P(X=8)=" << rv.binomial_pmf_c(20, 8, 0.4) << endl;
    cout << "P(X<=8)=" << rv.binomial_cdf_c(20, 8, 0.4) << endl;
    cout << "5 сэмплов: ";
    for (int i = 0; i < 5; i++) cout << rv.binomial_sample(20, 0.4) << " ";
    cout << endl;

    cout << "\n--- Пуассон Po(5) ---" << endl;
    cout << "P(X=3)=" << rv.poisson_pmf_c(3, 5.0) << endl;
    cout << "P(X<=3)=" << rv.poisson_cdf_c(3, 5.0) << endl;
    cout << "5 сэмплов: ";
    for (int i = 0; i < 5; i++) cout << rv.poisson_sample(5.0) << " ";
    cout << endl;

    cout << "\n--- Геометрическое Geom(0.3) ---" << endl;
    cout << "P(X=1)=" << rv.geometric_pmf(1, 0.3)
         << " P(X=3)=" << rv.geometric_pmf(3, 0.3) << endl;
    cout << "P(X<=3)=" << rv.geometric_cdf(3, 0.3) << endl;

    cout << "\n--- Гипергеометрическое Hg(50, 10, 5) ---" << endl;
    for (int k = 0; k <= 5; k++)
        cout << "P(X=" << k << ")=" << rv.hypergeometric_pmf(k, 50, 10, 5) << " ";
    cout << endl;

    cout << "\n=== F. НЕПРЕРЫВНЫЕ РАСПРЕДЕЛЕНИЯ ===" << endl;

    cout << "--- Равномерное U(0, 10) ---" << endl;
    cout << "E=" << rv.uniform_mean(0, 10) << " D=" << rv.uniform_var(0, 10) << endl;
    cout << "f(5)=" << rv.uniform_pdf(5, 0, 10)
         << " F(5)=" << rv.uniform_cdf(5, 0, 10) << endl;

    cout << "\n--- Нормальное N(0, 1) ---" << endl;
    cout << "Phi(0)=" << rv.phi_c(0) << " Phi(1)=" << rv.phi_c(1)
         << " Phi(2)=" << rv.phi_c(2) << " Phi(3)=" << rv.phi_c(3) << endl;
    cout << "f(0)=" << rv.normal_pdf(0, 0, 1) << endl;
    cout << "N(5, 4): 5 сэмплов: ";
    for (int i = 0; i < 5; i++) cout << rv.normal_sample(5, 2) << " ";
    cout << endl;

    cout << "\n--- Экспоненциальное Exp(2) ---" << endl;
    cout << "E=" << rv.exponential_mean(2) << " D=" << rv.exponential_var(2) << endl;
    cout << "f(1)=" << rv.exponential_pdf(1, 2)
         << " F(1)=" << rv.exponential_cdf(1, 2) << endl;

    cout << "\n--- хи-квадрат chi^2(5) ---" << endl;
    cout << "E=5 D=10" << endl;
    cout << "f(3)=" << rv.chi2_pdf(3, 5) << endl;

    cout << "\n--- Стьюден t(10) ---" << endl;
    cout << "f(0)=" << rv.students_t_pdf(0, 10) << endl;

    cout << "\n=== G. МЕТОД ПРЕОБРАЗОВАНИЯ ===" << endl;

    cout << "--- Инверсия CDF (Exp(1) через U(0,1)) ---" << endl;
    cout << "5 сэмплов Exp(1): ";
    for (int i = 0; i < 5; i++)
        cout << -log((double)rand() / RAND_MAX) << " ";
    cout << endl;

    cout << "\n--- Acceptance-Rejection (N(0,1) через Exp(1)) ---" << endl;
    // Принимаем Exp(1) как обёртку для N(0,1): M ~ 1.32
    cout << "5 сэмплов N(0,1) через AR: ";
    for (int i = 0; i < 5; i++) {
        double x = rv.exponential_sample(1.0);
        double u = (double)rand() / RAND_MAX;
        if (u < rv.normal_pdf(x, 0, 1) / (1.32 * rv.exponential_pdf(x, 1.0)))
            cout << x << " ";
        else
            i--;
    }
    cout << endl;

    cout << "\n=== H. ДОПОЛНИТЕЛЬНЫЕ СВОЙСТВА ===" << endl;

    cout << "--- MGF ---" << endl;
    cout << "MGF_Ber(0.5, t=1) = " << rv.mgf_bernoulli(1, 0.5) << endl;
    cout << "MGF_N(0,1, t=1) = " << rv.mgf_normal(1, 0, 1) << endl;
    cout << "MGF_Po(3, t=1) = " << rv.mgf_poisson(1, 3) << endl;

    cout << "\n--- Характеристическая функция ---" << endl;
    auto [re, im] = rv.cf_normal(1, 0, 1);
    cout << "CF_N(0,1, t=1) = " << re << " + " << im << "i" << endl;

    cout << "\n--- KS-статистика (100 сэмплов N(0,1) vs Phi) ---" << endl;
    vector<double> sample100;
    for (int i = 0; i < 100; i++) sample100.push_back(rv.normal_sample(0, 1));
    // KS нужна свободная функция CDF; используем лямбду через статический массив
    // (упрощённо: просто покажем, что KS работает)
    sort(sample100.begin(), sample100.end());
    double ks_max = 0.0;
    for (int i = 0; i < 100; i++) {
        double fn = (double)(i + 1) / 100.0;
        double fx = rv.phi_c(sample100[i]);
        ks_max = max(ks_max, fabs(fn - fx));
    }
    cout << "KS D = " << ks_max << endl;

    return 0;
}
#endif

#endif // PROB_AND_STATS_C_CPP
