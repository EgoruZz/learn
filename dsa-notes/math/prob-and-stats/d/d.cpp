#ifndef PROB_AND_STATS_D_CPP
#define PROB_AND_STATS_D_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
using namespace std;

// =============================================================
// D. МАТЕМАТИЧЕСКОЕ ОЖИДАНИЕ И СВОЙСТВА
// =============================================================
// Структура md: A. Построение математического ожидания
//               → B. Свойства математического ожидания
//               → C. Формулы подсчёта
//               → D. Медиана и квантили
//
// Expectation — наследует RandomVariables (c.cpp).
// Построение E[X] для различных классов СВ, свойства (линейность,
// аддитивность, итерированное), формулы через плотность/CDF,
// условные ожидания, медиана и квантили.

#ifndef INSIDE_PROB_AND_STATS_D
#define INSIDE_PROB_AND_STATS_D
#include "../c/c.cpp"
#undef INSIDE_PROB_AND_STATS_D
#endif

struct Expectation : RandomVariables {

// =============================================================
// A. ПОСТРОЕНИЕ МАТЕМАТИЧЕСКОГО ОЖИДАНИЯ
// =============================================================

// --- A.1. Математическое ожидание дискретной СВ ---
// values[i] — значение x_i, probs[i] = P(X = x_i).
// E[X] = sum x_i * p_i.
// O(n) время.
double expectation_discrete(const vector<double>& values,
                            const vector<double>& probs) {
    double result = 0.0;
    for (int i = 0; i < (int)values.size(); i++)
        result += values[i] * probs[i];
    return result;
}

// --- A.2. E[g(X)] для дискретной СВ ---
// g(x) — функция (лямбда или указатель).
// O(n) время.
double expectation_discrete_func(const vector<double>& values,
                                 const vector<double>& probs,
                                 double (*g)(double)) {
    double result = 0.0;
    for (int i = 0; i < (int)values.size(); i++)
        result += g(values[i]) * probs[i];
    return result;
}

// --- A.3. E[X] через хвостовые вероятности (неотрицательная СВ) ---
// E[X] = integral_0^inf P(X > t) dt.
// Аппроксимация: численное интегрирование по сетке [0, max_sample].
// O(n_steps) время.
double expectation_tail_probability(const vector<double>& sample,
                                    int n_steps = 1000) {
    double max_val = 0.0;
    for (double x : sample)
        max_val = max(max_val, x);
    int n = (int)sample.size();
    double h = max_val / n_steps;
    double result = 0.0;
    for (int i = 0; i < n_steps; i++) {
        double t = (i + 0.5) * h;
        int count = 0;
        for (double x : sample)
            if (x > t) count++;
        result += (double)count / n * h;
    }
    return result;
}

// --- A.4. E[X] через интеграл по плотности (численно) ---
// pdf — плотность, [lo, hi] — область интегрирования.
// Метод Симпсона.
double expectation_pdf(double (*pdf)(double), double lo, double hi,
                       int n_steps = 10000) {
    if (n_steps % 2 != 0) n_steps++;
    double h = (hi - lo) / n_steps;
    double sum = lo * pdf(lo) + hi * pdf(hi);
    for (int i = 1; i < n_steps; i++) {
        double x = lo + i * h;
        sum += x * pdf(x) * ((i % 2 == 0) ? 2.0 : 4.0);
    }
    return sum * h / 3.0;
}

// =============================================================
// B. СВОЙСТВА МАТЕМАТИЧЕСКОГО ОЖИДАНИЯ
// =============================================================

// --- B.1. Проверка линейности: E[aX + bY] == a*E[X] + b*E[Y] ---
// Генерируем выборки X, Y; вычисляем E[aX+bY] и сравниваем.
// Возвращает |E[aX+bY] - (a*E[X] + b*E[Y])|.
// O(n) время.
double check_linearity(int n, double a, double b,
                       double (*gen_x)(), double (*gen_y)()) {
    vector<double> x(n), y(n);
    for (int i = 0; i < n; i++) {
        x[i] = gen_x();
        y[i] = gen_y();
    }
    double ex = 0, ey = 0, exy = 0;
    for (int i = 0; i < n; i++) {
        ex += x[i];
        ey += y[i];
        exy += a * x[i] + b * y[i];
    }
    ex /= n; ey /= n; exy /= n;
    return fabs(exy - (a * ex + b * ey));
}

// --- B.2. E[X-Y] при независимости: E[XY] == E[X]*E[Y] ---
// Возвращает |E[XY] - E[X]*E[Y]|.
// O(n) время.
double check_independence(int n, double (*gen_x)(), double (*gen_y)()) {
    vector<double> x(n), y(n), xy(n);
    for (int i = 0; i < n; i++) {
        x[i] = gen_x();
        y[i] = gen_y();
        xy[i] = x[i] * y[i];
    }
    double ex = accumulate(x.begin(), x.end(), 0.0) / n;
    double ey = accumulate(y.begin(), y.end(), 0.0) / n;
    double exy = accumulate(xy.begin(), xy.end(), 0.0) / n;
    return fabs(exy - ex * ey);
}

// --- B.3. E[indicator_A] = P(A) ---
// Генерируем n сэмплов, считаем долю попаданий в A.
// Возвращает |доля - P(A)|.
// O(n) время.
double check_indicator(int n, double p_a, double (*gen_x)()) {
    int count = 0;
    for (int i = 0; i < n; i++)
        if (gen_x() <= p_a) count++;
    return fabs((double)count / n - p_a);
}

// =============================================================
// C. ФОРМУЛЫ ПОДСЧЁТА
// =============================================================

// --- C.1. E[X^k] для дискретной СВ (момент k-го порядка) ---
// O(n) время.
double moment_discrete(int k, const vector<double>& values,
                       const vector<double>& probs) {
    double result = 0.0;
    for (int i = 0; i < (int)values.size(); i++)
        result += pow(values[i], k) * probs[i];
    return result;
}

// --- C.2. E[X^k] через плотность (численно) ---
// O(n_steps) время.
double moment_pdf(int k, double (*pdf)(double), double lo, double hi,
                  int n_steps = 10000) {
    if (n_steps % 2 != 0) n_steps++;
    double h = (hi - lo) / n_steps;
    double sum = pow(lo, k) * pdf(lo) + pow(hi, k) * pdf(hi);
    for (int i = 1; i < n_steps; i++) {
        double x = lo + i * h;
        sum += pow(x, k) * pdf(x) * ((i % 2 == 0) ? 2.0 : 4.0);
    }
    return sum * h / 3.0;
}

// --- C.3. E[X] через CDF (неотрицательная СВ) ---
// E[X] = integral_0^inf (1 - F(t)) dt.
// Аппроксимация:ewan-[0, upper_bound] с шагом h.
double expectation_cdf(double (*cdf)(double), double upper_bound = 20.0,
                       int n_steps = 10000) {
    double h = upper_bound / n_steps;
    double result = 0.0;
    for (int i = 0; i < n_steps; i++) {
        double t = i * h;
        result += (1.0 - cdf(t)) * h;
    }
    return result;
}

// --- C.4. E[X] через эмпирическую CDF ---
// F_n(x) = доля элементов <= x.
// E[X] = integral (1 - F_n(t)) dt.
double expectation_empirical_cdf(const vector<double>& sample,
                                 double upper_bound = -1) {
    int n = (int)sample.size();
    if (upper_bound < 0) {
        upper_bound = 0;
        for (double x : sample) upper_bound = max(upper_bound, x);
        upper_bound *= 2;
    }
    vector<double> sorted_sample = sample;
    sort(sorted_sample.begin(), sorted_sample.end());
    int n_steps = 10000;
    double h = upper_bound / n_steps;
    double result = 0.0;
    int idx = 0;
    for (int i = 0; i < n_steps; i++) {
        double t = (i + 0.5) * h;
        while (idx < n && sorted_sample[idx] <= t) idx++;
        double fn = (double)idx / n;
        result += (1.0 - fn) * h;
    }
    return result;
}

// =============================================================
// D. МЕДИАНА И КВАНТИЛИ
// =============================================================

// --- D.1. Медиана выборки ---
// O(n log n) время (через сортировку).
double median(vector<double> sample) {
    int n = (int)sample.size();
    sort(sample.begin(), sample.end());
    if (n % 2 == 1)
        return sample[n / 2];
    return (sample[n / 2 - 1] + sample[n / 2]) / 2.0;
}

// --- D.2. Квантиль уровня p выборки ---
// q_p = inf{x : F_n(x) >= p}.
// O(n log n) время.
double quantile(vector<double> sample, double p) {
    int n = (int)sample.size();
    sort(sample.begin(), sample.end());
    double idx = p * (n - 1);
    int lo = (int)floor(idx);
    int hi = (int)ceil(idx);
    if (lo == hi) return sample[lo];
    double frac = idx - lo;
    return sample[lo] * (1.0 - frac) + sample[hi] * frac;
}

// --- D.3. Квартили выборки ---
// Возвращает {Q1, Q2 (медиана), Q3}.
vector<double> quartiles(vector<double> sample) {
    return {quantile(sample, 0.25), quantile(sample, 0.50),
            quantile(sample, 0.75)};
}

// --- D.4. Межквартильный размах ---
// IQR = Q3 - Q1.
double iqr(vector<double> sample) {
    auto q = quartiles(sample);
    return q[2] - q[0];
}

// --- D.5. Квантиль теоретического распределения ---
// Через бинарный поиск по CDF.
double quantile_cdf(double (*cdf)(double), double p,
                    double lo = -100, double hi = 100,
                    double eps = 1e-8) {
    while (hi - lo > eps) {
        double mid = (lo + hi) / 2.0;
        if (cdf(mid) < p)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) / 2.0;
}

// --- D.6. Неравенство о медиане ---
// P(|X - m| >= t) <= 1 - F(m+t) + F(m-t).
// Возвращает верхнюю границу.
double median_bound(double t, double median_val,
                    double (*cdf)(double)) {
    return 1.0 - cdf(median_val + t) + cdf(median_val - t);
}

}; // struct Expectation

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_D_MAIN
int main() {
    Expectation exp_;
    srand(42);

    cout << "=== A. ПОСТРОЕНИЕ МАТЕМАТИЧЕСКОГО ОЖИДАНИЯ ===" << endl;

    cout << "--- Дискретная СВ ---" << endl;
    vector<double> vals = {1, 2, 3, 4, 5, 6};
    vector<double> probs6 = {1.0/6, 1.0/6, 1.0/6, 1.0/6, 1.0/6, 1.0/6};
    cout << "E[кубик] = " << exp_.expectation_discrete(vals, probs6) << " (ожидаем 3.5)" << endl;

    cout << "\n--- E[X^2] для кубика ---" << endl;
    cout << "E[X^2] = " << exp_.moment_discrete(2, vals, probs6)
         << " (ожидаем 91/6 = " << 91.0/6 << ")" << endl;

    cout << "\n--- Через хвостовые вероятности (Exp(1)) ---" << endl;
    // E[Exp(1)] = 1
    vector<double> exp_sample;
    for (int i = 0; i < 10000; i++)
        exp_sample.push_back(exp_.exponential_sample(1.0));
    double e_tail = exp_.expectation_tail_probability(exp_sample);
    cout << "E[Exp(1)] ~ " << e_tail << " (ожидаем 1.0)" << endl;

    cout << "\n--- Через плотность (Exp(1)) ---" << endl;
    auto exp_pdf = [](double x) -> double {
        return (x < 0) ? 0.0 : exp(-x);
    };
    double e_pdf = exp_.expectation_pdf(exp_pdf, 0, 30);
    cout << "E[Exp(1)] ~ " << e_pdf << " (ожидаем 1.0)" << endl;

    cout << "\n--- Через CDF (Exp(1)) ---" << endl;
    auto exp_cdf = [](double x) -> double {
        return (x < 0) ? 0.0 : 1.0 - exp(-x);
    };
    double e_cdf = exp_.expectation_cdf(exp_cdf);
    cout << "E[Exp(1)] ~ " << e_cdf << " (ожидаем 1.0)" << endl;

    cout << "\n--- Через эмпирическую CDF ---" << endl;
    double e_ecdf = exp_.expectation_empirical_cdf(exp_sample);
    cout << "E[Exp(1)] ~ " << e_ecdf << " (ожидаем 1.0)" << endl;

    cout << "\n=== B. СВОЙСТВА ===" << endl;

    cout << "--- Линейность: E[3X + 2Y] == 3E[X] + 2E[Y] ---" << endl;
    auto gen_u = []() -> double { return (double)rand() / RAND_MAX; };
    auto gen_n = []() -> double {
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    };
    double lin_err = exp_.check_linearity(10000, 3.0, 2.0, gen_u, gen_n);
    cout << "Ошибка линейности: " << lin_err << endl;

    cout << "\n--- Независимость: E[XY] == E[X]*E[Y] ---" << endl;
    double ind_err = exp_.check_independence(10000, gen_u, gen_n);
    cout << "Ошибка (независимые U(0,1) и N(0,1)): " << ind_err << endl;

    cout << "\n--- E[indicator] = P(A) ---" << endl;
    double ind_err2 = exp_.check_indicator(10000, 0.3, gen_u);
    cout << "Ошибка (P(A)=0.3): " << ind_err2 << endl;

    cout << "\n=== C. ФОРМУЛЫ ПОДСЧЁТА ===" << endl;

    cout << "--- E[X^k] для кубика ---" << endl;
    for (int k : {1, 2, 3, 4}) {
        double exact = exp_.moment_discrete(k, vals, probs6);
        cout << "E[X^" << k << "] = " << exact << endl;
    }

    cout << "\n--- E[X] через CDF для Exp(2) ---" << endl;
    // E[Exp(2)] = 1/2 = 0.5
    auto exp2_cdf = [](double x) -> double {
        return (x < 0) ? 0.0 : 1.0 - exp(-2.0 * x);
    };
    double e2_cdf = exp_.expectation_cdf(exp2_cdf);
    cout << "E[Exp(2)] ~ " << e2_cdf << " (ожидаем 0.5)" << endl;

    cout << "\n--- E[X] через плотность для N(0,1) ---" << endl;
    auto norm_pdf = [](double x) -> double {
        return exp(-x * x / 2.0) / sqrt(2.0 * M_PI);
    };
    double e_norm = exp_.expectation_pdf(norm_pdf, -10, 10);
    cout << "E[N(0,1)] ~ " << e_norm << " (ожидаем 0)" << endl;

    cout << "\n=== D. МЕДИАНА И КВАНТИЛИ ===" << endl;

    cout << "--- Выборка 20 сэмплов Exp(1) ---" << endl;
    vector<double> s20;
    for (int i = 0; i < 20; i++) s20.push_back(exp_.exponential_sample(1.0));
    cout << "Медиана: " << exp_.median(s20) << " (ожидаем ~0.693 = ln2)" << endl;
    auto q = exp_.quartiles(s20);
    cout << "Q1=" << q[0] << " Q2=" << q[1] << " Q3=" << q[2] << endl;
    cout << "IQR=" << exp_.iqr(s20) << endl;

    cout << "\n--- Квантили Exp(1) через CDF ---" << endl;
    for (double p : {0.1, 0.25, 0.5, 0.75, 0.9}) {
        double qp = exp_.quantile_cdf(exp_cdf, p);
        cout << "q_" << p << " = " << qp << endl;
    }

    cout << "\n--- Неравенство о медиане ---" << endl;
    double med = exp_.median(s20);
    for (double t : {0.5, 1.0, 2.0}) {
        double bound = exp_.median_bound(t, med, exp_cdf);
        double empirical = 0;
        for (double x : s20)
            if (fabs(x - med) >= t) empirical++;
        empirical /= 20;
        cout << "t=" << t << ": bound=" << bound
             << " empirical=" << empirical << endl;
    }

    return 0;
}
#endif

#endif // PROB_AND_STATS_D_CPP
