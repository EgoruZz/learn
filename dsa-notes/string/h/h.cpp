#ifndef STRING_H_CPP
#define STRING_H_CPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cctype>
using namespace std;

// =============================================================
// H. ВАЛИДАЦИЯ И ПАТТЕРНЫ
// =============================================================
// Структура md: 1. Валидация данных
//               → 2. Распознавание паттернов
//               → 3. Регулярные выражения
//
// StringValidation — наследует StringCompression (g.cpp).

#ifndef INSIDE_STRING_H
#define INSIDE_STRING_H
#include "../g/g.cpp"
#endif

struct StringValidation : StringCompression {

// =============================================================
// 1. ВАЛИДАЦИЯ ДАННЫХ
// =============================================================

// --- 1.1. Email (базовая проверка) ---
bool is_valid_email(const string& email) {
    int at = -1, dot = -1;
    for (int i = 0; i < (int)email.size(); i++) {
        if (email[i] == '@') at = i;
        if (email[i] == '.' && i > at) dot = i;
    }
    if (at < 1 || dot < at + 2 || dot >= (int)email.size() - 1) return false;
    for (int i = 0; i < (int)email.size(); i++) {
        char c = email[i];
        if (!isalnum(c) && c != '@' && c != '.' && c != '+' && c != '-' && c != '_')
            return false;
    }
    return true;
}

// --- 1.2. Телефон (E.164: +{code}{number}, общая длина 8-15) ---
bool is_valid_phone(const string& phone) {
    if (phone.empty() || phone[0] != '+') return false;
    if (phone.size() < 8 || phone.size() > 15) return false;
    for (int i = 1; i < (int)phone.size(); i++)
        if (!isdigit(phone[i])) return false;
    return true;
}

// --- 1.3. Luhn Algorithm (кредитные карты, ID) ---
bool luhn_check(const string& number) {
    int n = number.size();
    int sum = 0;
    bool alternate = false;
    for (int i = n - 1; i >= 0; i--) {
        if (!isdigit(number[i])) return false;
        int digit = number[i] - '0';
        if (alternate) {
            digit *= 2;
            if (digit > 9) digit -= 9;
        }
        sum += digit;
        alternate = !alternate;
    }
    return sum % 10 == 0;
}

// --- 1.4. ISBN-13 ---
bool is_valid_isbn13(const string& isbn) {
    if (isbn.size() != 13) return false;
    int sum = 0;
    for (int i = 0; i < 13; i++) {
        if (!isdigit(isbn[i])) return false;
        int digit = isbn[i] - '0';
        sum += digit * ((i % 2 == 0) ? 1 : 3);
    }
    return sum % 10 == 0;
}

// --- 1.5. UPC-A (12 цифр) ---
bool is_valid_upc(const string& upc) {
    if (upc.size() != 12) return false;
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        if (!isdigit(upc[i])) return false;
        sum += (upc[i] - '0') * ((i % 2 == 0) ? 3 : 1);
    }
    return sum % 10 == 0;
}

// =============================================================
// 2. РАСПОЗНАВАНИЕ ПАТТЕРНОВ
// =============================================================

// --- 2.1. Word Pattern ---
// Проверяет: строка s следует паттерну pattern через биекцию.
// "dog cat dog" и "a b a" → true; "dog cat dog" и "a b c" → false.
bool word_pattern(const string& pattern, const string& s) {
    map<char, string> p2w;
    map<string, char> w2p;
    vector<string> words;
    istringstream iss(s);
    string word;
    while (iss >> word) words.push_back(word);
    if (pattern.size() != words.size()) return false;
    for (int i = 0; i < (int)pattern.size(); i++) {
        char p = pattern[i];
        string w = words[i];
        if (p2w.count(p) && p2w[p] != w) return false;
        if (w2p.count(w) && w2p[w] != p) return false;
        p2w[p] = w;
        w2p[w] = p;
    }
    return true;
}

// --- 2.2. Language Detection (упрощённо) ---
// Сравнивает частоты символов с эталонными профилями.
// Возвращает наиболее вероятный язык.
string detect_language(const string& text,
                       const map<string, vector<double>>& profiles) {
    vector<int> freq(26, 0);
    int total = 0;
    for (char c : text) {
        if (isalpha(c)) { freq[tolower(c) - 'a']++; total++; }
    }
    if (total == 0) return "unknown";

    string best = "unknown";
    double best_score = -1;
    for (auto& [lang, profile] : profiles) {
        double score = 0;
        for (int i = 0; i < 26; i++) {
            double observed = (double)freq[i] / total;
            score += min(observed, profile[i]);
        }
        if (score > best_score) { best_score = score; best = lang; }
    }
    return best;
}

// --- 2.3. Проверка DNA-последовательности ---
bool is_valid_dna(const string& dna) {
    for (char c : dna)
        if (c != 'A' && c != 'C' && c != 'G' && c != 'T')
            return false;
    return true;
}

// =============================================================
// 3. РЕГУЛЯРНЫЕ ВЫРАЖЕНИЯ (упрощённый backtracking engine)
// =============================================================
// Поддержка: ., *, +, ?, [abc], [^abc]
// О(n·2^m) худший, но простой.

bool regex_match_simple(const string& s, const string& pat) {
    function<bool(int,int)> match = [&](int si, int pi) -> bool {
        if (pi == (int)pat.size()) return si == (int)s.size();
        // Обработка [^abc] или [abc]
        if (pi + 1 < (int)pat.size() && (pat[pi + 1] == '*' || pat[pi + 1] == '+'
                                          || pat[pi + 1] == '?')) {
            char op = pat[pi + 1];
            bool negate = (pi + 2 < (int)pat.size() && pat[pi + 2] == '^');
            int set_start = pi + 2 + (negate ? 1 : 0);
            int set_end = pat.find(']', set_start);
            if (set_end != -1) {
                string chars = pat.substr(set_start, set_end - set_start);
                int next_pi = set_end + 1;
                bool has_star = (next_pi < (int)pat.size() && pat[next_pi] == '*');
                bool has_plus = (next_pi < (int)pat.size() && pat[next_pi] == '+');
                bool has_quest = (next_pi < (int)pat.size() && pat[next_pi] == '?');
                if (has_star || has_plus || has_quest) next_pi++;

                auto char_in_set = [&](char ch) -> bool {
                    bool found = chars.find(ch) != string::npos;
                    return negate ? !found : found;
                };

                if (has_star || has_quest) {
                    if (match(si, next_pi)) return true;
                }
                if (has_star || has_plus) {
                    for (int k = 1; si + k <= (int)s.size(); k++) {
                        if (!char_in_set(s[si + k - 1])) break;
                        if (match(si + k, next_pi)) return true;
                    }
                } else {
                    if (si < (int)s.size() && char_in_set(s[si]))
                        return match(si + 1, next_pi);
                }
                return false;
            }
        }
        // Обработка .
        if (pat[pi] == '.') {
            if (pi + 1 < (int)pat.size() && pat[pi + 1] == '*') {
                // .* — ноль и более любых символов
                for (int k = 0; si + k <= (int)s.size(); k++)
                    if (match(si + k, pi + 2)) return true;
                return false;
            }
            if (pi + 1 < (int)pat.size() && pat[pi + 1] == '+') {
                if (si >= (int)s.size()) return false;
                for (int k = 1; si + k <= (int)s.size(); k++)
                    if (match(si + k, pi + 2)) return true;
                return false;
            }
            if (si < (int)s.size()) return match(si + 1, pi + 1);
            return false;
        }
        // Обработка * после обычного символа
        if (pi + 1 < (int)pat.size() && pat[pi + 1] == '*') {
            char c = pat[pi];
            // Ноль вхождений
            if (match(si, pi + 2)) return true;
            // Одно и более вхождений
            for (int k = 1; si + k <= (int)s.size(); k++) {
                if (s[si + k - 1] != c) break;
                if (match(si + k, pi + 2)) return true;
            }
            return false;
        }
        if (pi + 1 < (int)pat.size() && pat[pi + 1] == '+') {
            char c = pat[pi];
            if (si >= (int)s.size() || s[si] != c) return false;
            for (int k = 1; si + k <= (int)s.size(); k++) {
                if (s[si + k - 1] != c) break;
                if (match(si + k, pi + 2)) return true;
            }
            return false;
        }
        if (pi + 1 < (int)pat.size() && pat[pi + 1] == '?') {
            if (si < (int)s.size() && s[si] == pat[pi])
                return match(si + 1, pi + 2);
            return match(si, pi + 2);
        }
        // Обычный символ
        if (si < (int)s.size() && s[si] == pat[pi])
            return match(si + 1, pi + 1);
        return false;
    };
    return match(0, 0);
}

}; // struct StringValidation

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_H_MAIN
int main() {
    StringValidation sv;

    cout << "=== Email ===" << endl;
    cout << sv.is_valid_email("user@example.com") << endl;
    cout << sv.is_valid_email("bad@.com") << endl;

    cout << "\n=== Phone ===" << endl;
    cout << sv.is_valid_phone("+1234567890") << endl;
    cout << sv.is_valid_phone("1234567890") << endl;

    cout << "\n=== Luhn ===" << endl;
    cout << "4111111111111111: " << sv.luhn_check("4111111111111111") << endl;
    cout << "1234567890123456: " << sv.luhn_check("1234567890123456") << endl;

    cout << "\n=== ISBN-13 ===" << endl;
    cout << "9780134685991: " << sv.is_valid_isbn13("9780134685991") << endl;

    cout << "\n=== UPC ===" << endl;
    cout << "012345678901: " << sv.is_valid_upc("012345678901") << endl;

    cout << "\n=== Word Pattern ===" << endl;
    cout << sv.word_pattern("abba", "dog cat cat dog") << endl;
    cout << sv.word_pattern("abba", "dog cat cat fish") << endl;

    cout << "\n=== DNA ===" << endl;
    cout << sv.is_valid_dna("ACGTACGT") << endl;
    cout << sv.is_valid_dna("ACGXACGT") << endl;

    cout << "\n=== Regex ===" << endl;
    cout << sv.regex_match_simple("abc", "a.c") << endl;
    cout << sv.regex_match_simple("abc", "ab*c") << endl;
    cout << sv.regex_match_simple("abc", "ab+c") << endl;
    cout << sv.regex_match_simple("abc", "ab?d") << endl;
    cout << sv.regex_match_simple("abc", "[a-c]+") << endl;

    return 0;
}
#endif

// =============================================================
// 4. ДВИЖОК DFA ДЛЯ РЕГУЛЯРНЫХ ВЫРАЖЕНИЙ
// =============================================================

// Упрощённый DFA: поддержка . и * (без |, +, ?).
// Построение: Thompson's construction (NFA) → subset construction (DFA).
// Для простых паттернов без альтернаций.
struct SimpleDFA {
    struct State {
        int next[256] = {};
        bool accepting = false;
        State() { fill(begin(next), end(next), -1); }
    };
    vector<State> states = {State()};

    void build(const string& pat) {
        // NFA states: 0..m (каждый символ — состояние + epsilon transitions)
        int m = pat.size();
        // Для простого паттерна (без |): DFA = NFA (детерминированный)
        // каждое состояние — индекс в паттерне
        states.clear();
        states.resize(m + 1);
        states[m].accepting = true;

        for (int i = 0; i < m; i++) {
            if (pat[i] == '.') {
                // Любой символ ведёт в следующее состояние
                for (int c = 0; c < 256; c++)
                    states[i].next[c] = i + 1;
            } else if (i + 1 < m && pat[i + 1] == '*') {
                // * : ноль или более вхождений pat[i]
                for (int c = 0; c < 256; c++) {
                    if (c == (unsigned char)pat[i] || pat[i] == '.')
                        states[i].next[c] = i;  // повтор
                    else
                        states[i].next[c] = i + 2;  // пропуск
                }
                // Пропуск * :epsilon transition
                states[i].next[(unsigned char)pat[i]] = i;  // повтор
            } else {
                states[i].next[(unsigned char)pat[i]] = i + 1;
            }
        }
    }

    bool match(const string& s) const {
        int state = 0;
        for (char c : s) {
            if (state >= (int)states.size()) return false;
            state = states[state].next[(unsigned char)c];
            if (state == -1) return false;
        }
        return state < (int)states.size() && states[state].accepting;
    }
};

#endif // STRING_H_CPP
