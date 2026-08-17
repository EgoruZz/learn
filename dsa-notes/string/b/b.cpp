#ifndef STRING_B_CPP
#define STRING_B_CPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

// =============================================================
// B. ПОИСК ПОДСТРОКИ (STRING MATCHING)
// =============================================================
// Структура md: 1. Наивный поиск
//               → 2. Префикс-функция и KMP
//               → 3. Z-функция
//               → 4. Rabin-Karp
//               → 5. Бойер-Мур
//               → 6. Border array
//               → 7. Специализированные (Манакер)
//               → 8. Поиск с шаблонами
//
// StringMatching — наследует StringContainers (a.cpp).

#ifndef INSIDE_STRING_B
#define INSIDE_STRING_B
#include "../a/a.cpp"
#endif

struct StringMatching : StringContainers {

// =============================================================
// 1. НАИВНЫЙ ПОИСК
// =============================================================
// Все вхождения pat в text; O(n·m) худший.
vector<int> naive_search(const string& text, const string& pat) {
    vector<int> result;
    int n = text.size(), m = pat.size();
    for (int i = 0; i <= n - m; i++) {
        bool match = true;
        for (int j = 0; j < m; j++) {
            if (text[i + j] != pat[j]) { match = false; break; }
        }
        if (match) result.push_back(i);
    }
    return result;
}

// =============================================================
// 2. ПРЕФИКС-ФУНКЦИЯ И KMP
// =============================================================

// --- 2.1. Префикс-функция ---
// pi[i] = длина наибольшего proper-префикса s[0..i], являющегося суффиксом.
// O(n) время.
vector<int> prefix_function(const string& s) {
    int n = s.size();
    vector<int> pi(n, 0);
    for (int i = 1; i < n; i++) {
        int j = pi[i - 1];
        while (j > 0 && s[i] != s[j])
            j = pi[j - 1];
        if (s[i] == s[j]) j++;
        pi[i] = j;
    }
    return pi;
}

// --- 2.2. KMP поиск ---
// Все вхождения pat в text; O(n + m) время.
vector<int> kmp_search(const string& text, const string& pat) {
    vector<int> result;
    string combined = pat + "#" + text;
    vector<int> pi = prefix_function(combined);
    int m = pat.size();
    for (int i = m + 1; i < (int)combined.size(); i++) {
        if (pi[i] == m)
            result.push_back(i - 2 * m);
    }
    return result;
}

// --- 2.3. Свойство периода ---
// Минимальный период строки s; если период не делит длину — период = n.
int min_period(const string& s) {
    vector<int> pi = prefix_function(s);
    int n = s.size();
    int p = n - pi[n - 1];
    return (n % p == 0) ? p : n;
}

// =============================================================
// 3. Z-ФУНКЦИЯ
// =============================================================
// z[i] = длина наибольшего префикса s[i..n), совпадающего с s[0..).
// O(n) время.
vector<int> z_function(const string& s) {
    int n = s.size();
    vector<int> z(n, 0);
    z[0] = n;
    int l = 0, r = 0;
    for (int i = 1; i < n; i++) {
        if (i <= r)
            z[i] = min(r - i + 1, z[i - l]);
        while (i + z[i] < n && s[z[i]] == s[i + z[i]])
            z[i]++;
        if (i + z[i] - 1 > r) {
            l = i;
            r = i + z[i] - 1;
        }
    }
    return z;
}

// Поиск pat через Z-функцию; O(n + m) время.
vector<int> z_search(const string& text, const string& pat) {
    vector<int> result;
    string combined = pat + "#" + text;
    vector<int> z = z_function(combined);
    int m = pat.size();
    for (int i = m + 1; i < (int)combined.size(); i++) {
        if (z[i] == m)
            result.push_back(i - m - 1);
    }
    return result;
}

// =============================================================
// 4. RABIN-KARP (ROLLING HASH)
// =============================================================
// Все вхождения pat в text; O(n + m) ожидаемо.
// Двойное хеширование для детерминированности.
vector<int> rabin_karp(const string& text, const string& pat,
                       long long m1 = 1e9 + 7, long long m2 = 1e9 + 9,
                       long long p = 31) {
    vector<int> result;
    int n = text.size(), m = pat.size();
    if (m > n) return result;

    // Предвычисление p^k
    vector<long long> pow1(n + 1, 1), pow2(n + 1, 1);
    for (int i = 1; i <= n; i++) {
        pow1[i] = pow1[i - 1] * p % m1;
        pow2[i] = pow2[i - 1] * p % m2;
    }

    // Хеш паттерна
    long long h1 = 0, h2 = 0;
    for (int i = 0; i < m; i++) {
        h1 = (h1 * p + pat[i]) % m1;
        h2 = (h2 * p + pat[i]) % m2;
    }

    // Хеш окна в тексте
    long long cur1 = 0, cur2 = 0;
    for (int i = 0; i < n; i++) {
        cur1 = (cur1 * p + text[i]) % m1;
        cur2 = (cur2 * p + text[i]) % m2;
        if (i >= m) {
            cur1 = (cur1 - text[i - m] * pow1[m] % m1 + m1) % m1;
            cur2 = (cur2 - text[i - m] * pow2[m] % m2 + m2) % m2;
        }
        if (i >= m - 1 && cur1 == h1 && cur2 == h2) {
            // Дополнительная проверка символов (защита от коллизий)
            bool match = true;
            for (int j = 0; j < m; j++) {
                if (text[i - m + 1 + j] != pat[j]) { match = false; break; }
            }
            if (match) result.push_back(i - m + 1);
        }
    }
    return result;
}

// =============================================================
// 5. БОЙЕР-МУР
// =============================================================
// Все вхождения pat в text; O(n·m) худший, O(n/m) лучший.
vector<int> boyer_moore(const string& text, const string& pat) {
    vector<int> result;
    int n = text.size(), m = pat.size();
    if (m == 0 || m > n) return result;

    // Bad character rule: последнее вхождение каждого символа в паттерне
    vector<int> bad_char(256, -1);
    for (int i = 0; i < m; i++)
        bad_char[(unsigned char)pat[i]] = i;

    // Good suffix rule (упрощённый: только сдвиг на m при полном совпадении)
    // Полная реализация Good Suffix требует построения suffix border массива
    int shift = 0;
    while (shift <= n - m) {
        int j = m - 1;
        while (j >= 0 && pat[j] == text[shift + j])
            j--;
        if (j < 0) {
            result.push_back(shift);
            shift += (shift + m < n) ? m - bad_char[(unsigned char)text[shift + m]] : 1;
        } else {
            shift += max(1, j - bad_char[(unsigned char)text[shift + j]]);
        }
    }
    return result;
}

// =============================================================
// 7.1. ALGORITM MANAKERA
// =============================================================

// --- 7.1. Bitap Algorithm (Shift-Or) ---
// Поиск pat в text для паттернов m ≤ 64 (размер машинного слова).
// O(n · ⌈m/w⌉) время.
vector<int> bitap_search(const string& text, const string& pat) {
    vector<int> result;
    int m = pat.size();
    if (m == 0 || m > 64) return result;

    // R[ch] — битовая маска: бит j установлен если pat[j] == ch
    uint64_t R[256] = {};
    for (int j = 0; j < m; j++)
        R[(unsigned char)pat[j]] |= (1ULL << j);

    uint64_t state = ~0ULL;  // все биты = 1 (нет совпадений)
    for (int i = 0; i < (int)text.size(); i++) {
        state = (state << 1) | R[(unsigned char)text[i]];
        if (!(state & (1ULL << (m - 1))))  // бит m-1 = совпадение
            result.push_back(i - m + 1);
    }
    return result;
}

// =============================================================
// 7.2. АЛГОРИТМ МАНАКЕРА
// =============================================================
// Все палиндромы в строке за O(n).
// d1[i] — радиус максимального нечётного палиндрома с центром i.
// d2[i] — радиус максимального чётного палиндрома между i−1 и i.
pair<vector<int>, vector<int>> manacher(const string& s) {
    int n = s.size();

    // Нечётные палиндромы
    vector<int> d1(n, 0);
    int l = 0, r = -1;
    for (int i = 0; i < n; i++) {
        int k = (i > r) ? 1 : min(d1[l + r - i], r - i + 1);
        while (i - k >= 0 && i + k < n && s[i - k] == s[i + k]) k++;
        d1[i] = k;
        if (i + k - 1 > r) { l = i - k + 1; r = i + k - 1; }
    }

    // Чётные палиндромы
    vector<int> d2(n, 0);
    l = 0; r = -1;
    for (int i = 0; i < n; i++) {
        int k = (i > r) ? 0 : min(d2[l + r - i + 1], r - i + 1);
        while (i - k - 1 >= 0 && i + k < n && s[i - k - 1] == s[i + k]) k++;
        d2[i] = k;
        if (i + k - 1 > r) { l = i - k; r = i + k - 1; }
    }

    return {d1, d2};
}

// Самый длинный палиндром (подстрока)
string longest_palindrome(const string& s) {
    auto [d1, d2] = manacher(s);
    int best_len = 0, best_center = 0;
    bool is_odd = true;
    for (int i = 0; i < (int)s.size(); i++) {
        if (2 * d1[i] - 1 > best_len) {
            best_len = 2 * d1[i] - 1;
            best_center = i;
            is_odd = true;
        }
        if (2 * d2[i] > best_len) {
            best_len = 2 * d2[i];
            best_center = i;
            is_odd = false;
        }
    }
    if (is_odd)
        return s.substr(best_center - best_len / 2, best_len);
    else
        return s.substr(best_center - best_len / 2, best_len);
}

// Число различных палиндромных подстрок
int count_palindromic_substrings(const string& s) {
    auto [d1, d2] = manacher(s);
    int count = 0;
    for (int i = 0; i < (int)s.size(); i++)
        count += d1[i] + d2[i];
    return count;
}

// =============================================================
// 8. ПОИСК С ШАБЛОНАМИ
// =============================================================

// --- 8.1. Wildcard Pattern Matching ---
// ? — один символ, * — любая последовательность (включая пустую).
// O(n·m) время, O(m) память.
bool wildcard_match(const string& s, const string& pat) {
    int n = s.size(), m = pat.size();
    vector<bool> dp(m + 1, false), prev(m + 1, false);
    prev[0] = true;
    for (int j = 1; j <= m; j++)
        prev[j] = prev[j - 1] && pat[j - 1] == '*';

    for (int i = 1; i <= n; i++) {
        dp[0] = false;
        for (int j = 1; j <= m; j++) {
            if (pat[j - 1] == '*')
                dp[j] = dp[j - 1] || prev[j];
            else if (pat[j - 1] == '?' || s[i - 1] == pat[j - 1])
                dp[j] = prev[j - 1];
            else
                dp[j] = false;
        }
        swap(dp, prev);
    }
    return prev[m];
}

// --- 8.2. Regex Matching ---
// . — один символ, * — ноль и более повторений предыдущего символа,
// | — alternation (дизъюнкция).
// O(n·m) время, O(n·m) память.
bool regex_match(const string& s, const string& pat) {
    // Обработка alternation: разбиваем по | и проверяем каждую альтернативу
    vector<string> alternatives;
    string current;
    int depth = 0;
    for (char c : pat) {
        if (c == '|' && depth == 0) {
            alternatives.push_back(current);
            current.clear();
        } else {
            if (c == '(') depth++;
            if (c == ')') depth--;
            current += c;
        }
    }
    alternatives.push_back(current);

    // DP для одной альтернативы (без |)
    auto match_one = [&](const string& s, const string& pat) -> bool {
        int n = s.size(), m = pat.size();
        vector<vector<bool>> dp(n + 1, vector<bool>(m + 1, false));
        dp[0][0] = true;
        for (int j = 1; j <= m; j++)
            if (pat[j - 1] == '*') dp[0][j] = dp[0][j - 2];
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= m; j++) {
                if (pat[j - 1] == '*') {
                    dp[i][j] = dp[i][j - 2];
                    if (pat[j - 2] == '.' || pat[j - 2] == s[i - 1])
                        dp[i][j] = dp[i][j] || dp[i - 1][j];
                } else if (pat[j - 1] == '.' || pat[j - 1] == s[i - 1]) {
                    dp[i][j] = dp[i - 1][j - 1];
                }
            }
        }
        return dp[n][m];
    };

    for (auto& alt : alternatives)
        if (match_one(s, alt)) return true;
    return false;
}

}; // struct StringMatching

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_B_MAIN
int main() {
    StringMatching sm;

    cout << "=== Naive ===" << endl;
    auto r1 = sm.naive_search("ababcabcababc", "abc");
    for (int i : r1) cout << i << " "; cout << endl;

    cout << "\n=== KMP ===" << endl;
    auto r2 = sm.kmp_search("ababcabcababc", "abc");
    for (int i : r2) cout << i << " "; cout << endl;
    auto pi = sm.prefix_function("ababcabcababc");
    cout << "prefix func: ";
    for (int x : pi) cout << x << " "; cout << endl;
    cout << "min period of 'abcabc': " << sm.min_period("abcabc") << endl;

    cout << "\n=== Z-function ===" << endl;
    auto r3 = sm.z_search("ababcabcababc", "abc");
    for (int i : r3) cout << i << " "; cout << endl;
    auto z = sm.z_function("ababcabcababc");
    cout << "z-func: ";
    for (int x : z) cout << x << " "; cout << endl;

    cout << "\n=== Rabin-Karp ===" << endl;
    auto r4 = sm.rabin_karp("ababcabcababc", "abc");
    for (int i : r4) cout << i << " "; cout << endl;

    cout << "\n=== Boyer-Moore ===" << endl;
    auto r5 = sm.boyer_moore("ababcabcababc", "abc");
    for (int i : r5) cout << i << " "; cout << endl;

    cout << "\n=== Bitap ===" << endl;
    auto r6 = sm.bitap_search("ababcabcababc", "abc");
    for (int i : r6) cout << i << " "; cout << endl;

    cout << "\n=== Manacher ===" << endl;
    string s = "abacabad";
    auto [d1, d2] = sm.manacher(s);
    cout << "d1 (odd):  "; for (int x : d1) cout << x << " "; cout << endl;
    cout << "d2 (even): "; for (int x : d2) cout << x << " "; cout << endl;
    cout << "longest palindrome: " << sm.longest_palindrome(s) << endl;
    cout << "count palindromic substrings: " << sm.count_palindromic_substrings(s) << endl;

    cout << "\n=== Wildcard ===" << endl;
    cout << sm.wildcard_match("hello", "h*o") << endl;
    cout << sm.wildcard_match("hello", "h*l?") << endl;

    cout << "\n=== Regex ===" << endl;
    cout << sm.regex_match("aab", "c*a*b") << endl;
    cout << sm.regex_match("mississippi", "mis*is*p*.") << endl;

    return 0;
}
#endif

#endif // STRING_B_CPP
