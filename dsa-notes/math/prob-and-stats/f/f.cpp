#ifndef PROB_AND_STATS_F_CPP
#define PROB_AND_STATS_F_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
using namespace std;

// =============================================================
// F. ЧИСЛОВЫЕ ХАРАКТЕРИСТИКИ И НЕРАВЕНСТВА
// =============================================================
// Структура md: A. Дисперсия и ковариация
//               → B. Моменты и центральные моменты
//               → C. Неравенства
//               → D. Корреляционные свойства
//
// NumericalCharacteristics — наследует MultivariateDistributions (e.cpp).
// Дисперсия, ковариация, корреляция, матрица ковариаций,
// моменты и центральные моменты (асимметрия, эксцесс),
// MGF, характеристическая функция, неравенства.

#ifndef INSIDE_PROB_AND_STATS_F
#define INSIDE_PROB_AND_STATS_F
#include "../e/e.cpp"
#undef INSIDE_PROB_AND_STATS_F
#endif

struct NumericalCharacteristics : MultivariateDistributions {

// =============================================================
// A. ДИСПЕРСИЯ И КОВАРИАЦИЯ
// =============================================================

// --- A.1. Дисперсия выборки ---
// D = (1/n) * sum (x_i - mean)^2.
// O(n) время.
double variance_sample(const vector<double>& sample) {
    int n = (int)sample.size();
    double mean = 0;
    for (double x : sample) mean += x;
    mean /= n;
    double var = 0;
    for (double x : sample) var += (x - mean) * (x - mean);
    return var / n;
}

// --- A.2. Несмещённая дисперсия (Bessel's correction) ---
// S^2 = (1/(n-1)) * sum (x_i - mean)^2.
// O(n) время.
double variance_unbiased(const vector<double>& sample) {
    int n = (int)sample.size();
    if (n <= 1) return 0;
    double mean = 0;
    for (double x : sample) mean += x;
    mean /= n;
    double var = 0;
    for (double x : sample) var += (x - mean) * (x - mean);
    return var / (n - 1);
}

// --- A.3. Ковариация двух выборок ---
// cov(X,Y) = (1/n) * sum (x_i - mean_x)(y_i - mean_y).
// O(n) время.
double covariance_sample(const vector<double>& x, const vector<double>& y) {
    int n = (int)x.size();
    double mx = 0, my = 0;
    for (int i = 0; i < n; i++) { mx += x[i]; my += y[i]; }
    mx /= n; my /= n;
    double cov = 0;
    for (int i = 0; i < n; i++)
        cov += (x[i] - mx) * (y[i] - my);
    return cov / n;
}

// --- A.4. Корреляция Пирсона ---
// rho = cov(X,Y) / (sigma_x * sigma_y).
// O(n) время.
double correlation_sample(const vector<double>& x, const vector<double>& y) {
    int n = (int)x.size();
    double mx = 0, my = 0;
    for (int i = 0; i < n; i++) { mx += x[i]; my += y[i]; }
    mx /= n; my /= n;
    double cov = 0, vx = 0, vy = 0;
    for (int i = 0; i < n; i++) {
        double dx = x[i] - mx;
        double dy = y[i] - my;
        cov += dx * dy;
        vx += dx * dx;
        vy += dy * dy;
    }
    double denom = sqrt(vx * vy);
    return (denom > 1e-300) ? cov / denom : 0.0;
}

// --- A.5. Матрица ковариаций ---
// samples[d][n] — d выборок по n элементов.
// Возвращает d x d матрицу ковариаций.
vector<vector<double>> covariance_matrix(
    const vector<vector<double>>& samples) {
    int d = (int)samples.size();
    int n = (int)samples[0].size();
    vector<double> means(d, 0.0);
    for (int i = 0; i < d; i++)
        for (int j = 0; j < n; j++)
            means[i] += samples[i][j];
    for (int i = 0; i < d; i++) means[i] /= n;

    vector<vector<double>> sigma(d, vector<double>(d, 0.0));
    for (int i = 0; i < d; i++) {
        for (int j = i; j < d; j++) {
            double cov = 0;
            for (int k = 0; k < n; k++)
                cov += (samples[i][k] - means[i]) * (samples[j][k] - means[j]);
            sigma[i][j] = cov / n;
            sigma[j][i] = sigma[i][j];
        }
    }
    return sigma;
}

// =============================================================
// B. МОМЕНТЫ И ЦЕНТРАЛЬНЫЕ МОМЕНТЫ
// =============================================================

// --- B.1. Момент k-го порядка ---
// m_k = (1/n) * sum x_i^k.
// O(n) время.
double moment(const vector<double>& sample, int k) {
    double result = 0;
    for (double x : sample) result += pow(x, k);
    return result / (int)sample.size();
}

// --- B.2. Центральный момент k-го порядка ---
// mu_k = (1/n) * sum (x_i - mean)^k.
// O(n) время.
double central_moment(const vector<double>& sample, int k) {
    double mean = 0;
    for (double x : sample) mean += x;
    mean /= (int)sample.size();
    double result = 0;
    for (double x : sample) result += pow(x - mean, k);
    return result / (int)sample.size();
}

// --- B.3. Асимметрия (skewness) ---
// gamma_1 = mu_3 / sigma^3.
// O(n) время.
double skewness(const vector<double>& sample) {
    double mu3 = central_moment(sample, 3);
    double sigma = sqrt(central_moment(sample, 2));
    return (sigma > 1e-300) ? mu3 / (sigma * sigma * sigma) : 0.0;
}

// --- B.4. Эксцесс (excess kurtosis) ---
// gamma_2 = mu_4 / sigma^4 - 3.
// O(n) время.
double kurtosis(const vector<double>& sample) {
    double mu4 = central_moment(sample, 4);
    double var = central_moment(sample, 2);
    return (var > 1e-300) ? mu4 / (var * var) - 3.0 : 0.0;
}

// --- B.5. Моментная порождающая функция (MGF) ---
// M(t) = E[e^{tX}] = (1/n) * sum e^{t*x_i}.
// O(n) время.
double mgf(const vector<double>& sample, double t) {
    double result = 0;
    for (double x : sample) result += exp(t * x);
    return result / (int)sample.size();
}

// --- B.6. Характеристическая функция ---
// phi(t) = E[e^{itX}] = (1/n) * sum e^{i*t*x_i}.
// Возвращает {Re, Im}.
pair<double, double> characteristic_function(const vector<double>& sample,
                                            double t) {
    double re = 0, im = 0;
    for (double x : sample) {
        re += cos(t * x);
        im += sin(t * x);
    }
    int n = (int)sample.size();
    return {re / n, im / n};
}

// =============================================================
// C. НЕРАВЕНСТВА
// =============================================================

// --- C.1. Неравенство Маркова ---
// P(|X| >= eps) <= E[|X|^k] / eps^k.
// Возвращает верхнюю границу.
double markov_bound(const vector<double>& sample, double eps, int k) {
    double e_xk = 0;
    for (double x : sample) e_xk += pow(fabs(x), k);
    e_xk /= (int)sample.size();
    return e_xk / pow(eps, k);
}

// --- C.2. Неравенство Чебышёва ---
// P(|X - mu| >= eps) <= D[X] / eps^2.
double chebyshev_bound(const vector<double>& sample, double eps) {
    double var = variance_sample(sample);
    return var / (eps * eps);
}

// --- C.3. Фактическая вероятность для сравнения ---
// P(|X - mu| >= eps) — эмпирическая.
double empirical_exceedance(const vector<double>& sample, double eps) {
    double mean = 0;
    for (double x : sample) mean += x;
    mean /= (int)sample.size();
    int count = 0;
    for (double x : sample)
        if (fabs(x - mean) >= eps) count++;
    return (double)count / (int)sample.size();
}

// --- C.4. Проверка неравенства Йенсена ---
// Для выпуклой phi: phi(E[X]) <= E[phi(X)].
// Возвращает |phi(E[X]) - E[phi(X])| (должно быть >= 0).
double jensen_check(const vector<double>& sample,
                    double (*phi)(double)) {
    double mean = 0;
    for (double x : sample) mean += x;
    mean /= (int)sample.size();
    double phi_mean = phi(mean);
    double mean_phi = 0;
    for (double x : sample) mean_phi += phi(x);
    mean_phi /= (int)sample.size();
    return mean_phi - phi_mean;  // >= 0 если phi выпуклая
}

// --- C.5. Неравенство Коши-Буняковского (Гёльдер p=q=2) ---
// |E[XY]| <= sqrt(E[X^2] * E[Y^2]).
// Возвращает |E[XY]| / sqrt(E[X^2]*E[Y^2]) — должен быть <= 1.
double cauchy_schwarz_ratio(const vector<double>& x,
                            const vector<double>& y) {
    int n = (int)x.size();
    double exy = 0, ex2 = 0, ey2 = 0;
    for (int i = 0; i < n; i++) {
        exy += x[i] * y[i];
        ex2 += x[i] * x[i];
        ey2 += y[i] * y[i];
    }
    double denom = sqrt(ex2 * ey2);
    return (denom > 1e-300) ? fabs(exy) / denom : 0.0;
}

// =============================================================
// D. КОРРЕЛЯЦИОННЫЕ СВОЙСТВА
// =============================================================

// --- D.1. Ковариация через D[X+Y] ---
// cov(X,Y) = (D[X+Y] - D[X] - D[Y]) / 2.
// O(n) время.
double covariance_via_variance(const vector<double>& x,
                               const vector<double>& y) {
    int n = (int)x.size();
    vector<double> sum_xy(n);
    for (int i = 0; i < n; i++) sum_xy[i] = x[i] + y[i];
    double dxy = variance_sample(sum_xy);
    double dx = variance_sample(x);
    double dy = variance_sample(y);
    return (dxy - dx - dy) / 2.0;
}

// --- D.2. Проверка: некоррелированные vs независимые ---
// Генерируем X ~ U(-1,1), Y = X^2.
// corr(X,Y) = 0, но Y = f(X) → зависимы.
pair<double, bool> corr_vs_independence(int n) {
    vector<double> x(n), y(n);
    for (int i = 0; i < n; i++) {
        x[i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
        y[i] = x[i] * x[i];
    }
    double corr = correlation_sample(x, y);
    // Факторизация: f(x,y) != fX(x)*fY(y)
    // Проверяем: E[XY] vs E[X]*E[Y]
    double ex = 0, ey = 0, exy = 0;
    for (int i = 0; i < n; i++) {
        ex += x[i]; ey += y[i]; exy += x[i] * y[i];
    }
    ex /= n; ey /= n; exy /= n;
    bool independent = (fabs(exy - ex * ey) < 0.01);
    return {corr, independent};
}

// --- D.3. Rang корреляционной матрицы ---
// Через подсчёт ненулевых собственных значений (приближённо).
// Сравниваем диагональные элементы приizations.
int covariance_rank(const vector<vector<double>>& sigma,
                    double eps = 1e-6) {
    int d = (int)sigma.size();
    // Простая эврика: rank через Gaussian elimination на sigma
    vector<vector<double>> mat = sigma;
    int rank = 0;
    for (int col = 0; col < d; col++) {
        // Ищем опорный элемент
        int pivot = -1;
        for (int row = rank; row < d; row++) {
            if (fabs(mat[row][col]) > eps) {
                pivot = row;
                break;
            }
        }
        if (pivot == -1) continue;
        swap(mat[rank], mat[pivot]);
        double div = mat[rank][col];
        for (int j = col; j < d; j++)
            mat[rank][j] /= div;
        for (int row = 0; row < d; row++) {
            if (row == rank) continue;
            double factor = mat[row][col];
            for (int j = col; j < d; j++)
                mat[row][j] -= factor * mat[rank][j];
        }
        rank++;
    }
    return rank;
}

}; // struct NumericalCharacteristics

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_F_MAIN
int main() {
    NumericalCharacteristics nc;
    srand(42);

    cout << "=== A. ДИСПЕРСИЯ И КОВАРИАЦИЯ ===" << endl;

    cout << "--- Дисперсия выборки ---" << endl;
    vector<double> s10 = {2, 4, 4, 4, 5, 5, 7, 9};
    cout << "Выборка: {2,4,4,4,5,5,7,9}" << endl;
    cout << "E[X] = " << nc.moment(s10, 1) << endl;
    cout << "D[X] (смещённая) = " << nc.variance_sample(s10) << endl;
    cout << "D[X] (несмещённая) = " << nc.variance_unbiased(s10) << endl;

    cout << "\n--- Ковариация и корреляция ---" << endl;
    vector<double> x = {1, 2, 3, 4, 5};
    vector<double> y = {2, 4, 5, 4, 5};
    cout << "X = {1,2,3,4,5}, Y = {2,4,5,4,5}" << endl;
    cout << "cov(X,Y) = " << nc.covariance_sample(x, y) << endl;
    cout << "corr(X,Y) = " << nc.correlation_sample(x, y) << endl;
    cout << "cov через D[X+Y] = " << nc.covariance_via_variance(x, y) << endl;

    cout << "\n--- Матрица ковариаций ---" << endl;
    int d = 3, n = 1000;
    vector<vector<double>> samples(d, vector<double>(n));
    for (int i = 0; i < n; i++) {
        auto [z1, z2] = nc.box_muller();
        double z3 = nc.box_muller().first;
        samples[0][i] = z1;
        samples[1][i] = z1 + 0.5 * z2;
        samples[2][i] = 0.3 * z1 - 0.7 * z3;
    }
    auto sigma = nc.covariance_matrix(samples);
    cout << "Sigma = [" << endl;
    for (int i = 0; i < d; i++) {
        cout << "  [";
        for (int j = 0; j < d; j++) {
            if (j > 0) cout << ", ";
            cout << sigma[i][j];
        }
        cout << "]" << endl;
    }
    cout << "]" << endl;
    cout << "Ранг = " << nc.covariance_rank(sigma) << " (ожидаем 3)" << endl;

    cout << "\n=== B. МОМЕНТЫ ===" << endl;

    cout << "--- Моменты нормального N(0,1) ---" << endl;
    vector<double> norm_samp;
    for (int i = 0; i < 10000; i++)
        norm_samp.push_back(nc.normal_sample(0, 1));
    cout << "E[X] = " << nc.moment(norm_samp, 1) << " (ожидаем 0)" << endl;
    cout << "E[X^2] = " << nc.moment(norm_samp, 2) << " (ожидаем 1)" << endl;
    cout << "E[X^3] = " << nc.moment(norm_samp, 3) << " (ожидаем 0)" << endl;
    cout << "E[X^4] = " << nc.moment(norm_samp, 4) << " (ожидаем 3)" << endl;

    cout << "\n--- Центральные моменты и формы распределения ---" << endl;
    cout << "Asimmetriya (skewness) N(0,1) = " << nc.skewness(norm_samp)
         << " (ожидаем ~0)" << endl;
    cout << "Excess kurtosis N(0,1) = " << nc.kurtosis(norm_samp)
         << " (ожидаем ~0)" << endl;

    // Экспоненциальное — асимметричное, положительный skewness
    vector<double> exp_samp;
    for (int i = 0; i < 10000; i++) exp_samp.push_back(nc.exponential_sample(1.0));
    cout << "Skewness Exp(1) = " << nc.skewness(exp_samp)
         << " (ожидаем 2)" << endl;
    cout << "Kurtosis Exp(1) = " << nc.kurtosis(exp_samp)
         << " (ожидаем 6)" << endl;

    cout << "\n--- MGF и характеристическая функция ---" << endl;
    cout << "MGF_N(0,1)(t=0) = " << nc.mgf(norm_samp, 0)
         << " (ожидаем 1)" << endl;
    cout << "MGF_N(0,1)(t=1) = " << nc.mgf(norm_samp, 1)
         << " (ожидаем e^{0.5}=" << exp(0.5) << ")" << endl;
    auto cf = nc.characteristic_function(norm_samp, 1.0);
    cout << "CF_N(0,1)(t=1) = " << cf.first << " + " << cf.second << "i"
         << " (ожидаем e^{-0.5}cos(1)=" << exp(-0.5) * cos(1.0)
         << ", e^{-0.5}sin(1)=" << exp(-0.5) * sin(1.0) << ")" << endl;

    cout << "\n=== C. НЕРАВЕНСТВА ===" << endl;

    cout << "--- Неравенство Маркова и Чебышёва ---" << endl;
    vector<double> pos_samp;
    for (int i = 0; i < 5000; i++) pos_samp.push_back(nc.exponential_sample(1.0));
    double mean_pos = nc.moment(pos_samp, 1);
    cout << "E[Exp(1)] ~ " << mean_pos << endl;
    for (double eps : {1.0, 2.0, 3.0}) {
        double mk = nc.markov_bound(pos_samp, eps, 1);
        double ch = nc.chebyshev_bound(pos_samp, eps);
        double emp = nc.empirical_exceedance(pos_samp, eps);
        cout << "eps=" << eps << ": Markov<=" << mk
             << " Chebyshev<=" << ch
             << " empirical=" << emp << endl;
    }

    cout << "\n--- Неравенство Йенсена: phi(x) = x^2 ---" << endl;
    auto square = [](double x) -> double { return x * x; };
    double jensen_val = nc.jensen_check(norm_samp, square);
    cout << "E[X^2] - (E[X])^2 = " << jensen_val
         << " (ожидаем >= 0 = D[X] = " << nc.variance_sample(norm_samp) << ")" << endl;

    cout << "\n--- Коши-Буняковский: |E[XY]| <= sqrt(E[X^2]E[Y^2]) ---" << endl;
    double cs_ratio = nc.cauchy_schwarz_ratio(x, y);
    cout << "|E[XY]|/sqrt(E[X^2]E[Y^2]) = " << cs_ratio
         << " (ожидаем <= 1)" << endl;

    cout << "\n=== D. КОРРЕЛЯЦИОННЫЕ СВОЙСТВА ===" << endl;

    cout << "--- Некоррелированные vs независимые ---" << endl;
    auto [corr_val, is_ind] = nc.corr_vs_independence(5000);
    cout << "X ~ U(-1,1), Y = X^2:" << endl;
    cout << "corr(X,Y) = " << corr_val << " (~0)" << endl;
    cout << "Независимы: " << (is_ind ? "ДА" : "НЕТ (ожидаем НЕТ)") << endl;

    return 0;
}
#endif

#endif // PROB_AND_STATS_F_CPP
