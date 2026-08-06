#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <numeric>
using namespace std;
typedef long long ll;

// Подключаем e.cpp (наследует b.cpp → a.cpp)
#define PRIME_NUMBERS_STANDALONE
#include "../number-theory/e-folder/e.cpp"

// Подключаем c.cpp (DigitOps: sum_digits_base и др.)
#define DIGIT_OPS_MAIN
#include "../number-theory/c-folder/c.cpp"

// =============================================================
// G. СПЕЦИАЛЬНЫЕ ЧИСЛА
// =============================================================
// Наследует PrimeNumbers из e.cpp (→ ModArithmetic → Divisibility)
// Из a.cpp: sigma_sum, divide, factorize, linear_sieve, mu
// Из b.cpp: powmod, modinv
// Из c.cpp: sum_digits_base, is_perfect_square (DigitOps)
// Из e.cpp: is_prime, factorize_rho, lucas_lehmer
//
// Содержит:
//   A. Совершенные, избыточные, недостаточные, странные
//   B. Практические числа (Stewart)
//   C. Дружественные и социабельные
//   D. Числа Кармайкла (Korselt)
//   E. Гладкие числа (smooth, Hamming)
//   F. Свойства цифр (Харшад, Смит, атоморфные, Капрейар)
//   G. Треугольные и проничные
//   H. Мерсенна и Ферма
//   I. Счастливые
//   J. Кейта
//   K. Нарциссические
//   L. Дополнительные (тетраэдральные, центрированные)

struct SpecialNumbers : PrimeNumbers, DigitOps {

    // =========================================================
    // A. КЛАССИФИКАЦИЯ ПО СУММЕ ДЕЛИТЕЛЕЙ
    // =========================================================

    // Сумма собственных делителей: s(n) = σ(n) - n
    ll aliquot_sum(ll n) {
        if (n <= 1) return 0;
        auto spf = linear_sieve(n);
        return sigma_sum(n, spf) - n;
    }

    // Совершенное число: s(n) = n, или σ(n) = 2n
    bool is_perfect(ll n) {
        return aliquot_sum(n) == n;
    }

    // Избыточное: s(n) > n, или σ(n) > 2n
    bool is_abundant(ll n) {
        return aliquot_sum(n) > n;
    }

    // Недостаточное: s(n) < n
    bool is_deficient(ll n) {
        return aliquot_sum(n) < n;
    }

    // Проверка: n представимо как сумма различных собственных делителей (semi-perfect)
    bool is_semiperfect(ll n) {
        if (n <= 0) return false;
        auto divs = divide(n);
        // divide() возвращает делители в zigzag порядке; берём все кроме n
        vector<ll> props;
        for (ll d : divs) {
            if (d < n) props.push_back(d);
        }
        int k = props.size();
        if (k == 0) return false;
        // Битовая маска: перебираем все подмножества
        int total = 1 << k;
        for (int mask = 1; mask < total; mask++) {
            ll s = 0;
            for (int i = 0; i < k; i++) {
                if (mask & (1 << i)) {
                    s += props[i];
                    if (s > n) break; // early exit
                }
            }
            if (s == n) return true;
        }
        return false;
    }

    // Странные числа: избыточные, но не semi-perfect
    bool is_weird(ll n) {
        return is_abundant(n) && !is_semiperfect(n);
    }

    // Все избыточные числа ≤ limit
    vector<ll> abundant_numbers(ll limit) {
        vector<ll> res;
        for (ll i = 12; i <= limit; i++) {
            if (is_abundant(i)) res.push_back(i);
        }
        return res;
    }

    // Все недостаточные числа ≤ limit
    vector<ll> deficient_numbers(ll limit) {
        vector<ll> res;
        for (ll i = 1; i <= limit; i++) {
            if (is_deficient(i)) res.push_back(i);
        }
        return res;
    }

    // =========================================================
    // B. ПРАКТИЧЕСКИЕ ЧИСЛА
    // =========================================================

    // Критерий Стюарта: n = p₁^a₁ · p₂^a₂ · ... · pₖ^aₖ (primes asc)
    // practical ⟺ p₁ = 2 и ∀ i ≥ 2: pᵢ ≤ 1 + σ(p₁^a₁ · ... · pᵢ₋₁^aᵢ₋₁)
    bool is_practical(ll n) {
        if (n <= 0) return false;
        if (n == 1) return true;
        vector<ll> primes = factorize_rho(n);
        sort(primes.begin(), primes.end());
        // factorize_rho возвращает отсортированный список простых (с повторами)
        // Группируем: (prime, power)
        vector<pair<ll,ll>> pp;
        for (ll p : primes) {
            if (pp.empty() || pp.back().first != p) {
                pp.push_back({p, 1});
            } else {
                pp.back().second++;
            }
        }
        // Первый простой должен быть 2
        if (pp[0].first != 2) return false;
        // σ накопленного произведения
        ll sigma_acc = 1; // σ(2^a₁) = 2^(a₁+1) - 1
        ll pa = 1;
        for (int t = 0; t <= pp[0].second; t++) pa *= 2;
        sigma_acc = pa - 1; // (2^{a₁+1} - 1)/(2-1)
        for (int i = 1; i < (int)pp.size(); i++) {
            if (pp[i].first > sigma_acc + 1) return false;
            // Обновляем σ: σ(acc · p_i^{a_i}) = σ(acc) · σ(p_i^{a_i})
            ll pi = pp[i].first, ai = pp[i].second;
            ll pai = 1;
            for (int t = 0; t <= ai; t++) pai *= pi;
            sigma_acc *= (pai - 1) / (pi - 1);
        }
        return true;
    }

    // Все practical числа ≤ limit (перебор с критерием Стюарта)
    vector<ll> practical_numbers(ll limit) {
        vector<ll> res;
        for (ll i = 1; i <= limit; i++) {
            if (is_practical(i)) res.push_back(i);
        }
        return res;
    }

    // =========================================================
    // C. ДРУЖЕСТВЕННЫЕ И СОЦИАБЕЛЬНЫЕ
    // =========================================================

    // Дружественная пара: s(a) = b и s(b) = a, a ≠ b
    bool is_amicable(ll a, ll b) {
        return a != b && aliquot_sum(a) == b && aliquot_sum(b) == a;
    }

    // Социабельная цепочка длины k: s(a₁)=a₂, ..., s(aₖ)=a₁
    // Возвращает цепочку или пустой вектор
    vector<ll> sociable_chain(ll start, int max_len) {
        vector<ll> chain;
        unordered_set<ll> visited;
        ll cur = start;
        for (int i = 0; i < max_len; i++) {
            chain.push_back(cur);
            if (visited.count(cur) && chain.front() == cur) {
                // Проверяем что цикл замкнулся на старте
                if (chain.size() >= 3) return chain;
                break;
            }
            if (visited.count(cur)) break;
            visited.insert(cur);
            cur = aliquot_sum(cur);
        }
        return {};
    }

    // =========================================================
    // D. ЧИСЛА КАРМАЙКЛА
    // =========================================================

    // Критерий Корсельта: нечётное, squarefree, ∀ p|n: (p-1)|(n-1)
    bool is_carmichael(ll n) {
        if (n < 2 || n % 2 == 0) return false;
        // factorize_rho возвращает простые с повторами → проверяем squarefree
        auto factors = factorize_rho(n);
        sort(factors.begin(), factors.end());
        // Проверяем squarefree: нет повторяющихся простых
        for (int i = 1; i < (int)factors.size(); i++) {
            if (factors[i] == factors[i-1]) return false;
        }
        // Каждый p|n: (p-1)|(n-1)
        for (ll p : factors) {
            if ((n - 1) % (p - 1) != 0) return false;
        }
        return true;
    }

    // Все числа Кармайкла ≤ limit (перебор нечётных составных)
    vector<ll> carmichael_numbers(ll limit) {
        vector<ll> res;
        for (ll n = 561; n <= limit; n += 2) {
            if (!is_prime(n) && is_carmichael(n)) res.push_back(n);
        }
        return res;
    }

    // =========================================================
    // E. ГЛАДКИЕ ЧИСЛА
    // =========================================================

    // Проверка: n — B-smooth (все простые делители ≤ B)
    bool is_smooth(ll n, ll B) {
        if (n <= 1) return true;
        vector<ll> fac = factorize_rho(n);
        for (ll p : fac) {
            if (p > B) return false;
        }
        return true;
    }

    // Генерация всех B-smooth ≤ limit через 3 указателя (B=2,3,5: Hamming)
    vector<ll> hamming_numbers(ll limit) {
        vector<ll> res;
        for (ll i2 = 1; i2 <= limit; i2 *= 2) {
            for (ll i3 = i2; i3 <= limit; i3 *= 3) {
                for (ll i5 = i3; i5 <= limit; i5 *= 5) {
                    res.push_back(i5);
                }
            }
        }
        sort(res.begin(), res.end());
        return res;
    }

    // Генерация всех P-smooth ≤ limit (общий алгоритм через min-heap)
    vector<ll> smooth_numbers(ll limit, ll B) {
        vector<ll> res;
        set<ll> heap;
        heap.insert(1);
        while (!heap.empty()) {
            ll x = *heap.begin();
            heap.erase(heap.begin());
            if (x > limit) break;
            res.push_back(x);
            // Получаем простые делители B
            auto spf = linear_sieve(B);
            for (ll p = 2; p <= B; p++) {
                if (spf[p] == p && x <= limit / p) {
                    heap.insert(x * p);
                }
            }
        }
        sort(res.begin(), res.end());
        res.erase(unique(res.begin(), res.end()), res.end());
        return res;
    }

    // =========================================================
    // F. СВОЙСТВА ЦИФР
    // =========================================================

    // Харшад (Нивен): n | S(n)
    bool is_harshad(ll n) {
        if (n <= 0) return false;
        ll sd = sum_digits_base(n, 10);
        return sd > 0 && n % sd == 0;
    }

    // Число Смита: sum_digits(n) = sum_digits всех простых делителей с кратностями
    bool is_smith(ll n) {
        if (n <= 9) return false; // однозначные — по определению не Смит
        ll target = sum_digits_base(n, 10);
        vector<ll> factors = factorize_rho(n);
        ll fact_sum = 0;
        for (ll p : factors) {
            fact_sum += sum_digits_base(p, 10);
        }
        return target == fact_sum;
    }

    // Атоморфное: n² "заканчивается" на n, т.е. n² ≡ n (mod 10^d)
    bool is_automorphic(ll n) {
        if (n <= 0) return false;
        ll n2 = n * (ll)n;
        ll d = to_string(n).size();
        ll mod = 1;
        for (int i = 0; i < d; i++) mod *= 10;
        return n2 % mod == n;
    }

    // Число Капрейара: n² = A·10^d + B, A + B = n
    bool is_kaprekar(ll n) {
        if (n <= 0) return false;
        ll n2 = n * (ll)n;
        string s = to_string(n2);
        int len = s.size();
        for (int i = 1; i < len; i++) {
            ll a = stoll(s.substr(0, i));
            ll b = stoll(s.substr(i));
            if (a + b == n && b != 0) return true; // b != 0 чтобы избежать тривиальных
        }
        return n == 1; // единица — тривиальный Капрейар
    }

    // =========================================================
    // G. ТРЕУГОЛЬНЫЕ И ПРОНИЧНЫЕ
    // =========================================================

    // Треугольное: n = k(k+1)/2 → 8n+1 = (2k+1)²
    bool is_triangular(ll n) {
        if (n <= 0) return false;
        return is_perfect_square(8 * n + 1);
    }

    // Проничное: n = k(k+1) → 4n+1 = (2k+1)²
    bool is_pronic(ll n) {
        if (n <= 0) return false;
        return is_perfect_square(4 * n + 1);
    }

    // k-е треугольное число
    ll triangular(ll k) {
        return k * (k + 1) / 2;
    }

    // k-е проничное число
    ll pronic(ll k) {
        return k * (k + 1);
    }

    // =========================================================
    // H. МЕРСЕННА И ФЕРМА
    // =========================================================

    // Простое Мерсенна: M_p = 2^p - 1
    bool is_mersenne_prime(ll p) {
        if (p == 2) return true; // M_2 = 3 — простое
        if (!is_prime(p)) return false;
        return lucas_lehmer(p);
    }

    // Число Ферма: F_n = 2^(2^n) + 1
    ll fermat_number(int n) {
        ll res = 2;
        for (int i = 0; i < n; i++) {
            res = res * res; // overflow при n ≥ 5, но F₅ — уже составное
        }
        return res + 1;
    }

    // =========================================================
    // I. СЧАСТЛИВЫЕ ЧИСЛА
    // =========================================================

    // Одна итерация: сумма квадратов цифр
    ll digit_square_sum(ll n) {
        ll s = 0;
        while (n > 0) {
            ll d = n % 10;
            s += d * d;
            n /= 10;
        }
        return s;
    }

    // Счастливое: итеративная сумма квадратов → 1
    bool is_happy(ll n) {
        unordered_set<ll> visited;
        while (n != 1 && !visited.count(n)) {
            visited.insert(n);
            n = digit_square_sum(n);
        }
        return n == 1;
    }

    // =========================================================
    // J. ЧИСЛА КЕЙТА
    // =========================================================

    // Число Кейта: n появляется в последовательности, начинающейся с цифр n
    bool is_keith(ll n) {
        if (n < 10) return false;
        vector<int> digits;
        ll tmp = n;
        while (tmp > 0) {
            digits.push_back(tmp % 10);
            tmp /= 10;
        }
        reverse(digits.begin(), digits.end());
        int k = digits.size();
        unordered_set<ll> seq(digits.begin(), digits.end());
        while (true) {
            ll next = 0;
            for (int i = (int)digits.size() - k; i < (int)digits.size(); i++) {
                next += digits[i];
            }
            if (next == n) return true;
            if (next > n) return false; // последовательность монотонно возрастает
            digits.push_back(next);
        }
    }

    // =========================================================
    // K. НАРЦИССИЧЕСКИЕ ЧИСЛА (ARMSTRONG)
    // =========================================================

    // Нарциссическое: Σ(цифраᵢ)^d = n, где d — количество цифр
    bool is_narcissistic(ll n) {
        if (n <= 0) return false;
        vector<int> digits;
        ll tmp = n;
        while (tmp > 0) {
            digits.push_back(tmp % 10);
            tmp /= 10;
        }
        int d = digits.size();
        ll sum = 0;
        for (int dig : digits) {
            sum += powmod(dig, d, (ll)1e18); // точное возведение
            if (sum > n) return false; // early exit
        }
        return sum == n;
    }

    // =========================================================
    // L. ДОПОЛНИТЕЛЬНЫЕ
    // =========================================================

    // Тетраэдральное число: Te(n) = n(n+1)(n+2)/6
    ll tetrahedral(ll n) {
        return (ll)n * (n + 1) * (n + 2) / 6;
    }

    // Проверка: n — тетраэдральное
    bool is_tetrahedral(ll n) {
        // Бинарный поиск по k: k(k+1)(k+2)/6 = n
        ll lo = 1, hi = 2600; // Te(2600) > 10^10
        while (lo <= hi) {
            ll mid = (lo + hi) / 2;
            ll val = mid * (mid + 1) * (mid + 2) / 6;
            if (val == n) return true;
            if (val < n) lo = mid + 1;
            else hi = mid - 1;
        }
        return false;
    }

    // Центрированное треугольное: 3n(n-1)/2 + 1
    ll centered_triangular(ll n) {
        return 3 * n * (n - 1) / 2 + 1;
    }

    // Центрированное квадратное: n² + (n-1)²
    ll centered_square(ll n) {
        return n * n + (n - 1) * (n - 1);
    }

    // Центрированное пятиугольное: (5n² - 5n + 2)/2
    ll centered_pentagonal(ll n) {
        return (5 * n * n - 5 * n + 2) / 2;
    }
};

// =============================================================
// MAIN — ДЕМОНСТРАЦИЯ
// =============================================================
#ifndef SPECIAL_NUMBERS_STANDALONE
signed main() {
    SpecialNumbers sp;

    cout << "=== A. КЛАССИФИКАЦИЯ ПО СУММЕ ДЕЛИТЕЛЕЙ ===" << endl;
    for (ll n : {6, 28, 12, 8, 70}) {
        cout << n << ": perfect=" << sp.is_perfect(n)
             << " abundant=" << sp.is_abundant(n)
             << " deficient=" << sp.is_deficient(n)
             << " semiperfect=" << sp.is_semiperfect(n)
             << " weird=" << sp.is_weird(n) << endl;
    }

    cout << "\n=== B. ПРАКТИЧЕСКИЕ ЧИСЛА ===" << endl;
    auto pract = sp.practical_numbers(50);
    cout << "Practical ≤ 50: ";
    for (ll x : pract) cout << x << " ";
    cout << endl;

    cout << "\n=== C. ДРУЖЕСТВЕННЫЕ ЧИСЛА ===" << endl;
    cout << "220-284 friendly: " << sp.is_amicable(220, 284) << endl;
    cout << "1184-1210 friendly: " << sp.is_amicable(1184, 1210) << endl;
    cout << "12-14 not friendly: " << sp.is_amicable(12, 14) << endl;

    cout << "\n=== D. ЧИСЛА КАРМАЙКЛА ===" << endl;
    for (ll n : {561, 1105, 1729, 2465, 2821, 6601, 8911}) {
        cout << n << ": " << (sp.is_carmichael(n) ? "YES" : "NO") << endl;
    }

    cout << "\n=== E. ГЛАДКИЕ ЧИСЛА ===" << endl;
    cout << "60 is 5-smooth: " << sp.is_smooth(60, 5) << endl;
    cout << "42 is 7-smooth: " << sp.is_smooth(42, 7) << endl;
    auto ham = sp.hamming_numbers(100);
    cout << "Hamming ≤ 100: ";
    for (ll x : ham) cout << x << " ";
    cout << endl;

    cout << "\n=== F. СВОЙСТВА ЦИФР ===" << endl;
    for (ll n : {18, 20, 100, 4, 22, 27, 376, 625, 45, 297}) {
        cout << n << ": harshad=" << sp.is_harshad(n)
             << " smith=" << sp.is_smith(n)
             << " automorphic=" << sp.is_automorphic(n)
             << " kaprekar=" << sp.is_kaprekar(n) << endl;
    }

    cout << "\n=== G. ТРЕУГОЛЬНЫЕ И ПРОНИЧНЫЕ ===" << endl;
    for (ll n : {1, 3, 6, 10, 15, 2, 6, 12, 20, 30}) {
        cout << n << ": triangular=" << sp.is_triangular(n)
             << " pronic=" << sp.is_pronic(n) << endl;
    }

    cout << "\n=== H. МЕРСЕННА И ФЕРМА ===" << endl;
    for (ll p : {2, 3, 5, 7, 11, 13, 17, 19}) {
        cout << "M_" << p << " = " << ((1LL << p) - 1)
             << " prime: " << (sp.is_mersenne_prime(p) ? "YES" : "NO") << endl;
    }
    for (int n = 0; n <= 4; n++) {
        cout << "F_" << n << " = " << sp.fermat_number(n) << endl;
    }

    cout << "\n=== I. СЧАСТЛИВЫЕ ЧИСЛА ===" << endl;
    for (ll n : {1, 7, 10, 13, 19, 23, 28, 2, 3, 4, 5}) {
        cout << n << " happy: " << sp.is_happy(n) << endl;
    }

    cout << "\n=== J. ЧИСЛА КЕЙТА ===" << endl;
    for (ll n : {14, 19, 28, 47, 54, 61, 64, 86, 104, 133}) {
        cout << n << " keith: " << sp.is_keith(n) << endl;
    }

    cout << "\n=== K. НАРЦИССИЧЕСКИЕ ЧИСЛА ===" << endl;
    for (ll n : {1, 153, 370, 371, 407, 1634, 8208, 9474, 54748}) {
        cout << n << " narcissistic: " << sp.is_narcissistic(n) << endl;
    }

    cout << "\n=== L. ДОПОЛНИТЕЛЬНЫЕ ===" << endl;
    for (ll n : {1, 4, 10, 20, 35, 56, 84, 120}) {
        cout << "Te: " << n << " tetrahedral=" << sp.is_tetrahedral(n) << endl;
    }
    for (int n = 1; n <= 5; n++) {
        cout << "Centered: n=" << n
             << " tri=" << sp.centered_triangular(n)
             << " sq=" << sp.centered_square(n)
             << " pent=" << sp.centered_pentagonal(n) << endl;
    }

    return 0;
}
#endif
