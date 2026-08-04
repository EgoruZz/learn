#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;
typedef long long ll;

// =============================================================
// E. СПЕЦИАЛЬНЫЕ АЛГОРИТМЫ ДЛЯ ФАКТОРИАЛОВ
// =============================================================
// Содержит:
//   A. Степень делителя в факториале     legendre_formula, trailing_zeros, kummer, smallest_n_for_factorial
//   B. Факториал по модулю               factorial_mod, factorial_mod_clean
//   C. Троичная сбалансированная система  to_balanced_ternary, from_balanced_ternary
//   D. Факториальная система счисления   to_factorial_base, from_factorial_base

struct FactorialAlgorithms {

// =============================================================
// A. СТЕПЕНЬ ДЕЛИТЕЛЯ В ФАКТОРИАЛЕ
// =============================================================
// Формула Лежандра: v_p(n!) = ⌊n/p⌋ + ⌊n/p²⌋ + ⌊n/p³⌋ + ...
//
// Сложность: O(log_p n)
// Пример: legendre_formula(100, 5) = 20 + 4 + 0 = 24

ll legendre_formula(ll n, ll p) {
    ll res = 0;
    for (ll pk = p; pk <= n; pk *= p)
        res += n / pk;
    return res;
}

// Количество нулей в конце n! в десятичной системе = v₅(n!)
ll trailing_zeros(ll n) {
    return legendre_formula(n, 5);
}

// --- A. Теорема Кюммера: v_p(C(n,k)) = количество переносов при k+(n-k) в base p ---
// Сложность: O(log_p n)
ll kummer(ll n, ll k, ll p) {
    ll carries = 0;
    ll carry = 0;
    ll nk = n - k;
    while (k > 0 || nk > 0) {
        ll sum = k % p + nk % p + carry;
        if (sum >= p) {
            carries++;
            carry = 1;
        } else {
            carry = 0;
        }
        k /= p;
        nk /= p;
    }
    return carries;
}

// --- A. Обратная задача: наименьший n: m | n! ---
// Для каждого p^e в разложении m ищем最小ое n: v_p(n!) ≥ e
// Сложность: O(log²m) на простой делитель
ll smallest_n_for_factorial(ll m) {
    if (m <= 1) return 0;
    ll result = 0;
    for (ll p = 2; p * p <= m; p++) {
        if (m % p == 0) {
            ll e = 0;
            while (m % p == 0) { m /= p; e++; }
            ll lo = 0, hi = e * p;
            while (lo < hi) {
                ll mid = (lo + hi) / 2;
                if (legendre_formula(mid, p) >= e) hi = mid;
                else lo = mid + 1;
            }
            result = max(result, lo);
        }
    }
    if (m > 1) {
        result = max(result, m);
    }
    return result;
}

// =============================================================
// B. ФАКТОРИАЛ ПО МОДУЛЮ
// =============================================================

// --- B.1. Наивный: n! mod p за O(n) ---
ll factorial_mod(ll n, ll p) {
    ll res = 1;
    for (ll i = 2; i <= n; i++)
        res = res * i % p;
    return res;
}

// --- B.2. n! mod p без кратных p (рекурсивно) ---
// Идея: n! = p^v · F(n) (mod p), где F(n) — произведение "чистых" элементов
// F(n) = F(⌊n/p⌋) · (∏ᵢ₌₁^{p-1} i)^{⌊n/p⌋} · (∏ᵢ₌₁^{n mod p} i) (mod p)
//
// Сложность: O(p · log_p n)
ll factorial_mod_clean(ll n, ll p) {
    if (n == 0) return 1;
    ll res = factorial_mod_clean(n / p, p);

    // Произведение 1·2·...·(p-1) mod p = -1 mod p (теорема Вильсона)
    for (ll i = (n % p) + 1; i < p; i++)
        res = res * i % p;

    // ⌊n/p⌋ раз умножаем на (p-1)! mod p = -1
    if ((n / p) % 2 == 1)
        res = p - res; // умножение на -1

    return res;
}

// --- B.3. n! mod p (полное, с учётом кратных p) ---
// Возвращает {v_p(n!), n! / p^v mod p} — степень и остаток без кратных p
pair<ll, ll> factorial_mod_full(ll n, ll p) {
    ll v = legendre_formula(n, p);
    ll clean = factorial_mod_clean(n, p);
    return {v, clean};
}

// =============================================================
// C. ТРОИЧНАЯ СБАЛАНСИРОВАННАЯ СИСТЕМА СЧИСЛЕНИЯ
// =============================================================
// Цифры: T(-1), 0, 1
// Каждое целое имеет唯一ное представление

// --- C.1. Перевод в троичную сбалансированную ---
// Алгоритм: делим на 3, остаток 2 → цифра T, перенос +1
// Для отрицательных: остаток -2 → цифра 1, перенос -1
vector<int> to_balanced_ternary(ll n) {
    if (n == 0) return {0};
    vector<int> digits;
    while (n > 0 || n < 0) {
        ll r = n % 3;
        if (r == 0) {
            digits.push_back(0);
            n /= 3;
        } else if (r == 1 || r == -2) {
            digits.push_back(1);
            n = (n - 1) / 3;
        } else { // r == 2 || r == -1
            digits.push_back(-1); // T
            n = (n + 1) / 3;
        }
    }
    reverse(digits.begin(), digits.end());
    return digits;
}

// --- C.2. Перевод из троичной сбалансированной ---
ll from_balanced_ternary(const vector<int>& digits) {
    ll res = 0, base = 1;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
        res += digits[i] * base;
        base *= 3;
    }
    return res;
}

// --- C.3. Вывод в виде строки ---
string to_string_bt(const vector<int>& digits) {
    string s;
    for (int d : digits) {
        if (d == -1) s += "T";
        else s += to_string(d);
    }
    return s;
}

// =============================================================
// D. ФАКТОРИАЛЬНАЯ СИСТЕМА СЧИСЛЕНИЯ
// =============================================================
// Цифры: d_i ∈ [0, i], представление: n = Σ d_i · i!
// Каждое неотрицательное целое имеет уникальное представление

// --- D.1. Перевод в факториальную систему ---
// Алгоритм: делим依次 на 1, 2, 3, ... и собираем остатки
vector<int> to_factorial_base(ll n) {
    if (n == 0) return {0};
    vector<int> digits;
    for (int i = 1; n > 0; i++) {
        digits.push_back(n % i);
        n /= i;
    }
    reverse(digits.begin(), digits.end());
    return digits;
}

// --- D.2. Перевод из факториальной системы ---
ll from_factorial_base(const vector<int>& digits) {
    ll res = 0, fact = 1;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
        res += digits[i] * fact;
        fact *= (digits.size() - i);
    }
    return res;
}

// --- D.3. Вывод в виде строки ---
string to_string_fb(const vector<int>& digits) {
    string s;
    for (int i = 0; i < (int)digits.size(); i++) {
        if (i > 0) s += "!";
        s += to_string(digits[i]);
    }
    return s;
}

}; // конец struct FactorialAlgorithms

// =============================================================
// ТЕСТЫ
// =============================================================
#ifndef FACTORIAL_ALGORITHMS_STANDALONE
signed main() {
    FactorialAlgorithms fa;

    cout << "=== A. Степень делителя в факториале ===" << endl;
    cout << "legendre_formula(100, 5) = " << fa.legendre_formula(100, 5) << " (ожид: 24)" << endl;
    cout << "legendre_formula(10, 2) = " << fa.legendre_formula(10, 2) << " (ожид: 8)" << endl;
    cout << "trailing_zeros(100) = " << fa.trailing_zeros(100) << " (ожид: 24)" << endl;
    cout << "trailing_zeros(10) = " << fa.trailing_zeros(10) << " (ожид: 2)" << endl;

    cout << "\n=== A. Теорема Кюммера ===" << endl;
    cout << "kummer(10, 3, 2) = " << fa.kummer(10, 3, 2) << " (ожид: 3, v_2(C(10,3))=v_2(120)=3)" << endl;
    cout << "kummer(10, 5, 2) = " << fa.kummer(10, 5, 2) << " (ожид: 2, v_2(C(10,5))=v_2(252)=2)" << endl;

    cout << "\n=== A. Обратная задача ===" << endl;
    cout << "smallest_n_for_factorial(100) = " << fa.smallest_n_for_factorial(100) << " (ожид: 10)" << endl;
    cout << "smallest_n_for_factorial(12) = " << fa.smallest_n_for_factorial(12) << " (ожид: 4)" << endl;
    cout << "smallest_n_for_factorial(6) = " << fa.smallest_n_for_factorial(6) << " (ожид: 3)" << endl;

    cout << "\n=== B. Факториал по модулю ===" << endl;
    cout << "factorial_mod(5, 7) = " << fa.factorial_mod(5, 7) << " (ожид: 120%7=1)" << endl;
    cout << "factorial_mod_clean(5, 7) = " << fa.factorial_mod_clean(5, 7) << endl;
    auto [v, clean] = fa.factorial_mod_full(100, 7);
    cout << "factorial_mod_full(100, 7): v=" << v << " clean=" << clean << endl;

    cout << "\n=== C. Троичная сбалансированная ===" << endl;
    vector<pair<ll, string>> tests = {{5, "1TT"}, {7, "1T1"}, {-5, "T11"}, {0, "0"}, {1, "1"}, {-1, "T"}, {2, "1T"}, {-7, "T1T"}};
    for (auto [n, expected] : tests) {
        auto digits = fa.to_balanced_ternary(n);
        string s = fa.to_string_bt(digits);
        ll back = fa.from_balanced_ternary(digits);
        cout << n << " → " << s << " (ожид: " << expected << ") → back=" << back
             << (back == n ? " OK" : " FAIL") << endl;
    }

    cout << "\n=== D. Факториальная система счисления ===" << endl;
    vector<ll> fact_tests = {0, 1, 2, 5, 10, 46, 100};
    for (ll n : fact_tests) {
        auto digits = fa.to_factorial_base(n);
        string s = fa.to_string_fb(digits);
        ll back = fa.from_factorial_base(digits);
        cout << n << " → " << s << " → back=" << back
             << (back == n ? " OK" : " FAIL") << endl;
    }
}
#endif
