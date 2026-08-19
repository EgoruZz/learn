#ifndef PROB_AND_STATS_A_CPP
#define PROB_AND_STATS_A_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

// =============================================================
// A. ОСНОВЫ ТЕОРИИ ВЕРОЯТНОСТЕЙ (КЛАССИЧЕСКАЯ ПАРАДИГМА)
// =============================================================
// Структура md: A. Классическое определение вероятности
//               → B. Задача о днях рождения
//               → C. Геометрические вероятности
//               → D. Задача о монетах и симметрия
//               → E. Условная вероятность и формула Байеса
//               → F. Схема повторных испытаний (Бернулли, Лапласа, Пуассона)
//
// ProbabilityBasics — базовый класс всей ветки prob-and-stats.
// Вводит фундаментальные понятия: вероятность, условная вероятность,
// формулу Байеса, биномиальное распределение, приближения Лапласа
// и Пуассона. Все последующие разделы (B–I) строятся на этих примитивах.

// =============================================================
// A. КЛАССИЧЕСКОЕ ОПРЕДЕЛЕНИЕ ВЕРОЯТНОСТИ
// =============================================================

struct ProbabilityBasics {

// --- A.1. Вероятность равновозможных исходов ---
// |A| — число благоприятных исходов, |Omega| — общее число исходов.
// Все исходы равновозможны и Omega конечно.
// O(1) время.
double classical_probability(long long favorable, long long total) {
    return (double)favorable / (double)total;
}

// --- A.2. Вероятность дополнения ---
// P(not A) = 1 - P(A).
// O(1) время.
double complement_probability(double p_a) {
    return 1.0 - p_a;
}

// --- A.3. Формула включений-исключений для двух событий ---
// P(A or B) = P(A) + P(B) - P(A and B).
// O(1) время.
double inclusion_exclusion_2(double p_a, double p_b, double p_a_and_b) {
    return p_a + p_b - p_a_and_b;
}

// --- A.4. Вероятность объединения несовместных событий ---
// events — вектор вероятностей несовместных событий P(A_i).
// P(A_1 or ... or or A_n) = sum P(A_i).
// O(n) время.
double union_disjoint(const vector<double>& events) {
    double result = 0.0;
    for (double p : events)
        result += p;
    return result;
}

// =============================================================
// B. ЗАДАЧА О ДНЯХ РОЖДЕНИЯ
// =============================================================

// --- B.1. Вероятность совпадения дней рождения ---
// n — число людей, k — число дней в году (n <= k).
// P(хотя бы одно совпадение) = 1 - k! / ((k-n)! * k^n).
// Вычисляем через произведение: prod_{i=0}^{n-1} (k-i)/k.
// O(n) время, O(1) память.
double birthday_collision(int n, int k) {
    if (n > k) return 1.0;
    if (n <= 1) return 0.0;
    double p_no_collision = 1.0;
    for (int i = 0; i < n; i++)
        p_no_collision *= (double)(k - i) / (double)k;
    return 1.0 - p_no_collision;
}

// --- B.2. Асимптотика при n << k ---
// P ~ 1 - exp(-n*(n-1)/(2k)).
// O(1) время.
double birthday_approx(int n, int k) {
    return 1.0 - exp(-(double)n * (n - 1) / (2.0 * k));
}

// --- B.3. Минимальное число людей для заданной вероятности ---
// Наименьшее n такое, что P(совпадение | n, k) >= target.
// Линейный поиск; O(k) в худшем случае, но реально O(sqrt(k)).
int birthday_min_n(int k, double target) {
    double p_no_collision = 1.0;
    for (int n = 1; n <= k; n++) {
        p_no_collision *= (double)(k - n + 1) / (double)k;
        if (1.0 - p_no_collision >= target)
            return n;
    }
    return k + 1;
}

// =============================================================
// C. ГЕОМЕТРИЧЕСКИЕ ВЕРОЯТНОСТИ
// =============================================================

// --- C.1. Задача о встрече ---
// Два человека приходят в интервале [0, T]; каждый ждёт не дольше t минут.
// P(встреча) = 1 - (1 - t/T)^2 при t <= T.
// O(1) время.
double meeting_probability(double t, double T) {
    if (t >= T) return 1.0;
    if (t <= 0) return 0.0;
    double ratio = t / T;
    return 1.0 - (1.0 - ratio) * (1.0 - ratio);
}

// =============================================================
// D. ЗАДАЧА О МОНЕТАХ И СИММЕТРИЯ
// =============================================================

// --- D.1. Вероятность ровно k орлов в n бросках ---
// P(X = k) = C(n, k) * p^k * (1-p)^{n-k}.
// Честная монета: p = 1/2.
// O(k) время (через C_iter).
double binomial_exact(long long n, long long k, double p) {
    if (k < 0 || k > n) return 0.0;
    long long c = 1;
    k = min(k, n - k);
    for (long long i = 0; i < k; i++)
        c = c * (n - i) / (i + 1);
    double q = 1.0 - p;
    return (double)c * pow(p, k) * pow(q, n - k);
}

// --- D.2. Вероятность серии из r подряд орлов в n бросках ---
// Рекуррентное соотношение:
//   p(i) = вероятность серии длины r в первых i бросках
//   p(i) = 0, i < r
//   p(r) = p^r
//   p(i) = p(i-1) + (1 - p(i-r-1)) * p^r, i > r
// O(n) время, O(r) память (скользящее окно).
double streak_probability(int n, int r, double p) {
    if (r > n) return 0.0;
    if (r == 0) return 1.0;
    double p_r = pow(p, r);
    vector<double> dp(n + 1, 0.0);
    dp[r] = p_r;
    for (int i = r + 1; i <= n; i++)
        dp[i] = dp[i - 1] + (1.0 - dp[i - r - 1]) * p_r;
    return dp[n];
}

// =============================================================
// E. УСЛОВНАЯ ВЕРОЯТНОСТЬ И ФОРМУЛА БАЙЕСА
// =============================================================

// --- E.1. Условная вероятность ---
// P(A|B) = P(A and B) / P(B), P(B) > 0.
// O(1) время.
double conditional_probability(double p_a_and_b, double p_b) {
    if (p_b == 0.0) return 0.0;
    return p_a_and_b / p_b;
}

// --- E.2. Формула полной вероятности ---
// P(A) = sum_{j=1}^{m} P(B_j) * P(A|B_j).
// p_b[j] = P(B_j), p_a_given_b[j] = P(A|B_j).
// Условие: сумма p_b = 1, B_j попарно несовместны.
// O(m) время.
double total_probability(const vector<double>& p_b,
                         const vector<double>& p_a_given_b) {
    double result = 0.0;
    int m = (int)p_b.size();
    for (int j = 0; j < m; j++)
        result += p_b[j] * p_a_given_b[j];
    return result;
}

// --- E.3. Формула Байеса ---
// P(B_j | A) = P(A|B_j) * P(B_j) / P(A).
// Возвращает вектор апостериорных вероятностей.
// O(m) время.
vector<double> bayes(const vector<double>& p_b,
                     const vector<double>& p_a_given_b) {
    int m = (int)p_b.size();
    double p_a = total_probability(p_b, p_a_given_b);
    vector<double> posterior(m);
    for (int j = 0; j < m; j++)
        posterior[j] = (p_a > 0.0) ? (p_a_given_b[j] * p_b[j] / p_a) : 0.0;
    return posterior;
}

// --- E.4. Наивный Байесовский классификатор ---
// Классы: 0..num_classes-1.
// prior[c] = P(class=c).
// likelihood[c][i] = P(feature_i = value_i | class = c).
// features — индексы значений признаков для объекта.
// Возвращает номер наиболее вероятного класса.
// O(num_classes * num_features) время.
int naive_bayes_classifier(const vector<double>& prior,
                           const vector<vector<double>>& likelihood,
                           const vector<int>& features) {
    int num_classes = (int)prior.size();
    int best_class = 0;
    double best_score = -1e300;
    for (int c = 0; c < num_classes; c++) {
        double log_prob = log(prior[c] + 1e-300);
        for (int i = 0; i < (int)features.size(); i++)
            log_prob += log(likelihood[c][features[i]] + 1e-300);
        if (log_prob > best_score) {
            best_score = log_prob;
            best_class = c;
        }
    }
    return best_class;
}

// =============================================================
// F. СХЕМА ПОВТОРНЫХ ИСПЫТАНИЙ
// =============================================================

// --- F.1. Биномиальное распределение: P(S_n = k) ---
// P(S_n = k) = C(n, k) * p^k * (1-p)^{n-k}.
// Точная формула; O(k) время.
double binomial_pmf(long long n, long long k, double p) {
    if (k < 0 || k > n) return 0.0;
    long long c = 1;
    k = min(k, n - k);
    for (long long i = 0; i < k; i++)
        c = c * (n - i) / (i + 1);
    return (double)c * pow(p, k) * pow(1.0 - p, n - k);
}

// --- F.2. Кумулятивная функция биномиального: P(S_n <= k) ---
// Суммируем binomial_pmf от 0 до k.
// O(k) время.
double binomial_cdf(long long n, long long k, double p) {
    double result = 0.0;
    for (long long i = 0; i <= k; i++)
        result += binomial_pmf(n, i, p);
    return result;
}

// --- F.3. Нормальная функция распределения Phi(x) ---
// Phi(x) = (1/sqrt(2*pi)) * integral_{-inf}^{x} exp(-t^2/2) dt.
// Приближение: полиномиальная аппроксимация Абрамовица и Стегуна (формула 7.1.26).
// |ошибка| < 7.5e-8.
// O(1) время.
double phi(double x) {
    return 0.5 * erfc(-x / sqrt(2.0));
}

// --- F.4. Локальная теорема Лапласа ---
// P(S_n = k) ~ (1/sqrt(2*pi*n*p*q)) * exp(-(k - n*p)^2 / (2*n*p*q)).
// x = (k - n*p) / sqrt(n*p*q).
// O(1) время.
double laplace_local(long long n, long long k, double p) {
    double q = 1.0 - p;
    double mu = n * p;
    double sigma = sqrt(n * p * q);
    if (sigma == 0.0) return (k == (long long)mu) ? 1.0 : 0.0;
    double x = (k - mu) / sigma;
    return exp(-x * x / 2.0) / (sigma * sqrt(2.0 * M_PI));
}

// --- F.5. Интегральная теорема Лапласа ---
// P(a <= S_n <= b) ~ Phi((b - n*p)/sqrt(n*p*q)) - Phi((a - n*p)/sqrt(n*p*q)).
// O(1) время.
double laplace_integral(long long n, long long a, long long b, double p) {
    double q = 1.0 - p;
    double mu = n * p;
    double sigma = sqrt(n * p * q);
    if (sigma == 0.0) return (a <= (long long)mu && (long long)mu <= b) ? 1.0 : 0.0;
    return phi((b - mu) / sigma) - phi((a - mu) / sigma);
}

// --- F.6. Поправка Уisoftа (continuity correction) ---
// P(S_n <= k) ~ Phi((k + 0.5 - n*p) / sqrt(n*p*q)).
// O(1) время.
double laplace_continuity(long long n, long long k, double p) {
    double q = 1.0 - p;
    double mu = n * p;
    double sigma = sqrt(n * p * q);
    if (sigma == 0.0) return (k >= (long long)mu) ? 1.0 : 0.0;
    return phi((k + 0.5 - mu) / sigma);
}

// --- F.7. Приближение Пуассона ---
// P(S_n = k) ~ e^{-lambda} * lambda^k / k!, lambda = n*p.
// Применимо при n велико, p мало, lambda фиксировано.
// O(k) время.
double poisson_pmf(long long k, double lambda) {
    if (lambda <= 0.0) return (k == 0) ? 1.0 : 0.0;
    double log_result = -lambda + k * log(lambda);
    for (long long i = 2; i <= k; i++)
        log_result -= log((double)i);
    return exp(log_result);
}

// --- F.8. Кумулятивная функция Пуассона ---
// P(X <= k) = sum_{i=0}^{k} e^{-lambda} * lambda^i / i!.
// O(k) время.
double poisson_cdf(long long k, double lambda) {
    double result = 0.0;
    double term = exp(-lambda);
    result += term;
    for (long long i = 1; i <= k; i++) {
        term *= lambda / (double)i;
        result += term;
    }
    return result;
}

// --- F.9. Критерий выбор приближения ---
// Возвращает:
//   0 — используй точную формулу (Berndlli)
//   1 — используй Лапласа (нормальное приближение)
//   2 — используй Пуассона
int approximation_choice(long long n, double p) {
    double np = n * p;
    if (np <= 10.0 && p <= 0.05 && n >= 20)
        return 2;  // Пуассон: n велико, p мало
    if (n >= 30 && p > 0.05 && p < 0.95)
        return 1;  // Лаплас: n велико, p фиксировано
    return 0;      // Точная формула
}

};

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_A_MAIN
int main() {
    ProbabilityBasics pb;
    srand(42);

    cout << "=== A. КЛАССИЧЕСКОЕ ОПРЕДЕЛЕНИЕ ВЕРОЯТНОСТИ ===" << endl;
    cout << "P(6 на кубике) = " << pb.classical_probability(1, 6) << endl;
    cout << "P(not A) при P(A)=0.3: " << pb.complement_probability(0.3) << endl;
    cout << "P(A or B): " << pb.inclusion_exclusion_2(0.3, 0.4, 0.1) << endl;

    cout << "\n=== B. ЗАДАЧА О ДНЯХ РОЖДЕНИЯ ===" << endl;
    for (int n : {10, 23, 30, 50}) {
        double exact = pb.birthday_collision(n, 365);
        double approx = pb.birthday_approx(n, 365);
        cout << "n=" << n << ": exact=" << exact
             << " approx=" << approx << endl;
    }
    cout << "Минимум n для P>=0.5: " << pb.birthday_min_n(365, 0.5) << endl;

    cout << "\n=== C. ГЕОМЕТРИЧЕСКИЕ ВЕРОЯТНОСТИ ===" << endl;
    cout << "P(встреча, t=15, T=60): " << pb.meeting_probability(15, 60) << endl;

    cout << "\n=== D. ЗАДАЧА О МОНЕТАХ ===" << endl;
    cout << "P(ровно 5 орлов в 10 бросках): "
         << pb.binomial_exact(10, 5, 0.5) << endl;
    cout << "P(серия из 3 орлов в 10 бросках): "
         << pb.streak_probability(10, 3, 0.5) << endl;

    cout << "\n=== E. УСЛОВНАЯ ВЕРОЯТНОСТЬ И БАЙЕС ===" << endl;
    cout << "P(A|B) при P(A&B)=0.15, P(B)=0.5: "
         << pb.conditional_probability(0.15, 0.5) << endl;
    // Задача о двух фабриках
    vector<double> p_b = {0.6, 0.4};
    vector<double> p_a_given_b = {0.01, 0.05};
    auto post = pb.bayes(p_b, p_a_given_b);
    cout << "Bayes: P(фабрика 1|брак) = " << post[0]
         << ", P(фабрика 2|брак) = " << post[1] << endl;

    cout << "\n=== F. СХЕМА ПОВТОРНЫХ ИСПЫТАНИЙ ===" << endl;
    long long n = 100;
    double p = 0.3;
    cout << "B(100,0.3): E=" << n * p << " D=" << n * p * (1 - p) << endl;
    cout << "P(S_100 = 30) точное: " << pb.binomial_pmf(n, 30, p) << endl;
    cout << "P(S_100 = 30) Лаплас: " << pb.laplace_local(n, 30, p) << endl;
    cout << "P(S_100 = 30) Пуассон(l=30): " << pb.poisson_pmf(30, 30.0) << endl;
    cout << "P(25 <= S_100 <= 35) Лаплас: "
         << pb.laplace_integral(n, 25, 35, p) << endl;
    cout << "P(S_100 <= 30) с поправкой: "
         << pb.laplace_continuity(n, 30, p) << endl;
    cout << "Рекомендация: " << pb.approximation_choice(n, p) << endl;

    return 0;
}
#endif

#endif // PROB_AND_STATS_A_CPP
