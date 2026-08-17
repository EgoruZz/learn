#ifndef STRING_C_CPP
#define STRING_C_CPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

// =============================================================
// C. СТРОКОВЫЕ РАССТОЯНИЯ И РЕДАКТИРОВАНИЕ
// =============================================================
// Структура md: 1. Левенштейн (Edit Distance)
//               → 2. Дамерау-Левенштейн
//               → 3. Хэмминг
//               → 4. LCS
//               → 5. Longest Common Substring
//               → 6. Джарро-Винклер
//               → 7. Другие метрики
//               → 8. Оптимизации
//
// StringDistances — наследует StringMatching (b.cpp).

#ifndef INSIDE_STRING_C
#define INSIDE_STRING_C
#include "../b/b.cpp"
#endif

struct StringDistances : StringMatching {

// =============================================================
// 1. РАССТОЯНИЕ ЛЕВЕНШТЕЙНА (EDIT DISTANCE)
// =============================================================

// --- 1.1. Edit Distance (минимальное число операций) ---
// O(n·m) время, O(min(n,m)) память (rolling array).
int edit_distance(const string& s, const string& t) {
    int n = s.size(), m = t.size();
    if (n < m) return edit_distance(t, s);
    vector<int> prev(m + 1, 0), cur(m + 1, 0);
    for (int j = 0; j <= m; j++) prev[j] = j;

    for (int i = 1; i <= n; i++) {
        cur[0] = i;
        for (int j = 1; j <= m; j++) {
            if (s[i - 1] == t[j - 1])
                cur[j] = prev[j - 1];
            else
                cur[j] = 1 + min({prev[j], cur[j - 1], prev[j - 1]});
        }
        swap(prev, cur);
    }
    return prev[m];
}

// --- 1.2. Edit Distance с параметризованными стоимостями ---
// cost_ins, cost_del, cost_rep — стоимости вставки, удаления, замены.
// O(n·m) время, O(min(n,m)) память.
int edit_distance_cost(const string& s, const string& t,
                       int cost_ins, int cost_del, int cost_rep) {
    int n = s.size(), m = t.size();
    if (n < m) return edit_distance_cost(t, s, cost_ins, cost_del, cost_rep);
    vector<int> prev(m + 1, 0), cur(m + 1, 0);
    for (int j = 0; j <= m; j++) prev[j] = j * cost_ins;

    for (int i = 1; i <= n; i++) {
        cur[0] = i * cost_del;
        for (int j = 1; j <= m; j++) {
            if (s[i - 1] == t[j - 1])
                cur[j] = prev[j - 1];
            else
                cur[j] = min({prev[j] + cost_del,
                              cur[j - 1] + cost_ins,
                              prev[j - 1] + cost_rep});
        }
        swap(prev, cur);
    }
    return prev[m];
}

// --- 1.3. Edit Distance с восстановлением пути ---
// Возвращает: {расстояние, путь операций}.
// Путь: пары (операция, символ); операции: 'I' (вставка), 'D' (удаление), 'R' (замена), '=' (совпадение)
pair<int, vector<pair<char,char>>> edit_distance_path(const string& s, const string& t) {
    int n = s.size(), m = t.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));
    vector<vector<pair<int,int>>> prev(n + 1, vector<pair<int,int>>(m + 1, {0, 0}));

    for (int i = 0; i <= n; i++) dp[i][0] = i;
    for (int j = 0; j <= m; j++) dp[0][j] = j;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (s[i - 1] == t[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
                prev[i][j] = {i - 1, j - 1};
            } else {
                int ins = dp[i][j - 1] + 1;
                int del = dp[i - 1][j] + 1;
                int rep = dp[i - 1][j - 1] + 1;
                if (ins <= del && ins <= rep) {
                    dp[i][j] = ins; prev[i][j] = {i, j - 1};
                } else if (del <= rep) {
                    dp[i][j] = del; prev[i][j] = {i - 1, j};
                } else {
                    dp[i][j] = rep; prev[i][j] = {i - 1, j - 1};
                }
            }
        }
    }

    // Восстановление пути
    vector<pair<char,char>> ops;
    int i = n, j = m;
    while (i > 0 || j > 0) {
        auto [pi, pj] = prev[i][j];
        if (pi == i - 1 && pj == j - 1) {
            if (s[i - 1] == t[j - 1])
                ops.push_back({'=', s[i - 1]});
            else
                ops.push_back({'R', t[j - 1]});
        } else if (pi == i - 1 && pj == j) {
            ops.push_back({'D', s[i - 1]});
        } else {
            ops.push_back({'I', t[j - 1]});
        }
        i = pi; j = pj;
    }
    reverse(ops.begin(), ops.end());
    return {dp[n][m], ops};
}

// =============================================================
// 2. РАССТОЯНИЕ ДАМЕРАУ-ЛЕВЕНШТЕЙНА
// =============================================================
// + транспозиция соседних символов. O(n·m) время.
int damerau_levenshtein(const string& s, const string& t) {
    int n = s.size(), m = t.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));
    for (int i = 0; i <= n; i++) dp[i][0] = i;
    for (int j = 0; j <= m; j++) dp[0][j] = j;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            int cost = (s[i - 1] == t[j - 1]) ? 0 : 1;
            dp[i][j] = min({dp[i - 1][j] + 1,      // удаление
                            dp[i][j - 1] + 1,      // вставка
                            dp[i - 1][j - 1] + cost}); // замена/совпадение
            // Транспозиция
            if (i > 1 && j > 1 && s[i - 1] == t[j - 2] && s[i - 2] == t[j - 1])
                dp[i][j] = min(dp[i][j], dp[i - 2][j - 2] + 1);
        }
    }
    return dp[n][m];
}

// =============================================================
// 3. РАССТОЯНИЕ ХЭММИНГА
// =============================================================
// Только для строк одинаковой длины. O(n) время.
int hamming_distance(const string& s, const string& t) {
    int n = min(s.size(), t.size());
    int dist = 0;
    for (int i = 0; i < n; i++)
        if (s[i] != t[i]) dist++;
    return dist + abs((int)s.size() - (int)t.size());
}

// =============================================================
// 4. LCS (LONGEST COMMON SUBSEQUENCE)
// =============================================================
// O(n·m) время, O(min(n,m)) память.
int lcs_length(const string& s, const string& t) {
    int n = s.size(), m = t.size();
    if (n < m) return lcs_length(t, s);
    vector<int> prev(m + 1, 0), cur(m + 1, 0);
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (s[i - 1] == t[j - 1])
                cur[j] = prev[j - 1] + 1;
            else
                cur[j] = max(prev[j], cur[j - 1]);
        }
        swap(prev, cur);
        fill(cur.begin(), cur.end(), 0);
    }
    return prev[m];
}

// LCS с восстановлением строки.
string lcs_string(const string& s, const string& t) {
    int n = s.size(), m = t.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            dp[i][j] = (s[i - 1] == t[j - 1]) ? dp[i - 1][j - 1] + 1
                                               : max(dp[i - 1][j], dp[i][j - 1]);
    string result;
    int i = n, j = m;
    while (i > 0 && j > 0) {
        if (s[i - 1] == t[j - 1]) { result += s[i - 1]; i--; j--; }
        else if (dp[i - 1][j] > dp[i][j - 1]) i--;
        else j--;
    }
    reverse(result.begin(), result.end());
    return result;
}

// =============================================================
// 5. LONGEST COMMON SUBSTRING
// =============================================================
// O(n·m) время, O(min(n,m)) память.
string longest_common_substring(const string& s, const string& t) {
    int n = s.size(), m = t.size();
    vector<int> prev(m + 1, 0), cur(m + 1, 0);
    int best_len = 0, best_end = 0;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (s[i - 1] == t[j - 1]) {
                cur[j] = prev[j - 1] + 1;
                if (cur[j] > best_len) {
                    best_len = cur[j];
                    best_end = i;
                }
            } else {
                cur[j] = 0;
            }
        }
        swap(prev, cur);
        fill(cur.begin(), cur.end(), 0);
    }
    return s.substr(best_end - best_len, best_len);
}

// =============================================================
// 6. РАССТОЯНИЕ ДЖАРРО-ВИНКЛЕРА
// =============================================================
// Для коротких строк/имён.
double jaro_distance(const string& s, const string& t) {
    int n = s.size(), m = t.size();
    if (n == 0 && m == 0) return 1.0;
    if (n == 0 || m == 0) return 0.0;

    int match_range = max(n, m) / 2 - 1;
    if (match_range < 0) match_range = 0;
    vector<bool> s_match(n, false), t_match(m, false);
    int matches = 0, transpositions = 0;

    for (int i = 0; i < n; i++) {
        int lo = max(0, i - match_range);
        int hi = min(i + match_range + 1, m);
        for (int j = lo; j < hi; j++) {
            if (t_match[j] || s[i] != t[j]) continue;
            s_match[i] = true;
            t_match[j] = true;
            matches++;
            break;
        }
    }
    if (matches == 0) return 0.0;

    int k = 0;
    for (int i = 0; i < n; i++) {
        if (!s_match[i]) continue;
        while (!t_match[k]) k++;
        if (s[i] != t[k]) transpositions++;
        k++;
    }

    return ((double)matches / n + (double)matches / m +
            (double)(matches - transpositions / 2) / matches) / 3.0;
}

double jaro_winkler(const string& s, const string& t, double prefix_bonus = 0.1) {
    double jaro = jaro_distance(s, t);
    int prefix_len = 0;
    int max_prefix = min(4, (int)min(s.size(), t.size()));
    for (int i = 0; i < max_prefix; i++) {
        if (s[i] == t[i]) prefix_len++;
        else break;
    }
    return jaro + prefix_len * prefix_bonus * (1.0 - jaro);
}

// =============================================================
// 7. ДРУГИЕ МЕТРИКИ СХОДСТВА
// =============================================================

// --- 7.1. Cosine Similarity (через частоты символов) ---
double cosine_similarity(const string& s, const string& t) {
    int freq_s[256] = {}, freq_t[256] = {};
    for (char c : s) freq_s[(unsigned char)c]++;
    for (char c : t) freq_t[(unsigned char)c]++;
    long long dot = 0, norm_s = 0, norm_t = 0;
    for (int i = 0; i < 256; i++) {
        dot += (long long)freq_s[i] * freq_t[i];
        norm_s += (long long)freq_s[i] * freq_s[i];
        norm_t += (long long)freq_t[i] * freq_t[i];
    }
    if (norm_s == 0 || norm_t == 0) return 0.0;
    return (double)dot / (sqrt((double)norm_s) * sqrt((double)norm_t));
}

// --- 7.2. Jaccard Similarity (через множество символов) ---
double jaccard_similarity(const string& s, const string& t) {
    bool set_s[256] = {}, set_t[256] = {};
    for (char c : s) set_s[(unsigned char)c] = true;
    for (char c : t) set_t[(unsigned char)c] = true;
    int inter = 0, union_size = 0;
    for (int i = 0; i < 256; i++) {
        if (set_s[i] || set_t[i]) union_size++;
        if (set_s[i] && set_t[i]) inter++;
    }
    return union_size == 0 ? 0.0 : (double)inter / union_size;
}

// --- 7.3. Sørensen-Dice Coefficient ---
double sorensen_dice(const string& s, const string& t) {
    int freq_s[256] = {}, freq_t[256] = {};
    for (char c : s) freq_s[(unsigned char)c]++;
    for (char c : t) freq_t[(unsigned char)c]++;
    int common = 0;
    for (int i = 0; i < 256; i++)
        common += min(freq_s[i], freq_t[i]);
    return (s.size() + t.size()) == 0 ? 0.0 :
           2.0 * common / (s.size() + t.size());
}

// =============================================================
// 8. ОПТИМИЗАЦИИ
// =============================================================

// --- 8.1. Edit Distance (Ukkonen — обрезка по d) ---
// O(n·d) время для малых расстояний d.
int edit_distance_ukkonen(const string& s, const string& t, int max_dist) {
    int n = s.size(), m = t.size();
    if (abs(n - m) > max_dist) return max_dist + 1;

    vector<int> prev(m + 1, 0), cur(m + 1, 0);
    for (int j = 0; j <= m; j++) prev[j] = j;

    for (int i = 1; i <= n; i++) {
        cur[0] = i;
        int lo = max(1, i - max_dist);
        int hi = min(m, i + max_dist);
        cur[0] = i;
        for (int j = 1; j <= m; j++) {
            if (s[i - 1] == t[j - 1])
                cur[j] = prev[j - 1];
            else
                cur[j] = 1 + min({prev[j], cur[j - 1], prev[j - 1]});
        }
        swap(prev, cur);
    }
    return prev[m];
}

}; // struct StringDistances

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_C_MAIN
int main() {
    StringDistances sd;

    cout << "=== Edit Distance ===" << endl;
    cout << "kitten→sitting: " << sd.edit_distance("kitten", "sitting") << endl;
    cout << "sunday→saturday: " << sd.edit_distance("sunday", "saturday") << endl;

    cout << "\n=== Edit Distance (cost) ===" << endl;
    cout << "kitten→sitting (ins=1,del=1,rep=2): "
         << sd.edit_distance_cost("kitten", "sitting", 1, 1, 2) << endl;

    cout << "\n=== Edit Distance (path) ===" << endl;
    auto [dist, path] = sd.edit_distance_path("kitten", "sitting");
    cout << "distance: " << dist << endl;
    for (auto& [op, ch] : path)
        cout << op << ch << " ";
    cout << endl;

    cout << "\n=== Damerau-Levenshtein ===" << endl;
    cout << "ab→ba: " << sd.damerau_levenshtein("ab", "ba") << endl;
    cout << "abc→acb: " << sd.damerau_levenshtein("abc", "acb") << endl;
    cout << "abc→ca: " << sd.damerau_levenshtein("abc", "ca") << endl;

    cout << "\n=== Hamming ===" << endl;
    cout << "karolin→kathrin: " << sd.hamming_distance("karolin", "kathrin") << endl;

    cout << "\n=== LCS ===" << endl;
    cout << "ABCBDAB→BDCAB: " << sd.lcs_length("ABCBDAB", "BDCAB") << endl;
    cout << "LCS string: " << sd.lcs_string("ABCBDAB", "BDCAB") << endl;

    cout << "\n=== Longest Common Substring ===" << endl;
    cout << "GeeksforGeeks→GeeksQuiz: " << sd.longest_common_substring("GeeksforGeeks", "GeeksQuiz") << endl;

    cout << "\n=== Jaro-Winkler ===" << endl;
    cout << "martha→marhta: " << sd.jaro_winkler("martha", "marhta") << endl;
    cout << "dixon→dicksonx: " << sd.jaro_winkler("dixon", "dicksonx") << endl;

    cout << "\n=== Edit Distance (Ukkonen) ===" << endl;
    cout << "kitten→sitting (max_dist=3): " << sd.edit_distance_ukkonen("kitten", "sitting", 3) << endl;

    return 0;
}
#endif

#endif // STRING_C_CPP
