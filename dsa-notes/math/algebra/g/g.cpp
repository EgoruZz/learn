#ifndef ALGEBRA_G_CPP
#define ALGEBRA_G_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <complex>
#include <cmath>
using namespace std;

// =============================================================
// VII. БЫСТРОЕ УМНОЖЕНИЕ МНОГОЧЛЕНОВ
// =============================================================
// Структура md: A. Алгоритмы умножения
//               → B. Быстрое преобразование Фурье (FFT)
//
// FastMultiplication наследует Interpolation (f.cpp). Переиспользует:
//   * Polynomial (a.cpp, A) — базовая структура;
//   * divide/mod (b.cpp, A.2) — деление;
//   * horner (a.cpp, B.5) — вычисление.

#include "../f/f.cpp"

struct FastMultiplication : Interpolation {

// =============================================================
// A. АЛГОРИТМЫ УМНОЖЕНИЯ
// =============================================================

// --- A.2. Алгоритм Карацубы ---
// Умножение двух многочленов за O(n^{log₂3}).
// Разбиение: P = P₁xᵏ + P₀, Q = Q₁xᵏ + Q₀.
// Рекурсия: 3 умножения вместо 4.
Polynomial karatsuba(const Polynomial& A, const Polynomial& B) {
    int n = max(A.degree(), B.degree()) + 1;
    if (n <= 32) return A * B; // базовый случай: наивное умножение

    int k = n / 2;
    // P = P₁xᵏ + P₀
    Polynomial P_low, P_high, Q_low, Q_high;
    for (int i = 0; i < min(k, (int)A.coeffs.size()); i++)
        P_low.coeffs.push_back(A[i]);
    for (int i = k; i < (int)A.coeffs.size(); i++)
        P_high.coeffs.push_back(A[i]);
    for (int i = 0; i < min(k, (int)B.coeffs.size()); i++)
        Q_low.coeffs.push_back(B[i]);
    for (int i = k; i < (int)B.coeffs.size(); i++)
        Q_high.coeffs.push_back(B[i]);

    P_low.normalize(); P_high.normalize();
    Q_low.normalize(); Q_high.normalize();

    Polynomial z2 = karatsuba(P_high, Q_high);        // P₁·Q₁
    Polynomial z0 = karatsuba(P_low, Q_low);           // P₀·Q₀
    Polynomial z1 = karatsuba(P_low + P_high, Q_low + Q_high) - z0 - z2;

    // Результат: z2·x^{2k} + z1·xᵏ + z0
    Polynomial result;
    int maxdeg = z2.degree() + 2 * k + 1;
    result.coeffs.assign(maxdeg + 1, 0);
    for (int i = 0; i <= z2.degree(); i++)
        result.coeffs[i + 2 * k] += z2[i];
    for (int i = 0; i <= z1.degree(); i++)
        result.coeffs[i + k] += z1[i];
    for (int i = 0; i <= z0.degree(); i++)
        result.coeffs[i] += z0[i];
    result.normalize();
    return result;
}

// --- A.3. Алгоритм Toom-3 (обобщение Карацубы на 3 части) ---
// Разбиение P = P₂x^{2k} + P₁xᵏ + P₀, Q = Q₂x^{2k} + Q₁xᵏ + Q₀.
// 5 умножений вместо 9: O(n^{log₃5}) ≈ O(n^{1.465}).
// Точки оценки: 0, 1, −1, 2, ∞ → 5 произведений, затем интерполяция.
Polynomial toom_3(const Polynomial& A, const Polynomial& B) {
    int n = max(A.degree(), B.degree()) + 1;
    if (n <= 48) return A * B; // базовый случай

    int k = (n + 2) / 3; // размер каждой части
    auto split = [&](const Polynomial& P) {
        Polynomial p0, p1, p2;
        for (int i = 0; i < min(k, (int)P.coeffs.size()); i++)
            p0.coeffs.push_back(P[i]);
        for (int i = k; i < min(2*k, (int)P.coeffs.size()); i++)
            p1.coeffs.push_back(P[i]);
        for (int i = 2*k; i < (int)P.coeffs.size(); i++)
            p2.coeffs.push_back(P[i]);
        return tuple<Polynomial, Polynomial, Polynomial>{p0, p1, p2};
    };

    auto [a0, a1, a2] = split(A);
    auto [b0, b1, b2] = split(B);

    // Вспомогательные: умножение полинома на скаляр
    auto scale = [](const Polynomial& p, long long s) {
        Polynomial r = p;
        for (auto& c : r.coeffs) c *= s;
        r.normalize();
        return r;
    };

    // 5 произведений в точках 0, 1, −1, 2, ∞
    Polynomial m0 = toom_3(a0, b0);                              // P(0)·Q(0)
    Polynomial m1 = toom_3(a0+a1+a2, b0+b1+b2);                 // P(1)·Q(1)
    Polynomial ma1 = toom_3(a0-a1+a2, b0-b1+b2);                // P(-1)·Q(-1)
    Polynomial P2 = a0 + scale(a1, 2) + scale(a2, 4);
    Polynomial Q2 = b0 + scale(b1, 2) + scale(b2, 4);
    Polynomial m2 = toom_3(P2, Q2);                             // P(2)·Q(2)
    Polynomial m_inf = toom_3(a2, b2);                           // P(∞)·Q(∞)

    // Интерполяция: восстанавливаем коэффициенты r₀..r₄
    Polynomial r3 = m_inf;
    Polynomial r0 = m0;
    // r4 = (m2 - 2*m1 + ma1) / 6
    Polynomial r4 = m2 - scale(m1, 2) + ma1;
    for (auto& c : r4.coeffs) c /= 6;
    r4.normalize();
    // r2 = (m1 + ma1)/2 - m0 - r4
    Polynomial r2_tmp = m1 + ma1;
    for (auto& c : r2_tmp.coeffs) c /= 2;
    r2_tmp.normalize();
    Polynomial r2 = r2_tmp - m0 - r4;
    // r1 = m1 - m0 - r2 - r3 - r4
    Polynomial r1 = m1 - m0 - r2 - r3 - r4;

    // Сборка: R = r₀ + r₁x + r₂x² + r₃x³ + r₄x⁴
    Polynomial result;
    result.coeffs.resize(5*k + 10, 0);
    for (int i = 0; i <= r0.degree(); i++) result.coeffs[i] += r0[i];
    for (int i = 0; i <= r1.degree(); i++) result.coeffs[i+k] += r1[i];
    for (int i = 0; i <= r2.degree(); i++) result.coeffs[i+2*k] += r2[i];
    for (int i = 0; i <= r3.degree(); i++) result.coeffs[i+3*k] += r3[i];
    for (int i = 0; i <= r4.degree(); i++) result.coeffs[i+4*k] += r4[i];
    result.normalize();
    return result;
}

// =============================================================
// B. БЫСТРОЕ ПРЕОБРАЗОВАНИЕ ФУРЬЕ (FFT)
// =============================================================
// Используем комплексные числа для FFT.

using cd = complex<double>;
const double PI = acos(-1.0);

// --- B.6. FFT: прямое преобразование ---
// a: входной вектор коэффициентов (дополняется нулями до n, n — степень двойки).
// inverse = false → прямое DFT; inverse = true → обратное (IFFT).
void fft(vector<cd>& a, bool inverse) {
    int n = (int)a.size();
    if (n == 1) return;

    // Bit-reversal перестановка
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    // Бабочки
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (inverse ? -1 : 1);
        cd wlen(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (inverse)
        for (cd& x : a) x /= n;
}

// --- B.8. Умножение многочленов через FFT ---
// Дополняет до степени двойки, FFT, поэлементное, IFFT.
Polynomial multiply_fft(const Polynomial& A, const Polynomial& B) {
    int n = 1;
    while (n < (int)A.coeffs.size() + (int)B.coeffs.size())
        n <<= 1;

    vector<cd> fa(n), fb(n);
    for (int i = 0; i < (int)A.coeffs.size(); i++)
        fa[i] = cd(A[i], 0);
    for (int i = 0; i < (int)B.coeffs.size(); i++)
        fb[i] = cd(B[i], 0);

    fft(fa, false);
    fft(fb, false);
    for (int i = 0; i < n; i++)
        fa[i] *= fb[i];
    fft(fa, true);

    vector<long long> res(n);
    for (int i = 0; i < n; i++)
        res[i] = (long long)round(fa[i].real());
    return Polynomial(res);
}

// --- B.10. NTT: Number Theoretic Transform ---
// NTT — модулярный аналог FFT. Использует простой модуль p,
// для которого p−1 делится на 2^k (нужно для длин butterfly).
// Стандартные модули: 998244353, 1004535809, 469762049 (все с корнем 3).

static long long powmod(long long base, long long exp, long long mod) {
    base %= mod;
    if (base < 0) base += mod;
    long long result = 1;
    while (exp > 0) {
        if (exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

// Примитивный корень для известных модулей (root必须 быть примитивным корнем модуля mod).
// Для 998244353 = 119·2^23+1 → g=3;  1004535809 = 479·2^21+1 → g=3;  469762049 = 7·2^26+1 → g=3.
// Возвращает g, такой что g^(mod-1) ≡ 1 (mod mod) и g^((mod-1)/2) ≢ 1 (mod mod).
long long primitive_root(long long mod) {
    if (mod == 998244353 || mod == 1004535809 || mod == 469762049)
        return 3;
    // Общий случай: полный перебор (медленно, только для отладки)
    for (long long g = 2; g < mod; g++) {
        bool ok = true;
        long long phi = mod - 1;
        for (long long p = 2; p * p <= phi; p++) {
            if (phi % p == 0) {
                if (powmod(g, phi / p, mod) == 1) { ok = false; break; }
                while (phi % p == 0) phi /= p;
            }
        }
        if (phi > 1 && powmod(g, phi, mod) == 1) ok = false;
        if (ok) return g;
    }
    return -1;
}

// Прямое NTT-преобразование (in-place).
// a: вектор коэффициентов, n: размер (степень двойки), mod: модуль, root: примитивный корень.
void ntt(vector<long long>& a, bool inverse, long long mod, long long root) {
    int n = (int)a.size();
    // Bit-reversal перестановка
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    // Бабочки с модулярной арифметикой
    for (int len = 2; len <= n; len <<= 1) {
        // wlen = g^{(mod-1)/len} mod mod  (первый корень из единицы порядка len)
        long long wlen = powmod(root, (mod - 1) / len, mod);
        if (inverse) wlen = powmod(wlen, mod - 2, mod); // обратный корень

        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < len / 2; j++) {
                long long u = a[i + j];
                long long v = a[i + j + len / 2] * w % mod;
                a[i + j] = (u + v) % mod;
                a[i + j + len / 2] = (u - v + mod) % mod;
                w = w * wlen % mod;
            }
        }
    }

    // Обратное преобразование: деление на n
    if (inverse) {
        long long inv_n = powmod(n, mod - 2, mod);
        for (long long& x : a)
            x = x * inv_n % mod;
    }
}

// Умножение многочленов через NTT по модулю mod.
// Требует mod простое, mod−1 кратно 2^k (где k ≥ ceil(log₂(n+m))).
// root — примитивный корень mod.
vector<long long> ntt_multiply(const vector<long long>& a, const vector<long long>& b,
                               long long mod, long long root) {
    int n = 1;
    while (n < (int)a.size() + (int)b.size())
        n <<= 1;

    vector<long long> fa(a.begin(), a.end());
    vector<long long> fb(b.begin(), b.end());
    fa.resize(n, 0);
    fb.resize(n, 0);

    ntt(fa, false, mod, root);
    ntt(fb, false, mod, root);
    for (int i = 0; i < n; i++)
        fa[i] = fa[i] * fb[i] % mod;
    ntt(fa, true, mod, root);

    return fa;
}

// --- B.11. Свёртка через FFT ---
// cₖ = ∑ aᵢbⱼ при i+j=k
vector<long long> convolution(const vector<long long>& a, const vector<long long>& b) {
    Polynomial A(a), B(b);
    Polynomial C = multiply_fft(A, B);
    return C.coeffs;
}

}; // struct FastMultiplication

#endif // ALGEBRA_G_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_G_MAIN
int main() {
    using Poly = FastMultiplication::Polynomial;
    FastMultiplication fastMult;

    // Пример A.2: Карацуба
    Poly a({1, 1, 1}); // 1 + x + x²
    Poly b({1, 1, 1}); // 1 + x + x²
    Poly c = fastMult.karatsuba(a, b);
    cout << "Karatsuba: "; c.print(); cout << "\n";

    // Пример B.8: FFT
    Poly d = fastMult.multiply_fft(a, b);
    cout << "FFT: "; d.print(); cout << "\n";

    // Проверка
    Poly naive = a * b;
    cout << "Naive: "; naive.print(); cout << "\n";

    return 0;
}
#endif // ALGEBRA_G_MAIN
