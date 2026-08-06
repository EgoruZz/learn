#ifndef COMBINATORICS_A_CPP
#define COMBINATORICS_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <functional>
#include <climits>
using namespace std;

// =============================================================
// A. ОСНОВЫ КОМБИНАТОРИКИ И КОМБИНАТОРНЫЕ АЛГОРИТМЫ
// =============================================================
// Структура md: A. Факториалы и биномиальные коэффициенты
//               → B. Включения-исключения
//               → D. Генерация комбинаторных объектов
//               → E. Специальные комбинаторные конфигурации
//
// C_iter — каноническая реализация C(n,k) живёт ЗДЕСЬ (комбинаторика);
// number-theory (dk_via_fact в Divisibility) использует её через свободную
// функцию C_iter ниже. Чтобы разорвать циклический include
// (combinatorics → b.cpp → a-folder/a.cpp), файл устроен так:
//   * свободная C_iter определена ДО подключения number-theory и видна всюду;
//   * подключение number-theory и struct Combinatorics выполняются только
//     когда файл включён напрямую, а не изнутри number-theory
//     (макрос INSIDE_NUMBER_THEORY).
//
// Наследует ModArithmetic (b.cpp) и Divisibility (a.cpp). Общие методы
// берём из базы, а не пишем заново:
//   powmod  (вместо modpow),  powll (вместо ipow),
//   egcd    (вместо extgcd),  modinv,
//   crt(a, m)                 (вместо локального crt по парам),
//   fi_single (вместо euler_phi).
//
// Содержит:
//   A. Факториалы и перестановки: fact, fact_mod, fact_precompute,
//      inv_fact_precompute, double_fact, multifact, legendre, legendre_C,
//      trailing_zeros, kummer, smallest_n_for_factorial, factorial_mod_clean,
//      factorial_mod_full, fact_mod_ppow, fact_mod_composite,
//      cyclic_perms, perms_with_rep, next_perm,
//      permutations_rec, kth_perm, perm_rank
//   A. Биномы: C_iter, C_mod, pascal, multinomial, binom_general, C_neg,
//      C_rep, arrangements, arrangements_rep, combinations_rec,
//      combinations_mask, next_comb_gosper, lucas
//   A. Составной модуль: fact_free, C_mod_ppow, C_composite
//   A. Системы счисления: to_balanced_ternary, from_balanced_ternary,
//      to_string_bt, to_factorial_base, from_factorial_base, to_string_fb
//   B. Включения-исключения: forbidden_positions, words_required,
//      surjections, derangements, derangements_incl_excl
//   D. Генерация: all_subsets, submasks_of, gray_encode, gray_decode,
//      gen_parentheses, gen_parentheses_iter, is_valid_parentheses
//   E. Специальные конфигурации: josephus, josephus_fast, n_queens_count,
//      Sudoku, Crossword, DancingLinks, KnightTour, necklace_count,
//      bracelet_count, rook_placements, rook_placements_mn,
//      bishop_ways, bishop_placements
//
// ВНИМАНИЕ (скрытие имён): fact (n!) и legendre (v_p(n!)) здесь по смыслу
// ДРУГИЕ функции, чем одноимённые из базы (fact — факторизация числа,
// legendre — символ Лежандра). Они скрывают базовые версии — так и должно
// быть; базовые версии в этих двух точках не используются.

// --- C_iter: биномиальный коэффициент C(n, k) O(k) ---
// C(n,k) = n!/(k!·(n−k)!) — количество k-элементных подмножеств из n.
// res = res·(n-i)/(i+1), i = 0..k-1. Деление всегда точное: произведение
// k последовательных чисел делится на k!. Работает в int64 до n = 66
// (C(67,33) ≈ 1.42e19 переполняет int64).
// Свободная функция без зависимостей: её вызывает dk_via_fact из
// number-theory (Divisibility), а Combinatorics::C_iter — тонкая обёртка.
long long C_iter(long long n, long long k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    k = min(k, n - k);
    long long res = 1;
    for (long long i = 0; i < k; i++)
        res = res * (n - i) / (i + 1);
    return res;
}

#ifndef INSIDE_NUMBER_THEORY

// Подключаем b.cpp (ModArithmetic → Divisibility), чтобы переиспользовать
// уже реализованные методы вместо повторного написания их тел.
#define DISCRETE_LOG_MAIN
#include "../../number-theory/b-folder/b.cpp"

struct Combinatorics : ModArithmetic {

// =============================================================
// A. ФАКТОРИАЛЫ И ПЕРЕСТАНОВКИ
// =============================================================

// --- A.0. Быстрое возведение в степень O(log b) ---
// a^b mod mod — база для обратных элементов (малая теорема Ферма)
// и размещений с повторениями n^k mod p.
// Реализация в базе: ModArithmetic::powmod (b.cpp) — используем её.

// --- A.1. Факториал наивный O(n) ---
// int64 вмещает 20!, поэтому в олимпиадах почти всегда по модулю.
long long fact(int n) {
    long long res = 1;
    for (int i = 2; i <= n; i++) res *= i;
    return res;
}

// --- A.1. Факториал по модулю O(n) ---
// n! mod p. Проблема: при n ≥ p результат всегда 0 (в n! есть множитель p).
long long fact_mod(int n, int mod) {
    long long res = 1 % mod;
    for (int i = 2; i <= n; i++) res = res * i % mod;
    return res;
}

// --- A.1. Предподсчёт всех факториалов O(N) ---
// fact[i] = i! mod p для всех i ≤ N, дальше запросы за O(1).
vector<long long> fact_precompute(int n, int mod) {
    vector<long long> f(n + 1, 1);
    for (int i = 1; i <= n; i++) f[i] = f[i - 1] * i % mod;
    return f;
}

// --- A.1. Предподсчёт обратных факториалов O(N) ---
// inv_fact[i] = (i!)^(-1) mod p.
// inv_fact[n] = powmod(fact[n], p-2, p) (Ферма), затем идём назад:
// inv_fact[i] = inv_fact[i+1] · (i+1) — потому что i! = (i+1)!/(i+1).
vector<long long> inv_fact_precompute(int n, int mod) {
    vector<long long> inv_f(n + 1);
    long long f = 1;
    for (int i = 1; i <= n; i++) f = f * i % mod;
    inv_f[n] = powmod(f, mod - 2, mod);
    for (int i = n - 1; i >= 0; i--) inv_f[i] = inv_f[i + 1] * (i + 1) % mod;
    return inv_f;
}

// --- A.1. Двойной факториал O(n) ---
// n!! = n·(n-2)·(n-4)·...
// Чётное n = 2k:  n!! = 2^k·k!
// Нечётное n = 2k+1: n!! = (2k+1)!/(2^k·k!)
// Применение: (2n-1)!! — число способов разбить 2n предметов на n пар.
long long double_fact(int n) {
    long long res = 1;
    for (int i = n; i > 0; i -= 2) res *= i;
    return res;
}

// --- A.1. Обобщённый факториал (мультифакториал) O(n/m) ---
// n!⁽ᵐ⁾ = n·(n−m)·(n−2m)·... — произведение с шагом m.
// m = 1 → n!, m = 2 → n!!. При n = m·k все множители кратны m:
// n!⁽ᵐ⁾ = m^k·k!.
long long multifact(int n, int m) {
    long long res = 1;
    for (int i = n; i > 0; i -= m) res *= i;
    return res;
}

// --- A.1. Формула Лежандра O(log_p n) ---
// v_p(n!) = max{k : p^k | n!} = Σ ⌊n/p⌋ + ⌊n/p²⌋ + ⌊n/p³⌋ + ...
long long legendre(long long n, long long p) {
    long long res = 0;
    while (n) {
        n /= p;
        res += n;
    }
    return res;
}

// --- A.1. Степень простого в биномиальном коэффициенте ---
// v_p(C(n,k)) = v_p(n!) − v_p(k!) − v_p((n−k)!)
// Теорема Кюммера: равно числу переносов при сложении k и (n−k)
// в системе счисления по основанию p.
long long legendre_C(long long n, long long k, long long p) {
    return legendre(n, p) - legendre(k, p) - legendre(n - k, p);
}

// --- A.1. Хвостовые нули n! в десятичной записи = v₅(n!) ---
long long trailing_zeros(long long n) {
    return legendre(n, 5);
}

// --- A.1. Теорема Кюммера O(log_p n) ---
// v_p(C(n,k)) = число переносов при сложении k + (n−k) в base p.
long long kummer(long long n, long long k, long long p) {
    long long carries = 0, carry = 0;
    long long nk = n - k;
    while (k > 0 || nk > 0) {
        long long sum = k % p + nk % p + carry;
        if (sum >= p) { carries++; carry = 1; }
        else carry = 0;
        k /= p;
        nk /= p;
    }
    return carries;
}

// --- A.1. Обратная задача: наименьший n: m | n! ---
// Для каждого простого делителя p^e числа m бинарным поиском ищем
// минимальное n_p: v_p(n_p!) ≥ e; ответ = max по всем делителям.
// Сложность: O(log²m) на простой делитель.
long long smallest_n_for_factorial(long long m) {
    if (m <= 1) return 0;
    long long result = 0;
    for (long long p = 2; p * p <= m; p++) {
        if (m % p == 0) {
            long long e = 0;
            while (m % p == 0) { m /= p; e++; }
            long long lo = 0, hi = e * p;
            while (lo < hi) {
                long long mid = (lo + hi) / 2;
                if (legendre(mid, p) >= e) hi = mid;
                else lo = mid + 1;
            }
            result = max(result, lo);
        }
    }
    if (m > 1) result = max(result, m);
    return result;
}

// --- A.1. n! mod p без кратных p (рекурсивно) O(p·log_p n) ---
// n! = p^v · F(n) mod p, где F(n) — «чистый» остаток (без множителей p).
// F(n) = F(⌊n/p⌋) · (−1)^{⌊n/p⌋} · (n mod p)! (mod p):
//   каждое число — либо p·t (вклад уходит в F(⌊n/p⌋)), либо «голое»;
//   «голые» множители дают ⌊n/p⌋ блоков по (p−1)! ≡ −1 (Вильсон)
//   и хвост (n mod p)!.
long long factorial_mod_clean(long long n, long long p) {
    if (n == 0) return 1;
    long long res = factorial_mod_clean(n / p, p);
    for (long long i = 1; i <= n % p; i++)
        res = res * i % p;
    if ((n / p) % 2 == 1)
        res = p - res;
    return res;
}

// --- A.1. n! mod p полностью: {v_p(n!), n!/p^v mod p} ---
// Степень p — формула Лежандра, «чистый» остаток — factorial_mod_clean.
pair<long long, long long> factorial_mod_full(long long n, long long p) {
    long long v = legendre(n, p);
    long long clean = factorial_mod_clean(n, p);
    return {v, clean};
}

// --- A.1. Циклические перестановки ---
// (n−1)! — рассаживания за круглым столом: линейных n!, каждое
// круговое расположение считается n раз (n поворотов).
long long cyclic_perms(int n) {
    return fact(n - 1);
}

// --- A.1. Перестановки с повторениями mod p ---
// n!/(n₁!·n₂!·...·n_k!), где nᵢ — размеры групп одинаковых элементов.
// Пример: «АБРАКАДАБРА» = 11!/(5!·2!·2!·1!·1!).
long long perms_with_rep(const vector<int>& counts, int mod,
                         const vector<long long>& fact, const vector<long long>& inv_fact) {
    int n = 0;
    for (int c : counts) n += c;
    long long res = fact[n];
    for (int c : counts) res = res * inv_fact[c] % mod;
    return res;
}

// --- A.1. Алгоритм Нарайаны O(n) ---
// Следующая лексикографически большая перестановка (аналог
// std::next_permutation). Шаги:
//   1) max i: a[i] < a[i+1]
//   2) max j: a[j] > a[i]
//   3) swap(a[i], a[j])
//   4) reverse(a[i+1..n])
// Возвращает false (и сбрасывает в минимальную), когда перестановок больше нет.
bool next_perm(vector<int>& a) {
    int n = (int)a.size();
    int i = n - 2;
    while (i >= 0 && a[i] >= a[i + 1]) i--;
    if (i < 0) {
        reverse(a.begin(), a.end());
        return false;
    }
    int j = n - 1;
    while (a[j] <= a[i]) j--;
    swap(a[i], a[j]);
    reverse(a.begin() + i + 1, a.end());
    return true;
}

// --- A.1. Рекурсивный перебор перестановок O(n!) ---
// На каждой позиции пробуем все ещё не использованные элементы.
// С повторениями: не используем used и/или пропускаем одинаковые соседние.
void permutations_rec(vector<int>& cur, vector<bool>& used, const vector<int>& elems) {
    if ((int)cur.size() == (int)elems.size()) {
        for (int x : cur) cout << x << ' ';
        cout << '\n';
        return;
    }
    for (int i = 0; i < (int)elems.size(); i++) {
        if (used[i]) continue;
        used[i] = true;
        cur.push_back(elems[i]);
        permutations_rec(cur, used, elems);
        cur.pop_back();
        used[i] = false;
    }
}

// --- A.1. K-я лексикографическая перестановка O(n²) (Lehmer code) ---
// Факториальная система счисления: k = c₁·(n−1)! + c₂·(n−2)! + ... + c_n·0!,
// на позиции i выбираем цифру k/(n−i−1)! и элемент с этим номером.
// k с 0, работает до n! в long long.
vector<int> kth_perm(int n, long long k) {
    vector<long long> f(n + 1, 1);
    for (int i = 1; i <= n; i++) f[i] = f[i - 1] * i;
    vector<int> rest(n), res(n);
    for (int i = 0; i < n; i++) rest[i] = i;
    for (int i = 0; i < n; i++) {
        long long block = f[n - 1 - i];
        int idx = (int)(k / block);
        k %= block;
        res[i] = rest[idx];
        rest.erase(rest.begin() + idx);
    }
    return res;
}

// --- A.1. Ранг перестановки O(n²) (обратное к Lehmer) ---
// Считаем, сколько меньших неиспользованных элементов правее каждого
// элемента, и собираем число в факториальной системе счисления.
long long perm_rank(const vector<int>& p) {
    int n = (int)p.size();
    vector<long long> f(n + 1, 1);
    for (int i = 1; i <= n; i++) f[i] = f[i - 1] * i;
    vector<bool> used(n, false);
    long long rank = 0;
    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int x = 0; x < p[i]; x++)
            if (!used[x]) cnt++;
        rank += cnt * f[n - 1 - i];
        used[p[i]] = true;
    }
    return rank;
}

// =============================================================
// A. БИНОМИАЛЬНЫЕ КОЭФФИЦИЕНТЫ
// =============================================================

// --- A.2. C(n,k) итеративно O(k) ---
// res = res·(n-i)/(i+1), i = 0..k-1.
// Деление всегда точное: произведение k последовательных чисел
// делится на k!. Работает в int64 до n = 66 (C(67,33) переполняет int64).
// Каноническая реализация — свободная функция C_iter в начале файла;
// здесь — тонкая обёртка (её же использует dk_via_fact из number-theory).
long long C_iter(long long n, long long k) {
    return ::C_iter(n, k);
}

// --- A.2. C(n,k) mod p через предподсчёт O(1) на запрос ---
// C(n,k) = fact[n]·inv_fact[k]·inv_fact[n−k] mod p.
// Ограничение: нужен n < p (иначе знаменатель может стать 0 mod p).
// Для n ≥ p используем теорему Люка (lucas).
long long C_mod(int n, int k, int mod, const vector<long long>& fact, const vector<long long>& inv_fact) {
    if (k < 0 || k > n) return 0;
    return fact[n] * inv_fact[k] % mod * inv_fact[n - k] % mod;
}

// --- A.2. Треугольник Паскаля O(N²) ---
// C(n,k) = C(n−1,k) + C(n−1,k−1) — сумма двух родителей сверху.
// Без деления и без переполнения (по модулю). Строка n — C(n,0..n).
vector<vector<long long>> pascal(int n, int mod) {
    vector<vector<long long>> t(n + 1);
    for (int i = 0; i <= n; i++) {
        t[i].assign(i + 1, 1);
        for (int j = 1; j < i; j++)
            t[i][j] = (t[i - 1][j - 1] + t[i - 1][j]) % mod;
    }
    return t;
}

// --- A.2. Мультиномиальный коэффициент mod p ---
// n!/(k₁!·k₂!·...·k_m!), где Σ kᵢ = n — коэффициент при a₁^{k₁}...a_m^{k_m}
// в разложении (a₁+...+a_m)ⁿ. Связь с биномом: произведение последовательных
// биномов C(n,k₁)·C(n−k₁,k₂)·...
long long multinomial(const vector<int>& ks, int mod,
                      const vector<long long>& fact, const vector<long long>& inv_fact) {
    long long res = 1;
    int n = 0;
    for (int x : ks) {
        res = res * inv_fact[x] % mod;
        n += x;
    }
    return res * fact[n] % mod;
}

// --- A.2. Обобщённый биномиальный коэффициент C(a,k) ---
// Для произвольного a: C(a,k) = a·(a−1)·...·(a−k+1)/k!.
// Случай отрицательного целого: C(−n,k) = (−1)^k·C(n+k−1,k) — см. C_neg.
long double binom_general(double a, int k) {
    long double res = 1;
    for (int i = 0; i < k; i++) res = res * (a - i) / (i + 1);
    return res;
}

// --- A.2. C(-n,k) = (-1)^k·C(n+k-1,k) ---
// Используется в разложении (1-x)^(-n) = Σ C(n+k-1, k)·x^k.
long long C_neg(long long n, long long k) {
    long long c = C_iter(n + k - 1, k);
    return (k & 1) ? -c : c;
}

// --- A.2. Сочетания с повторениями — «звёзды и барьеры» ---
// C(n+k−1, k) — выбор k предметов из n видов с повторениями;
// число решений x₁+...+x_n = k, xᵢ ≥ 0 (раздать k конфет n детям).
long long C_rep(long long n, long long k) {
    return C_iter(n + k - 1, k);
}

// --- A.2. Размещения без повторений A(n,k) = n!/(n−k)! ---
// Упорядоченные k-наборы из n элементов. Связь: A(n,k) = C(n,k)·k!.
long long arrangements(long long n, long long k) {
    long long res = 1;
    for (long long i = 0; i < k; i++) res *= n - i;
    return res;
}

// --- A.2. Размещения с повторениями A'(n,k) = n^k mod mod ---
// Каждый из k выборов — n способов (правило произведения).
long long arrangements_rep(long long n, long long k, long long mod) {
    return powmod(n, k, mod);
}

// --- A.2. Генерация биномиальных коэффициентов: рекурсивная O(C(n,k)) ---
// Каждое сочетание выводится ровно один раз: следующий элемент > предыдущего.
void combinations_rec(vector<int>& cur, int n, int k, int start) {
    if ((int)cur.size() == k) {
        for (int x : cur) cout << x << ' ';
        cout << '\n';
        return;
    }
    for (int x = start; x <= n; x++) {
        cur.push_back(x);
        combinations_rec(cur, n, k, x + 1);
        cur.pop_back();
    }
}

// --- A.2. Генерация биномиальных коэффициентов: битовые маски O(2ⁿ·n) ---
// Медленнее рекурсии, но проще в коде.
void combinations_mask(int n, int k) {
    for (int mask = 0; mask < (1 << n); mask++) {
        if (__builtin_popcount(mask) != k) continue;
        for (int i = 0; i < n; i++)
            if (mask >> i & 1) cout << i + 1 << ' ';
        cout << '\n';
    }
}

// --- A.2. Генерация биномиальных коэффициентов: Gosper's Hack O(1) ---
// Следующая маска с k единичными битами из x:
//   c = x & -x; r = x + c; x = (((r ^ x) >> 2) / c) | r
// Применение: перебор всех C(n,k) подмножеств без O(2ⁿ) лишних шагов.
unsigned long long next_comb_gosper(unsigned long long x) {
    unsigned long long c = x & -x;
    unsigned long long r = x + c;
    return (((r ^ x) >> 2) / c) | r;
}

// --- A.2. Теорема Люка O(log_p n) на запрос ---
// При n ≥ p предподсчёт не работает (факториал ≡ 0 mod p).
// Раскладываем n, k по основанию p:
//   C(n,k) ≡ C(n/p, k/p)·C(n mod p, k mod p) (mod p)
// Следствие: C(n,k) ≠ 0 mod p ⟺ каждый разряд k ≤ разряда n.
long long lucas(long long n, long long k, int p,
                const vector<long long>& fact, const vector<long long>& inv_fact) {
    if (k < 0 || k > n) return 0;
    if (n < p) return C_mod((int)n, (int)k, p, fact, inv_fact);
    return lucas(n / p, k / p, p, fact, inv_fact)
         * C_mod((int)(n % p), (int)(k % p), p, fact, inv_fact) % p;
}

// =============================================================
// A. БИНОМИАЛЬНЫЕ КОЭФФИЦИЕНТЫ ПО СОСТАВНОМУ МОДУЛЮ
// =============================================================

// --- A.2. Расширенный алгоритм Евклида ---
// Возвращает g = gcd(a, b) и коэффициенты x, y: a·x + b·y = g.
// Реализация в базе: ModArithmetic::egcd (b.cpp) — используем её.

// --- A.2. Обратный элемент по модулю (взаимная простота) ---
// a·inv ≡ 1 (mod mod). Требует gcd(a, mod) = 1; при gcd ≠ 1 возвращает -1.
// Реализация в базе: ModArithmetic::modinv (b.cpp) — используем её.

// --- A.2. p-свободная часть факториала f(n) mod p^e ---
// f(n) = n! / p^{v_p(n!)} mod p^e — произведение чисел ≤ n с выброшенными
// всеми множителями p. Рекурсия:
//   f(n) = P^{⌊n/p^e⌋} · Q(n mod p^e) · f(⌊n/p⌋) mod p^e,
// где P — произведение чисел 1..p^e, не делящихся на p (период),
// Q(t) — произведение чисел 1..t, не делящихся на p.
long long fact_free(long long n, long long p, long long pe) {
    if (n == 0) return 1;
    long long P = 1;
    for (long long i = 1; i <= pe; i++)
        if (i % p != 0) P = P * i % pe;
    long long res = powmod(P, n / pe, pe);
    for (long long i = 1; i <= n % pe; i++)
        if (i % p != 0) res = res * i % pe;
    return res * fact_free(n / p, p, pe) % pe;
}

// --- A.1. n! mod p^e (обобщение 1.5 на степень простого) ---
// n! = p^v · f(n), где v — формула Лежандра, f(n) — p-свободная часть
// (fact_free). Если v ≥ e — ответ 0 (в n! «набралось» p^e); иначе
// n! mod p^e = p^v · f(n) mod p^e. Обратные не нужны.
long long fact_mod_ppow(long long n, long long p, long long e) {
    long long pe = 1;
    for (long long i = 0; i < e; i++) pe *= p;
    long long v = legendre(n, p);
    if (v >= e) return 0;
    long long res = fact_free(n, p, pe);
    for (long long i = 0; i < v; i++) res = res * p % pe;
    return res;
}

// --- A.1. n! mod m для составного m (обобщение 1.5) ---
// Разлагаем m на простые степени, считаем n! mod p_i^{e_i} (fact_mod_ppow),
// собираем CRT. В отличие от C(n,k) mod m (C_composite) обратные не нужны —
// n! не делится на m в явной формуле.
long long fact_mod_composite(long long n, long long m) {
    if (m == 1) return 0;
    vector<pair<long long, int>> pp;       // (p, e) — факторизация m
    long long tmp = m;
    for (long long p = 2; p * p <= tmp; p++) {
        if (tmp % p == 0) {
            int e = 0;
            while (tmp % p == 0) { tmp /= p; e++; }
            pp.push_back({p, e});
        }
    }
    if (tmp > 1) pp.push_back({tmp, 1});
    vector<long long> rems, mods;
    for (auto& pr : pp) {
        long long pe = 1;
        for (int i = 0; i < pr.second; i++) pe *= pr.first;
        mods.push_back(pe);
        rems.push_back(fact_mod_ppow(n, pr.first, pr.second));
    }
    return crt(rems, mods) % m;
}

// --- A.2. C(n,k) mod p^e (обобщение Люка на степень простого) ---
// 1) v = v_p(n!) − v_p(k!) − v_p((n−k)!). Если v ≥ e — ответ 0.
// 2) C(n,k) = p^v · f(n) · f(k)⁻¹ · f(n−k)⁻¹ mod p^e.
// Обратные существуют: f-части не содержат p и взаимно просты с p^e.
long long C_mod_ppow(long long n, long long k, long long p, long long e) {
    if (k < 0 || k > n) return 0;
    long long pe = 1;
    for (long long i = 0; i < e; i++) pe *= p;
    long long v = legendre(n, p) - legendre(k, p) - legendre(n - k, p);
    if (v >= e) return 0;
    long long res = fact_free(n, p, pe);
    res = res * modinv(fact_free(k, p, pe), pe) % pe;
    res = res * modinv(fact_free(n - k, p, pe), pe) % pe;
    for (long long i = 0; i < v; i++) res = res * p % pe;
    return res;
}

// --- A.2. Китайская теорема об остатках (CRT) ---
// По остаткам с попарно взаимно простыми модулями восстанавливает
// единственный остаток по произведению модулей.
// Реализация в базе: ModArithmetic::crt(a, m) (b.cpp) — используем её
// (a — остатки, m — модули).

// --- A.2. C(n,k) mod m для составного m (обобщение) ---
// Разлагаем m на простые степени, считаем C(n,k) mod p_i^{e_i} (обобщённый
// Люка), собираем CRT. Предподсчёта нет — каждый запрос независим.
long long C_composite(long long n, long long k, long long m) {
    if (m == 1) return 0;
    if (k < 0 || k > n) return 0;
    vector<pair<long long, int>> pp;       // (p, e) — факторизация m
    long long tmp = m;
    for (long long p = 2; p * p <= tmp; p++) {
        if (tmp % p == 0) {
            int e = 0;
            while (tmp % p == 0) { tmp /= p; e++; }
            pp.push_back({p, e});
        }
    }
    if (tmp > 1) pp.push_back({tmp, 1});
    vector<long long> rems, mods;
    for (auto& pr : pp) {
        long long pe = 1;
        for (int i = 0; i < pr.second; i++) pe *= pr.first;
        mods.push_back(pe);
        rems.push_back(C_mod_ppow(n, k, pr.first, pr.second));
    }
    return crt(rems, mods) % m;
}

// =============================================================
// A.4. СПЕЦИАЛЬНЫЕ СИСТЕМЫ СЧИСЛЕНИЯ
// =============================================================

// --- A.4. Троичная сбалансированная система: цифры T(-1), 0, 1 ---
// Каждое целое число имеет единственное представление; знак «встроен»
// в цифры, отдельного знакового разряда нет.
// Перевод делением на 3: остаток 2 (−1) → цифра T, перенос +1;
// остаток −2 → цифра 1, n = (n−1)/3. Пример: 5 → 1TT₃ = 9−3−1 = 5.
vector<int> to_balanced_ternary(long long n) {
    if (n == 0) return {0};
    vector<int> digits;
    while (n != 0) {
        long long r = n % 3;
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

// --- A.4. Троичная сбалансированная: обратный перевод O(log₃ n) ---
long long from_balanced_ternary(const vector<int>& digits) {
    long long res = 0, base = 1;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
        res += digits[i] * base;
        base *= 3;
    }
    return res;
}

// --- A.4. Троичная сбалансированная: вывод в строку (T, 0, 1) ---
string to_string_bt(const vector<int>& digits) {
    string s;
    for (int d : digits) s += (d == -1 ? "T" : to_string(d));
    return s;
}

// --- A.4. Факториальная система счисления: n = Σ dᵢ·i!, dᵢ ∈ [0, i] ---
// Каждое неотрицательное целое имеет единственное представление.
// Перевод делением на 1, 2, 3, ...: остатки — цифры с младшего разряда.
vector<int> to_factorial_base(long long n) {
    if (n == 0) return {0};
    vector<int> digits;
    for (int i = 1; n > 0; i++) {
        digits.push_back((int)(n % i));
        n /= i;
    }
    reverse(digits.begin(), digits.end());
    return digits;
}

// --- A.4. Факториальная система: обратный перевод ---
// Цифры идут от старшего разряда: digits[0] имеет вес size!,
// digits[1] — (size−1)!, ..., последняя цифра — вес 1!.
long long from_factorial_base(const vector<int>& digits) {
    long long res = 0, fact = 1;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
        res += digits[i] * fact;
        fact *= (digits.size() - i);
    }
    return res;
}

// --- A.4. Факториальная система: вывод в строку (цифры через !) ---
string to_string_fb(const vector<int>& digits) {
    string s;
    for (int i = 0; i < (int)digits.size(); i++) {
        if (i > 0) s += "!";
        s += to_string(digits[i]);
    }
    return s;
}

// =============================================================
// B. ВКЛЮЧЕНИЯ-ИСКЛЮЧЕНИЯ
// =============================================================

// --- B.2. Перестановки с запрещёнными позициями (задача о ладьях) ---
// Считаем перестановки π, у которых π(i) ∉ forb[i] (битовые маски запретов).
// Включения-исключения: ответ = Σ (−1)^r · R_r · (n−r)!,
// где R_r — число способов расставить r не бьющих друг друга ладей на
// запрещённых клетках. R_r получаем ДП по битовым маскам столбцов.
// Проверка: forb[i] = {i} даёт беспорядки D(n).
long long forbidden_positions(int n, const vector<unsigned int>& forb) {
    vector<long long> dp(1 << n, 0), R(n + 1, 0);
    dp[0] = 1;
    for (int row = 0; row < n; row++)
        for (int mask = 0; mask < (1 << n); mask++)
            if (dp[mask]) {
                unsigned int free = forb[row] & ~(unsigned int)mask;
                while (free) {
                    int b = (int)(free & -free);
                    free ^= (unsigned int)b;
                    dp[mask | b] += dp[mask];
                }
            }
    for (int mask = 0; mask < (1 << n); mask++)
        R[__builtin_popcount(mask)] += dp[mask];
    long long res = 0;
    for (int r = 0; r <= n; r++) {
        long long term = R[r] * fact(n - r);
        res += (r & 1) ? -term : term;
    }
    return res;
}

// --- B.2. Слова с обязательными буквами mod p ---
// Алфавит размера k, слова длины n, каждая из t «обязательных» букв
// встретилась хотя бы раз: Σ (−1)^i·C(t,i)·(k−i)ⁿ.
// t = k — частный случай «использованы все буквы» = сюръекции (см. ниже).
long long words_required(int n, int k, int t, int mod,
                         const vector<long long>& fact, const vector<long long>& inv_fact) {
    long long res = 0;
    for (int i = 0; i <= t; i++) {
        long long term = C_mod(t, i, mod, fact, inv_fact) * powmod(k - i, n, mod) % mod;
        res = (res + ((i & 1) ? mod - term : term)) % mod;
    }
    return res;
}

// --- B.2. Сюръективные отображения mod p ---
// Σ_{i=0}^{k} (−1)^i·C(k,i)·(k−i)^n — из всех k^n отображений выкидываем
// те, что не используют какие-то элементы. Нужен k < p для C_mod.
long long surjections(long long n, long long k, int mod,
                      const vector<long long>& fact, const vector<long long>& inv_fact) {
    long long res = 0;
    for (int i = 0; i <= k; i++) {
        long long term = C_mod((int)k, i, mod, fact, inv_fact) * powmod(k - i, n, mod) % mod;
        res = (res + ((i & 1) ? mod - term : term)) % mod;
    }
    return res;
}

// --- B.3. Беспорядки D(n) mod p O(n) ---
// D(n) = (n−1)·(D(n−1) + D(n−2)), D(0)=1, D(1)=0, D(2)=1.
// Почему: элемент 1 отправляем в позицию i (n−1 способов); если элемент i
// тоже на месте 1 — осталось D(n−2), иначе D(n−1).
long long derangements(int n, int mod) {
    if (n == 0) return 1 % mod;
    long long d0 = 1, d1 = 0;              // D(0)=1, D(1)=0
    for (int i = 2; i <= n; i++) {
        long long d = (i - 1) * ((d0 + d1) % mod) % mod;
        d0 = d1;
        d1 = d;
    }
    return d1;
}

// --- B.3. Беспорядки включениями-исключениями mod p ---
// D(n) = n!·Σ_{i=0}^{n} (−1)ⁱ/i! — из n! перестановок вычитаем
// с одной фиксированной точкой, прибавляем с двумя, и т.д.
long long derangements_incl_excl(int n, int mod,
                                 const vector<long long>& fact, const vector<long long>& inv_fact) {
    long long s = 0;
    for (int i = 0; i <= n; i++) {
        long long term = inv_fact[i];
        s = (s + ((i & 1) ? mod - term : term)) % mod;
    }
    return s * fact[n] % mod;
}

// =============================================================
// D. ГЕНЕРАЦИЯ КОМБИНАТОРНЫХ ОБЪЕКТОВ
// =============================================================

// --- D.1. Перебор всех подмножеств битовыми масками O(2ⁿ·n) ---
void all_subsets(int n) {
    for (int mask = 0; mask < (1 << n); mask++) {
        for (int i = 0; i < n; i++)
            if (mask >> i & 1) cout << i + 1 << ' ';
        cout << '\n';
    }
}

// --- D.1. Все подмаски данной маски ---
// Приём: for (int sub = mask; ; sub = (sub-1) & mask) { ...; if (sub == 0) break; }
// Суммарно по всем маскам: Σ 2^popcount(mask) = 3ⁿ — основа ДП по подмножествам.
vector<unsigned int> submasks_of(unsigned int mask) {
    vector<unsigned int> res;
    for (unsigned int sub = mask; ; sub = (sub - 1) & mask) {
        res.push_back(sub);
        if (sub == 0) break;
    }
    return res;
}

// --- D.2. Серый код: кодирование ---
// gray(k) = k ^ (k >> 1); соседние коды отличаются ровно в одном бите.
unsigned int gray_encode(unsigned int k) {
    return k ^ (k >> 1);
}

// --- D.2. Серый код: декодирование ---
// k = g; k ^= k>>1; k ^= k>>2; k ^= k>>4; ... (степени двойки до 32).
unsigned int gray_decode(unsigned int g) {
    unsigned int k = g;
    for (unsigned int s = 1; s < 32; s <<= 1) k ^= k >> s;
    return k;
}

// --- D.3. Генерация правильных скобочных последовательностей O(C(n)) ---
// '(' можно, пока open < n; ')' — пока close < open.
// Количество: числа Каталана C_n = C(2n,n)/(n+1).
void gen_parentheses(string& cur, int open, int close, int n) {
    if ((int)cur.size() == 2 * n) {
        cout << cur << '\n';
        return;
    }
    if (open < n) {
        cur.push_back('(');
        gen_parentheses(cur, open + 1, close, n);
        cur.pop_back();
    }
    if (close < open) {
        cur.push_back(')');
        gen_parentheses(cur, open, close + 1, n);
        cur.pop_back();
    }
}

// --- D.3. Генерация скобок итеративно ---
// Начинаем с "(((...)))", применяем next_permutation и фильтруем корректные.
vector<string> gen_parentheses_iter(int n) {
    vector<string> res;
    string s(n, '(');
    s += string(n, ')');
    do {
        if (is_valid_parentheses(s)) res.push_back(s);
    } while (next_permutation(s.begin(), s.end()));
    return res;
}

// --- D.3. Проверка корректности скобочной строки O(n) ---
// '(' → +1, ')' → −1; счётчик никогда не уходит в минус и в конце равен 0.
bool is_valid_parentheses(const string& s) {
    int bal = 0;
    for (char c : s) {
        bal += (c == '(') ? 1 : -1;
        if (bal < 0) return false;
    }
    return bal == 0;
}

// =============================================================
// E. СПЕЦИАЛЬНЫЕ КОМБИНАТОРНЫЕ КОНФИГУРАЦИИ
// =============================================================

// --- E.1. Задача Иосифа Флавия O(n) ---
// J(1,k)=0; J(i,k) = (J(i−1,k) + k) % i. После удаления позиции k−1
// круг перенумеровывается, размер уменьшается на 1.
// Ответ с индексацией с 1: J(n,k) + 1.
int josephus(int n, int k) {
    int res = 0;
    for (int i = 2; i <= n; i++)
        res = (res + k) % i;
    return res;
}

// --- E.1. Иосиф для больших n и малых k O(k·log n) ---
// Когда k << n, большинство шагов не «заворачивают» по модулю: на каждом
// шаге res += k, i += 1. Прыгаем сразу на m = (i−res−1)/(k−1) таких шагов,
// потом делаем один шаг с заворотом res = (res + k) % (i+1).
long long josephus_fast(long long n, long long k) {
    if (k == 1) return n - 1;            // каждый k-й — просто последний
    long long res = 0, i = 1;            // res = J(i, k), i = 1..n
    while (i < n) {
        long long m = (i - res - 1) / (k - 1);   // шагов без заворота
        if (i + m >= n) {                // доходим до n напрямую
            res += (n - i) * k;
            i = n;
            break;
        }
        res += m * k;
        i += m;
        res = (res + k) % (i + 1);       // один шаг с заворотом
        i++;
    }
    return res % n;
}

// --- E.2. N-Queens: число расстановок (бэктрекинг с битовыми масками) ---
// cols/ldiag/rdiag — занятые столбцы и две диагональные семьи. Идём по
// строкам, битовой операцией получаем все доступные клетки строки.
int n_queens_count(int n) {
    int cnt = 0;
    function<void(int, int, int, int)> dfs = [&](int row, int cols, int ldiag, int rdiag) {
        if (row == n) { cnt++; return; }
        int avail = ((1 << n) - 1) & ~(cols | ldiag | rdiag);
        while (avail) {
            int bit = avail & -avail;
            avail ^= bit;
            dfs(row + 1, cols | bit, (ldiag | bit) << 1, (rdiag | bit) >> 1);
        }
    };
    dfs(0, 0, 0, 0);
    return cnt;
}

// --- E.2. Решатель судоку (backtracking + битовые маски + MRV) ---
// rows/cols/boxes хранят битовые маски поставленных цифр (биты 1..9),
// проверка кандидата за O(1). Каждый ход выбираем клетку с минимальным
// числом вариантов (MRV — most constrained variable).
struct Sudoku {
    int g[9][9];
    int rows[9], cols[9], boxes[9];

    Sudoku(const vector<vector<int>>& a) {
        for (int i = 0; i < 9; i++) rows[i] = cols[i] = boxes[i] = 0;
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++) {
                g[r][c] = a[r][c];
                if (g[r][c]) {
                    int d = g[r][c];
                    rows[r] |= 1 << d;
                    cols[c] |= 1 << d;
                    boxes[(r / 3) * 3 + c / 3] |= 1 << d;
                }
            }
    }

    // битовая маска допустимых цифр для клетки (r,c)
    int allowed(int r, int c) {
        int used = rows[r] | cols[c] | boxes[(r / 3) * 3 + c / 3];
        return ~used & 0x3FE;   // биты 1..9
    }

    bool solve() {
        int br = -1, bc = -1, best = 10;
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                if (g[r][c] == 0) {
                    int cnt = __builtin_popcount(allowed(r, c));
                    if (cnt < best) { best = cnt; br = r; bc = c; }
                    if (best == 1) break;
                }
        if (br == -1) return true;   // все клетки заполнены
        int mask = allowed(br, bc);
        for (int d = 1; d <= 9; d++) {
            if (!(mask >> d & 1)) continue;
            g[br][bc] = d;
            rows[br] |= 1 << d;
            cols[bc] |= 1 << d;
            boxes[(br / 3) * 3 + bc / 3] |= 1 << d;
            if (solve()) return true;
            boxes[(br / 3) * 3 + bc / 3] &= ~(1 << d);
            cols[bc] &= ~(1 << d);
            rows[br] &= ~(1 << d);
            g[br][bc] = 0;
        }
        return false;
    }
};

// --- E.2. Кроссворд (backtracking по слотам с MRV) ---
// Сетка: '.' — свободная клетка, '#' — стена, буквы — вписанные.
// Находим слоты (сегменты длиной ≥ 2 по горизонтали/вертикали), для каждого
// — слова из словаря, совместимые с текущими буквами. Каждый шаг выбираем
// слот с минимальным числом кандидатов (MRV), пересечения согласуем через
// проверку совместимости по уже вписанным буквам.
struct Crossword {
    int n, m;
    vector<string> grid, orig;
    vector<string> dict;

    struct Slot {
        int r, c;        // начальная клетка
        int dr, dc;      // направление: (0,1) или (1,0)
        int len;         // 0 = слот уже назначен
        vector<string> cand;
    };
    vector<Slot> slots;

    Crossword(const vector<string>& g, const vector<string>& w) : grid(g), dict(w) {
        n = (int)grid.size();
        m = (int)grid[0].size();
        orig = grid;
        find_slots();
    }

    void find_slots() {
        for (int r = 0; r < n; r++)                     // горизонтальные
            for (int c = 0; c < m; c++)
                if (grid[r][c] != '#' && (c == 0 || grid[r][c - 1] == '#')) {
                    int len = 0;
                    while (c + len < m && grid[r][c + len] != '#') len++;
                    if (len >= 2) slots.push_back({r, c, 0, 1, len, {}});
                }
        for (int c = 0; c < m; c++)                     // вертикальные
            for (int r = 0; r < n; r++)
                if (grid[r][c] != '#' && (r == 0 || grid[r - 1][c] == '#')) {
                    int len = 0;
                    while (r + len < n && grid[r + len][c] != '#') len++;
                    if (len >= 2) slots.push_back({r, c, 1, 0, len, {}});
                }
    }

    // слово w подходит в слот si, если совпадает длина и все уже вписанные буквы
    bool compatible(int si, const string& w) {
        Slot& s = slots[si];
        if ((int)w.size() != s.len) return false;
        for (int i = 0; i < s.len; i++) {
            char cur = grid[s.r + i * s.dr][s.c + i * s.dc];
            if (cur != '.' && cur != w[i]) return false;
        }
        return true;
    }

    void refresh_candidates(int si) {
        Slot& s = slots[si];
        s.cand.clear();
        for (const string& w : dict)
            if (compatible(si, w)) s.cand.push_back(w);
    }

    void place(int si, const string& w) {
        Slot& s = slots[si];
        for (int i = 0; i < s.len; i++)
            if (grid[s.r + i * s.dr][s.c + i * s.dc] == '.')
                grid[s.r + i * s.dr][s.c + i * s.dc] = w[i];
        s.len = 0;   // маркер «назначен»
    }

    void unplace(int si) {
        Slot& s = slots[si];
        s.len = (int)s.cand[0].size();
        for (int i = 0; i < s.len; i++)
            grid[s.r + i * s.dr][s.c + i * s.dc] = orig[s.r + i * s.dr][s.c + i * s.dc];
    }

    bool solve() {
        int best = -1, best_cnt = INT_MAX;
        for (int i = 0; i < (int)slots.size(); i++) {
            if (slots[i].len == 0) continue;   // уже назначен
            refresh_candidates(i);
            if (slots[i].cand.empty()) return false;
            if ((int)slots[i].cand.size() < best_cnt) {
                best_cnt = (int)slots[i].cand.size();
                best = i;
                if (best_cnt == 1) break;
            }
        }
        if (best == -1) return true;           // все слоты назначены
        for (const string& w : slots[best].cand) {
            place(best, w);
            if (solve()) return true;
            unplace(best);
        }
        return false;
    }
};

// --- E.2. Алгоритм Кнута X (Dancing Links / DLX) ---
// Точное покрытие: дана 0-1 матрица (столбцы — ограничения, строки —
// варианты), выбрать строки так, чтобы каждый столбец был покрыт ровно
// один раз.
//
// Алгоритм X: выбираем столбец с минимумом единиц (MRV), пробуем каждую
// его строку, удаляем конфликтующие столбцы и строки, рекурсивно идём
// дальше, откатываем.
//
// Dancing Links — реализация: матрица = сеть циклических двусвязных
// списков (ссылки l/r/u/d), удаление и восстановление узла — перезапись
// четырёх ссылок за O(1), без копирования матриц. Вычёркивание узла
// автоматически переадресует D-ссылку предшественника, поэтому обход
// колонки пропускает уже удалённые строки.
struct DancingLinks {
    struct Node { int l, r, u, d, col; };
    vector<Node> nd;
    vector<int> sz, rw;      // sz[c] — размер столбца, rw[i] — строка узла i
    vector<int> st;          // выбранные строки
    int head, cols;

    DancingLinks(int cols) : head(0), cols(cols) {
        nd.resize(cols + 1);
        for (int c = 0; c <= cols; c++) {      // головы столбцов: 1..cols
            nd[c].l = c - 1;
            nd[c].r = c + 1;
            nd[c].u = c;
            nd[c].d = c;
            nd[c].col = c;
        }
        nd[0].l = cols;
        nd[cols].r = 0;
        sz.assign(cols + 1, 0);
        rw.assign(cols + 1, 0);
    }

    // строка r покрывает столбцы ones — добавляем её узлы в матрицу
    void add_row(int r, const vector<int>& ones) {
        vector<int> in_row;
        for (int c : ones) {
            int id = (int)nd.size();
            Node n;
            n.col = c;
            n.u = nd[c].u;
            n.d = c;
            nd[c].u = id;
            nd[n.u].d = id;
            nd.push_back(n);
            sz[c]++;
            rw.push_back(r);
            in_row.push_back(id);
        }
        int m = (int)in_row.size();            // связываем узлы строки
        for (int i = 0; i < m; i++) {
            nd[in_row[i]].l = in_row[(i - 1 + m) % m];
            nd[in_row[i]].r = in_row[(i + 1) % m];
        }
    }

    // удалить столбец c: выключить из горизонтального списка, все его
    // строки — из вертикальных списков их столбцов
    void cover(int c) {
        nd[nd[c].r].l = nd[c].l;
        nd[nd[c].l].r = nd[c].r;
        for (int i = nd[c].d; i != c; i = nd[i].d)
            for (int j = nd[i].r; j != i; j = nd[j].r) {
                nd[nd[j].d].u = nd[j].u;
                nd[nd[j].u].d = nd[j].d;
                sz[nd[j].col]--;
            }
    }

    // восстановить столбец c в обратном порядке
    void uncover(int c) {
        for (int i = nd[c].u; i != c; i = nd[i].u)
            for (int j = nd[i].l; j != i; j = nd[j].l) {
                sz[nd[j].col]++;
                nd[nd[j].u].d = j;
                nd[nd[j].d].u = j;
            }
        nd[nd[c].r].l = c;
        nd[nd[c].l].r = c;
    }

    bool dfs() {
        if (nd[head].r == head) return true;   // все столбцы покрыты
        int c = nd[head].r;                    // MRV: столбец с минимумом единиц
        for (int i = nd[head].r; i != head; i = nd[i].r)
            if (sz[i] < sz[c]) c = i;
        if (sz[c] == 0) return false;          // безнадёжно
        cover(c);
        for (int i = nd[c].d; i != c; i = nd[i].d) {
            st.push_back(rw[i]);
            for (int j = nd[i].r; j != i; j = nd[j].r)
                cover(nd[j].col);
            if (dfs()) return true;
            for (int j = nd[i].l; j != i; j = nd[j].l)
                uncover(nd[j].col);
            st.pop_back();
        }
        uncover(c);
        return false;
    }

    bool solve() {
        st.clear();
        return dfs();
    }
};

// --- E.3. Ход коня: эвристика Варнсдорфа ---
// На каждом шаге идём в клетку с минимальным числом доступных ходов дальше
// (degree heuristic): сначала закрываем «трудные» клетки, пока они доступны.
// Без эвристики O(8^(n²)) — нереально; с эвристикой доска 8×8 мгновенно.
struct KnightTour {
    int n;
    vector<vector<int>> board;   // номер шага посещения, 0 = не посещено
    int moves[8][2] = {{2,1},{1,2},{-1,2},{-2,1},{-2,-1},{-1,-2},{1,-2},{2,-1}};

    KnightTour(int n) : n(n), board(n, vector<int>(n, 0)) {}

    bool inside(int r, int c) { return r >= 0 && r < n && c >= 0 && c < n; }

    int degree(int r, int c) {
        int cnt = 0;
        for (auto& mv : moves) {
            int nr = r + mv[0], nc = c + mv[1];
            if (inside(nr, nc) && board[nr][nc] == 0) cnt++;
        }
        return cnt;
    }

    bool dfs(int r, int c, int step) {
        board[r][c] = step;
        if (step == n * n) return true;
        vector<array<int, 3>> cand;   // {degree, r, c}
        for (auto& mv : moves) {
            int nr = r + mv[0], nc = c + mv[1];
            if (inside(nr, nc) && board[nr][nc] == 0)
                cand.push_back({degree(nr, nc), nr, nc});
        }
        sort(cand.begin(), cand.end());
        for (auto& c3 : cand)
            if (dfs(c3[1], c3[2], step + 1)) return true;
        board[r][c] = 0;
        return false;
    }
};

// --- E.4. Ожерелья: число орбит под действием поворотов (лемма Бернсайда) ---
// N(k,n) = (1/n)·Σ_{d|n} φ(d)·k^{n/d}. Поворот на i позиций оставляет слово
// неподвижным, если оно периодично с периодом gcd(i,n) — таких k^{gcd(i,n)};
// среднее число неподвижных точек по всем поворотам и есть ответ.
// φ(d) — из базы (Divisibility::fi_single), k^m — из базы (ModArithmetic::powll).
long long necklace_count(long long k, long long n) {
    long long sum = 0;
    for (long long d = 1; d * d <= n; d++)
        if (n % d == 0) {
            sum += fi_single(d) * powll(k, n / d);
            if (d * d != n) sum += fi_single(n / d) * powll(k, d);
        }
    return sum / n;
}

// --- E.4. Браслеты: ожерелья + переворот (группа диэдра D_n) ---
// К поворотам добавляются отражения: n·k^((n+1)/2) при нечётном n,
// (n/2)·(k^(n/2+1) + k^(n/2)) при чётном; усредняем по 2n элементам.
long long bracelet_count(long long k, long long n) {
    long long rot = 0;
    for (long long d = 1; d * d <= n; d++)
        if (n % d == 0) {
            rot += fi_single(d) * powll(k, n / d);
            if (d * d != n) rot += fi_single(n / d) * powll(k, d);
        }
    long long refl;
    if (n & 1) refl = n * powll(k, (n + 1) / 2);
    else refl = (n / 2) * (powll(k, n / 2 + 1) + powll(k, n / 2));
    return (rot + refl) / (2 * n);
}

// --- E.5. Ладьи: число расстановок k не бьющих ладей ---
// k ладей на n×n: C(n,k)²·k! (выбрать k строк, k столбцов, биекция между
// ними); на доске m×n: C(m,k)·C(n,k)·k!.
long long rook_placements(int n, int k) {
    return C_iter(n, k) * C_iter(n, k) * fact(k);
}

long long rook_placements_mn(int m, int n, int k) {
    return C_iter(m, k) * C_iter(n, k) * fact(k);
}

// --- E.5. Слоны: расстановки k не бьющих слонов ---
// Клетки одного цвета (r+c mod 2) образуют «лестницу» (доску Феррера):
// "/"-диагонали — столбцы, "\"-диагонали — строки. Длины диагоналей цвета
// len(s) = min(s, 2n−2−s)+1 для s ≡ parity (mod 2); сортируем по возрастанию
// и считаем многочлен ладей на доске Феррера:
//   dp[i][j] = dp[i−1][j] + dp[i−1][j−1]·(h_i − j + 1)
// (j-ю ладью в столбец высоты h_i можно поставить h_i − (j−1) способами).
// Ответ для k слонов = Σ_j ways_even[j]·ways_odd[k−j].
vector<long long> bishop_ways(int n, int parity) {
    vector<int> h;
    for (int s = parity; s <= 2 * n - 2; s += 2)
        h.push_back(min(s, 2 * n - 2 - s) + 1);
    sort(h.begin(), h.end());
    vector<long long> dp(h.size() + 1, 0), ndp;
    dp[0] = 1;
    for (int len : h) {
        ndp = dp;
        for (int j = 1; j <= (int)h.size(); j++)
            if (len >= j) ndp[j] += dp[j - 1] * (len - j + 1);
        dp = ndp;
    }
    return dp;
}

long long bishop_placements(int n, int k) {
    vector<long long> e = bishop_ways(n, 0), o = bishop_ways(n, 1);
    long long res = 0;
    for (int j = 0; j <= k; j++)
        if (j < (int)e.size() && k - j < (int)o.size())
            res += e[j] * o[k - j];
    return res;
}

}; // конец struct Combinatorics

#endif // INSIDE_NUMBER_THEORY

#ifndef INSIDE_NUMBER_THEORY
#ifndef COMBINATORICS_MAIN
signed main() {
    Combinatorics c;
    const int MOD = 1000000007;

    cout << "=== A. Факториалы и перестановки ===" << endl;
    auto f = c.fact_precompute(20, MOD);
    auto inv_f = c.inv_fact_precompute(20, MOD);
    cout << "10! mod p = " << f[10] << endl;
    cout << "5!! = " << c.double_fact(5) << ", 6!! = " << c.double_fact(6) << endl;
    cout << "6!!! (m=3) = " << c.multifact(6, 3) << ", 10!^(2) при n=10,m=2 = " << c.multifact(10, 2) << endl;
    cout << "v2(10!) = " << c.legendre(10, 2) << endl;
    cout << "v2(C(10,3)) = " << c.legendre_C(10, 3, 2) << " (Кюммер: 3 переноса, C(10,3)=120)" << endl;
    cout << "Циклические перестановки C_cyc(5) = " << c.cyclic_perms(5) << endl;
    cout << "Перестановки с повторениями «МАМА» = " << c.perms_with_rep({2, 2}, MOD, f, inv_f) << endl;

    cout << "\n=== A. Степень делителя и факториал по модулю ===" << endl;
    cout << "trailing_zeros(100) = " << c.trailing_zeros(100) << " (ожидаем 24)" << endl;
    cout << "kummer(10,3,2) = " << c.kummer(10, 3, 2) << " vs legendre_C = "
         << c.legendre_C(10, 3, 2) << " (ожидаем 3)" << endl;
    cout << "smallest_n_for_factorial(100) = " << c.smallest_n_for_factorial(100)
         << " (ожидаем 10), для 12 = " << c.smallest_n_for_factorial(12) << " (ожидаем 4)" << endl;
    auto [v, clean] = c.factorial_mod_full(100, 7);
    cout << "factorial_mod_full(100,7): v = " << v << ", clean = " << clean << endl;
    {
        // Сверка с наивным пересчётом n!/p^{v_p(n!)} mod p для малых n
        bool ok = true;
        for (long long p : {3LL, 5LL, 7LL, 11LL, 13LL, 17LL})
            for (long long n = 0; n <= 60; n++) {
                long long brute = 1, x = 1;
                for (long long i = 2; i <= n; i++) {
                    long long t = i;
                    while (t % p == 0) t /= p;
                    x = x * t % p;
                }
                if (n == 0) x = 1;
                brute = x;
                if (c.factorial_mod_clean(n, p) != brute) {
                    cout << "  FAIL clean(" << n << ", " << p << "): got "
                         << c.factorial_mod_clean(n, p) << " want " << brute << endl;
                    ok = false;
                }
            }
        cout << (ok ? "  clean(n,p) vs n!/p^v mod p: OK (все p в {3,5,7,11,13,17}, n ≤ 60)"
                    : "  clean: FAIL") << endl;
    }
    cout << "n! mod составной: 10! mod 9 = " << c.fact_mod_composite(10, 9)
         << " (ожидаем 0), 10! mod 100 = " << c.fact_mod_composite(10, 100)
         << " (ожидаем 0), 4! mod 10 = " << c.fact_mod_composite(4, 10)
         << " (ожидаем 4), 7! mod 6 = " << c.fact_mod_composite(7, 6)
         << " (ожидаем 0)" << endl;
    {
        // Сверка fact_mod_composite с наивным пересчётом n! mod m
        bool ok = true;
        for (long long m = 2; m <= 30; m++)
            for (long long n = 0; n <= 20; n++) {
                long long brute = 1 % m;
                for (long long i = 2; i <= n; i++) brute = brute * i % m;
                if (c.fact_mod_composite(n, m) != brute) {
                    cout << "  FAIL fact_mod_composite(" << n << ", " << m << "): got "
                         << c.fact_mod_composite(n, m) << " want " << brute << endl;
                    ok = false;
                }
            }
        cout << (ok ? "  fact_mod_composite(n,m) vs naive: OK (m ≤ 30, n ≤ 20)"
                    : "  fact_mod_composite: FAIL") << endl;
    }

    cout << "\n=== A. Специальные системы счисления ===" << endl;
    vector<pair<long long, string>> bt = {{5, "1TT"}, {7, "1T1"}, {-5, "T11"}, {0, "0"},
                                          {1, "1"}, {-1, "T"}, {2, "1T"}, {-7, "T1T"}};
    for (auto& pr : bt) {
        auto d = c.to_balanced_ternary(pr.first);
        long long back = c.from_balanced_ternary(d);
        cout << pr.first << " → " << c.to_string_bt(d) << " (ожид: " << pr.second << ")"
             << " → back=" << back << (back == pr.first && c.to_string_bt(d) == pr.second ? " OK" : " FAIL") << endl;
    }
    for (long long n : {0LL, 1LL, 2LL, 5LL, 10LL, 46LL, 100LL}) {
        auto d = c.to_factorial_base(n);
        long long back = c.from_factorial_base(d);
        cout << n << " → " << c.to_string_fb(d) << " → back=" << back
             << (back == n ? " OK" : " FAIL") << endl;
    }

    cout << "\n=== A. Биномиальные коэффициенты ===" << endl;
    cout << "C(10,3) = " << c.C_iter(10, 3) << endl;
    cout << "C(10,3) mod p = " << c.C_mod(10, 3, MOD, f, inv_f) << endl;
    cout << "Мультиномиальный (2,2) «МАМА» = " << c.multinomial({2, 2}, MOD, f, inv_f) << endl;
    cout << "Мультиномиальный (3,1,0) в (a1+a2+a3)^4: " << c.multinomial({3, 1, 0}, MOD, f, inv_f) << endl;
    cout << "C(-4,3) = " << c.C_neg(4, 3) << endl;
    cout << "C_rep(5,3) (звёзды и барьеры) = " << c.C_rep(5, 3) << endl;
    cout << "A(10,3) = " << c.arrangements(10, 3) << ", A'(10,3) = " << c.arrangements_rep(10, 3, MOD) << endl;

    cout << "Генерация биномов: Gosper от 0b0011 → " << c.next_comb_gosper(0b0011) << " (ожидаем 5)" << endl;
    auto kp = c.kth_perm(4, 5);
    cout << "5-я (с 0) перестановка {1,2,3,4}: ";
    for (int x : kp) cout << x + 1 << ' ';
    cout << endl;
    cout << "rank({2,1,4,3}) = " << c.perm_rank({1, 0, 3, 2}) << " (ожидаем 7)" << endl;
    vector<int> p = {1, 2, 3};
    c.next_perm(p);
    cout << "next_perm({1,2,3}) → ";
    for (int x : p) cout << x << ' ';
    cout << endl;

    cout << "\n=== A. Люка (по простому) ===" << endl;
    auto f5 = c.fact_precompute(4, 5);
    auto inv_f5 = c.inv_fact_precompute(4, 5);
    cout << "Lucas C(12,5) mod 5 = " << c.lucas(12, 5, 5, f5, inv_f5) << " (ожидаем 2)" << endl;

    cout << "\n=== A. По составному модулю (обобщение) ===" << endl;
    cout << "C(10,3) mod 9 = " << c.C_composite(10, 3, 9) << " (ожидаем 3)" << endl;
    cout << "C(50,10) mod 100 = " << c.C_composite(50, 10, 100)
         << " vs C_iter % 100 = " << c.C_iter(50, 10) % 100 << endl;
    cout << "C(10,3) mod 6 = " << c.C_composite(10, 3, 6) << " (ожидаем 0)" << endl;
    cout << "C(100,50) mod 360 = " << c.C_composite(100, 50, 360) << endl;

    cout << "\n=== B. Включения-исключения ===" << endl;
    vector<unsigned int> forb = {1, 2, 4, 8};   // π(i) ≠ i — беспорядки D(4)
    cout << "Запрещённые позиции D(4) = " << c.forbidden_positions(4, forb)
         << " (ожидаем 9)" << endl;
    cout << "Слова n=3, алфавит 3, обязательные 2 (a и b): "
         << c.words_required(3, 3, 2, MOD, f, inv_f) << " (ожидаем 12)" << endl;
    cout << "Слова, использующие все 3 буквы (сюръекции n=3,k=3): "
         << c.words_required(3, 3, 3, MOD, f, inv_f) << " (ожидаем 6)" << endl;
    cout << "D(4) = " << c.derangements(4, MOD) << ", включениями-исключениями = "
         << c.derangements_incl_excl(4, MOD, f, inv_f) << " (ожидаем 9)" << endl;

    cout << "\n=== D. Генерация комбинаторных объектов ===" << endl;
    cout << "Подмаски 0b101: ";
    for (auto s : c.submasks_of(0b101)) cout << s << ' ';
    cout << endl;
    cout << "gray_encode(5) = " << c.gray_encode(5) << ", decode обратно = "
         << c.gray_decode(c.gray_encode(5)) << endl;
    cout << "Проверка \"(())\": " << c.is_valid_parentheses("(())") << endl;
    cout << "Итеративные скобки n=2: ";
    for (auto& s : c.gen_parentheses_iter(2)) cout << s << ' ';
    cout << endl;

    cout << "\n=== E. Специальные конфигурации ===" << endl;
    cout << "Иосиф(7,3) с 1 = " << c.josephus(7, 3) + 1 << " (ожидаем 4)" << endl;
    cout << "Иосиф быстрый(7,3) = " << c.josephus_fast(7, 3) << endl;
    cout << "Иосиф быстрый(1e18, 2) = " << c.josephus_fast(1000000000000000000LL, 2) << endl;
    cout << "N-Queens(8) = " << c.n_queens_count(8) << " (ожидаем 92)" << endl;

    // Судоку: обычный решатель
    vector<vector<int>> sudoku = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };
    Combinatorics::Sudoku s(sudoku);
    cout << "Судоку (MRV) решено: " << (s.solve() ? "да" : "нет") << endl;

    // Судоку через алгоритм Кнута X (Dancing Links)
    Combinatorics::DancingLinks dl(324);
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            for (int d = 1; d <= 9; d++) {
                int cell = r * 9 + c + 1;
                int rowc = 81 + r * 9 + (d - 1) + 1;
                int colc = 162 + c * 9 + (d - 1) + 1;
                int blkc = 243 + ((r / 3) * 3 + c / 3) * 9 + (d - 1) + 1;
                dl.add_row(r * 81 + c * 9 + (d - 1), {cell, rowc, colc, blkc});
            }
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            if (sudoku[r][c]) {
                int d = sudoku[r][c];
                dl.cover(r * 9 + c + 1);
                dl.cover(81 + r * 9 + (d - 1) + 1);
                dl.cover(162 + c * 9 + (d - 1) + 1);
                dl.cover(243 + ((r / 3) * 3 + c / 3) * 9 + (d - 1) + 1);
            }
    bool ok_dlx = dl.solve();
    vector<vector<int>> grid2 = sudoku;
    for (int opt : dl.st) {
        int d = opt % 9 + 1, c = (opt / 9) % 9, r = opt / 81;
        grid2[r][c] = d;
    }
    cout << "Судоку (DLX) решено: " << (ok_dlx ? "да" : "нет") << endl;
    if (ok_dlx) {
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) cout << grid2[r][c] << ' ';
            cout << endl;
        }
    }

    // Кроссворд
    Combinatorics::Crossword cw({"...", "...", "..."}, {"abc", "bcd", "cda"});
    cout << "Кроссворд решён: " << (cw.solve() ? "да" : "нет") << endl;
    for (auto& row : cw.grid) cout << row << endl;

    // Ход коня
    Combinatorics::KnightTour kt(8);
    cout << "Ход коня 8×8 найден: " << (kt.dfs(0, 0, 1) ? "да" : "нет") << endl;

    cout << "\n=== E. Ожерелья и расстановки на доске ===" << endl;
    cout << "Ожерелья (k=2, n=4) = " << c.necklace_count(2, 4) << " (ожидаем 6), (k=2, n=5) = "
         << c.necklace_count(2, 5) << " (ожидаем 8), (k=3, n=2) = " << c.necklace_count(3, 2)
         << " (ожидаем 6)" << endl;
    cout << "Браслеты (k=2, n=4) = " << c.bracelet_count(2, 4) << " (ожидаем 6), (k=2, n=6) = "
         << c.bracelet_count(2, 6) << " (ожидаем 13), (k=3, n=2) = " << c.bracelet_count(3, 2)
         << " (ожидаем 6)" << endl;
    cout << "Ладьи: 2 на 4×4 = " << c.rook_placements(4, 2) << " (ожидаем 72), 3 на 4×5 = "
         << c.rook_placements_mn(4, 5, 3) << " (ожидаем 240)" << endl;
    cout << "Слоны: 1 на 3×3 = " << c.bishop_placements(3, 1) << " (ожидаем 9), 2 на 3×3 = "
         << c.bishop_placements(3, 2) << " (ожидаем 26), 4 на 3×3 = " << c.bishop_placements(3, 4)
         << " (ожидаем 8)" << endl;
    cout << "Слоны: 2 на 2×2 = " << c.bishop_placements(2, 2) << " (ожидаем 4), 2 на 4×4 = "
         << c.bishop_placements(4, 2) << " (ожидаем 92), 3 на 4×4 = " << c.bishop_placements(4, 3)
         << " (ожидаем 232)" << endl;
    cout << "N-Queens(8) = " << c.n_queens_count(8) << " (ожидаем 92), (10) = "
         << c.n_queens_count(10) << " (ожидаем 724)" << endl;
}
#endif // COMBINATORICS_MAIN
#endif // INSIDE_NUMBER_THEORY
#endif // COMBINATORICS_A_CPP
