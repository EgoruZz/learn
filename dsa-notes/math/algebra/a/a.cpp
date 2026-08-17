#ifndef ALGEBRA_A_CPP
#define ALGEBRA_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <cmath>
using namespace std;

// =============================================================
// I. ОСНОВЫ МНОГОЧЛЕНОВ
// =============================================================
// Структура md: A. Определение и кольцо многочленов
//               → B. Операции над многочленами
//
// Polynomials — базовый класс ветки algebra. Вводит определение
// многочлена, кольцо K[x] (сложение, умножение, свёртку),
// подстановку (схему Горнера), формальное дифференцирование
// и композицию. Не зависит от других веток algebra, но
// используется всеми последующими разделами (b–k).
//
// Коэффициенты: long long (для модулярных вычислений — заменить
// на вычисление по модулю p через модульную арифметику).

struct Polynomials {

// =============================================================
// A. ОПРЕДЕЛЕНИЕ И КОЛЬЦО МНОГОЧЛЕНОВ
// =============================================================

// --- A.1. Многочлен от одной переменной ---
// Хранение: вектор коэффициентов coeffs[i] = aᵢ (коэффициент при xⁱ).
// coords[0] — свободный член, coords[n] — старший коэффициент.
// Удаление ведущих нулей: normalize().

struct Polynomial {
    vector<long long> coeffs;

    Polynomial() {}
    Polynomial(const vector<long long>& c) : coeffs(c) { normalize(); }
    Polynomial(initializer_list<long long> init) : coeffs(init) { normalize(); }

    void normalize() {
        while ((int)coeffs.size() > 1 && coeffs.back() == 0)
            coeffs.pop_back();
    }

    int degree() const {
        if (coeffs.empty() || (coeffs.size() == 1 && coeffs[0] == 0))
            return -1; // deg 0 = -∞
        return (int)coeffs.size() - 1;
    }

    long long leading() const {
        if (coeffs.empty()) return 0;
        return coeffs.back();
    }

    long long operator[](int i) const {
        return (i < (int)coeffs.size()) ? coeffs[i] : 0;
    }

    // A.2. Сложение: O(max(n, m))
    Polynomial operator+(const Polynomial& other) const {
        int n = (int)coeffs.size(), m = (int)other.coeffs.size();
        vector<long long> res(max(n, m), 0);
        for (int i = 0; i < n; i++) res[i] += coeffs[i];
        for (int i = 0; i < m; i++) res[i] += other.coeffs[i];
        return Polynomial(res);
    }

    // A.2. Вычитание: O(max(n, m))
    Polynomial operator-(const Polynomial& other) const {
        int n = (int)coeffs.size(), m = (int)other.coeffs.size();
        vector<long long> res(max(n, m), 0);
        for (int i = 0; i < n; i++) res[i] += coeffs[i];
        for (int i = 0; i < m; i++) res[i] -= other.coeffs[i];
        return Polynomial(res);
    }

    // A.2. Умножение (свертка): O(n·m)
    Polynomial operator*(const Polynomial& other) const {
        int n = (int)coeffs.size(), m = (int)other.coeffs.size();
        if (n == 0 || m == 0) return Polynomial();
        vector<long long> res(n + m - 1, 0);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                res[i + j] += coeffs[i] * other.coeffs[j];
        return Polynomial(res);
    }

    // A.2. Умножение на скаляр
    Polynomial operator*(long long scalar) const {
        vector<long long> res(coeffs.size());
        for (int i = 0; i < (int)coeffs.size(); i++)
            res[i] = coeffs[i] * scalar;
        return Polynomial(res);
    }

    bool operator==(const Polynomial& other) const {
        return coeffs == other.coeffs;
    }

    bool operator!=(const Polynomial& other) const {
        return !(*this == other);
    }

    void print(const string& var = "x") const {
        if (coeffs.empty()) { cout << "0"; return; }
        bool first = true;
        for (int i = (int)coeffs.size() - 1; i >= 0; i--) {
            if (coeffs[i] == 0) continue;
            if (!first && coeffs[i] > 0) cout << "+";
            if (coeffs[i] < 0) cout << "-";
            long long c = abs(coeffs[i]);
            if (i == 0 || c != 1) cout << c;
            if (i > 0) {
                cout << var;
                if (i > 1) cout << "^" << i;
            }
            first = false;
        }
        if (first) cout << "0";
    }
};

// A.4. Пример: P(x) = 2x³ + 3x − 1
// Polynomial P({-1, 3, 0, 2});
// P.degree() == 3, P.leading() == 2, P[0] == -1, P[2] == 0

// =============================================================
// B. ОПЕРАЦИИ НАД МНОГОЧЛЕНАМИ
// =============================================================

// --- B.5. Подстановка (схема Горнера) ---
// Вычисление P(a) за O(n).
// Рекуррентность: bₙ = aₙ; bₖ = aₖ + a·b_{k+1}; P(a) = b₀.
long long horner(const Polynomial& P, long long a) {
    long long result = 0;
    for (int i = (int)P.coeffs.size() - 1; i >= 0; i--)
        result = result * a + P.coeffs[i];
    return result;
}

// Горнера-расширенный: одновременно P(a) и Q(x) от P(x)/(x−a)
// Возвращает {P(a), коэффициенты Q}.
pair<long long, Polynomial> horner_extended(const Polynomial& P, long long a) {
    int n = P.degree();
    if (n < 0) return {0, Polynomial({0})};
    vector<long long> q(n + 1, 0);
    q[n] = 0; // коэффициент xⁿ в Q (старше n−1 не бывает)
    long long remainder = 0;
    for (int i = n; i >= 0; i--) {
        remainder = remainder * a + P[i];
        if (i > 0) q[i - 1] = remainder;
    }
    // q — коэффициенты Q(x) = P(x)/(x−a) (степени n−1)
    vector<long long> qcoeffs(q.begin(), q.begin() + n);
    return {remainder, Polynomial(qcoeffs)};
}

// --- B.6. Формальное дифференцирование ---
// P'(x) = ∑_{i=1}^{n} i·aᵢ xⁱ⁻¹
Polynomial derivative(const Polynomial& P) {
    int n = P.degree();
    if (n <= 0) return Polynomial({0});
    vector<long long> res(n);
    for (int i = 1; i <= n; i++)
        res[i - 1] = P[i] * i;
    return Polynomial(res);
}

// --- B.7. Композиция (P ∘ Q)(x) = P(Q(x)) ---
// Наивный алгоритм: O(n·m²) где n = deg P, m = deg Q.
// Вычисляем P(Q(x)) = ∑ aᵢ Q(x)ⁱ через последовательное возведение.
Polynomial compose(const Polynomial& P, const Polynomial& Q) {
    int n = P.degree();
    if (n < 0) return Polynomial({0});
    Polynomial result({0});    // текущая сумма
    Polynomial qpow({1});      // Q^0 = 1
    for (int i = 0; i <= n; i++) {
        result = result + qpow * P[i];
        if (i < n) qpow = qpow * Q;
    }
    return result;
}

// --- B.8. Формальное обращение ---
// Найти Q(x) = 1/P(x) в K[[x]] при a₀ ≠ 0.
// Рекуррентность: b₀ = 1/a₀; bₙ = −(1/a₀) ∑_{k=1}^{n} aₖ b_{n−k}.
// Версия для ℚ (inv_a0 = 1, т.к. a₀ = 1 для монических многочленов).
// Для ℤ/ℤ_p: заменить inv_a0 = modinv(P[0], p), см. g.cpp (modinv).
Polynomial inverse(const Polynomial& P, int max_deg) {
    assert(P[0] != 0 && "a₀ must be nonzero for formal inverse");
    long long inv_a0 = 1; // ℚ-версия; для ℤ/ℤ_p — вызов modinv(P[0], p)
    vector<long long> result(max_deg + 1, 0);
    result[0] = inv_a0;
    for (int n = 1; n <= max_deg; n++) {
        long long sum = 0;
        for (int k = 1; k <= n && k <= P.degree(); k++)
            sum += P[k] * result[n - k];
        result[n] = -sum * inv_a0;
    }
    return Polynomial(result);
}

// --- B.9. Формальное обращение по модулю ---
// Найти Q(x) = 1/P(x) в (ℤ/ℤ_p)[[x]] при a₀ ≠ 0 (mod p).
// Использует малую теорему Ферма для a₀⁻¹: a₀^(p−2) mod p.
// Рекуррентность та же, но вся арифметика по модулю p.
// Требует, чтобы p был простым.
static long long powmod(long long base, long long exp, long long mod) {
    base %= mod;
    long long result = 1;
    while (exp > 0) {
        if (exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

Polynomial inverse_mod(const Polynomial& P, int max_deg, long long mod) {
    assert(P[0] % mod != 0 && "a₀ must be nonzero mod p for formal inverse");
    long long inv_a0 = powmod(P[0], mod - 2, mod); // Малая теорема Ферма
    vector<long long> result(max_deg + 1, 0);
    result[0] = inv_a0;
    for (int n = 1; n <= max_deg; n++) {
        long long sum = 0;
        for (int k = 1; k <= n && k <= P.degree(); k++)
            sum = (sum + P[k] % mod * result[n - k]) % mod;
        result[n] = (mod - sum % mod) * inv_a0 % mod;
    }
    return Polynomial(result);
}

// --- Вспомогательные функции ---

// Создание многочлена xᵏ
Polynomial monomial(int k, long long coeff = 1) {
    if (k == 0) return Polynomial({coeff});
    vector<long long> v(k + 1, 0);
    v[k] = coeff;
    return Polynomial(v);
}

// Вычисление P(x) mod (xᵐ − a): циклический с_shift
// Результат: ∑_{i=0}^{m-1} cᵢ xⁱ где cᵢ = ∑_{j≡i (mod m)} aⱼ·a^{⌊j/m⌋}
Polynomial mod_cyclic(const Polynomial& P, int m, long long a) {
    vector<long long> res(m, 0);
    long long apow = 1;
    for (int i = 0; i <= P.degree(); i++) {
        res[i % m] += P[i] * apow;
        if (i % m == m - 1) apow *= a; // a^{⌊(i+1)/m⌋}
    }
    return Polynomial(res);
}

}; // struct Polynomials

#endif // ALGEBRA_A_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_A_MAIN
int main() {
    using Poly = Polynomials::Polynomial;
    Polynomials poly;

    // Пример A.1–A.2: сложение и умножение
    Poly a({1, 2, 3});       // 3x² + 2x + 1
    Poly b({4, 5});          // 5x + 4
    Poly c = a * b;          // 15x³ + 22x² + 13x + 4
    cout << "a = "; a.print(); cout << "\n";
    cout << "b = "; b.print(); cout << "\n";
    cout << "a*b = "; c.print(); cout << "\n";

    // Пример B.5: Горнера
    cout << "P(2) = " << poly.horner(a, 2) << "\n"; // 3·4 + 2·2 + 1 = 17

    // Пример B.6: производная
    Poly d = poly.derivative(a); // 6x + 2
    cout << "P' = "; d.print(); cout << "\n";

    // Пример B.8: обратный (первые 5 членов)
    Poly u({1, 1});          // 1 + x
    Poly u_inv = poly.inverse(u, 4); // 1 − x + x² − x³ + x⁴
    cout << "1/(1+x) = "; u_inv.print(); cout << "\n";

    return 0;
}
#endif // ALGEBRA_A_MAIN
