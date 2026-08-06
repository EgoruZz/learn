#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
using namespace std;

// =============================================================
// C. СВОЙСТВА ЧИСЕЛ: ЦИФРОВЫЕ ОПЕРАЦИИ
// =============================================================
// Структура md: A. Делимость через сумму цифр → B. Целочисленный корень
//               → D. Манипуляции с цифрами → E. Цифровой корень
//               → F. Переносы и влияние на сумму цифр
//
// Содержит:
//   A. Делимость: sum_digits, alt_sum, weighted_sum, print_divisibility_table,
//                  verify_divisibility_rule
//   B. Целочисленный корень: isqrt_newton, kth_root_newton,
//                            is_perfect_square, is_perfect_power
//   C. Манипуляции: get_digit, remove_digit, insert_digit, reverse, count_digits
//   D. Цифровой корень: digital_root_base, digital_root_iterative
//   E. Переносы: count_carry_sum, count_carry_product

struct DigitOps {

// =============================================================
// A. ДЕЛИМОСТЬ ЧЕРЕЗ СУММУ ЦИФР
// =============================================================
// Фундаментальное наблюдение: если b ≡ r (mod m), то b^k ≡ r^k (mod m)
// → n mod m = (a₀·r⁰ + a₁·r¹ + a₂·r² + ...) mod m

// --- A.1. Сумма цифр числа n в base b ---
// Свойство: n mod (b-1) = S_b(n) mod (b-1)
// Почему: b ≡ 1 (mod b-1), поэтому b^k ≡ 1 (mod b-1)
//
// Сложность: O(log_b n)
int sum_digits_base(long long n, int b) {
    int s = 0;
    while (n > 0) s += n % b, n /= b;
    return s;
}

// --- A.2. Таблица: b mod m для m от 1 до 2b ---
// Ключевые случаи:
//   m = b-k: r = k → n mod m = Σ aᵢ·k^i mod m
//   m = b+k: r = -k → n mod m = Σ aᵢ·(-k)^i mod m
//   m = b: r = 0 → n mod m = a₀ mod m
void print_divisibility_table(int b) {
    cout << "m:   ";
    for (int m = 1; m <= 2 * b; m++) cout << m << "\t";
    cout << endl;
    cout << "r:   ";
    for (int m = 1; m <= 2 * b; m++) {
        int r = b % m;
        if (r > m / 2) r -= m;
        cout << r << "\t";
    }
    cout << endl;
}

// --- A.3. Чередующаяся сумма цифр в base b (для m = b+k) ---
// Формула: alt_sum_b(n) = a₀ - a₁ + a₂ - a₃ + ... (с младшего разряда)
// Свойство: n mod (b+1) = alt_sum_b(n) mod (b+1)
// Почему: b ≡ -1 (mod b+1), поэтому b^k ≡ (-1)^k (mod b+1)
//
// Сложность: O(log_b n)
int alt_sum_digits_base(long long n, int b) {
    int s = 0, sign = 1;
    while (n > 0) s += sign * (n % b), n /= b, sign *= -1;
    return s;
}

// --- A.4. Взвешенная сумма цифр (общий случай) ---
// Формула: weighted_sum(n, r, b) = a₀·r⁰ + a₁·r¹ + a₂·r² + ...
// Свойство: если b ≡ r (mod m), то n mod m = weighted_sum(n, r, b) mod m
//
// Частные случаи:
//   r = 1: weighted_sum = sum_digits (делимость на b-1)
//   r = -1: weighted_sum = alt_sum (делимость на b+1)
//   r = 2: weighted_sum = a₀ + 2a₁ + 4a₂ + ... (делимость на b-2)
//   r = -2: weighted_sum = a₀ - 2a₁ + 4a₂ - ... (делимость на b+2)
//
// Сложность: O(log_b n)
long long weighted_sum_base(long long n, int r, int b) {
    long long result = 0, power = 1;
    while (n > 0) {
        result += (n % b) * power;
        power *= r;
        n /= b;
    }
    return result;
}

// --- A.5. Проверка правила делимости для любого m ---
// Проверяет что weighted_sum_base(n, b % m, b) % m == n % m
//
// Сложность: O(log_b n)
bool verify_divisibility_rule(long long n, int m, int b) {
    int r = b % m;
    long long ws = weighted_sum_base(n, r, b);
    return ((ws % m) + m) % m == n % m;
}

// =============================================================
// B. ЦЕЛОЧИСЛЕННЫЙ КОРЕНЬ (МЕТОД НЬЮТОНА)
// =============================================================

// --- B.1. Целочисленный квадратный корень O(log log n) ---
// Формула: x_{k+1} = (x_k + n / x_k) / 2
//
// Почему работает: метод Ньютона для f(x) = x² - n
//   f'(x) = 2x → x_{k+1} = x_k - f(x_k)/f'(x_k) = (x_k + n/x_k)/2
// Сходность: квадратичная (число правильных цифр удваивается)
long long isqrt_newton(long long n) {
    if (n == 0) return 0;
    long long x = n;
    while (x * x > n) x = (x + n / x) / 2;
    return x;
}

// --- B.2. Целочисленный k-й корень O(k · log log n) ---
// Формула: x_{k+1} = ((k-1) * x_k + n / x_k^{k-1}) / k
//
// Почему работает: метод Ньютона для f(x) = x^k - n
long long kth_root_newton(long long n, int k) {
    if (n <= 1) return n;
    long long x = n;
    while (true) {
        long long xk1 = 1;
        for (int i = 0; i < k - 1; i++) {
            if (xk1 > n / x) { xk1 = n; break; }
            xk1 *= x;
        }
        long long xk = xk1 * x;
        if (xk <= n) break;
        x = ((k - 1) * x + n / xk1) / k;
    }
    return x;
}

// --- B.3. Проверка: является ли n квадратом O(log log n) ---
bool is_perfect_square(long long n) {
    long long r = isqrt_newton(n);
    return r * r == n;
}

// --- B.4. Проверка: является ли n k-й степенью O(k · log log n) ---
bool is_perfect_power(long long n, int k) {
    long long r = kth_root_newton(n, k);
    long long power = 1;
    for (int i = 0; i < k; i++) {
        if (power > n / r) return false;
        power *= r;
    }
    return power == n;
}

// =============================================================
// C. МАНИПУЛЯЦИИ С ЦИФРАМИ
// =============================================================

// --- C.1. Извлечение цифры по позиции в base b ---
// Формула: (n / b^k) % b
// k = 0 — единицы, k = 1 — "b-ицы", k = 2 — "b²-ки", ...
//
// Сложность: O(log_b n)
int get_digit_base(long long n, int k, int b) {
    for (int i = 0; i < k; i++) n /= b;
    return n % b;
}

// --- C.2. Удаление цифры по позиции в base b ---
// Формула: (n / b^{k+1}) * b^k + (n % b^k)
//
// Сложность: O(log_b n)
long long remove_digit_base(long long n, int k, int b) {
    long long bp = 1;
    for (int i = 0; i < k; i++) bp *= b;
    long long left = n / (bp * b);
    long long right = n % bp;
    return left * bp + right;
}

// --- C.3. Вставка цифры по позиции в base b ---
// Формула: (n / b^k) * b^{k+1} + d * b^k + (n % b^k)
//
// Сложность: O(log_b n)
long long insert_digit_base(long long n, int d, int k, int b) {
    long long bp = 1;
    for (int i = 0; i < k; i++) bp *= b;
    long long left = n / bp;
    long long right = n % bp;
    return left * bp * b + d * bp + right;
}

// --- C.4. Реверс числа в base b ---
// Свойство: reverse_b(reverse_b(n, b), b) = n (для чисел без ведущих нулей)
//
// Сложность: O(log_b n)
long long reverse_base(long long n, int b) {
    long long rev = 0;
    while (n > 0) rev = rev * b + n % b, n /= b;
    return rev;
}

// --- C.5. Количество цифр в base b ---
// Формула: ⌊log_b(n)⌋ + 1
//
// Сложность: O(log_b n)
int count_digits_base(long long n, int b) {
    if (n == 0) return 1;
    int cnt = 0;
    while (n > 0) n /= b, cnt++;
    return cnt;
}

// =============================================================
// D. ЦИФРОВОЙ КОРЕНЬ
// =============================================================

// --- D.1. Цифровой корень в base b (формула) O(1) ---
// Формула: dr_b(n) = 1 + (n-1) mod (b-1) для n > 0, dr_b(0) = 0
//
// Это результат итеративного сложения цифр до однозначного.
// Пример: 12345 → 15 → 6, dr(12345) = 6
//
// Свойства:
//   dr(a + b) = dr(dr(a) + dr(b))
//   dr(a · b) = dr(dr(a) · dr(b))
//   dr(dr(n)) = dr(n) (сходится за один шаг)
int digital_root_base(long long n, int b) {
    if (n == 0) return 0;
    return 1 + (n - 1) % (b - 1);
}

// --- D.2. Цифровой корень итеративно (для проверки формулы) ---
// Применяем sum_digits пока не получим однозначное число
//
// Сложность: O(log_b n · log_b n) в худшем случае
int digital_root_iterative(long long n, int b) {
    while (n >= b) n = sum_digits_base(n, b);
    return (int)n;
}

// =============================================================
// E. ПЕРЕНОСЫ И ИХ ВЛИЯНИЕ НА СУММУ ЦИФР
// =============================================================

// --- E.1. Подсчёт переносов при сложении a + b в base b ---
// Формула: S(a + b) = S(a) + S(b) - (b-1) · carry_count
//
// Почему: каждый перенос уменьшает сумму цифр на (b-1)
//   Без переноса: a + b = c, сумма цифр = c
//   С переносом: a + b = c + b·1, сумма цифр = c + 1
//   Разница: (a+b) - (c+1) = b-1
//
// Сложность: O(log_b max(a,b))
int count_carry_sum(long long a, long long b, int base) {
    int carries = 0;
    int carry = 0;
    while (a > 0 || b > 0 || carry > 0) {
        int sum = a % base + b % base + carry;
        if (sum >= base) carries++, carry = 1;
        else carry = 0;
        a /= base;
        b /= base;
    }
    return carries;
}

// --- E.2. Подсчёт переносов при умножении a * b в base b ---
//
// Сложность: O(log_b a · log_b b)
int count_carry_product(long long a, long long b, int base) {
    int total_carries = 0;
    long long aa = a;
    while (aa > 0) {
        long long bb = b;
        int inner_carry = 0;
        while (bb > 0) {
            int digit_a = aa % base;
            int digit_b = bb % base;
            int prod_digit = digit_a * digit_b + inner_carry;
            if (prod_digit >= base) total_carries++;
            inner_carry = prod_digit / base;
            bb /= base;
        }
        aa /= base;
    }
    return total_carries;
}

}; // конец struct DigitOps

// =============================================================
// ТЕСТОВЫЕ ВЫЗОВЫ
// =============================================================

#ifndef DIGIT_OPS_MAIN
signed main() {
    DigitOps digit;

    cout << "=== A. Делимость через сумму цифр ===" << endl;
    cout << "sum_digits_base(12345, 10) = " << digit.sum_digits_base(12345, 10) << endl;
    cout << "12345 mod 9 = " << 12345 % 9 << ", sum_digits mod 9 = " << digit.sum_digits_base(12345, 10) % 9 << endl;
    cout << "alt_sum_digits_base(12345, 10) = " << digit.alt_sum_digits_base(12345, 10) << endl;
    cout << "12345 mod 11 = " << 12345 % 11 << ", alt_sum mod 11 = " << (digit.alt_sum_digits_base(12345, 10) % 11 + 11) % 11 << endl;

    cout << "\n=== A.2. Таблица b mod m (base=10) ===" << endl;
    digit.print_divisibility_table(10);

    cout << "\n=== A.4. Взвешенная сумма (r=2, base 10) ===" << endl;
    cout << "weighted_sum_base(12345, 2, 10) = " << digit.weighted_sum_base(12345, 2, 10) << endl;
    cout << "12345 mod 8 = " << 12345 % 8 << ", weighted_sum mod 8 = " << (digit.weighted_sum_base(12345, 2, 10) % 8 + 8) % 8 << endl;

    cout << "\n=== A.4. Взвешенная сумма (r=-2, base 10) ===" << endl;
    cout << "weighted_sum_base(12345, -2, 10) = " << digit.weighted_sum_base(12345, -2, 10) << endl;
    cout << "12345 mod 12 = " << 12345 % 12 << ", weighted_sum mod 12 = " << (digit.weighted_sum_base(12345, -2, 10) % 12 + 12) % 12 << endl;

    cout << "\n=== A.5. Проверка правила делимости ===" << endl;
    cout << "base=10, n=12345:" << endl;
    for (int m = 2; m <= 20; m++)
        cout << "  mod " << m << ": " << (digit.verify_divisibility_rule(12345, m, 10) ? "OK" : "FAIL") << endl;

    cout << "\n=== A. В base 2 ===" << endl;
    cout << "sum_digits_base(42, 2) = " << digit.sum_digits_base(42, 2) << " (101010)" << endl;
    cout << "42 mod 3 = " << 42 % 3 << ", sum_digits mod 3 = " << digit.sum_digits_base(42, 2) % 3 << endl;

    cout << "\n=== B. Целочисленный корень ===" << endl;
    cout << "isqrt_newton(10) = " << digit.isqrt_newton(10) << endl;
    cout << "isqrt_newton(0) = " << digit.isqrt_newton(0) << endl;
    cout << "isqrt_newton(100) = " << digit.isqrt_newton(100) << endl;
    cout << "kth_root_newton(100, 3) = " << digit.kth_root_newton(100, 3) << endl;
    cout << "kth_root_newton(27, 3) = " << digit.kth_root_newton(27, 3) << endl;
    cout << "is_perfect_square(9) = " << digit.is_perfect_square(9) << endl;
    cout << "is_perfect_square(10) = " << digit.is_perfect_square(10) << endl;
    cout << "is_perfect_power(27, 3) = " << digit.is_perfect_power(27, 3) << endl;
    cout << "is_perfect_power(28, 3) = " << digit.is_perfect_power(28, 3) << endl;

    cout << "\n=== B. Проверка isqrt_newton ===" << endl;
    for (long long n = 0; n <= 1000000; n++) {
        long long r = digit.isqrt_newton(n);
        if (r * r > n || (r + 1) * (r + 1) <= n) {
            cout << "FAIL: n=" << n << " isqrt=" << r << endl;
        }
    }
    cout << "isqrt_newton — все 1000001 тестов пройдены" << endl;

    cout << "\n=== C. Манипуляции с цифрами ===" << endl;
    cout << "get_digit_base(12345, 2, 10) = " << digit.get_digit_base(12345, 2, 10) << endl;
    cout << "remove_digit_base(12345, 2, 10) = " << digit.remove_digit_base(12345, 2, 10) << endl;
    cout << "insert_digit_base(1245, 3, 2, 10) = " << digit.insert_digit_base(1245, 3, 2, 10) << endl;
    cout << "reverse_base(12345, 10) = " << digit.reverse_base(12345, 10) << endl;
    cout << "count_digits_base(12345, 10) = " << digit.count_digits_base(12345, 10) << endl;
    cout << "count_digits_base(12345, 2) = " << digit.count_digits_base(12345, 2) << endl;
    cout << "reverse_base(reverse_base(12345, 10), 10) = " << digit.reverse_base(digit.reverse_base(12345, 10), 10) << endl;

    cout << "\n=== D. Цифровой корень ===" << endl;
    cout << "digital_root_base(12345, 10) = " << digit.digital_root_base(12345, 10) << endl;
    cout << "digital_root_iterative(12345, 10) = " << digit.digital_root_iterative(12345, 10) << endl;
    cout << "digital_root_base(99, 10) = " << digit.digital_root_base(99, 10) << endl;
    cout << "digital_root_iterative(99, 10) = " << digit.digital_root_iterative(99, 10) << endl;
    cout << "digital_root_base(0, 10) = " << digit.digital_root_base(0, 10) << endl;

    cout << "\n=== D.3. Свойства dr(a+b) = dr(dr(a)+dr(b)) ===" << endl;
    for (int a = 1; a <= 20; a++) {
        for (int b = 1; b <= 20; b++) {
            int dr_ab = digit.digital_root_base(a + b, 10);
            int dr_dr = digit.digital_root_base(digit.digital_root_base(a, 10) + digit.digital_root_base(b, 10), 10);
            if (dr_ab != dr_dr) {
                cout << "FAIL: a=" << a << " b=" << b << " dr(a+b)=" << dr_ab << " dr(dr(a)+dr(b))=" << dr_dr << endl;
            }
        }
    }
    cout << "dr(a+b) = dr(dr(a)+dr(b)) — все 400 тестов пройдены" << endl;

    cout << "\n=== D.3. Свойства dr(a·b) = dr(dr(a)·dr(b)) ===" << endl;
    for (int a = 1; a <= 20; a++) {
        for (int b = 1; b <= 20; b++) {
            int dr_ab = digit.digital_root_base(a * b, 10);
            int dr_dr = digit.digital_root_base(digit.digital_root_base(a, 10) * digit.digital_root_base(b, 10), 10);
            if (dr_ab != dr_dr) {
                cout << "FAIL: a=" << a << " b=" << b << " dr(a*b)=" << dr_ab << " dr(dr(a)*dr(b))=" << dr_dr << endl;
            }
        }
    }
    cout << "dr(a·b) = dr(dr(a)·dr(b)) — все 400 тестов пройдены" << endl;

    cout << "\n=== D.3. Свойство dr(dr(n)) = dr(n) ===" << endl;
    for (int n = 1; n <= 200; n++) {
        int dr_n = digit.digital_root_base(n, 10);
        int dr_dr_n = digit.digital_root_base(dr_n, 10);
        if (dr_n != dr_dr_n) {
            cout << "FAIL: n=" << n << " dr(n)=" << dr_n << " dr(dr(n))=" << dr_dr_n << endl;
        }
    }
    cout << "dr(dr(n)) = dr(n) — все 200 тестов пройдены" << endl;

    cout << "\n=== E. Переносы ===" << endl;
    cout << "S(a+b) = S(a) + S(b) - 9·carry_count (base=10):" << endl;
    for (int a = 1; a <= 100; a++) {
        for (int b = 1; b <= 100; b++) {
            int sa = digit.sum_digits_base(a, 10);
            int sb = digit.sum_digits_base(b, 10);
            int sab = digit.sum_digits_base(a + b, 10);
            int carries = digit.count_carry_sum(a, b, 10);
            int expected = sa + sb - 9 * carries;
            if (sab != expected) {
                cout << "FAIL: a=" << a << " b=" << b << " S(a+b)=" << sab << " S(a)+S(b)-9*carry=" << expected << endl;
            }
        }
    }
    cout << "S(a+b) = S(a)+S(b)-9·carry — все 10000 тестов пройдены" << endl;

    cout << "\n=== E. S(a+b) ≤ S(a) + S(b) ===" << endl;
    for (int a = 1; a <= 100; a++) {
        for (int b = 1; b <= 100; b++) {
            int sab = digit.sum_digits_base(a + b, 10);
            int sa_sb = digit.sum_digits_base(a, 10) + digit.sum_digits_base(b, 10);
            if (sab > sa_sb) {
                cout << "FAIL: a=" << a << " b=" << b << " S(a+b)=" << sab << " > S(a)+S(b)=" << sa_sb << endl;
            }
        }
    }
    cout << "S(a+b) ≤ S(a)+S(b) — все 10000 тестов пройдены" << endl;

    cout << "\n=== E. S(a·b) ≤ S(a)·S(b) ===" << endl;
    for (int a = 1; a <= 50; a++) {
        for (int b = 1; b <= 50; b++) {
            int sab = digit.sum_digits_base(a * b, 10);
            int sa_sb = digit.sum_digits_base(a, 10) * digit.sum_digits_base(b, 10);
            if (sab > sa_sb) {
                cout << "FAIL: a=" << a << " b=" << b << " S(a*b)=" << sab << " > S(a)*S(b)=" << sa_sb << endl;
            }
        }
    }
    cout << "S(a·b) ≤ S(a)·S(b) — все 2500 тестов пройдены" << endl;
}
#endif
