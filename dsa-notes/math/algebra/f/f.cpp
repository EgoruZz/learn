#ifndef ALGEBRA_F_CPP
#define ALGEBRA_F_CPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// VI. ИНТЕРПОЛЯЦИЯ
// =============================================================
// Структура md: A. Интерполяция многочленами
//               → B. Эрмитова интерполяция
//               → C. Быстрая интерполяция
//
// Interpolation наследует Roots (e.cpp). Переиспользует:
//   * horner (a.cpp, B.5) — вычисление P(a);
//   * divide/mod (b.cpp, A.2) — деление;
//   * inverse (a.cpp, B.8) — формальное обращение;
//   * gcd_classic (c.cpp, B.4) — НОД.

#include "../e/e.cpp"

struct Interpolation : Roots {

// =============================================================
// A. ИНТЕРПОЛЯЦИЯ МНОГОЧЛЕНАМИ
// =============================================================

// --- A.2. Интерполяция Лагранжа: O(n²) ---
// По точкам (x[i], y[i]), i=0..n, возвращает P степени ≤ n.
Polynomial lagrange(const vector<long long>& x, const vector<long long>& y) {
    int n = (int)x.size() - 1;
    Polynomial result({0});

    for (int i = 0; i <= n; i++) {
        // Lᵢ(x) = ∏_{j≠i} (x − xⱼ)/(xᵢ − xⱼ)
        Polynomial L({1});
        long long denom = 1;
        for (int j = 0; j <= n; j++) {
            if (i == j) continue;
            L = L * Polynomial({-x[j], 1}); // L *= (x − xⱼ)
            denom *= (x[i] - x[j]);
        }
        // L *= y[i] / denom
        result = result + L * (y[i] / denom);
    }
    return result;
}

// --- A.3. Интерполяция Ньютона через разделённые разности ---
// f[xᵢ,...,xⱼ] вычисляется через динамическое программирование.
Polynomial newton(const vector<long long>& x, const vector<long long>& y) {
    int n = (int)x.size() - 1;
    // Таблица разделённых разностей: dd[i][j] = f[xᵢ,...,x_{i+j}]
    vector<vector<long long>> dd(n + 1, vector<long long>(n + 1, 0));
    for (int i = 0; i <= n; i++)
        dd[i][0] = y[i];
    for (int j = 1; j <= n; j++)
        for (int i = 0; i + j <= n; i++)
            dd[i][j] = (dd[i + 1][j - 1] - dd[i][j - 1]) / (x[i + j] - x[i]);

    // P(x) = dd[0][0] + dd[0][1](x−x₀) + dd[0][2](x−x₀)(x−x₁) + ...
    Polynomial result({dd[0][0]});
    Polynomial product({1});
    for (int i = 1; i <= n; i++) {
        product = product * Polynomial({-x[i - 1], 1});
        result = result + product * dd[0][i];
    }
    return result;
}

// =============================================================
// B. ЭРМИТОВА ИНТЕРПОЛЯЦИЯ
// =============================================================

// B.4. Разделённые разности с кратными узлами
// x_mult: расширенный список точек (с повторами для кратных узлов)
// y_mult: соответствующие значения (включая производные)
Polynomial hermite(const vector<long long>& x_mult, const vector<long long>& y_mult) {
    int n = (int)x_mult.size() - 1;
    vector<vector<long long>> dd(n + 1, vector<long long>(n + 1, 0));
    for (int i = 0; i <= n; i++)
        dd[i][0] = y_mult[i];
    for (int j = 1; j <= n; j++)
        for (int i = 0; i + j <= n; i++) {
            if (x_mult[i + j] == x_mult[i]) {
                // Совпадающие узлы: dd[i][j] = f^{(j)}(a) / j!
                // y_mult[i+j] хранит значение производной (по контракту)
                long long fact_j = 1;
                for (int f = 2; f <= j; f++) fact_j *= f;
                dd[i][j] = y_mult[i + j] / fact_j;
            } else {
                dd[i][j] = (dd[i + 1][j - 1] - dd[i][j - 1]) / (x_mult[i + j] - x_mult[i]);
            }
        }

    Polynomial result({dd[0][0]});
    Polynomial product({1});
    for (int i = 1; i <= n; i++) {
        product = product * Polynomial({-x_mult[i - 1], 1});
        result = result + product * dd[0][i];
    }
    return result;
}

// =============================================================
// C. БЫСТРАЯ ИНТЕРПОЛЯЦИЯ
// =============================================================

// C.5. Интерполяция в корнях из единицы (DFT)
// Если x[i] = ωⁱ, то это DFT (упрощённо — наивно)
// Полная реализация — через FFT (раздел VII, g)

// --- C.6. Многоточечное вычисление через дерево произведений: O(n log² n) ---
// Строим дерево: tree[k] = ∏(x − xᵢ) для подмножества точек.
// Рекурсивно вычисляем P mod tree[k] в каждой вершине.
// В листьях: P mod (x − xᵢ) = P(xᵢ).
// Все вычисления по модулю `mod`.

// Вспомогательная функция: вычисление P mod (x − a) по модулю.
// P(x) mod (x − a) = P(a) (теорема Безу).
static long long poly_mod_linear(const Polynomial& P, long long a, long long mod) {
    long long result = 0;
    long long power = 1;
    for (int i = 0; i < (int)P.coeffs.size(); i++) {
        result = (result + (P[i] % mod) * power) % mod;
        power = power * a % mod;
    }
    return (result % mod + mod) % mod;
}

// Вспомогательная функция: деление P / (x − a) по модулю (monic линейный делитель).
// Используем схему Горнера: P(x) = (x − a) · Q(x) + P(a).
static Polynomial poly_div_linear(const Polynomial& P, long long a, long long mod) {
    int n = P.degree();
    if (n <= 0) return Polynomial({0});
    vector<long long> q(n, 0);
    q[n - 1] = P[n] % mod;
    for (int i = n - 2; i >= 0; i--)
        q[i] = (P[i + 1] + a * q[i + 1]) % mod;
    return Polynomial(q);
}

// Вспомогательная функция: умножение двух многочленов по модулю.
static Polynomial poly_mul_mod(const Polynomial& A, const Polynomial& B, long long mod) {
    int n = A.degree(), m = B.degree();
    if (n < 0 || m < 0) return Polynomial({0});
    int deg = n + m;
    vector<long long> res(deg + 1, 0);
    for (int i = 0; i <= n; i++)
        for (int j = 0; j <= m; j++)
            res[i + j] = (res[i + j] + (A[i] % mod) * (B[j] % mod)) % mod;
    return Polynomial(res);
}

// Вспомогательная функция: P mod M по модулю (general case).
static Polynomial poly_mod_general(const Polynomial& P, const Polynomial& M, long long mod) {
    Polynomial R(P);
    // Приводим коэффициенты к [0, mod)
    for (auto& c : R.coeffs) c = (c % mod + mod) % mod;
    int m = M.degree();
    if (m < 0) return Polynomial({0});
    // Инверсия старшего коэффициента M
    long long lc_inv = 1;
    long long lc = M[m] % mod;
    // Быстрое возведение в степень mod−2 (малый модуль — перебор; большой — бинарное)
    long long base = lc, exp = mod - 2;
    lc_inv = 1;
    while (exp > 0) {
        if (exp & 1) lc_inv = lc_inv * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    while (R.degree() >= m) {
        int d = R.degree();
        long long coeff = R[d] * lc_inv % mod;
        for (int j = 0; j <= m; j++)
            R.coeffs[d - m + j] = (R.coeffs[d - m + j] - coeff * (M[j] % mod) % mod + mod) % mod;
        R.normalize();
    }
    return R;
}

// --- C.6. Построение дерева произведений ---
// tree[0] = вектор полиномов ∏(x − xᵢ) по поддеревьям.
// Возвращает корневой полином дерева (∏(x − xᵢ) для всех точек).
Polynomial build_prod_tree(const vector<long long>& points, long long mod,
                           vector<vector<Polynomial>>& tree) {
    int n = (int)points.size();
    if (n == 0) return Polynomial({1});
    // Листья: (x − xᵢ)
    vector<Polynomial> leaves(n);
    for (int i = 0; i < n; i++)
        leaves[i] = Polynomial({(-points[i] % mod + mod) % mod, 1});
    tree.clear();
    tree.push_back(leaves);
    // Строим дерево снизу вверх
    while ((int)tree.back().size() > 1) {
        auto& prev = tree.back();
        vector<Polynomial> next;
        for (int i = 0; i < (int)prev.size(); i += 2) {
            if (i + 1 < (int)prev.size())
                next.push_back(poly_mul_mod(prev[i], prev[i + 1], mod));
            else
                next.push_back(prev[i]);
        }
        tree.push_back(next);
    }
    return tree.back()[0];
}

// --- C.6. Многоточечное вычисление через дерево: O(n log² n) ---
vector<long long> multi_point_eval_mod(const Polynomial& P, const vector<long long>& points, long long mod) {
    int n = (int)points.size();
    if (n == 0) return {};

    // Строим дерево произведений
    vector<vector<Polynomial>> tree;
    build_prod_tree(points, mod, tree);

    int height = (int)tree.size();
    // Рекурсивно: в каждой вершине вычисляем P mod tree[k][i]
    // Начинаем с корня: корневой полином = tree[height-1][0]
    // Проходим сверху вниз, вычисляя остатки
    vector<vector<Polynomial>> remainders(height);
    remainders[height - 1].push_back(P);  // P mod (корневой полином) = P (если deg P < deg корневого)

    for (int level = height - 1; level >= 1; level--) {
        int sz = (int)tree[level].size();
        for (int i = 0; i < sz; i++) {
            Polynomial R = remainders[level][i];
            int left = 2 * i, right = 2 * i + 1;
            if (left < (int)tree[level - 1].size()) {
                // Левый потомок: R mod tree[level-1][left]
                remainders[level - 1].push_back(poly_mod_general(R, tree[level - 1][left], mod));
            }
            if (right < (int)tree[level - 1].size()) {
                // Правый потомок: R mod tree[level-1][right]
                remainders[level - 1].push_back(poly_mod_general(R, tree[level - 1][right], mod));
            }
        }
    }

    // В листьях: remainders[0][i] — это P mod (x − xᵢ) = константа = P(xᵢ)
    vector<long long> result(n);
    for (int i = 0; i < n; i++) {
        if (i < (int)remainders[0].size()) {
            Polynomial& leaf = remainders[0][i];
            result[i] = leaf.degree() >= 0 ? (leaf[0] % mod + mod) % mod : 0;
        } else {
            result[i] = poly_mod_general(P, tree[0][i], mod)[0] % mod;
        }
    }
    return result;
}

// --- C.7. Быстрая интерполяция через divide-and-conquer: O(n log² n) ---
// Даны точки x[i] и значения y[i]. Возвращаем многочлен P степени < n.
// Алгоритм:
//   1. Разбиваем точки на две половины.
//   2. Рекурсивно интерполируем левую и правую половины → P_left, P_right.
//   3. Вычисляем M_left = ∏_{right} (x − xⱼ) по правым точкам.
//   4. Вычисляем вспомогательные значения v[i] = (y[i] − P_left(x[i])) / M_left(x[i]).
//   5. Интерполируем v по левым точкам → Q_left.
//   6. Результат: P_left + M_left · Q_left.
// Это O(n log² n) при использовании multi_point_eval_mod для вычисления значений.

Polynomial interpolate_mod_helper(const vector<long long>& x, const vector<long long>& y, long long mod) {
    int n = (int)x.size();
    if (n == 1) {
        // P(x) = y[0] (константа)
        return Polynomial({(y[0] % mod + mod) % mod});
    }
    int mid = n / 2;

    // Левая половина точек
    vector<long long> x_left(x.begin(), x.begin() + mid);
    vector<long long> y_left(y.begin(), y.begin() + mid);
    Polynomial P_left = interpolate_mod_helper(x_left, y_left, mod);

    // M_left(x) = ∏_{j=mid}^{n-1} (x − x[j]) — произведение по правым точкам
    Polynomial M_left({1});
    for (int j = mid; j < n; j++)
        M_left = poly_mul_mod(M_left, Polynomial({(-x[j] % mod + mod) % mod, 1}), mod);

    // Вычисляем M_left(x[i]) для левых точек
    vector<long long> M_left_vals = multi_point_eval_mod(M_left, x_left, mod);

    // Вычисляем P_left(x[i]) для левых точек
    vector<long long> P_left_vals = multi_point_eval_mod(P_left, x_left, mod);

    // Новые значения: v[i] = (y[i] − P_left(x[i])) · M_left(x[i])⁻¹
    vector<long long> v(mid);
    for (int i = 0; i < mid; i++) {
        long long num = (y[i] - P_left_vals[i]) % mod;
        // Обратный элемент: M_left_vals[i]^(mod−2)
        long long inv = 1, base = (M_left_vals[i] % mod + mod) % mod, exp = mod - 2;
        while (exp > 0) {
            if (exp & 1) inv = inv * base % mod;
            base = base * base % mod;
            exp >>= 1;
        }
        v[i] = (num % mod * inv % mod + mod) % mod;
    }

    // Интерполируем v по левым точкам
    Polynomial Q_left = interpolate_mod_helper(x_left, v, mod);

    // Результат: P_left + M_left · Q_left
    return (P_left + poly_mul_mod(M_left, Q_left, mod));
}

// --- C.7. Быстрая интерполяция: O(n log² n) ---
Polynomial interpolate_mod(const vector<long long>& x, const vector<long long>& y, long long mod) {
    int n = (int)x.size();
    assert(n == (int)y.size() && "Number of points must match");
    return interpolate_mod_helper(x, y, mod);
}

}; // struct Interpolation

#endif // ALGEBRA_F_CPP

// =============================================================
// Демонстрация
// =============================================================
#ifdef ALGEBRA_F_MAIN
int main() {
    using Poly = Interpolation::Polynomial;
    Interpolation interp;

    // Пример A.2: Лагранж
    vector<long long> x = {0, 1, 2};
    vector<long long> y = {1, 3, 7}; // P(x) = x² + x + 1
    Poly P = interp.lagrange(x, y);
    cout << "Lagrange: "; P.print(); cout << "\n";

    // Пример A.3: Ньютон
    Poly Q = interp.newton(x, y);
    cout << "Newton: "; Q.print(); cout << "\n";

    // Проверка
    cout << "P(0) = " << interp.horner(P, 0) << " (ожидаем 1)\n";
    cout << "P(1) = " << interp.horner(P, 1) << " (ожидаем 3)\n";
    cout << "P(2) = " << interp.horner(P, 2) << " (ожидаем 7)\n";

    return 0;
}
#endif // ALGEBRA_F_MAIN
