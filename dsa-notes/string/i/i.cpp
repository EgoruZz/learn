#ifndef STRING_I_CPP
#define STRING_I_CPP

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
using namespace std;

// =============================================================
// I. ФОНЕТИЧЕСКИЕ И ИГРОВЫЕ АЛГОРИТМЫ
// =============================================================
// Структура md: 1. Фонетические (Soundex, Metaphone, NYSIIS)
//               → 2. Игры (Word Ladder, Boggle)
//
// PhoneticAlgorithms — наследует StringValidation (h.cpp).

#ifndef INSIDE_STRING_I
#define INSIDE_STRING_I
#include "../h/h.cpp"
#endif

struct PhoneticAlgorithms : StringValidation {

// =============================================================
// 1. ФОНЕТИЧЕСКИЕ АЛГОРИТМЫ
// =============================================================

// --- 1.1. Soundex ---
// Код: первая буква + 3 цифры (по звучанию).
// O(n) время.
string soundex(const string& s) {
    if (s.empty()) return "";
    string result;
    result += toupper(s[0]);

    auto code = [](char c) -> char {
        c = toupper(c);
        if (c == 'B' || c == 'F' || c == 'P' || c == 'V') return '1';
        if (c == 'C' || c == 'G' || c == 'J' || c == 'K' ||
            c == 'Q' || c == 'S' || c == 'X' || c == 'Z') return '2';
        if (c == 'D' || c == 'T') return '3';
        if (c == 'L') return '4';
        if (c == 'M' || c == 'N') return '5';
        if (c == 'R') return '6';
        return '0';  // A, E, I, O, U, H, W, Y — игнорируются
    };

    char prev = code(s[0]);
    for (int i = 1; i < (int)s.size() && (int)result.size() < 4; i++) {
        char c = code(s[i]);
        if (c != '0' && c != prev) {
            result += c;
            prev = c;
        } else if (c != '0') {
            prev = c;  // обновляем предыдущий для подавления дублей
        }
    }

    while ((int)result.size() < 4) result += '0';
    return result;
}

// --- 1.2. Metaphone (упрощённый) ---
// Более точный фонетический код для английского.
// O(n) время.
string metaphone(const string& s) {
    string result;
    string upper = s;
    transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    int n = upper.size();
    for (int i = 0; i < n; i++) {
        char c = upper[i];
        char next = (i + 1 < n) ? upper[i + 1] : '\0';
        char prev = (i > 0) ? upper[i - 1] : '\0';

        // Двойные буквы → одна
        if (c == next && c != 'C' && c != 'G' && c != 'P' && c != 'S' && c != 'T') {
            if (i == 0) result += c;
            continue;
        }

        // Специальные комбинации
        if (c == 'C' && (next == 'E' || next == 'I' || next == 'Y')) { result += 'S'; continue; }
        if (c == 'C') { result += 'K'; continue; }
        if (c == 'G' && (next == 'E' || next == 'I' || next == 'Y') && prev != 'G') { result += 'J'; continue; }
        if (c == 'G') { result += 'K'; continue; }
        if (c == 'S' && next == 'H') { result += 'X'; i++; continue; }
        if (c == 'T' && next == 'H') { result += '0'; i++; continue; }  // 0 = "th"
        if (c == 'P' && next == 'H') { result += 'F'; i++; continue; }
        if (c == 'W' && next == 'R') { result += 'R'; i++; continue; }
        if (c == 'W' && (next == 'A' || next == 'E')) { result += 'A'; i++; continue; }
        if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
            if (i == 0) result += c;
            continue;
        }
        if (c == 'H' && prev && (prev == 'A' || prev == 'E' || prev == 'I' ||
                                  prev == 'O' || prev == 'U')) continue;
        if (c == 'H') { result += 'H'; continue; }
        if (c == 'W' || c == 'Y') continue;
        result += c;
    }
    return result;
}

// --- 1.3. NYSIIS (упрощённый) ---
// O(n) время.
string nysiis(const string& s) {
    string result;
    string upper = s;
    transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper.size() > 0) {
        if (upper[0] == 'A') upper[0] = 'A';
        else if (upper[0] == 'E') upper[0] = 'A';
        else if (upper[0] == 'I') upper[0] = 'A';
        else if (upper[0] == 'O') upper[0] = 'A';
        else if (upper[0] == 'U') upper[0] = 'A';
    }

    result = upper;
    // Упрощённые правила замены (основные паттерны)
    for (int i = 1; i < (int)result.size(); i++) {
        if (result[i] == 'A' || result[i] == 'E' || result[i] == 'I' ||
            result[i] == 'O' || result[i] == 'U') {
            result[i] = 'A';
        }
    }

    // Удаление дублей
    string filtered;
    for (int i = 0; i < (int)result.size(); i++) {
        if (filtered.empty() || result[i] != filtered.back())
            filtered += result[i];
    }

    // Удаление окончания
    if (filtered.size() > 1) {
        if (filtered.substr(filtered.size() - 2) == "AS" ||
            filtered.substr(filtered.size() - 2) == "AD")
            filtered = filtered.substr(0, filtered.size() - 2);
        if (filtered.back() == 'S' || filtered.back() == 'D')
            filtered.pop_back();
    }

    return filtered;
}

// =============================================================
// 2. ИГРЫ СО СТРОКАМИ
// =============================================================

// --- 2.1. Word Ladder ---
// BFS на графе слов: расстояние Хэмминга = 1.
// Возвращает минимальное число шагов или -1.
int word_ladder(const string& begin, const string& end,
                const unordered_set<string>& word_list) {
    if (begin == end) return 0;
    unordered_set<string> dict = word_list;
    dict.insert(begin);
    queue<pair<string,int>> q;
    q.push({begin, 0});
    unordered_set<string> visited;
    visited.insert(begin);

    while (!q.empty()) {
        auto [word, dist] = q.front(); q.pop();
        string next = word;
        for (int i = 0; i < (int)word.size(); i++) {
            char orig = next[i];
            for (char c = 'a'; c <= 'z'; c++) {
                if (c == orig) continue;
                next[i] = c;
                if (dict.count(next) && !visited.count(next)) {
                    if (next == end) return dist + 1;
                    visited.insert(next);
                    q.push({next, dist + 1});
                }
            }
            next[i] = orig;
        }
    }
    return -1;
}

// --- 2.2. Boggle ---
// DFS из каждой клетки + Trie для проверки префиксов.
// Возвращает множество допустимых слов.
unordered_set<string> boggle(vector<vector<char>> board,
                              const unordered_set<string>& dictionary) {
    int rows = board.size(), cols = board[0].size();
    unordered_set<string> found;

    // Строим Trie через базовую реализацию из e.cpp
    StringAutomata::Trie<26> trie;
    for (auto& word : dictionary)
        trie.insert(word);

    function<void(int,int,int,string&)> dfs = [&](int r, int c, int node, string& path) {
        if (r < 0 || r >= rows || c < 0 || c >= cols) return;
        char ch = tolower(board[r][c]);
        if (ch < 'a' || ch > 'z') return;
        int idx = ch - 'a';

        // Проверяем через Trie
        if (node >= (int)trie.nodes.size()) return;
        if (!trie.nodes[node].children[idx]) return;

        int next_node = trie.nodes[node].children[idx];
        path += ch;

        if (trie.nodes[next_node].words > 0)
            found.insert(path);

        char orig = board[r][c];
        board[r][c] = '#';
        for (int dr = -1; dr <= 1; dr++)
            for (int dc = -1; dc <= 1; dc++)
                if (dr != 0 || dc != 0)
                    dfs(r + dr, c + dc, next_node, path);
        board[r][c] = orig;
        path.pop_back();
    };

    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++) {
            string path;
            dfs(r, c, 0, path);
        }

    return found;
}

// --- 2.3. Pig Latin ---
// Правила: если слово начинается с гласной → + "way"; если с согласной → перенести
// согласный префикс в конец + "ay".
string pig_latin(const string& word) {
    if (word.empty()) return word;
    string vowels = "aeiouAEIOU";
    if (vowels.find(word[0]) != string::npos)
        return word + "way";
    int i = 0;
    while (i < (int)word.size() && vowels.find(word[i]) == string::npos)
        i++;
    return word.substr(i) + word.substr(0, i) + "ay";
}

// --- 2.4. Scrabble Word Validation ---
// Проверяет, является ли слово допустимым в словаре (через Trie).
bool is_valid_scrabble(const string& word, const vector<string>& dictionary) {
    // Быстрая проверка через set
    static unordered_set<string> dict;
    static bool initialized = false;
    if (!initialized) {
        for (auto& w : dictionary) dict.insert(w);
        initialized = true;
    }
    return dict.count(word) > 0;
}

// Сброс словаря (для тестов).
void reset_scrabble_dict() {
    // В реальной реализации — пересоздание dict
}

}; // struct PhoneticAlgorithms

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_I_MAIN
int main() {
    PhoneticAlgorithms pa;

    cout << "=== Soundex ===" << endl;
    cout << "Robert → " << pa.soundex("Robert") << endl;
    cout << "Rupert → " << pa.soundex("Rupert") << endl;
    cout << "Ashcraft → " << pa.soundex("Ashcraft") << endl;
    cout << "Tymczak → " << pa.soundex("Tymczak") << endl;

    cout << "\n=== Metaphone ===" << endl;
    cout << "KNIGHT → " << pa.metaphone("KNIGHT") << endl;
    cout << "PHOTO → " << pa.metaphone("PHOTO") << endl;
    cout << "SCHWARZ → " << pa.metaphone("SCHWARZ") << endl;

    cout << "\n=== NYSIIS ===" << endl;
    cout << "Robert → " << pa.nysiis("Robert") << endl;
    cout << "Tymczak → " << pa.nysiis("Tymczak") << endl;

    cout << "\n=== Word Ladder ===" << endl;
    unordered_set<string> dict = {"hot","dot","dog","lot","log","cog"};
    cout << "hit → cog: " << pa.word_ladder("hit", "cog", dict) << endl;
    unordered_set<string> dict2 = {"hot","dot","dog","lot","log"};
    cout << "hit → cog: " << pa.word_ladder("hit", "cog", dict2) << endl;

    cout << "\n=== Boggle ===" << endl;
    vector<vector<char>> board = {
        {'G','I','Z'},
        {'U','E','K'},
        {'Q','S','E'}
    };
    unordered_set<string> boggle_dict = {"gee","ise","seek","quez","giz","eke"};
    auto words = pa.boggle(board, boggle_dict);
    cout << "found: ";
    for (auto& w : words) cout << w << " ";
    cout << endl;

    return 0;
}
#endif

#endif // STRING_I_CPP
