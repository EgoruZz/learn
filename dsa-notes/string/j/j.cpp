#ifndef STRING_J_CPP
#define STRING_J_CPP

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
using namespace std;

// =============================================================
// J. БИТОВЫЕ И НИЗКОУРОВНЕВЫЕ ОПЕРАЦИИ
// =============================================================
// Структура md: 1. Bitmask, Shift-Or/Shift-And
//               → 2. Кодировки (ASCII, UTF-8, UTF-16, UTF-32)
//
// BitStringOperations — наследует PhoneticAlgorithms (i.cpp).

#ifndef INSIDE_STRING_J
#define INSIDE_STRING_J
#include "../i/i.cpp"
#endif

struct BitStringOperations : PhoneticAlgorithms {

// =============================================================
// 1. БИТОВЫЕ МАНИПУЛЯЦИИ
// =============================================================

// --- 1.1. Bitmask для множества символов ---
// Множество символов из малого алфавита (a-z) через 32-битную маску.
uint32_t char_mask(const string& s) {
    uint32_t mask = 0;
    for (char c : s)
        mask |= (1u << (tolower(c) - 'a'));
    return mask;
}

// Проверка вхождения символа в множество.
bool char_in_mask(char c, uint32_t mask) {
    return mask & (1u << (tolower(c) - 'a'));
}

// Число различных символов в множестве (popcount).
int mask_popcount(uint32_t mask) {
    return __builtin_popcount(mask);
}

// Объединение, пересечение, разность множеств.
uint32_t mask_union(uint32_t a, uint32_t b) { return a | b; }
uint32_t mask_intersection(uint32_t a, uint32_t b) { return a & b; }
uint32_t mask_difference(uint32_t a, uint32_t b) { return a & ~b; }

// --- 1.2. Shift-Or / Shift-And (bit-parallel matching) ---
// Поиск pat в text для малых паттернов (m ≤ 64).
// Shift-Or: отрицательное состояние = совпадение.
// Возвращает все вхождения.
vector<int> shift_or_search(const string& text, const string& pat) {
    vector<int> result;
    int m = pat.size();
    if (m == 0 || m > 64) return result;

    // Маска для каждого символа: R[ch] = биты, где pat[j] == ch
    uint64_t R[256] = {};
    for (int j = 0; j < m; j++)
        R[(unsigned char)pat[j]] |= (1ULL << j);

    uint64_t state = ~0ULL;  // все биты = 1 (нет совпадений)
    for (int i = 0; i < (int)text.size(); i++) {
        state = (state << 1) | R[(unsigned char)text[i]];
        if (!(state & (1ULL << (m - 1))))  // бит m-1 установлен = совпадение
            result.push_back(i - m + 1);
    }
    return result;
}

// --- 1.3. Поиск с нечётким соответствием (k ошибок) через bit-parallel ---
// Каждый «ряд» DP хранится как битовая маска.
// Сложность: O(n · ⌈m/w⌉ · k) для k ошибок.
int count_matches_with_k_errors(const string& text, const string& pat, int k) {
    int n = text.size(), m = pat.size();
    if (m == 0) return n;
    int matches = 0;

    // dp[j] = минимальное число ошибок для подстроки text[0..i) и pat[0..j)
    vector<int> dp(m + 1);
    for (int j = 0; j <= m; j++) dp[j] = j;

    for (int i = 1; i <= n; i++) {
        int prev = dp[0];
        dp[0] = i;
        for (int j = 1; j <= m; j++) {
            int temp = dp[j];
            if (text[i-1] == pat[j-1])
                dp[j] = prev;
            else
                dp[j] = 1 + min({prev, dp[j-1], dp[j]});
            prev = temp;
        }
        if (dp[m] <= k) matches++;
    }
    return matches;
}

// =============================================================
// 2. КОДИРОВКИ
// =============================================================

// --- 2.1. UTF-8: длина символа по первому байту ---
int utf8_char_len(unsigned char first_byte) {
    if (first_byte < 0x80) return 1;
    if ((first_byte & 0xE0) == 0xC0) return 2;
    if ((first_byte & 0xF0) == 0xE0) return 3;
    if ((first_byte & 0xF8) == 0xF0) return 4;
    return 1;  // невалидный — считаем 1 байтом
}

// Число Unicode-символов в UTF-8 строке.
int utf8_strlen(const string& s) {
    int count = 0;
    int i = 0;
    while (i < (int)s.size()) {
        i += utf8_char_len((unsigned char)s[i]);
        count++;
    }
    return count;
}

// Декодирование UTF-8 символа в codepoint.
uint32_t utf8_decode(const string& s, int& pos) {
    unsigned char c = s[pos];
    uint32_t cp;
    int len;
    if (c < 0x80) { cp = c; len = 1; }
    else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; len = 2; }
    else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; len = 3; }
    else { cp = c & 0x07; len = 4; }
    for (int i = 1; i < len; i++)
        cp = (cp << 6) | (s[pos + i] & 0x3F);
    pos += len;
    return cp;
}

// Кодирование codepoint в UTF-8.
string utf8_encode(uint32_t cp) {
    string result;
    if (cp < 0x80) {
        result += (char)cp;
    } else if (cp < 0x800) {
        result += (char)(0xC0 | (cp >> 6));
        result += (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        result += (char)(0xE0 | (cp >> 12));
        result += (char)(0x80 | ((cp >> 6) & 0x3F));
        result += (char)(0x80 | (cp & 0x3F));
    } else {
        result += (char)(0xF0 | (cp >> 18));
        result += (char)(0x80 | ((cp >> 12) & 0x3F));
        result += (char)(0x80 | ((cp >> 6) & 0x3F));
        result += (char)(0x80 | (cp & 0x3F));
    }
    return result;
}

// --- 2.2. UTF-16: определение surrogate pair ---
bool is_utf16_high_surrogate(uint16_t code) {
    return code >= 0xD800 && code <= 0xDBFF;
}

// --- 2.3. Проверка валидности UTF-8 ---
bool is_valid_utf8(const string& s) {
    int i = 0;
    while (i < (int)s.size()) {
        unsigned char c = s[i];
        int expected = utf8_char_len(c);
        if (expected == 1 && c >= 0x80) return false;
        for (int j = 1; j < expected; j++) {
            if (i + j >= (int)s.size()) return false;
            if ((s[i + j] & 0xC0) != 0x80) return false;
        }
        i += expected;
    }
    return true;
}

// --- 2.4. Определение кодировки строки ---
enum Encoding { ENC_ASCII, ENC_UTF8, ENC_UTF16_LE, ENC_UTF16_BE, ENC_UTF32_LE, ENC_UTF32_BE, ENC_UNKNOWN };

Encoding detect_encoding(const string& s) {
    if (s.empty()) return ENC_ASCII;
    // BOM
    if (s.size() >= 3 && (unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF)
        return ENC_UTF8;
    if (s.size() >= 2 && (unsigned char)s[0] == 0xFF &&
        (unsigned char)s[1] == 0xFE) return ENC_UTF16_LE;
    if (s.size() >= 2 && (unsigned char)s[0] == 0xFE &&
        (unsigned char)s[1] == 0xFF) return ENC_UTF16_BE;
    if (s.size() >= 4 && (unsigned char)s[0] == 0x00 &&
        (unsigned char)s[1] == 0x00 && (unsigned char)s[2] == 0xFE &&
        (unsigned char)s[3] == 0xFF) return ENC_UTF32_BE;
    if (s.size() >= 4 && (unsigned char)s[0] == 0xFF &&
        (unsigned char)s[1] == 0xFE && (unsigned char)s[2] == 0x00 &&
        (unsigned char)s[3] == 0x00) return ENC_UTF32_LE;

    // Проверка UTF-8 (без BOM)
    if (is_valid_utf8(s)) {
        bool has_non_ascii = false;
        for (unsigned char c : s)
            if (c >= 0x80) { has_non_ascii = true; break; }
        return has_non_ascii ? ENC_UTF8 : ENC_ASCII;
    }
    return ENC_UNKNOWN;
}

// --- 2.5. Unicode Normalization (упрощённо: NFD/NFC для латиницы) ---
// NFD:分解 составных символов (é → e + ́); NFC: композиция
// Упрощённо для демонстрации.
string to_nfc_latin(const string& s) {
    // Простая замена: é→e, и т.д.
    string result;
    for (int i = 0; i < (int)s.size(); i++) {
        unsigned char c = s[i];
        if (c == 0xC3) {  // начало 2-байтового UTF-8
            if (i + 1 < (int)s.size()) {
                unsigned char next = s[i + 1];
                // é = C3 A9, è = C3 A8
                if (next == 0xA9) { result += 'e'; i++; continue; }
                if (next == 0xA8) { result += 'e'; i++; continue; }
                if (next == 0xA0) { result += 'a'; i++; continue; }
            }
        }
        result += c;
    }
    return result;
}

// --- 2.6. String Encoding Conversion ---
// Конвертация UTF-8 → UTF-32 (массив codepoints).
vector<uint32_t> utf8_to_utf32(const string& s) {
    vector<uint32_t> result;
    int i = 0;
    while (i < (int)s.size()) {
        result.push_back(utf8_decode(s, i));
    }
    return result;
}

// Конвертация UTF-32 → UTF-8.
string utf32_to_utf8(const vector<uint32_t>& codepoints) {
    string result;
    for (uint32_t cp : codepoints)
        result += utf8_encode(cp);
    return result;
}

// Конвертация UTF-8 → UTF-16 (массив uint16_t).
vector<uint16_t> utf8_to_utf16(const string& s) {
    vector<uint16_t> result;
    int i = 0;
    while (i < (int)s.size()) {
        uint32_t cp = utf8_decode(s, i);
        if (cp < 0x10000) {
            result.push_back((uint16_t)cp);
        } else {
            // Surrogate pair
            cp -= 0x10000;
            result.push_back(0xD800 + (cp >> 10));
            result.push_back(0xDC00 + (cp & 0x3FF));
        }
    }
    return result;
}

// Конвертация UTF-16 → UTF-8.
string utf16_to_utf8(const vector<uint16_t>& codepoints) {
    string result;
    for (int i = 0; i < (int)codepoints.size(); i++) {
        uint16_t c = codepoints[i];
        if (is_utf16_high_surrogate(c) && i + 1 < (int)codepoints.size()) {
            uint32_t cp = 0x10000 + ((c - 0xD800) << 10) + (codepoints[i+1] - 0xDC00);
            result += utf8_encode(cp);
            i++;
        } else {
            result += utf8_encode(c);
        }
    }
    return result;
}

}; // struct BitStringOperations

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_J_MAIN
int main() {
    BitStringOperations bs;

    cout << "=== Bitmask ===" << endl;
    uint32_t mask = bs.char_mask("hello");
    cout << "mask('hello') = " << mask << endl;
    cout << "popcount = " << bs.mask_popcount(mask) << endl;
    cout << "'e' in mask: " << bs.char_in_mask('e', mask) << endl;
    cout << "'x' in mask: " << bs.char_in_mask('x', mask) << endl;

    cout << "\n=== Shift-Or ===" << endl;
    auto matches = bs.shift_or_search("ababcabcababc", "abc");
    for (int i : matches) cout << i << " "; cout << endl;

    cout << "\n=== Fuzzy Match ===" << endl;
    cout << "k=1 'abc' in 'axbcyabc': "
         << bs.count_matches_with_k_errors("axbcyabc", "abc", 1) << endl;

    cout << "\n=== UTF-8 ===" << endl;
    string utf8_str = "Привет";  // русские буквы — 2 байта каждая
    cout << "strlen(bytes): " << utf8_str.size() << endl;
    cout << "utf8_strlen: " << bs.utf8_strlen(utf8_str) << endl;

    cout << "encode('Я'): ";
    string encoded = bs.utf8_encode(0x42F);  // Я
    for (unsigned char c : encoded) cout << hex << (int)c << " ";
    cout << dec << endl;

    cout << "\n=== Detect Encoding ===" << endl;
    cout << "ASCII: " << bs.detect_encoding("hello") << endl;
    cout << "UTF-8 BOM: " << bs.detect_encoding("\xEF\xBB\xBFhello") << endl;
    cout << "UTF-16 LE BOM: " << bs.detect_encoding("\xFF\xFEh\x00e\x00") << endl;

    cout << "\n=== NFC ===" << endl;
    cout << "NFC('café'): " << bs.to_nfc_latin("café") << endl;

    return 0;
}
#endif

#endif // STRING_J_CPP
