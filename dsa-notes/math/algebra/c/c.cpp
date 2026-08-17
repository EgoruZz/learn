#ifndef ALGEBRA_C_CPP
#define ALGEBRA_C_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// III. НОД МНОГОЧЛЕНОВ И АЛГОРИТМ ЕВКЛИДА
// =============================================================
// Структура md: A. Наибольший общий делитель
//               → B. Алгоритм Евклида для многочленов
//
// PolynomialGCD наследует Division (b.cpp). Переиспользует:
//   * divide/mod/div (b.cpp, A.2) — деление с остатком;
//   * derivative (a.cpp, B.6) — формальное дифференцирование;
//   * is_root, root_multiplicity (b.cpp, B.5–B.6).
//
// Порядок методов строго соответствует порядку md (A → B).

#include "../b/b.cpp"

struct PolynomialGCD : Division {

// =============================================================
// A. НАИБОЛЬШИЙ ОБЩИЙ ДЕЛИТЕЛЬ
// =============================================================

// --- A.1. Нормализация (lc = 1) ---
Polynomial normalize_gcd(const Polynomial& P) {
    if (P.degree() < 0) return P;
    long long lc = P.leading();
    if (lc == 0 || lc == 1) return P;
    vector<long long> res(P.coeffs.size());
    for (int i = 0; i < (int)P.coeffs.size(); i++)
        res[i] = P[i] / lc;
    return Polynomial(res);
}

// =============================================================
// B. АЛГОРИТМ ЕВКЛИДА
// =============================================================

// --- B.4. Классический алгоритм Евклида ---
Polynomial gcd_classic(Polynomial P, Polynomial Q) {
    while (Q.degree() >= 0) {
        Polynomial R = mod(P, Q);
        P = Q;
        Q = R;
    }
    return normalize_gcd(P);
}

// --- B.5. Расширенный алгоритм Евклида ---
struct ExtendedGCD {
    Polynomial gcd, u, v;
};

ExtendedGCD gcd_extended(Polynomial P, Polynomial Q) {
    Polynomial S_prev({1}), S_cur({0});
    Polynomial T_prev({0}), T_cur({1});

    while (Q.degree() >= 0) {
        auto [quotient, remainder] = divide(P, Q);
        Polynomial S_new = S_prev - quotient * S_cur;
        Polynomial T_new = T_prev - quotient * T_cur;
        S_prev = S_cur; S_cur = S_new;
        T_prev = T_cur; T_cur = T_new;
        P = Q;
        Q = remainder;
    }
    return {normalize_gcd(P), S_prev, T_prev};
}

// --- A.2. Линейная комбинация (Bezout): gcd = U·P + V·Q ---
Polynomial bezout_u(Polynomial P, Polynomial Q) {
    return gcd_extended(P, Q).u;
}

Polynomial bezout_v(Polynomial P, Polynomial Q) {
    return gcd_extended(P, Q).v;
}

// --- B.6. Субрезультантный алгоритм Евклида (для ℤ[x]) ---
Polynomial gcd_subresultant(Polynomial P, Polynomial Q) {
    while (Q.degree() >= 0) {
        int delta = P.degree() - Q.degree();
        Polynomial R = mod(P, Q);
        if (R.degree() < 0) break;
        long long lcP = P.leading();
        long long lcPow = 1;
        for (int i = 0; i < delta - 1 && lcP != 1 && lcP != -1; i++)
            lcPow *= lcP;
        if (lcPow != 1 && lcPow != -1) {
            vector<long long> scaled(R.coeffs.size());
            for (int i = 0; i < (int)R.coeffs.size(); i++)
                scaled[i] = R[i] / lcPow;
            R = Polynomial(scaled);
        }
        P = Q;
        Q = R;
    }
    return normalize_gcd(P);
}

// --- A.3. НОД нескольких многочленов ---
Polynomial gcd_multiple(const vector<Polynomial>& polys) {
    assert(!polys.empty());
    Polynomial result = polys[0];
    for (int i = 1; i < (int)polys.size(); i++)
        result = gcd_classic(result, polys[i]);
    return result;
}

}; // struct PolynomialGCD

#endif // ALGEBRA_C_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_C_MAIN
int main() {
    using Poly = PolynomialGCD::Polynomial;
    PolynomialGCD polyGcd;

    // Пример B.4: gcd(x²−1, x−1) = x−1
    Poly a({-1, 0, 1});  // x² − 1
    Poly b({-1, 1});     // x − 1
    Poly g = polyGcd.gcd_classic(a, b);
    cout << "gcd(x²-1, x-1) = "; g.print(); cout << "\n"; // x − 1

    // Пример B.5: расширенный Евклид
    auto [gcd, u, v] = polyGcd.gcd_extended(a, b);
    cout << "U = "; u.print(); cout << "\n";
    cout << "V = "; v.print(); cout << "\n";
    // U·a + V·b = gcd

    return 0;
}
#endif // ALGEBRA_C_MAIN
