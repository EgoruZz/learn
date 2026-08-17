#ifndef STRING_D_CPP
#define STRING_D_CPP

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

// =============================================================
// D. ХЕШИРОВАНИЕ СТРОК
// =============================================================
// Структура md: 1. Полиномиальное хеширование
//               → 2. Двойное хеширование
//               → 3. Применения
//               → 4. Коллизии и безопасность
//
// StringHashing — наследует StringDistances (c.cpp).

#ifndef INSIDE_STRING_D
#define INSIDE_STRING_D
#include "../c/c.cpp"
#endif

struct StringHashing : StringDistances {

// =============================================================
// 1. ПОЛИНОМИАЛЬНОЕ ХЕШИРОВАНИЕ
// =============================================================

// --- 1.1. Полиномиальный хеш (одинарный) ---
// pref[i+1] = pref[i] · p + s[i]; хеш s[l..r) = pref[r] − pref[l] · p^{r−l}.
struct PolyHash {
    long long p, m;
    vector<long long> pref, pow_p;

    PolyHash(const string& s, long long p = 31, long long m = 1e9 + 7)
        : p(p), m(m), pref(s.size() + 1, 0), pow_p(s.size() + 1, 1) {
        for (int i = 0; i < (int)s.size(); i++) {
            pref[i + 1] = (pref[i] * p + s[i]) % m;
            pow_p[i + 1] = pow_p[i] * p % m;
        }
    }

    // Хеш подстроки s[l..r) за O(1)
    long long get(int l, int r) const {
        long long h = (pref[r] - pref[l] * pow_p[r - l] % m + m) % m;
        return h;
    }
};

// --- 1.2. Double Hashing ---
// Два хеша с разными (p₁, m₁) и (p₂, m₂); вероятность коллизии O(1/(m₁·m₂)).
struct DoubleHash {
    PolyHash h1, h2;

    DoubleHash(const string& s,
               long long p1 = 31, long long m1 = 1e9 + 7,
               long long p2 = 37, long long m2 = 1e9 + 9)
        : h1(s, p1, m1), h2(s, p2, m2) {}

    // Пара хешей подстроки s[l..r)
    pair<long long, long long> get(int l, int r) const {
        return {h1.get(l, r), h2.get(l, r)};
    }
};

// --- 1.3. Rolling Hash ---
// Динамическое добавление/удаление символов.
struct RollingHash {
    long long p, m;
    long long hash = 0;
    int size = 0;

    RollingHash(long long p = 31, long long m = 1e9 + 7) : p(p), m(m) {}

    // Добавление символа: h = h · p + ch
    void push_back(char ch) {
        hash = (hash * p + ch) % m;
        size++;
    }

    // Добавление строки
    void push_string(const string& s) {
        for (char c : s) push_back(c);
    }

    long long get() const { return hash; }
    int get_size() const { return size; }
};

// =============================================================
// 3. ПРИМЕНЕНИЯ
// =============================================================

// --- 3.1. Поиск палиндромов через хеш ---
// Проверяет, является ли s[l..r) палиндромом за O(1) после предподсчёта.
bool is_palindrome_hash(const DoubleHash& dh, const string& s, int l, int r) {
    int n = s.size();
    // Хеш прямого s[l..r)
    auto h_fwd = dh.get(l, r);
    // Хеш обратного: s[l..r) в обратном порядке = s[n−r..n−l) в прямом
    auto h_rev = dh.get(n - r, n - l);
    return h_fwd == h_rev;
}

// --- 3.2. Подсчёт различных подстрок через хеш ---
// O(n² log n) время, O(n²) память.
int count_distinct_substrings_hash(const string& s) {
    int n = s.size();
    DoubleHash dh(s);
    set<pair<long long, long long>> hashes;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j <= n; j++)
            hashes.insert(dh.get(i, j));
    return hashes.size();
}

// --- 3.3. Проверка изоморфизма ---
// Две строки изоморфны ⟺ биекция символов; O(n) время, O(Σ) память.
bool is_isomorphic(const string& s, const string& t) {
    if (s.size() != t.size()) return false;
    char map_s[256] = {}, map_t[256] = {};
    for (int i = 0; i < (int)s.size(); i++) {
        unsigned char cs = s[i], ct = t[i];
        if (map_s[cs] == 0 && map_t[ct] == 0) {
            map_s[cs] = ct;
            map_t[ct] = cs;
        } else if (map_s[cs] != ct || map_t[ct] != cs) {
            return false;
        }
    }
    return true;
}

// --- 3.4. Поиск всех вхождений паттерна через хеш ---
// O(n + m) ожидаемо (с двойным хешированием).
vector<int> hash_search(const string& text, const string& pat,
                        long long p1 = 31, long long m1 = 1e9 + 7,
                        long long p2 = 37, long long m2 = 1e9 + 9) {
    vector<int> result;
    int n = text.size(), m = pat.size();
    if (m > n) return result;

    DoubleHash dh_text(text, p1, m1, p2, m2);
    DoubleHash dh_pat(pat, p1, m1, p2, m2);
    auto pat_hash = dh_pat.get(0, m);

    for (int i = 0; i <= n - m; i++) {
        if (dh_text.get(i, i + m) == pat_hash)
            result.push_back(i);
    }
    return result;
}

// =============================================================
// 5. ЦИКЛИЧЕСКИЕ СДВИГИ ЧЕРЕЗ ХЕШ
// =============================================================

// Минимальный циклический сдвиг через хеш + бинарный поиск.
// O(n log n) время.
string min_cyclic_shift_hash(const string& s) {
    int n = s.size();
    if (n == 0) return "";
    string doubled = s + s;
    DoubleHash dh(doubled);
    auto best = dh.get(0, n);
    int best_pos = 0;
    for (int i = 1; i < n; i++) {
        auto cur = dh.get(i, i + n);
        if (cur < best) { best = cur; best_pos = i; }
    }
    return doubled.substr(best_pos, n);
}

// =============================================================
// 6. ТЕСТИРОВАНИЕ КОЛЛИЗИЙ
// =============================================================

// Оценка вероятности коллизии через birthday paradox.
// k — число хешей, m — модуль.
double collision_probability(int k, long long m) {
    return 1.0 - exp(-(double)k * k / (2.0 * m));
}

}; // struct StringHashing

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_D_MAIN
int main() {
    StringHashing sh;

    cout << "=== PolyHash ===" << endl;
    StringHashing::PolyHash ph("hello");
    cout << "hash('hello') = " << ph.get(0, 5) << endl;
    cout << "hash('ell') = " << ph.get(1, 4) << endl;

    cout << "\n=== DoubleHash ===" << endl;
    StringHashing::DoubleHash dh("hello");
    auto [h1, h2] = dh.get(0, 5);
    cout << "double hash('hello') = (" << h1 << ", " << h2 << ")" << endl;

    cout << "\n=== RollingHash ===" << endl;
    StringHashing::RollingHash rh;
    rh.push_string("hello");
    cout << "rolling hash('hello') = " << rh.get() << endl;

    cout << "\n=== Palindrome via Hash ===" << endl;
    string s = "abacaba";
    StringHashing::DoubleHash dh2(s);
    for (int i = 0; i < (int)s.size(); i++)
        for (int j = i + 1; j <= (int)s.size(); j++)
            if (sh.is_palindrome_hash(dh2, s, i, j))
                cout << "palindrome: " << s.substr(i, j - i) << endl;

    cout << "\n=== Distinct Substrings ===" << endl;
    cout << "abab: " << sh.count_distinct_substrings_hash("abab") << endl;
    cout << "abcd: " << sh.count_distinct_substrings_hash("abcd") << endl;

    cout << "\n=== Isomorphic ===" << endl;
    cout << "egg→add: " << sh.is_isomorphic("egg", "add") << endl;
    cout << "foo→bar: " << sh.is_isomorphic("foo", "bar") << endl;

    cout << "\n=== Hash Search ===" << endl;
    auto res = sh.hash_search("ababcabcababc", "abc");
    for (int i : res) cout << i << " "; cout << endl;

    return 0;
}
#endif

#endif // STRING_D_CPP
