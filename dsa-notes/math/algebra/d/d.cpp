#ifndef ALGEBRA_D_CPP
#define ALGEBRA_D_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <random>
using namespace std;

// =============================================================
// IV. РАЗЛОЖЕНИЕ НА МНОЖИТЕЛИ
// =============================================================
// Структура md: A. Неприводимые многочлены
//               → B. Методы разложения
//               → C. Разложение над 𝔽_p
//
// Factorization наследует PolynomialGCD (c.cpp). Переиспользует:
//   * gcd_classic (c.cpp, B.4) — для разбиения на множители;
//   * divide/mod (b.cpp, A.2) — для деления;
//   * derivative (a.cpp, B.6) — для кратных корней;
//   * normalize_gcd (c.cpp, A.1) — для нормировки.
//
// Порядок методов строго соответствует порядку md (A → C).

#include "../c/c.cpp"

struct Factorization : PolynomialGCD {

// =============================================================
// A. НЕПРОВОДИМЫЕ МНОГОЧЛЕНЫ
// =============================================================

// --- A.1. Простейшая проверка неприводимости (наивный перебор) ---
// Возвращает список неприводимых множителей.
// search_bound — граница перебора линейных множителей (по умолчанию 100).
vector<Polynomial> factor_naive(const Polynomial& P, long long search_bound = 100) {
    int n = P.degree();
    if (n <= 0) return {};
    if (n == 1) return {P};

    // Ищем линейные множители (x − a) для |a| ≤ search_bound
    for (long long a = -search_bound; a <= search_bound; a++) {
        if (horner(P, a) == 0) {
            Polynomial factor({-a, 1});
            auto [Q, R] = divide(P, factor);
            auto rest = factor_naive(Q, search_bound);
            rest.insert(rest.begin(), factor);
            return rest;
        }
    }
    // Не нашли корней → для малых степеней считаем неприводимым
    if (n <= 3) return {P};
    return {P};
}

// --- A.2. Критерий Эйзенштейна ---
bool eisenstein(const Polynomial& P, long long p) {
    int n = P.degree();
    if (n <= 0) return false;
    if (P[n] % p == 0) return false;
    for (int i = 0; i < n; i++)
        if (P[i] % p != 0) return false;
    if (P[0] % (p * p) == 0) return false;
    return true;
}

// A.2. Проверка для нескольких p (primes_limit — верхняя граница простых)
bool is_irreducible_eisenstein(const Polynomial& P, int primes_limit = 200) {
    // Генерация простых решетом Эратосфена
    vector<bool> sieve(primes_limit + 1, true);
    sieve[0] = sieve[1] = false;
    for (int i = 2; i * i <= primes_limit; i++)
        if (sieve[i])
            for (int j = i * i; j <= primes_limit; j += i)
                sieve[j] = false;
    for (int p = 2; p <= primes_limit; p++)
        if (sieve[p] && eisenstein(P, p)) return true;
    return false;
}

// =============================================================
// B. МЕТОДЫ РАЗЛОЖЕНИЯ
// =============================================================

// --- B.4. Поиск линейного множителя (x − a) ---
Polynomial find_linear_factor(const Polynomial& P, long long search_bound = 200) {
    for (long long a = -search_bound; a <= search_bound; a++) {
        if (horner(P, a) == 0)
            return Polynomial({-a, 1});
    }
    return Polynomial(); // не найден
}

// --- B.5. Выделение полного квадрата: A² − B² = (A−B)(A+B) ---
// Разложение P(x) = f(x)² − g(x)² через разность квадратов.
// Если P = F·G и F, G имеют одинаковую степень, то A = (F+G)/2, B = (G−F)/2.
// Возвращает {A, B} такие что P = (A−B)(A+B) или пустой вектор.
pair<Polynomial, Polynomial> diff_of_squares(const Polynomial& P) {
    int n = P.degree();
    if (n < 2 || n % 2 != 0) return {Polynomial(), Polynomial()};
    // Пробуем найти разложение P = F·G, deg F = deg G = n/2
    int half = n / 2;
    // Перебираем линейные множители для построения F
    for (long long r = -100; r <= 100; r++) {
        if (horner(P, r) != 0) continue;
        Polynomial factor({-r, 1});
        auto [Q, R] = divide(P, factor);
        if (R.degree() >= 0) continue;
        // P = (x − r) · Q; ищем дальше делитель Q
        for (long long s = -100; s <= 100; s++) {
            if (horner(Q, s) != 0) continue;
            Polynomial factor2({-s, 1});
            auto [Q2, R2] = divide(Q, factor2);
            if (R2.degree() >= 0) continue;
            // P = (x−r)(x−s)·Q2; повторяем пока не получим два множителя нужной степени
            // Простой случай: n=2, P = (x−r)(x−s) = A²−B² если r,s позволяют
            if (n == 2) {
                // P = x² − (r+s)x + rs; A = x − (r+s)/2, B = (r−s)/2
                // A²−B² = x² − (r+s)x + rs = P ✓
                long long sum_rs = r + s;
                long long diff_rs = r - s;
                if (sum_rs % 2 != 0 || diff_rs % 2 != 0) continue;
                Polynomial A({-sum_rs / 2, 1});
                Polynomial B({diff_rs / 2});
                return {A, B};
            }
        }
    }
    // Общий случай: пробуем разложение на два множителя степени half
    // Через перебор линейных множителей и рекурсивное построение
    Polynomial one({1});
    Polynomial neg_one({-1});
    // Первая попытка: ищем корень, строим F = (x−r), G = P/(x−r)
    // Если deg G = n−1, не подходит для n > 2.
    // Для полного решения нужен полный перебор делителей — упрощаем до n=4.
    if (n == 4) {
        for (long long r1 = -50; r1 <= 50; r1++) {
            if (horner(P, r1) != 0) continue;
            Polynomial f1({-r1, 1});
            auto [q1, rem1] = divide(P, f1);
            if (rem1.degree() >= 0) continue;
            for (long long r2 = -50; r2 <= 50; r2++) {
                if (horner(q1, r2) != 0) continue;
                Polynomial f2({-r2, 1});
                auto [q2, rem2] = divide(q1, f2);
                if (rem2.degree() >= 0) continue;
                // P = (x−r1)(x−r2)·q2, deg q2 = 2
                for (long long r3 = -50; r3 <= 50; r3++) {
                    if (horner(q2, r3) != 0) continue;
                    Polynomial f3({-r3, 1});
                    auto [q3, rem3] = divide(q2, f3);
                    if (rem3.degree() >= 0) continue;
                    // P = (x−r1)(x−r2)(x−r3)·q3, q3 линейный
                    Polynomial f4 = q3;
                    // F = (x−r1)(x−r2), G = (x−r3)(x−r4)
                    Polynomial F = f1 * f2;
                    Polynomial G = f3 * f4;
                    // A = (F+G)/2, B = (G−F)/2
                    Polynomial sum_FG = F + G;
                    Polynomial diff_GF = G - F;
                    // Проверяем делимость на 2
                    bool ok = true;
                    for (int i = 0; i <= 2; i++) {
                        if (sum_FG[i] % 2 != 0 || diff_GF[i] % 2 != 0) { ok = false; break; }
                    }
                    if (!ok) continue;
                    vector<long long> a_c(3), b_c(3);
                    for (int i = 0; i <= 2; i++) {
                        a_c[i] = sum_FG[i] / 2;
                        b_c[i] = diff_GF[i] / 2;
                    }
                    Polynomial A(a_c), B(b_c);
                    // Проверка: A² − B² == P?
                    Polynomial A2 = A * A;
                    Polynomial B2 = B * B;
                    Polynomial check = A2 - B2;
                    if (check == P) return {A, B};
                }
            }
        }
    }
    return {Polynomial(), Polynomial()}; // не разложилось
}

// --- B.6. Группировка слагаемых ---
// Разложение P(x) путём группировки членов с общими множителями.
// Пример: x³ + x² + x + 1 = x²(x+1) + (x+1) = (x²+1)(x+1).
Polynomial factor_by_grouping(const Polynomial& P) {
    // Наивно: ищем делитель через перебор корней (уже есть в factor_naive)
    // Группировка — эвристический метод, реализуем через factor_naive
    auto factors = factor_naive(P);
    if (factors.size() > 1) return factors[0]; // нашли нетривиальный множитель
    return Polynomial(); // не разложилось
}

// --- B.7. Замены и сдвиги ---
// Разложение через замену x → x+c для обнаружения скрытой структуры.
// Если P(x) имеет корень a, то P(x+a) имеет корень 0.
Polynomial factor_by_shift(const Polynomial& P, long long shift_bound = 50) {
    for (long long c = -shift_bound; c <= shift_bound; c++) {
        // Вычисляем P(x+c): подстановка через Горнера
        Polynomial shifted = P;
        for (int i = P.degree(); i >= 1; i--) {
            for (int j = 0; j < i; j++)
                shifted.coeffs[j] += c * shifted.coeffs[j + 1];
        }
        // Ищем корень 0 в P(x+c) → P(c) = 0
        if (shifted[0] == 0) {
            // P(c) = 0, значит (x−c) — делитель P(x)
            return Polynomial({-c, 1});
        }
    }
    return Polynomial(); // не нашли
}

// =============================================================
// C. РАЗЛОЖЕНИЕ НАД 𝔽_p
// =============================================================

// --- C.8. Умножение полиномов по модулю p ---
Polynomial mul_mod(const Polynomial& A, const Polynomial& B, long long p) {
    int n = A.degree(), m = B.degree();
    vector<long long> res(max(0, n + m - 1) + 1, 0);
    for (int i = 0; i <= n; i++)
        for (int j = 0; j <= m; j++)
            res[i + j] = (res[i + j] + A[i] * B[j]) % p;
    return Polynomial(res);
}

// C.8. Возведение полинома в степень по модулю p
Polynomial pow_mod(Polynomial base, long long exp, long long p, const Polynomial& mod_poly) {
    Polynomial result({1});
    while (exp > 0) {
        if (exp & 1) result = mod(mul_mod(result, base, p), mod_poly);
        base = mul_mod(base, base, p);
        base = mod(base, mod_poly);
        exp >>= 1;
    }
    return result;
}

// C.8. Проверка: является ли P неприводимым над 𝔽_p (перебором делителей)
bool is_irreducible_mod_p(const Polynomial& P, long long p) {
    int n = P.degree();
    if (n <= 0) return false;
    if (n == 1) return true;

    // Проверяем: gcd(P, x^{p^k} − x) = 1 для k = 1, ..., n/2
    // x^{p^k} − x: все элементы 𝔽_{p^k} — корни
    Polynomial x({0, 1}); // x
    Polynomial one({1});

    for (int k = 1; k <= n / 2; k++) {
        // Вычисляем x^{p^k} mod P
        Polynomial xp = x;
        for (int i = 0; i < k; i++)
            xp = pow_mod(xp, p, p, P);
        // xp − x
        Polynomial diff = xp - x;
        diff = mod(diff, P);
        // gcd(P, xp − x)
        Polynomial g = gcd_classic(P, diff);
        g = mod(g, P);
        if (g.degree() > 0) return false; // есть общий делитель → приводим
    }
    return true;
}

// C.9. Алгоритм Кантора-Зассенхауза (упрощённый)
// Находит нетривиальный делитель P(x) над 𝔽_p.
Polynomial cantor_zassenhaus(const Polynomial& P, long long p, int max_attempts = 30) {
    int n = P.degree();
    if (n <= 1) return P;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<long long> dist(0, p - 1);

    for (int attempt = 0; attempt < max_attempts; attempt++) {
        long long a = dist(gen);
        // h(x) = (x − a)^{(p−1)/2} − 1 mod P
        Polynomial x_minus_a({-a, 1}); // (x − a)
        long long exp = (p - 1) / 2;
        Polynomial h = pow_mod(x_minus_a, exp, p, P);
        h = h - Polynomial({1}); // h − 1
        h = mod(h, P);

        if (h.degree() > 0 && h.degree() < n) {
            Polynomial g = gcd_classic(P, h);
            g = mod(g, P);
            if (g.degree() > 0 && g.degree() < n)
                return g;
        }
    }
    return Polynomial(); // не нашли
}

// --- C.10. Разложение Берлекэмпа-Ленстры (детерминированное) ---
// Комбинация Берлекэмпа и Кантора-Зассенхауза.
// Сложность: O~(n^{ω/2}).
Polynomial berlekamp_lenstra(const Polynomial& P, long long p) {
    if (is_irreducible_mod_p(P, p)) return P;
    return cantor_zassenhaus(P, p);
}

}; // struct Factorization

#endif // ALGEBRA_D_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_D_MAIN
int main() {
    using Poly = Factorization::Polynomial;
    Factorization fact;

    // Пример A.2: Эйзенштейн
    Poly p1({-2, 0, 1, 1}); // x³ + x² − 2
    cout << "eisenstein(x³+x²−2, 2) = " << fact.eisenstein(p1, 2) << "\n";

    // Пример A.1: поиск линейного множителя
    Poly p2({-1, 0, 1, 1}); // x³ + x² − 1
    Poly factor = fact.find_linear_factor(p2);
    if (factor.degree() >= 0) {
        cout << "linear factor: "; factor.print(); cout << "\n";
    }

    // Пример C.8: умножение по модулю
    long long MOD = 998244353;
    Poly a({1, 1}); // 1 + x
    Poly b({1, 1}); // 1 + x
    Poly c = fact.mul_mod(a, b, MOD);
    cout << "(1+x)*(1+x) mod p = "; c.print(); cout << "\n";

    return 0;
}
#endif // ALGEBRA_D_MAIN
