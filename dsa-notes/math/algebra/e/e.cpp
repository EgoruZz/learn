#ifndef ALGEBRA_E_CPP
#define ALGEBRA_E_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// V. КОРНИ МНОГОЧЛЕНОВ
// =============================================================
// Структура md: A. Основная теорема алгебры
//               → B. Теорема Виета и формулы Ньютона
//               → C. Границы и распределение корней
//
// Roots наследует Factorization (d.cpp). Переиспользует:
//   * horner (a.cpp, B.5) — вычисление P(a);
//   * derivative (a.cpp, B.6) — производная;
//   * divide/mod (b.cpp, A.2) — деление;
//   * is_root, root_multiplicity (b.cpp, B.5–B.6);
//   * gcd_classic (c.cpp, B.4) — НОД;
//   * normalize_gcd (c.cpp, A.1) — нормировка.

#include "../d/d.cpp"

struct Roots : Factorization {

// =============================================================
// B. ТЕОРЕМА ВИЕТА И ФОРМУЛЫ НЬЮТОНА
// =============================================================

// --- B.3. Формулы Виета: вычисление элементарных симметрических ---
// Для P(x) = aₙxⁿ + ... + a₀: eₖ = (−1)ᵏ a_{n−k}/aₙ
vector<long long> viete(const Polynomial& P) {
    int n = P.degree();
    if (n < 0) return {};
    long long an = P[n];
    vector<long long> e(n + 1, 0);
    for (int k = 0; k <= n; k++)
        e[k] = ((k % 2 == 0 ? 1 : -1) * P[n - k]) / an;
    return e; // e[0]=1, e[1], ..., e[n]
}

// --- B.5. Степенные суммы через формулы Ньютона ---
// pₖ = ∑ zᵢᵏ, вычисляется по рекуррентности:
// pₖ = e₁pₖ₋₁ − e₂pₖ₋₂ + ... + (−1)^{k−1} keₖ
vector<long long> newton_power_sums(const Polynomial& P, int max_k) {
    auto e = viete(P);
    int n = P.degree();
    if (n < 0) return {};
    vector<long long> p(max_k + 1, 0);
    // p₀ = n (сумма единиц)
    p[0] = n;
    for (int k = 1; k <= max_k; k++) {
        long long sum = 0;
        for (int i = 1; i <= min(k, n); i++) {
            long long term = e[i] * p[k - i];
            if (i % 2 == 0) sum -= term;
            else sum += term;
        }
        if (k <= n) sum += (k % 2 == 0 ? 1 : -1) * k * e[k];
        p[k] = sum;
    }
    return p;
}

// =============================================================
// C. ГРАНИЦЫ И РАСПРЕДЕЛЕНИЕ КОРНЕЙ
// =============================================================

// --- C.6. Граница Коши: |zᵢ| ≤ 1 + max|aₖ/aₙ| ---
long long cauchy_bound(const Polynomial& P) {
    int n = P.degree();
    if (n <= 0) return 0;
    long long an = abs(P[n]);
    long long max_ratio = 0;
    for (int i = 0; i < n; i++)
        max_ratio = max(max_ratio, abs(P[i]) / an + 1);
    return max_ratio + 1;
}

// --- C.6. Уточнение Фуке ---
long long fuca_bound(const Polynomial& P) {
    int n = P.degree();
    if (n <= 0) return 0;
    long long an = abs(P[n]);
    long long sum = 0;
    for (int i = 0; i < n; i++)
        sum += abs(P[i]) / an;
    return max(1LL, sum) + 1;
}

// --- C.7. Правило знаков Декарта ---
// Число смен знаков в векторе коэффициентов.
int sign_changes(const vector<long long>& coeffs) {
    int changes = 0, last = 0;
    for (long long c : coeffs) {
        if (c == 0) continue;
        int s = (c > 0) ? 1 : -1;
        if (last != 0 && s != last) changes++;
        last = s;
    }
    return changes;
}

// Количество положительных корней (по Декарту, с точностью до чётности)
int descartes_positive_roots(const Polynomial& P) {
    return sign_changes(P.coeffs);
}

// --- C.8. Теорема Штурма ---
// Построение цепочки: P₀ = P, P₁ = P', Pᵢ₊₁ = −(Pᵢ₋₁ mod Pᵢ)
// Используем pseudo_mod для корректного целочисленного деления.
vector<Polynomial> sturm_chain(const Polynomial& P) {
    vector<Polynomial> chain;
    chain.push_back(P);
    chain.push_back(derivative(P));
    while (chain.back().degree() > 0) {
        Polynomial prev = chain[chain.size() - 2];
        Polynomial cur = chain.back();
        Polynomial r = pseudo_mod(prev, cur);
        // Умножаем на −1 (как в расширенном Евклиде)
        for (auto& c : r.coeffs) c = -c;
        r.normalize();
        chain.push_back(r);
    }
    return chain;
}

// Подсчёт числа корней на интервале (a, b]
int sturm_count(const vector<Polynomial>& chain, long long x) {
    int changes = 0, last = 0;
    for (const auto& P : chain) {
        long long val = horner(P, x);
        if (val == 0) continue;
        int s = (val > 0) ? 1 : -1;
        if (last != 0 && s != last) changes++;
        last = s;
    }
    return changes;
}

// Число вещественных корней на (a, b]
int sturm_roots_in_interval(const Polynomial& P, long long a, long long b) {
    auto chain = sturm_chain(P);
    return sturm_count(chain, a) - sturm_count(chain, b);
}

// --- C.9. Теорема Бюдана-Фурье (упрощённая) ---
// Число корней на (a, b] ≤ число смен знаков в P, P', ..., P⁽ⁿ⁾
int budan_bound(const Polynomial& P, long long a, long long b) {
    vector<Polynomial> derivatives;
    derivatives.push_back(P);
    Polynomial cur = P;
    for (int i = 1; i <= P.degree(); i++) {
        cur = derivative(cur);
        derivatives.push_back(cur);
    }

    auto count_signs = [&](long long x) {
        int changes = 0, last = 0;
        for (const auto& Q : derivatives) {
            long long val = horner(Q, x);
            if (val == 0) continue;
            int s = (val > 0) ? 1 : -1;
            if (last != 0 && s != last) changes++;
            last = s;
        }
        return changes;
    };

    return count_signs(a) - count_signs(b);
}

}; // struct Roots

#endif // ALGEBRA_E_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_E_MAIN
int main() {
    using Poly = Roots::Polynomial;
    Roots roots;

    // Пример B.3: Виета для x² − 5x + 6 = (x−2)(x−3)
    Poly P({6, -5, 1});
    auto e = roots.viete(P);
    cout << "e₁ = " << e[1] << " (ожидаем 5)\n";
    cout << "e₂ = " << e[2] << " (ожидаем 6)\n";

    // Пример B.5: степенные суммы
    auto p = roots.newton_power_sums(P, 4);
    cout << "p₁ = " << p[1] << " (ожидаем 5)\n";
    cout << "p₂ = " << p[2] << " (ожидаем 13)\n";

    // Пример C.7: Декарт
    cout << "positive roots bound (Dekart) = " << roots.descartes_positive_roots(P) << "\n";

    // Пример C.8: Штурм
    cout << "roots on (-10, 10) = " << roots.sturm_roots_in_interval(P, -10, 10) << "\n";
    cout << "roots on (0, 5) = " << roots.sturm_roots_in_interval(P, 0, 5) << "\n";

    return 0;
}
#endif // ALGEBRA_E_MAIN
