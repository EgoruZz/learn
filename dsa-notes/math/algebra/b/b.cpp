#ifndef ALGEBRA_B_CPP
#define ALGEBRA_B_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// II. ДЕЛЕНИЕ С ОСТАТКОМ И ТЕОРЕМА БЕЗУ
// =============================================================
// Структура md: A. Деление с остатком
//               → B. Теорема Безу и следствия
//
// Division наследует Polynomials (a.cpp). Переиспользует:
//   * Polynomial (a.cpp, A) — базовая структура многочлена;
//   * horner (a.cpp, B.5) — подстановка для вычисления P(a);
//   * horner_extended (a.cpp, B.5) — деление на (x−a);
//   * derivative (a.cpp, B.6) — для кратности корней.
//
// Порядок методов строго соответствует порядку md (A → B).

#include "../a/a.cpp"

struct Division : Polynomials {

// =============================================================
// A. ДЕЛЕНИЕ С ОСТАТКОМ
// =============================================================

// --- A.2. Деление «уголком»: P = Q·D + R, deg R < deg D ---
// Возвращает {Q, R}.
pair<Polynomial, Polynomial> divide(const Polynomial& P, const Polynomial& D) {
    assert(D.degree() >= 0 && "Деление на нулевой многочлен");
    int n = P.degree(), m = D.degree();
    if (n < m) return {Polynomial({0}), P};

    vector<long long> q(n - m + 1, 0);
    Polynomial R(P); // копия P как начальный остаток

    for (int i = n; i >= m; i--) {
        if (R.degree() < i) continue;
        // Деление коэффициентов: если D[m] не делит R[i], получаем дробь.
        // Для целых многочленов (ℤ[x]) — точное деление только при monic D.
        // Для ℚ[x], ℝ[x], ℂ[x] — деление всегда корректно.
        long long coeff = R[i] / D[m];
        q[i - m] = coeff;
        // R -= coeff · x^{i−m} · D
        for (int j = 0; j <= m; j++)
            R.coeffs[i - m + j] -= coeff * D[j];
    }
    R.normalize();
    return {Polynomial(q), R};
}

// А.2. Остаток от деления
Polynomial mod(const Polynomial& P, const Polynomial& D) {
    return divide(P, D).second;
}

// А.2. Псевдо-остаток: lc(D)^δ · P = Q·D + R, где δ = deg(P) − deg(D) + 1
// Гарантирует целочисленное деление даже при ℤ[x].
Polynomial pseudo_mod(const Polynomial& P, const Polynomial& D) {
    int n = P.degree(), m = D.degree();
    if (n < m) return P;
    int delta = n - m + 1;

    Polynomial R(P);
    long long lcpow = 1;
    for (int i = 0; i < delta; i++) lcpow *= D.leading();

    // Умножаем R на lc(D)^delta
    for (auto& c : R.coeffs) c *= lcpow;

    for (int i = n; i >= m; i--) {
        if (R.degree() < i) continue;
        long long coeff = R[i] / D[m]; // теперь деление точное
        for (int j = 0; j <= m; j++)
            R.coeffs[i - m + j] -= coeff * D[j];
    }
    R.normalize();
    return R;
}

// А.2. Частное от деления
Polynomial div(const Polynomial& P, const Polynomial& D) {
    return divide(P, D).first;
}

// --- A.4. Деление на (x − a) через Горнера ---
// Возвращает {P(a), Q(x) = P(x)/(x−a)}.
pair<long long, Polynomial> divide_by_linear(const Polynomial& P, long long a) {
    return horner_extended(P, a);
}

// =============================================================
// B. ТЕОРЕМА БЕЗУ И СЛЕДСТВИЯ
// =============================================================

// --- B.5. Проверка: является ли a корнем P ---
// P(a) = 0 ⟺ (x−a) | P(x)
bool is_root(const Polynomial& P, long long a) {
    return horner(P, a) == 0;
}

// --- B.6. Кратность корня a ---
// a — корень кратности k: P(a)=P'(a)=...=P^{(k-1)}(a)=0, P^{(k)}(a)≠0.
int root_multiplicity(const Polynomial& P, long long a) {
    Polynomial cur = P;
    int k = 0;
    while (cur.degree() >= 0 && horner(cur, a) == 0) {
        k++;
        cur = derivative(cur);
    }
    return k;
}

// --- B.7. Количество вещественных корней (наивный перебор) ---
// Возвращает число различных корней из заданного множества.
int count_roots_in(const Polynomial& P, const vector<long long>& candidates) {
    int cnt = 0;
    for (long long a : candidates)
        if (is_root(P, a)) cnt++;
    return cnt;
}

}; // struct Division

#endif // ALGEBRA_B_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_B_MAIN
int main() {
    using Poly = Division::Polynomial;
    Division div;

    // Пример A.2: деление «уголком»
    // P(x) = x³ − 2x² + x − 3, D(x) = x − 1
    Poly P({-3, 1, -2, 1}); // x³ − 2x² + x − 3
    Poly D({-1, 1});         // x − 1
    auto [Q, R] = div.divide(P, D);
    cout << "P / D = "; Q.print(); cout << "\n";
    cout << "P mod D = "; R.print(); cout << "\n";
    // Ожидаем: Q = x² − x, R = −3 (P(1) = 1 − 2 + 1 − 3 = −3)

    // Пример B.5: теорема Безу
    cout << "P(1) = " << div.horner(P, 1) << "\n";
    cout << "is_root(P, 1) = " << div.is_root(P, 1) << "\n";

    // Пример B.6: кратность
    // Q(x) = (x−1)² → кратность корня 1 равна 2
    Poly Q2({1, -2, 1}); // x² − 2x + 1 = (x−1)²
    cout << "mult(root=1 in (x-1)²) = " << div.root_multiplicity(Q2, 1) << "\n";

    return 0;
}
#endif // ALGEBRA_B_MAIN
