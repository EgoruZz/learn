#ifndef ALGEBRA_I_CPP
#define ALGEBRA_I_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// IX. РЕКУРРЕНТНЫЕ ПОСЛЕДОВАТЕЛЬНОСТИ
// =============================================================
// Структура md: A. Линейные рекурренты с постоянными коэффициентами
//               → B. Матричный метод
//               → C. Алгоритм Китамасы
//
// Recurrences наследует FastDivision (h.cpp). Переиспользует:
//   * horner (a.cpp, B.5) — вычисление значений;
//   * multiply_fft (g.cpp, B.8) — умножение через FFT;
//   * poly_pow (h.cpp, B.4) — возведение многочлена в степень;
//   * inverse_newton (h.cpp, A.1) — формальное обращение;
//   * divide/mod (b.cpp, A.2) — деление.

#include "../h/h.cpp"

struct Recurrences : FastDivision {

// =============================================================
// A. ЛИНЕЙНЫЕ РЕКУРРЕНТЫ С ПОСТОЯННЫМИ КОЭФФИЦИЕНТАМИ
// =============================================================

// --- A.2. Характеристический многочлен ---
// Для aₙ = c₁aₙ₋₁ + ... + cₖaₙ₋ₖ: T(x) = xᵏ − c₁xᵏ⁻¹ − ... − cₖ
Polynomial characteristic_poly(const vector<long long>& coeffs) {
    int k = (int)coeffs.size();
    vector<long long> res(k + 1, 0);
    res[k] = 1; // xᵏ
    for (int i = 0; i < k; i++)
        res[k - 1 - i] = -coeffs[i]; // −cᵢxⁱ
    return Polynomial(res);
}

// --- A.3. Метод матриц: вычисление aₙ через возведение матрицы ---
// Компаньонная матрица M: M[i][0] = cᵢ, M[i][i+1] = 1.
// aₙ = M^{n} · [a₀, a₁, ..., aₖ₋₁]ᵀ (сдвиг индексов).
vector<vector<long long>> mat_mul(const vector<vector<long long>>& A,
                                  const vector<vector<long long>>& B,
                                  long long mod = -1) {
    int n = (int)A.size(), m = (int)B[0].size(), p = (int)B.size();
    vector<vector<long long>> C(n, vector<long long>(m, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                C[i][j] += A[i][k] * B[k][j];
                if (mod > 0) C[i][j] %= mod;
            }
        }
    return C;
}

vector<vector<long long>> mat_pow(vector<vector<long long>> M, long long p,
                                  long long mod = -1) {
    int n = (int)M.size();
    vector<vector<long long>> result(n, vector<long long>(n, 0));
    for (int i = 0; i < n; i++) result[i][i] = 1; // единичная

    while (p > 0) {
        if (p & 1) result = mat_mul(result, M, mod);
        M = mat_mul(M, M, mod);
        p >>= 1;
    }
    return result;
}

// A.3. Вычисление aₙ через матричное возведение за O(k³ log n)
long long recurrence_nth_term(const vector<long long>& c, // коэффициенты c₁..cₖ
                               const vector<long long>& a0, // начальные a₀..aₖ₋₁
                               long long n) {
    int k = (int)c.size();
    if (n < k) return a0[n];

    // Компаньонная матрица k×k
    vector<vector<long long>> M(k, vector<long long>(k, 0));
    for (int i = 0; i < k; i++) M[0][i] = c[i];
    for (int i = 1; i < k; i++) M[i][i - 1] = 1;

    auto Mn = mat_pow(M, n - k + 1);
    long long result = 0;
    for (int i = 0; i < k; i++)
        result += Mn[0][i] * a0[k - 1 - i];
    return result;
}

// =============================================================
// B. МАТРИЧНЫЙ МЕТОД (продолжение)
// =============================================================

// --- B.7. Связь с собственными значениями ---
// M = PDP⁻¹ где D = diag(λ₁, ..., λₖ) → Mⁿ = PDⁿP⁻¹
// Это доказывает формулу aₙ = Σ αᵢ·λᵢⁿ через корни характеристического.
// Для вычисления собственных значений нужен QR-алгоритм или
// разложение характеристического многочлена (раздел V, e).

// =============================================================
// C. АЛГОРИТМ КИТАМАСЫ
// =============================================================

// --- C.9. xⁿ mod T(x) через бинарное возведения ---
// T(x) = xᵏ − c₁xᵏ⁻¹ − ... − cₖ
// Возвращает полином степени < k: xⁿ mod T(x) = ∑ dᵢ xⁱ
Polynomial kitamasa(const vector<long long>& c, long long n) {
    int k = (int)c.size();
    Polynomial T = characteristic_poly(c);

    // Возведение x в степень n мод T(x)
    // Для k < 64 — наивное умножение O(k²) нормально.
    // Для k ≥ 64 — рекомендуется multiply_fft из g.cpp: O(k log k).
    Polynomial result({1}); // x⁰ = 1
    Polynomial base({0, 1}); // x
    while (n > 0) {
        if (n & 1) {
            result = result * base;       // TODO: multiply_fft для k ≥ 64
            result = mod(result, T);
        }
        base = base * base;               // TODO: multiply_fft для k ≥ 64
        base = mod(base, T);
        n >>= 1;
    }
    return result;
}

// C.9. Вычисление aₙ через Китамасу: aₙ = ∑ dᵢ aᵢ
long long kitamasa_nth(const vector<long long>& c,
                        const vector<long long>& a0,
                        long long n) {
    int k = (int)c.size();
    if (n < k) return a0[n];
    auto d = kitamasa(c, n);
    long long result = 0;
    for (int i = 0; i < k; i++)
        result += d[i] * a0[i];
    return result;
}

// =============================================================
// C. АЛГОРИТМ КИТАМАСЫ (продолжение)
// =============================================================

// --- C.10. Оптимизация Kitamasa через FFT ---
// Для больших k (>64) умножение полиномов по модулю T(x)
// можно ускорить с O(k²) до O(k log k) через multiply_fft из g.cpp.
// Текущая реализация использует наивное умножение O(k²);
// для олимпиадных задач с k > 100 заменить mul() на multiply_fft + mod.

// =============================================================
// Пример: числа Фибоначчи
// =============================================================
// Fₙ = Fₙ₋₁ + Fₙ₋₂, F₀ = 0, F₁ = 1
long long fibonacci(long long n) {
    return recurrence_nth_term({1, 1}, {0, 1}, n);
}

long long fibonacci_kitamasa(long long n) {
    return kitamasa_nth({1, 1}, {0, 1}, n);
}

}; // struct Recurrences

#endif // ALGEBRA_I_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_I_MAIN
int main() {
    Recurrences rec;

    // Пример A.2: характеристический многочлен для Фибоначчи
    auto chi = rec.characteristic_poly({1, 1}); // x² − x − 1
    cout << "char poly (Fib): "; chi.print(); cout << "\n";

    // Пример A.3: 10-е число Фибоначчи
    cout << "F(10) = " << rec.fibonacci(10) << " (ожидаем 55)\n";

    // Пример C.9: Китамаса
    cout << "F(10) via Kitamasa = " << rec.fibonacci_kitamasa(10) << "\n";

    // Пример A.3: 20-е число Фибоначчи
    cout << "F(20) = " << rec.fibonacci(20) << " (ожидаем 6765)\n";

    return 0;
}
#endif // ALGEBRA_I_MAIN
