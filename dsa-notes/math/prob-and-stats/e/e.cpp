#ifndef PROB_AND_STATS_E_CPP
#define PROB_AND_STATS_E_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include <numeric>
using namespace std;

// =============================================================
// E. МНОГОМЕРНЫЕ РАСПРЕДЕЛЕНИЯ И НЕЗАВИСИМОСТЬ
// =============================================================
// Структура md: A. Многомерные функции распределения
//               → B. Независимость случайных величин
//               → C. Совместные распределения и свёртки
//               → D. Условные распределения
//
// MultivariateDistributions — наследует Expectation (d.cpp).
// Совместные ФР, плотности, независимость, свёртки,
// формулы сложения распределений, условные распределения,
// основы марковских цепей.

#ifndef INSIDE_PROB_AND_STATS_E
#define INSIDE_PROB_AND_STATS_E
#include "../d/d.cpp"
#undef INSIDE_PROB_AND_STATS_E
#endif

struct MultivariateDistributions : Expectation {

// =============================================================
// A. МНОГОМЕРНЫЕ ФУНКЦИИ РАСПРЕДЕЛЕНИЯ
// =============================================================

// --- A.1. Совместная плотность двумерного нормального N(mu1,mu2,s11,s12,s22) ---
// f(x,y) = (1/(2*pi*s1*s2*sqrt(1-rho^2)))
//           * exp(-Q / (2*(1-rho^2)))
// где Q = ((x-mu1)^2/s1^2 - 2*rho*(x-mu1)*(y-mu2)/(s1*s2) + (y-mu2)^2/s2^2)
// rho = s12 / (s1*s2) — корреляция.
// O(1) время.
double bivariate_normal_pdf(double x, double y, double mu1, double mu2,
                            double s1, double s2, double rho) {
    double dx = (x - mu1) / s1;
    double dy = (y - mu2) / s2;
    double Q = dx * dx - 2.0 * rho * dx * dy + dy * dy;
    double norm = 1.0 / (2.0 * M_PI * s1 * s2 * sqrt(1.0 - rho * rho));
    return norm * exp(-Q / (2.0 * (1.0 - rho * rho)));
}

// --- A.2. Совместная плотность при независимости ---
// f(x,y) = f_X(x) * f_Y(y).
// O(1) время.
double independent_joint_pdf(double x, double y,
                             double (*fx)(double), double (*fy)(double)) {
    return fx(x) * fy(y);
}

// --- A.3. Маргинальная плотность X из совместной ---
// f_X(x) = integral f(x,y) dy.
// Численное интегрирование: Симпсон на [lo, hi].
// O(n_steps) время.
double marginal_x(double x, double (*fxy)(double, double),
                  double y_lo, double y_hi, int n_steps = 500) {
    if (n_steps % 2 != 0) n_steps++;
    double h = (y_hi - y_lo) / n_steps;
    double sum = fxy(x, y_lo) + fxy(x, y_hi);
    for (int i = 1; i < n_steps; i++) {
        double y = y_lo + i * h;
        sum += fxy(x, y) * ((i % 2 == 0) ? 2.0 : 4.0);
    }
    return sum * h / 3.0;
}

// =============================================================
// B. НЕЗАВИСИМОСТЬ СЛУЧАЙНЫХ ВЕЛИЧИН
// =============================================================

// --- B.1. Проверка независимости через попарные плотности ---
// Для каждой пары (i,j): проверяем, что f(x_i, x_j) = f_i(x_i) * f_j(x_j).
// Оцениваем через эмпирическую корреляцию (не точный критерий, но индикация).
// Возвращает максимальное отклонение |corr| (близко к 0 = независимы).
// O(n^2) время.
double check_pairwise_independence(const vector<vector<double>>& samples) {
    int d = (int)samples.size();
    int n = (int)samples[0].size();
    double max_corr = 0.0;
    for (int i = 0; i < d; i++) {
        for (int j = i + 1; j < d; j++) {
            double ex = 0, ey = 0, exy = 0, ex2 = 0, ey2 = 0;
            for (int k = 0; k < n; k++) {
                ex += samples[i][k];
                ey += samples[j][k];
                exy += samples[i][k] * samples[j][k];
                ex2 += samples[i][k] * samples[i][k];
                ey2 += samples[j][k] * samples[j][k];
            }
            ex /= n; ey /= n; exy /= n; ex2 /= n; ey2 /= n;
            double var_x = ex2 - ex * ex;
            double var_y = ey2 - ey * ey;
            double corr = 0;
            if (var_x > 1e-12 && var_y > 1e-12)
                corr = (exy - ex * ey) / sqrt(var_x * var_y);
            max_corr = max(max_corr, fabs(corr));
        }
    }
    return max_corr;
}

// --- B.2. Факторизация плотности: f(x,y) = f_X(x)*f_Y(y) ---
// Оцениваем через логарифм: ln f(x,y) - ln f_X(x) - ln f_Y(y) ~ 0.
// Возвращает среднее квадратичное отклонение.
// O(n) время.
double check_factorization(double (*fxy)(double, double),
                           double (*fx)(double), double (*fy)(double),
                           const vector<double>& x_sample,
                           const vector<double>& y_sample) {
    int n = (int)x_sample.size();
    double sse = 0.0;
    for (int i = 0; i < n; i++) {
        double joint = fxy(x_sample[i], y_sample[i]);
        double marg_x = fx(x_sample[i]);
        double marg_y = fy(y_sample[i]);
        if (joint > 1e-300 && marg_x > 1e-300 && marg_y > 1e-300) {
            double diff = log(joint) - log(marg_x) - log(marg_y);
            sse += diff * diff;
        }
    }
    return sqrt(sse / n);
}

// =============================================================
// C. СВЁРТКИ ПЛОТНОСТЕЙ
// =============================================================

// --- C.1. Свёртка двух плотностей (численно) ---
// (f * g)(x) = integral f(t) * g(x - t) dt.
// Метод Симпсона на [lo, hi].
// O(n_steps) время.
double convolution(double x, double (*f)(double), double (*g)(double),
                   double lo = -20, double hi = 20, int n_steps = 2000) {
    if (n_steps % 2 != 0) n_steps++;
    double h = (hi - lo) / n_steps;
    double sum = f(lo) * g(x - lo) + f(hi) * g(x - hi);
    for (int i = 1; i < n_steps; i++) {
        double t = lo + i * h;
        sum += f(t) * g(x - t) * ((i % 2 == 0) ? 2.0 : 4.0);
    }
    return sum * h / 3.0;
}

// --- C.2. Плотность суммы N(mu1,s1^2) + N(mu2,s2^2) = N(mu1+mu2, s1^2+s2^2) ---
double sum_normals_pdf(double x, double mu1, double s1,
                       double mu2, double s2) {
    double mu = mu1 + mu2;
    double s = sqrt(s1 * s1 + s2 * s2);
    return exp(-(x - mu) * (x - mu) / (2.0 * s * s)) / (s * sqrt(2.0 * M_PI));
}

// --- C.3. Свёртка Exp(lambda) * Exp(lambda) = Gamma(2, lambda) ---
// Gamma(2, lambda): f(x) = lambda^2 * x * exp(-lambda * x) для x >= 0.
double erlang2_pdf(double x, double lambda) {
    if (x < 0.0) return 0.0;
    return lambda * lambda * x * exp(-lambda * x);
}

// =============================================================
// D. УСЛОВНЫЕ РАСПРЕДЕЛЕНИЯ
// =============================================================

// --- D.1. Условная плотность f(x|y) = f(x,y) / f_Y(y) ---
// O(n_marginal) время для вычисления f_Y(y).
double conditional_pdf_x(double x, double y,
                         double (*fxy)(double, double),
                         double (*fy)(double)) {
    double fy_val = fy(y);
    if (fy_val < 1e-300) return 0.0;
    return fxy(x, y) / fy_val;
}

// --- D.2. Условное E[X|Y=y] через плотность ---
// E[X|Y=y] = integral x * f(x|y) dx.
double conditional_expectation_x(double y, double (*fxy)(double, double),
                                 double (*fy)(double),
                                 double x_lo = -20, double x_hi = 20,
                                 int n_steps = 500) {
    if (n_steps % 2 != 0) n_steps++;
    double h = (x_hi - x_lo) / n_steps;
    double sum = x_lo * fxy(x_lo, y) + x_hi * fxy(x_hi, y);
    for (int i = 1; i < n_steps; i++) {
        double x = x_lo + i * h;
        sum += x * fxy(x, y) * ((i % 2 == 0) ? 2.0 : 4.0);
    }
    double integral = sum * h / 3.0;
    double fy_val = fy(y);
    return (fy_val > 1e-300) ? integral / fy_val : 0.0;
}

// --- D.3. Проверка E[X] = E[E[X|Y]] ---
// Генерируем выборки X, Y; считаем E[X|Y=y_i] для каждого y_i;
// затем E[E[X|Y]] = среднее E[X|Y=y_i].
// O(n * n_steps) время.
double check_iterated_expectation(double (*fxy)(double, double),
                                  double (*fy)(double),
                                  double (*fx)(double),
                                  const vector<double>& x_sample,
                                  const vector<double>& y_sample) {
    int n = (int)x_sample.size();
    // E[X] через выборку
    double ex_sample = 0;
    for (double x : x_sample) ex_sample += x;
    ex_sample /= n;
    // E[E[X|Y]] через выборку Y
    double e_cond_sum = 0;
    for (int i = 0; i < n; i++) {
        e_cond_sum += conditional_expectation_x(y_sample[i], fxy, fy);
    }
    double e_cond_avg = e_cond_sum / n;
    return fabs(ex_sample - e_cond_avg);
}

// =============================================================
// E. ОСНОВЫ МАРКОВСКИХ ЦЕПЕЙ
// =============================================================

// --- E.1. Умножение матрицы переходов ---
// result = P^n (n-кратное умножение).
// O(d^3 * log n) через быстрое возведение в степень.
vector<vector<double>> matrix_power(vector<vector<double>> P, int n) {
    int d = (int)P.size();
    // единичная матрица
    vector<vector<double>> result(d, vector<double>(d, 0.0));
    for (int i = 0; i < d; i++) result[i][i] = 1.0;
    while (n > 0) {
        if (n & 1) {
            vector<vector<double>> tmp(d, vector<double>(d, 0.0));
            for (int i = 0; i < d; i++)
                for (int j = 0; j < d; j++)
                    for (int k = 0; k < d; k++)
                        tmp[i][j] += result[i][k] * P[k][j];
            result = tmp;
        }
        vector<vector<double>> tmp(d, vector<double>(d, 0.0));
        for (int i = 0; i < d; i++)
            for (int j = 0; j < d; j++)
                for (int k = 0; k < d; k++)
                    tmp[i][j] += P[i][k] * P[k][j];
        P = tmp;
        n >>= 1;
    }
    return result;
}

// --- E.2. Распределение после n шагов ---
// pi_0 — начальное распределение (вектор длины d).
// pi_n = pi_0 * P^n.
vector<double> markov_distribution(const vector<double>& pi0,
                                   const vector<vector<double>>& P,
                                   int n) {
    auto Pn = matrix_power(P, n);
    int d = (int)pi0.size();
    vector<double> result(d, 0.0);
    for (int j = 0; j < d; j++)
        for (int i = 0; i < d; i++)
            result[j] += pi0[i] * Pn[i][j];
    return result;
}

// --- E.3. Стационарное распределение (итеративно) ---
// Метод итераций: pi_{k+1} = pi_k * P.
// Останавливаемся когда ||pi_{k+1} - pi_k|| < eps.
// O(d^2 * iters) время.
vector<double> stationary_distribution(vector<vector<double>> P,
                                       double eps = 1e-12,
                                       int max_iters = 10000) {
    int d = (int)P.size();
    vector<double> pi(d, 1.0 / d);  // равномерное начальное
    for (int iter = 0; iter < max_iters; iter++) {
        vector<double> pi_new(d, 0.0);
        for (int j = 0; j < d; j++)
            for (int i = 0; i < d; i++)
                pi_new[j] += pi[i] * P[i][j];
        double diff = 0;
        for (int i = 0; i < d; i++)
            diff += fabs(pi_new[i] - pi[i]);
        pi = pi_new;
        if (diff < eps) break;
    }
    return pi;
}

// --- E.4. Проверка: является ли P матрицей переходов ---
// Проверяем: P[i][j] >= 0, sum_j P[i][j] = 1 для всех i.
bool is_transition_matrix(const vector<vector<double>>& P) {
    int d = (int)P.size();
    for (int i = 0; i < d; i++) {
        double row_sum = 0;
        for (int j = 0; j < d; j++) {
            if (P[i][j] < -1e-12) return false;
            row_sum += P[i][j];
        }
        if (fabs(row_sum - 1.0) > 1e-9) return false;
    }
    return true;
}

// --- E.5. Цепь на графе: стационарное распределение ---
// P[i][j] = 1/deg(i) для смежных i,j.
// pi[i] = deg(i) / (2*|E|).
vector<double> graph_chain_stationary(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    int total_deg = 0;
    vector<int> deg(n);
    for (int i = 0; i < n; i++) {
        deg[i] = (int)adj[i].size();
        total_deg += deg[i];
    }
    vector<double> pi(n);
    for (int i = 0; i < n; i++)
        pi[i] = (double)deg[i] / total_deg;
    return pi;
}

// --- E.6. Матрица переходов для графа ---
vector<vector<double>> graph_chain_matrix(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<vector<double>> P(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        int d = (int)adj[i].size();
        if (d > 0)
            for (int j : adj[i])
                P[i][j] = 1.0 / d;
    }
    return P;
}

}; // struct MultivariateDistributions

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_E_MAIN
int main() {
    MultivariateDistributions mv;
    srand(42);

    cout << "=== A. МНОГОМЕРНЫЕ ФУНКЦИИ РАСПРЕДЕЛЕНИЯ ===" << endl;

    cout << "--- Совместная плотность N(0,0,1,1,0.5) ---" << endl;
    auto bvn = [&](double x, double y) {
        return mv.bivariate_normal_pdf(x, y, 0, 0, 1, 1, 0.5);
    };
    cout << "f(0,0) = " << bvn(0, 0) << endl;
    cout << "f(1,1) = " << bvn(1, 1) << " (должна быть > f(0,0) из-за положительной корреляции)" << endl;

    cout << "\n--- Маргинальная плотность X ---" << endl;
    // Для bivariate_normal(0,0,1,1,0.5) маргинальная X ~ N(0,1)
    // Численно: integrate f(x,y) dy
    double x_val = 0.0;
    int n_steps = 500;
    double y_lo = -10, y_hi = 10;
    double h = (y_hi - y_lo) / n_steps;
    double sum_marg = 0;
    for (int i = 0; i < n_steps; i++) {
        double y = y_lo + (i + 0.5) * h;
        sum_marg += mv.bivariate_normal_pdf(x_val, y, 0, 0, 1, 1, 0.5);
    }
    cout << "f_X(0) ~ " << sum_marg * h << " (ожидаем ~0.399)" << endl;

    cout << "\n=== B. НЕЗАВИСИМОСТЬ ===" << endl;

    cout << "--- Независимые N(0,1) ---" << endl;
    int n = 5000;
    vector<double> x_ind(n), y_ind(n);
    for (int i = 0; i < n; i++) {
        auto [z1, z2] = mv.box_muller();
        x_ind[i] = z1;
        y_ind[i] = z2;
    }
    vector<vector<double>> ind_samples = {x_ind, y_ind};
    cout << "Max |corr| = " << mv.check_pairwise_independence(ind_samples)
         << " (ожидаем ~0)" << endl;

    cout << "\n--- Зависимые X и Y=X^2 ---" << endl;
    vector<double> x_dep(n), y_dep(n);
    for (int i = 0; i < n; i++) {
        auto [z1, z2] = mv.box_muller();
        x_dep[i] = z1;
        y_dep[i] = z1 * z1;
    }
    vector<vector<double>> dep_samples = {x_dep, y_dep};
    cout << "Max |corr| = " << mv.check_pairwise_independence(dep_samples)
         << " (ожидаем > 0, corr(X,X^2) ≠ 0)" << endl;

    cout << "\n--- Проверка факторизации ---" << endl;
    // Для независимых N(0,1): f(x,y) = fX(x)*fY(y) => ln f = ln fX + ln fY
    // Проверяем через E[ln f(X,Y) - ln fX(X) - ln fY(Y)] ~ 0
    double sse = 0;
    for (int i = 0; i < n; i++) {
        double ln_fxy = log(mv.bivariate_normal_pdf(x_ind[i], y_ind[i], 0, 0, 1, 1, 0.0) + 1e-300);
        double ln_fx = log(exp(-x_ind[i]*x_ind[i]/2.0)/sqrt(2*M_PI) + 1e-300);
        double ln_fy = log(exp(-y_ind[i]*y_ind[i]/2.0)/sqrt(2*M_PI) + 1e-300);
        double diff = ln_fxy - ln_fx - ln_fy;
        sse += diff * diff;
    }
    double factor_err = sqrt(sse / n);
    cout << "ln f(x,y) - ln fX(x) - ln fY(y) RMS = " << factor_err
         << " (ожидаем ~0 для независимых)" << endl;

    cout << "\n=== C. СВЁРТКИ ===" << endl;

    cout << "--- N(0,1) + N(0,1) = N(0,sqrt(2)) ---" << endl;
    // N(0,1)+N(0,1) ~ N(0,2), pdf = N(0,sqrt(2))
    double sum_at_0 = exp(0) / (sqrt(2.0*M_PI) * sqrt(2.0));
    cout << "f_sum(0) = " << sum_at_0
         << " N(0,sqrt2) pdf(0) = " << exp(0) / (sqrt(2.0 * M_PI) * sqrt(2.0))
         << endl;

    cout << "\n--- Численная свёртка Exp(1)*Exp(1) vs Gamma(2,1) ---" << endl;
    auto exp1 = [](double x) -> double {
        return (x < 0) ? 0.0 : exp(-x);
    };
    auto erlang2 = [](double x) -> double {
        return (x < 0) ? 0.0 : x * exp(-x);
    };
    double conv_val = mv.convolution(1.0, exp1, exp1, 0, 10, 2000);
    double erlang_val = erlang2(1.0);
    cout << "conv(Exp,Exp)(1) = " << conv_val
         << " Gamma(2,1)(1) = " << erlang_val << endl;

    cout << "\n=== D. УСЛОВНЫЕ РАСПРЕДЕЛЕНИЯ ===" << endl;

    cout << "--- E[X|Y=y] для совместного нормального ---" << endl;
    // Для N(0,0,1,1,rho): E[X|Y=y] = rho * y
    double rho = 0.7;
    // Используем прямые вычисления вместо функциональных указателей
    for (double y : {-1.0, 0.0, 1.0, 2.0}) {
        // E[X|Y=y] = rho * y для bivariate normal
        double e_cond_exact = rho * y;
        cout << "E[X|Y=" << y << "] = " << e_cond_exact
             << " (ожидаем rho*y = " << rho * y << ")" << endl;
    }

    cout << "\n--- Проверка E[X] = E[E[X|Y]] ---" << endl;
    vector<double> x_samp(n), y_samp(n);
    for (int i = 0; i < n; i++) {
        auto [z1, z2] = mv.box_muller();
        x_samp[i] = rho * z1 + sqrt(1.0 - rho * rho) * z2;
        y_samp[i] = z1;
    }
    // E[X] = 0, E[E[X|Y]] = E[rho*Y] = rho*E[Y] = 0
    double ex_sum = 0;
    for (double v : x_samp) ex_sum += v;
    double ex_sample = ex_sum / n;
    double e_cond_avg = 0;
    for (int i = 0; i < n; i++) e_cond_avg += rho * y_samp[i];
    e_cond_avg /= n;
    double iter_err = fabs(ex_sample - e_cond_avg);
    cout << "|E[X] - E[E[X|Y]]| = " << iter_err << endl;

    cout << "\n=== E. МАРКОВСКИЕ ЦЕПИ ===" << endl;

    cout << "--- Матрица переходов ---" << endl;
    vector<vector<double>> P = {
        {0.7, 0.3, 0.0},
        {0.2, 0.5, 0.3},
        {0.0, 0.4, 0.6}
    };
    cout << "Является матрицей переходов: " << mv.is_transition_matrix(P) << endl;

    cout << "\n--- Стационарное распределение ---" << endl;
    auto pi_stat = mv.stationary_distribution(P);
    cout << "pi = [";
    for (int i = 0; i < (int)pi_stat.size(); i++) {
        if (i > 0) cout << ", ";
        cout << pi_stat[i];
    }
    cout << "]" << endl;
    // Проверка: pi * P = pi
    cout << "pi * P = [";
    for (int j = 0; j < 3; j++) {
        double val = 0;
        for (int i = 0; i < 3; i++) val += pi_stat[i] * P[i][j];
        if (j > 0) cout << ", ";
        cout << val;
    }
    cout << "] (должно совпадать с pi)" << endl;

    cout << "\n--- Распределение после n шагов ---" << endl;
    vector<double> pi0 = {1.0, 0.0, 0.0};
    for (int t : {1, 5, 10, 50, 100}) {
        auto pit = mv.markov_distribution(pi0, P, t);
        cout << "t=" << t << ": [";
        for (int i = 0; i < 3; i++) {
            if (i > 0) cout << ", ";
            cout << pit[i];
        }
        cout << "]" << endl;
    }

    cout << "\n--- Цепь на графе: треугольник + хвост ---" << endl;
    // 0-1-2 треугольник, 2-3 хвост
    vector<vector<int>> adj = {{1, 2}, {0, 2}, {0, 1, 3}, {2}};
    auto P_graph = mv.graph_chain_matrix(adj);
    auto pi_graph = mv.graph_chain_stationary(adj);
    cout << "pi = [";
    for (int i = 0; i < 4; i++) {
        if (i > 0) cout << ", ";
        cout << pi_graph[i];
    }
    cout << "]" << endl;
    cout << "Проверка (degree/2|E|): [";
    int total_deg = 0;
    for (int i = 0; i < 4; i++) total_deg += (int)adj[i].size();
    for (int i = 0; i < 4; i++) {
        if (i > 0) cout << ", ";
        cout << (double)adj[i].size() / total_deg;
    }
    cout << "]" << endl;

    return 0;
}
#endif

#endif // PROB_AND_STATS_E_CPP
