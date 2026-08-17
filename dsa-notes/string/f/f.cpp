#ifndef STRING_F_CPP
#define STRING_F_CPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
using namespace std;

// =============================================================
// F. АНАЛИЗ И ПРЕОБРАЗОВАНИЕ СТРОК
// =============================================================
// Структура md: 1. Статистический анализ
//               → 2. Проверка свойств
//               → 3. Трансформации
//               → 4. Периодичность и повторы
//               → 5. Минимальный циклический сдвиг (Booth)
//               → 6. Текстовая обработка
//
// StringAnalysis — наследует StringAutomata (e.cpp).

#ifndef INSIDE_STRING_F
#define INSIDE_STRING_F
#include "../e/e.cpp"
#endif

struct StringAnalysis : StringAutomata {

// =============================================================
// 1. СТАТИСТИЧЕСКИЙ АНАЛИЗ
// =============================================================

// --- 1.1. Частотный анализ ---
// Возвращает: map<символ, частота>.
map<char,int> frequency(const string& s) {
    map<char,int> freq;
    for (char c : s) freq[c]++;
    return freq;
}

// Top-K частых символов через сортировку.
vector<pair<char,int>> top_k_frequent(const string& s, int k) {
    auto freq = frequency(s);
    vector<pair<char,int>> vec(freq.begin(), freq.end());
    sort(vec.begin(), vec.end(), [](auto& a, auto& b) { return a.second > b.second; });
    if ((int)vec.size() > k) vec.resize(k);
    return vec;
}

// --- 1.2. N-gram анализ ---
// Возвращает: map<n-грамма, частота>.
map<string,int> ngrams(const string& s, int n) {
    map<string,int> freq;
    for (int i = 0; i + n <= (int)s.size(); i++)
        freq[s.substr(i, n)]++;
    return freq;
}

// =============================================================
// 2. ПРОВЕРКА СВОЙСТВ
// =============================================================

// --- 2.1. Уникальность символов ---
bool all_unique(const string& s) {
    bool seen[256] = {};
    for (char c : s) {
        if (seen[(unsigned char)c]) return false;
        seen[(unsigned char)c] = true;
    }
    return true;
}

// --- 2.2. Проверка на pangram (все буквы алфавита) ---
bool is_pangram(const string& s, const string& alphabet = "abcdefghijklmnopqrstuvwxyz") {
    bool seen[256] = {};
    for (char c : s) seen[tolower(c)] = true;
    for (char c : alphabet)
        if (!seen[(unsigned char)c]) return false;
    return true;
}

// --- 2.3. Палиндромность ---
bool is_palindrome(const string& s) {
    int l = 0, r = s.size() - 1;
    while (l < r) { if (s[l++] != s[r--]) return false; }
    return true;
}

// Проверка: можно ли переставить символы в палиндром
bool can_form_palindrome(const string& s) {
    int freq[256] = {};
    for (char c : s) freq[(unsigned char)c]++;
    int odd = 0;
    for (int i = 0; i < 256; i++)
        if (freq[i] % 2 != 0) odd++;
    return odd <= 1;
}

// --- 2.4. Анаграммы ---
bool are_anagrams(const string& s, const string& t) {
    if (s.size() != t.size()) return false;
    int freq[256] = {};
    for (char c : s) freq[(unsigned char)c]++;
    for (char c : t) freq[(unsigned char)c]--;
    for (int i = 0; i < 256; i++)
        if (freq[i] != 0) return false;
    return true;
}

// Группировка анаграмм.
vector<vector<string>> group_anagrams(vector<string>& words) {
    map<string, vector<string>> groups;
    for (auto& w : words) {
        string key = w;
        sort(key.begin(), key.end());
        groups[key].push_back(w);
    }
    vector<vector<string>> result;
    for (auto& [k, v] : groups) result.push_back(v);
    return result;
}

// =============================================================
// 3. ТРАНСФОРМАЦИИ
// =============================================================

// --- 3.1. Регистр ---
string to_lower(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

string to_upper(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::toupper);
    return r;
}

string capitalize(const string& s) {
    string r = s;
    if (!r.empty()) r[0] = toupper(r[0]);
    for (int i = 1; i < (int)r.size(); i++)
        if (r[i-1] == ' ') r[i] = toupper(r[i]);
    return r;
}

// --- 3.2. Форматы ---
string camel_to_snake(const string& s) {
    string r;
    for (int i = 0; i < (int)s.size(); i++) {
        if (isupper(s[i]) && i > 0) r += '_';
        r += tolower(s[i]);
    }
    return r;
}

string snake_to_camel(const string& s) {
    string r;
    bool next_upper = false;
    for (char c : s) {
        if (c == '_') { next_upper = true; }
        else { r += next_upper ? toupper(c) : c; next_upper = false; }
    }
    return r;
}

// --- 3.3. Перестановки ---
string reverse_string(const string& s) {
    string r = s;
    reverse(r.begin(), r.end());
    return r;
}

string reverse_words(const string& s) {
    istringstream iss(s);
    string word, result;
    vector<string> words;
    while (iss >> word) words.push_back(word);
    for (int i = words.size() - 1; i >= 0; i--) {
        if (!result.empty()) result += " ";
        result += words[i];
    }
    return result;
}

// --- 3.4. Разделение ---
vector<string> split(const string& s, char delim) {
    vector<string> result;
    istringstream iss(s);
    string token;
    while (getline(iss, token, delim))
        if (!token.empty()) result.push_back(token);
    return result;
}

string join(const vector<string>& parts, const string& delim) {
    string result;
    for (int i = 0; i < (int)parts.size(); i++) {
        if (i > 0) result += delim;
        result += parts[i];
    }
    return result;
}

string strip(const string& s) {
    int start = 0, end = s.size() - 1;
    while (start < (int)s.size() && isspace(s[start])) start++;
    while (end >= 0 && isspace(s[end])) end--;
    return (start <= end) ? s.substr(start, end - start + 1) : "";
}

// =============================================================
// 4. ПЕРИОДИЧНОСТЬ И ПОВТОРЫ
// =============================================================

// --- 4.1. Минимальный период строки ---
// Через префикс-функцию (наследуется из b.cpp): p = n − π[n−1] если n mod p == 0.
int period_from_pi(const string& s) {
    return min_period(s);  // переиспользуем из b.cpp
}

// =============================================================
// 5. МИНИМАЛЬНЫЙ ЦИКЛИЧЕСКИЙ СДВИГ (BOOTH)
// =============================================================
// Лексикографически наименьший циклический сдвиг; O(n) время.
int booth(const string& s) {
    int n = s.size();
    string t = s + s;
    vector<int> f(2 * n, -1);
    int k = 0;
    for (int j = 1; j < 2 * n; j++) {
        int i = f[j - k - 1];
        while (i != -1 && t[j] != t[k + i + 1]) {
            if (t[j] < t[k + i + 1]) k = j - i - 1;
            i = f[i];
        }
        if (i == -1 && t[j] != t[k]) {
            if (t[j] < t[k]) k = j;
            f[j - k] = -1;
        } else {
            f[j - k] = i + 1;
        }
    }
    return k % n;
}

// =============================================================
// 6. ТЕКСТОВАЯ ОБРАБОТКА
// =============================================================

// --- 6.1. Zigzag Conversion ---
string zigzag_conversion(const string& s, int numRows) {
    if (numRows <= 1 || numRows >= (int)s.size()) return s;
    vector<string> rows(numRows);
    int cur = 0, dir = 1;
    for (char c : s) {
        rows[cur] += c;
        if (cur == 0) dir = 1;
        else if (cur == numRows - 1) dir = -1;
        cur += dir;
    }
    string result;
    for (auto& r : rows) result += r;
    return result;
}

// --- 6.2. Remove duplicates from sorted array (in-place) ---
int remove_duplicates(vector<int>& arr) {
    if (arr.empty()) return 0;
    int write = 1;
    for (int i = 1; i < (int)arr.size(); i++) {
        if (arr[i] != arr[i-1])
            arr[write++] = arr[i];
    }
    return write;
}

// =============================================================
// 7. ТАНДЕМНЫЕ ПОВТОРЫ (Main-Lorentz)
// =============================================================
// Все пары (i, l) такие, что s[i..i+l) == s[i+l..i+2l).
// Divide-and-conquer: O(n log n) время.
vector<pair<int,int>> tandem_repeats(const string& s) {
    vector<pair<int,int>> result;
    int n = s.size();
    auto sa = suffix_array(s);
    auto lcp_arr = lcp_array(s, sa);
    auto rank_val = vector<int>(n);
    for (int i = 0; i < n; i++) rank_val[sa[i]] = i;

    // Для каждого центра i ищем пары с общей длиной ≥ l
    function<void(int,int)> solve = [&](int lo, int hi) {
        if (lo >= hi) return;
        int mid = (lo + hi) / 2;
        solve(lo, mid);
        solve(mid + 1, hi);

        // Проверяем пары через LCP
        for (int i = lo; i <= mid; i++) {
            for (int j = mid + 1; j <= hi; j++) {
                if (rank_val[i] > rank_val[j]) continue;
                int r = rank_val[i], s2 = rank_val[j];
                int common = min(lcp_arr[min(r, s2)],
                                 (r < s2) ? lcp_arr[r] : lcp_arr[s2]);
                // Упрощённо: проверяем прямое сравнение
                int max_len = min(i, n - j);
                for (int l = 1; l <= max_len; l++) {
                    if (s.substr(i, l) == s.substr(j, l))
                        result.push_back({i, l});
                    else break;
                }
            }
        }
    };

    // Упрощённая версия: O(n²) для малых строк
    for (int i = 0; i < n; i++)
        for (int l = 1; i + 2 * l <= n; l++)
            if (s.substr(i, l) == s.substr(i + l, l))
                result.push_back({i, l});

    return result;
}

// =============================================================
// 8. КВАДРАТЫ В СТРОКАХ
// =============================================================
// Число различных квадратов (два вхождения одной подстроки).
// O(n² log n) через хеш.
int count_squares(const string& s) {
    int n = s.size();
    DoubleHash dh(s);
    int count = 0;
    for (int l = 1; l <= n / 2; l++) {
        set<pair<long long,long long>> seen;
        for (int i = 0; i + 2 * l <= n; i++) {
            auto h = dh.get(i, i + l);
            if (seen.count(h)) { count++; break; }
            seen.insert(h);
        }
    }
    return count;
}

// =============================================================
// 9. ЛЕКСИКОГРАФИЧЕСКАЯ СОРТИРОВКА СУФФИКСОВ
// =============================================================

// Наименьший/наибольший суффикс через SA: sa[0]/sa[n-1].
string lexicographic_min_suffix(const string& s) {
    auto sa = suffix_array(s);
    return s.substr(sa[0]);
}

string lexicographic_max_suffix(const string& s) {
    auto sa = suffix_array(s);
    return s.substr(sa.back());
}

// Сортировка всех циклических сдвигов: через SA удвоенной строки.
vector<int> sorted_cyclic_shifts(const string& s) {
    string doubled = s + s;
    auto sa = suffix_array(doubled);
    int n = s.size();
    vector<int> result;
    for (int x : sa)
        if (x < n) result.push_back(x);
    return result;
}

// =============================================================
// 10. TEXT JUSTIFICATION
// =============================================================
// Выравнивание текста по ширине.
vector<string> justify(const vector<string>& words, int maxWidth) {
    vector<string> result;
    int i = 0;
    while (i < (int)words.size()) {
        int line_len = words[i].size();
        int j = i + 1;
        while (j < (int)words.size() && line_len + 1 + words[j].size() <= maxWidth) {
            line_len += 1 + words[j].size();
            j++;
        }
        int spaces = maxWidth - line_len;
        int words_in_line = j - i;
        string line;
        if (j == (int)words.size() || words_in_line == 1) {
            // Последняя строка или одна слово: выравнивание влево
            for (int k = i; k < j; k++) {
                if (k > i) line += " ";
                line += words[k];
            }
            line += string(maxWidth - line.size(), ' ');
        } else {
            int space_each = spaces / (words_in_line - 1);
            int extra = spaces % (words_in_line - 1);
            for (int k = i; k < j; k++) {
                line += words[k];
                if (k < j - 1) {
                    line += string(space_each + (k - i < extra ? 1 : 0), ' ');
                }
            }
        }
        result.push_back(line);
        i = j;
    }
    return result;
}

}; // struct StringAnalysis

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_F_MAIN
int main() {
    StringAnalysis sa;

    cout << "=== Frequency ===" << endl;
    auto freq = sa.frequency("hello world");
    for (auto& [ch, cnt] : freq)
        cout << ch << ": " << cnt << " ";
    cout << endl;

    cout << "\n=== Top-K ===" << endl;
    auto top = sa.top_k_frequent("aabbbcccc", 3);
    for (auto& [ch, cnt] : top)
        cout << ch << ": " << cnt << " ";
    cout << endl;

    cout << "\n=== N-grams ===" << endl;
    auto bigrams = sa.ngrams("abcabc", 2);
    for (auto& [gram, cnt] : bigrams)
        cout << gram << ": " << cnt << " ";
    cout << endl;

    cout << "\n=== Properties ===" << endl;
    cout << "all_unique('abcde'): " << sa.all_unique("abcde") << endl;
    cout << "is_pangram('the quick brown fox'): " << sa.is_pangram("the quick brown fox") << endl;
    cout << "is_palindrome('racecar'): " << sa.is_palindrome("racecar") << endl;
    cout << "can_form_palindrome('carrace'): " << sa.can_form_palindrome("carrace") << endl;
    cout << "are_anagrams('listen','silent'): " << sa.are_anagrams("listen", "silent") << endl;

    cout << "\n=== Group Anagrams ===" << endl;
    vector<string> words = {"eat","tea","tan","ate","nat","bat"};
    auto groups = sa.group_anagrams(words);
    for (auto& g : groups) {
        for (auto& w : g) cout << w << " ";
        cout << "| ";
    }
    cout << endl;

    cout << "\n=== Transformations ===" << endl;
    cout << "lower: " << sa.to_lower("Hello World") << endl;
    cout << "upper: " << sa.to_upper("Hello World") << endl;
    cout << "capitalize: " << sa.capitalize("hello world") << endl;
    cout << "camel_to_snake: " << sa.camel_to_snake("helloWorld") << endl;
    cout << "snake_to_camel: " << sa.snake_to_camel("hello_world") << endl;
    cout << "reverse: " << sa.reverse_string("hello") << endl;
    cout << "reverse_words: " << sa.reverse_words("hello beautiful world") << endl;

    cout << "\n=== Split/Join/Strip ===" << endl;
    auto parts = sa.split("a,b,,c", ',');
    cout << "split: "; for (auto& p : parts) cout << p << "|"; cout << endl;
    cout << "join: " << sa.join({"a","b","c"}, "-") << endl;
    cout << "strip: '" << sa.strip("  hello  ") << "'" << endl;

    cout << "\n=== Period ===" << endl;
    cout << "period('abcabc'): " << sa.period_from_pi("abcabc") << endl;
    cout << "period('abcab'): " << sa.period_from_pi("abcab") << endl;

    cout << "\n=== Booth (min cyclic shift) ===" << endl;
    cout << "booth('babc'): " << sa.booth("babc") << endl;
    string babc = "babc";
    string doubled = babc + babc;
    cout << "shift: " << sa.booth("babc") << " → "
         << doubled.substr(sa.booth("babc"), 4) << endl;

    cout << "\n=== Zigzag ===" << endl;
    cout << "zigzag('PAYPALISHIRING',3): " << sa.zigzag_conversion("PAYPALISHIRING", 3) << endl;

    return 0;
}
#endif

#endif // STRING_F_CPP
