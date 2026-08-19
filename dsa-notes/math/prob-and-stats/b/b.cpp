#ifndef PROB_AND_STATS_B_CPP
#define PROB_AND_STATS_B_CPP

#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
using namespace std;

// =============================================================
// B. АКСИОМАТИЧЕСКОЕ ПОСТРОЕНИЕ ТЕОРИИ ВЕРОЯТНОСТЕЙ (КОЛМОГОРОВ)
// =============================================================
// Структура md: A. Вероятностное пространство
//               → B. σ-алгебры и алгебры множеств
//               → C. Вероятностная мера и свойства
//               → D. Системы множеств и продолжение меры
//               → E. Непрерывность вероятностной меры
//
// MeasureTheoreticProb — наследует ProbabilityBasics (a.cpp).
// Расширяет классическое определение аксиоматическим подходом:
// вводит тройку (Omega, F, P), σ-алгебры, аксиомы меры,
// свойства вероятностной меры, π-системы, λ-системы,
// теорему Каратаодори о продолжении меры.

#ifndef INSIDE_PROB_AND_STATS_B
#define INSIDE_PROB_AND_STATS_B
#include "../a/a.cpp"
#undef INSIDE_PROB_AND_STATS_B
#endif

struct MeasureTheoreticProb : ProbabilityBasics {

// =============================================================
// A. ВЕРОЯТНОСТНОЕ ПРОСТРАНСТВО
// =============================================================

// --- A.1. Вероятностное пространство (конечный случай) ---
// omega_size — |Omega|, probs[i] = P({omega_i}).
// Условие: probs[i] >= 0, sum probs = 1.
// Возвращает true, если задано корректно.
bool validate_finite_space(int omega_size, const vector<double>& probs) {
    if ((int)probs.size() != omega_size) return false;
    double sum = 0.0;
    for (int i = 0; i < omega_size; i++) {
        if (probs[i] < 0.0 || probs[i] > 1.0) return false;
        sum += probs[i];
    }
    return fabs(sum - 1.0) < 1e-9;
}

// --- A.2. Вероятность события на конечном пространстве ---
// probs[i] = P({omega_i}), event — битовая маска (бит i = 1 <=> omega_i ∈ event).
// P(event) = sum_{i ∈ event} probs[i].
// O(n) время.
double event_probability(const vector<double>& probs, long long event) {
    double result = 0.0;
    for (int i = 0; i < (int)probs.size(); i++)
        if (event & (1LL << i))
            result += probs[i];
    return result;
}

// --- A.3. Объединение событий (конечный случай) ---
// events — вектор битовых масок; union_events — битовое OR.
// P(A_1 ∪ ... ∪ A_m) = P(union_events).
// O(n) время.
double union_probability(const vector<double>& probs,
                         const vector<long long>& events) {
    long long union_mask = 0;
    for (long long e : events)
        union_mask |= e;
    return event_probability(probs, union_mask);
}

// --- A.4. Пересечение событий (конечный случай) ---
// P(A ∩ B) через битовое AND.
// O(n) время.
double intersection_probability(const vector<double>& probs,
                                long long event_a, long long event_b) {
    return event_probability(probs, event_a & event_b);
}

// =============================================================
// B. σ-АЛГЕБРЫ И АЛГЕБРЫ МНОЖЕСТВ
// =============================================================

// --- B.1. Генерация σ-алгебры из системы множеств (конечный Omega) ---
// Omega = {0, 1, ..., n-1}, generators — вектор подмножеств (битовые маски).
// Возвращает σ-алгебру: замыкаем относительно дополнений и счётных объединений.
// Для конечного Omega: счётное объединение = конечное объединение.
// O(|F| * n) время в худшем случае.
vector<long long> generate_sigma_algebra(int n,
                                         const vector<long long>& generators) {
    set<long long> sigma;
    // Добавляем Omega и пустое множество
    long long full = (1LL << n) - 1;
    sigma.insert(full);
    sigma.insert(0LL);
    // Добавляем генераторы и их дополнения
    for (long long g : generators) {
        sigma.insert(g);
        sigma.insert(full ^ g);  // дополнение
    }
    // Замыкаем: повторяем пока не стабилизируется
    bool changed = true;
    while (changed) {
        changed = false;
        vector<long long> current(sigma.begin(), sigma.end());
        int sz = (int)current.size();
        for (int i = 0; i < sz; i++) {
            for (int j = 0; j < sz; j++) {
                long long u = current[i] | current[j];  // объединение
                long long inter = current[i] & current[j];  // пересечение
                if (!sigma.count(u)) { sigma.insert(u); changed = true; }
                if (!sigma.count(inter)) { sigma.insert(inter); changed = true; }
            }
        }
    }
    return vector<long long>(sigma.begin(), sigma.end());
}

// --- B.2. Проверка: является ли система σ-алгеброй ---
// F — вектор подмножеств (битовые маски), n — |Omega|.
// Проверяем: Omega ∈ F, замкнутость относительно дополнений и объединений.
// O(|F|^2 * n) время.
bool is_sigma_algebra(int n, const vector<long long>& F) {
    long long full = (1LL << n) - 1;
    set<long long> fs(F.begin(), F.end());
    // Omega ∈ F?
    if (!fs.count(full)) return false;
    // Замкнутость относительно дополнений
    for (long long a : F) {
        long long comp = full ^ a;
        if (!fs.count(comp)) return false;
    }
    // Замкнутость относительно объединений
    for (int i = 0; i < (int)F.size(); i++)
        for (int j = 0; j < (int)F.size(); j++) {
            long long u = F[i] | F[j];
            if (!fs.count(u)) return false;
        }
    return true;
}

// --- B.3. Проверка: является ли система π-системой ---
// Замкнутость относительно пересечений.
// O(|F|^2) время.
bool is_pi_system(const vector<long long>& F) {
    set<long long> fs(F.begin(), F.end());
    for (int i = 0; i < (int)F.size(); i++)
        for (int j = 0; j < (int)F.size(); j++) {
            long long inter = F[i] & F[j];
            if (!fs.count(inter)) return false;
        }
    return true;
}

// --- B.4. Проверка: является ли система λ-системой ---
// 1. Omega ∈ L, 2. замкнутость относительно дополнений,
// 3. замкнутость относительно несовместных объединений.
// O(|L|^2) время.
bool is_lambda_system(int n, const vector<long long>& L) {
    long long full = (1LL << n) - 1;
    set<long long> ls(L.begin(), L.end());
    if (!ls.count(full)) return false;
    for (long long a : L) {
        long long comp = full ^ a;
        if (!ls.count(comp)) return false;
    }
    for (int i = 0; i < (int)L.size(); i++)
        for (int j = i + 1; j < (int)L.size(); j++) {
            if (L[i] & L[j]) continue;  // не несовместны
            long long u = L[i] | L[j];
            if (!ls.count(u)) return false;
        }
    return true;
}

// =============================================================
// C. ВЕРОЯТНОСТНАЯ МЕРА И ЕЁ СВОЙСТВА
// =============================================================

// --- C.1. Проверка аксиом Колмогорова ---
// omega_size — |Omega|, probs — P({omega_i}).
// Возвращает номер нарушенной аксиомы (0 = все ок):
//   1 — неотрицательность, 2 — нормированность, 3 — sigma-аддитивность.
int verify_kolmogorov_axioms(int omega_size, const vector<double>& probs) {
    if ((int)probs.size() != omega_size) return 1;
    double sum = 0.0;
    for (int i = 0; i < omega_size; i++) {
        if (probs[i] < -1e-12) return 1;
        sum += probs[i];
    }
    if (fabs(sum - 1.0) > 1e-9) return 2;
    return 0;
}

// --- C.2. Проверка монотонности ---
// P(A) <= P(B) при A ⊆ B.
bool check_monotonicity(const vector<double>& probs,
                         long long event_a, long long event_b) {
    if (event_a & ~event_b) return false;
    return event_probability(probs, event_a) <=
           event_probability(probs, event_b) + 1e-12;
}

// --- C.3. Проверка субаддитивности (Бонферрони) ---
// P(A ∪ B) <= P(A) + P(B).
bool check_subadditivity(const vector<double>& probs,
                          long long event_a, long long event_b) {
    double p_union = event_probability(probs, event_a | event_b);
    double p_sum = event_probability(probs, event_a) +
                   event_probability(probs, event_b);
    return p_union <= p_sum + 1e-12;
}

// --- C.4. Формула включений-исключений (конечный случай) ---
// events — вектор подмножеств (битовые маски).
// O(2^m * n) время.
double inclusion_exclusion(const vector<double>& probs,
                           const vector<long long>& events) {
    int m = (int)events.size();
    double result = 0.0;
    for (int mask = 1; mask < (1 << m); mask++) {
        long long intersection = (1LL << (int)probs.size()) - 1;
        int bits = 0;
        for (int i = 0; i < m; i++) {
            if (mask & (1 << i)) {
                intersection &= events[i];
                bits++;
            }
        }
        double p_inter = event_probability(probs, intersection);
        result += (bits % 2 == 1) ? p_inter : -p_inter;
    }
    return result;
}

// =============================================================
// D. НЕПРЕРЫВНОСТЬ ВЕРОЯТНОСТНОЙ МЕРЫ
// =============================================================

// --- D.1. Симуляция возрастающей последовательности ---
// approximating — вектор приближений A_1 ⊆ A_2 ⊆ ... ⊆ A (битовые маски).
// limit_event — A = ∪_n A_n.
// Возвращает |P(A_n) - P(A)| для каждого n.
vector<double> ascending_continuity(const vector<double>& probs,
                                    const vector<long long>& approximating,
                                    long long limit_event) {
    double p_limit = event_probability(probs, limit_event);
    vector<double> errors;
    long long current = 0;
    for (long long a : approximating) {
        current |= a;
        double p_n = event_probability(probs, current);
        errors.push_back(fabs(p_n - p_limit));
    }
    return errors;
}

// --- D.2. Симуляция убывающей последовательности ---
// approximating — вектор приближений A_1 ⊇ A_2 ⊇ ... ⊇ A.
// limit_event — A = ∩_n A_n.
vector<double> descending_continuity(const vector<double>& probs,
                                     const vector<long long>& approximating,
                                     long long limit_event) {
    double p_limit = event_probability(probs, limit_event);
    vector<double> errors;
    long long current = (1LL << (int)probs.size()) - 1;
    for (long long a : approximating) {
        current &= a;
        double p_n = event_probability(probs, current);
        errors.push_back(fabs(p_n - p_limit));
    }
    return errors;
}

}; // struct MeasureTheoreticProb

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PROB_AND_STATS_B_MAIN
int main() {
    MeasureTheoreticProb mtp;

    cout << "=== A. ВЕРОЯТНОСТНОЕ ПРОСТРАНСТВО ===" << endl;
    // Omega = {0,1,2,3}, P({i}) = 1/4 для каждого i
    vector<double> probs4 = {0.25, 0.25, 0.25, 0.25};
    cout << "P({0,1,2,3}) корректно: "
         << mtp.validate_finite_space(4, probs4) << endl;
    // A = {0,1}, B = {1,2}
    long long A = 0b0011, B = 0b0110;
    cout << "P(A) = " << mtp.event_probability(probs4, A) << endl;
    cout << "P(A ∪ B) = " << mtp.union_probability(probs4, {A, B}) << endl;
    cout << "P(A ∩ B) = " << mtp.intersection_probability(probs4, A, B) << endl;

    cout << "\n=== B. σ-АЛГЕБРЫ ===" << endl;
    // Omega = {0,1}, генератор = {{0}}
    vector<long long> gens = {0b01};
    auto sigma = mtp.generate_sigma_algebra(2, gens);
    cout << "σ({{0}}) на {0,1}: ";
    for (long long s : sigma) cout << s << " ";
    cout << endl;
    cout << "Это σ-алгебра: " << mtp.is_sigma_algebra(2, sigma) << endl;
    cout << "Это π-система: " << mtp.is_pi_system(sigma) << endl;
    cout << "Это λ-система: " << mtp.is_lambda_system(2, sigma) << endl;

    cout << "\n=== C. СВОЙСТВА МЕРЫ ===" << endl;
    cout << "Аksiомы (1/4,1/4,1/4,1/4): "
         << mtp.verify_kolmogorov_axioms(4, probs4) << " (0=ok)" << endl;
    cout << "Монотонность {0} ⊆ {0,1}: "
         << mtp.check_monotonicity(probs4, 0b01, 0b11) << endl;
    cout << "Субаддитивность: "
         << mtp.check_subadditivity(probs4, 0b01, 0b10) << endl;
    // Включения-исключения для A={0}, B={1}, C={2}
    vector<long long> events3 = {0b001, 0b010, 0b100};
    cout << "P(A ∪ B ∪ C) через ВИ: "
         << mtp.inclusion_exclusion(probs4, events3) << endl;

    cout << "\n=== D. НЕПРЕРЫВНОСТЬ МЕРЫ ===" << endl;
    // Возрастающая: A_1={0}, A_2={0,1}, A_3={0,1,2}, A={0,1,2,3}
    vector<long long> ascending = {0b0001, 0b0011, 0b0111};
    long long limit_asc = 0b1111;
    auto err_asc = mtp.ascending_continuity(probs4, ascending, limit_asc);
    cout << "Возрастающая: ошибки = ";
    for (double e : err_asc) cout << e << " ";
    cout << endl;
    // Убывающая: A_1={0,1,2,3}, A_2={0,1,2}, A_3={0,1}, A={0}
    vector<long long> descending = {0b1111, 0b0111, 0b0011};
    long long limit_desc = 0b0001;
    auto err_desc = mtp.descending_continuity(probs4, descending, limit_desc);
    cout << "Убывающая: ошибки = ";
    for (double e : err_desc) cout << e << " ";
    cout << endl;

    return 0;
}
#endif

#endif // PROB_AND_STATS_B_CPP
