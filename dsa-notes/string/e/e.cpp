#ifndef STRING_E_CPP
#define STRING_E_CPP

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
using namespace std;

// =============================================================
// E. СТРОКОВЫЕ АВТОМАТЫ И СТРУКТУРЫ
// =============================================================
// Структура md: 1. Trie
//               → 2. Ахо-Корасик
//               → 3. Суффиксный массив + LCP
//               → 4. Суффиксный автомат
//               → 5. Дерево палиндромов (Eertree)
//               → 6. Суффиксное дерево (упрощённо)
//
// StringAutomata — наследует StringHashing (d.cpp).

#ifndef INSIDE_STRING_E
#define INSIDE_STRING_E
#include "../d/d.cpp"
#endif

struct StringAutomata : StringHashing {

// =============================================================
// 1. ПРЕФИКСНОЕ ДЕРЕВО (TRIE)
// =============================================================

// ПараметрSigma — размер алфавита (по умолчанию 26 для a-z).
template<int Sigma = 26>
struct Trie {
    struct Node {
        int children[Sigma] = {};
        int count = 0;
        int words = 0;
        Node() { fill(begin(children), end(children), 0); }
    };
    vector<Node> nodes = {Node()};

    void insert(const string& s) {
        int v = 0;
        for (char c : s) {
            int ch = c - 'a';
            if (!nodes[v].children[ch]) {
                nodes[v].children[ch] = nodes.size();
                nodes.push_back(Node());
            }
            v = nodes[v].children[ch];
            nodes[v].count++;
        }
        nodes[v].words++;
    }

    bool search(const string& s) const {
        int v = 0;
        for (char c : s) {
            int ch = c - 'a';
            if (!nodes[v].children[ch]) return false;
            v = nodes[v].children[ch];
        }
        return nodes[v].words > 0;
    }

    bool starts_with(const string& prefix) const {
        int v = 0;
        for (char c : prefix) {
            int ch = c - 'a';
            if (!nodes[v].children[ch]) return false;
            v = nodes[v].children[ch];
        }
        return true;
    }

    int count_words_with_prefix(const string& prefix) const {
        int v = 0;
        for (char c : prefix) {
            int ch = c - 'a';
            if (!nodes[v].children[ch]) return 0;
            v = nodes[v].children[ch];
        }
        return nodes[v].count;
    }
};

// =============================================================
// 2. АЛГОРИТМ АХО-КОРАСИК
// =============================================================

struct AhoCorasick {
    struct Node {
        int children[26] = {};
        int link = 0;       // failure link
        int out = -1;        // терминальная ссылка (ближайший паттерн)
        vector<int> patterns; // индексы паттернов, заканчивающихся здесь
    };
    vector<Node> nodes = {Node()};
    vector<string> pat_list;

    void add_pattern(const string& pat, int idx) {
        int v = 0;
        for (char c : pat) {
            int ch = c - 'a';
            if (!nodes[v].children[ch]) {
                nodes[v].children[ch] = nodes.size();
                nodes.push_back(Node());
            }
            v = nodes[v].children[ch];
        }
        nodes[v].patterns.push_back(idx);
    }

    void build() {
        queue<int> q;
        for (int c = 0; c < 26; c++) {
            if (nodes[0].children[c]) {
                nodes[nodes[0].children[c]].link = 0;
                q.push(nodes[0].children[c]);
            }
        }
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int c = 0; c < 26; c++) {
                if (nodes[v].children[c]) {
                    int u = nodes[v].children[c];
                    nodes[u].link = nodes[nodes[v].link].children[c];
                    nodes[u].out = nodes[nodes[u].link].patterns.empty()
                                   ? nodes[nodes[u].link].out : nodes[u].link;
                    q.push(u);
                } else {
                    nodes[v].children[c] = nodes[nodes[v].link].children[c];
                }
            }
        }
    }

    // Поиск всех вхождений: возвращает пары (позиция_конца, индекс_паттерна)
    vector<pair<int,int>> search(const string& text) {
        vector<pair<int,int>> result;
        int v = 0;
        for (int i = 0; i < (int)text.size(); i++) {
            v = nodes[v].children[text[i] - 'a'];
            for (int j = v; j != 0; j = nodes[j].out) {
                for (int pid : nodes[j].patterns)
                    result.push_back({i, pid});
            }
        }
        return result;
    }
};

// =============================================================
// 3. СУФФИКСНЫЙ МАССИВ + LCP
// =============================================================

// --- 3.1. Суффиксный массив (prefix doubling) ---
// O(n log n) время.
vector<int> suffix_array(const string& s) const {
    int n = s.size();
    vector<int> sa(n), rank_val(n), tmp(n);
    for (int i = 0; i < n; i++) { sa[i] = i; rank_val[i] = s[i]; }

    for (int gap = 1; gap < n; gap *= 2) {
        auto cmp = [&](int a, int b) {
            if (rank_val[a] != rank_val[b]) return rank_val[a] < rank_val[b];
            int ra = (a + gap < n) ? rank_val[a + gap] : -1;
            int rb = (b + gap < n) ? rank_val[b + gap] : -1;
            return ra < rb;
        };
        sort(sa.begin(), sa.end(), cmp);
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; i++)
            tmp[sa[i]] = tmp[sa[i-1]] + (cmp(sa[i-1], sa[i]) ? 1 : 0);
        rank_val = tmp;
        if (rank_val[sa[n-1]] == n-1) break;
    }
    return sa;
}

// --- 3.2. LCP массив (Kasai) ---
// O(n) время.
vector<int> lcp_array(const string& s, const vector<int>& sa) const {
    int n = s.size();
    vector<int> rank_val(n), lcp(n-1);
    for (int i = 0; i < n; i++) rank_val[sa[i]] = i;
    int k = 0;
    for (int i = 0; i < n; i++) {
        if (rank_val[i] == 0) { k = 0; continue; }
        int j = sa[rank_val[i] - 1];
        while (i + k < n && j + k < n && s[i+k] == s[j+k]) k++;
        lcp[rank_val[i] - 1] = k;
        if (k > 0) k--;
    }
    return lcp;
}

// Число различных подстрок: n(n+1)/2 − Σ lcp[i].
int count_distinct_substrings_sa(const string& s) {
    auto sa = suffix_array(s);
    auto lcp = lcp_array(s, sa);
    int n = s.size();
    int total = n * (n + 1) / 2;
    for (int x : lcp) total -= x;
    return total;
}

// =============================================================
// 4. СУФФИКСНЫЙ АВТОМАТ
// =============================================================

struct SuffixAutomaton {
    struct State {
        int len = 0, link = -1;
        int next[26] = {};
        int occ = 0;  // число вхождений подстроки
        State() { fill(begin(next), end(next), 0); }
    };
    vector<State> st = {State()};
    int last = 0;

    void extend(char c) {
        int cur = st.size();
        st.push_back(State());
        st[cur].len = st[last].len + 1;
        int p = last;
        while (p >= 0 && !st[p].next[c - 'a']) {
            st[p].next[c - 'a'] = cur;
            p = st[p].link;
        }
        if (p == -1) {
            st[cur].link = 0;
        } else {
            int q = st[p].next[c - 'a'];
            if (st[p].len + 1 == st[q].len) {
                st[cur].link = q;
            } else {
                int clone = st.size();
                st.push_back(st[q]);
                st[clone].len = st[p].len + 1;
                while (p >= 0 && st[p].next[c - 'a'] == q) {
                    st[p].next[c - 'a'] = clone;
                    p = st[p].link;
                }
                st[q].link = st[cur].link = clone;
            }
        }
        last = cur;
        st[cur].occ = 1;
    }

    // Число различных подстрок: Σ (len[v] − len[link[v]])
    int count_distinct_substrings() const {
        int total = 0;
        for (int i = 1; i < (int)st.size(); i++)
            total += st[i].len - st[st[i].link].len;
        return total;
    }
};

// =============================================================
// 5. ДЕРЕВО ПАЛИНДРОМОВ (EERTREE)
// =============================================================

struct PalindromicTree {
    struct Node {
        int len = 0;      // длина палиндрома
        int link = 0;     // суффиксная ссылка
        int next[26] = {};
        int count = 0;    // число вхождений
        Node() { fill(begin(next), end(next), 0); }
    };
    vector<Node> st;
    int last = 0;
    string s;
    int n = 0;

    PalindromicTree() {
        st.push_back(Node());  // корень для нечётных (len = -1)
        st.push_back(Node());  // корень для чётных (len = 0)
        st[0].link = 1;        // нечётные ссылаются на чётные
        st[1].link = 0;        // и наоборот (самоссылка)
    }

    void extend(char c) {
        s += c;
        n++;
        int cur = last;
        // Ищемимальный палиндром, который можно расширить символом c
        while (cur >= 2) {
            int curlen = st[cur].len;
            if (n - 2 - curlen >= 0 && s[n - 2 - curlen] == c) break;
            cur = st[cur].link;
        }
        // Проверяем, существует ли уже такой палиндром
        if (cur >= 0 && st[cur].next[c - 'a']) {
            last = st[cur].next[c - 'a'];
            st[last].count++;
            return;
        }
        // Создаём новый узел
        int now = st.size();
        st.push_back(Node());
        st[now].len = st[cur].len + 2;
        st[cur].next[c - 'a'] = now;

        if (st[now].len == 1) {
            st[now].link = 1;
            st[now].count = 1;
            last = now;
            return;
        }

        // Ищем суффиксную ссылку
        int link = st[cur].link;
        while (link >= 2) {
            int linklen = st[link].len;
            if (n - 2 - linklen >= 0 && s[n - 2 - linklen] == c) break;
            link = st[link].link;
        }
        st[now].link = (link >= 0 && st[link].next[c - 'a']) ? st[link].next[c - 'a'] : 1;
        st[now].count = 1;
        last = now;
    }

    // Число различных палиндромных подстрок
    int count_distinct_palindromes() const {
        return st.size() - 2;  // минус два корня
    }
};

// =============================================================
// 6. СУФФИКСНОЕ ДЕРЕВО (упрощённо — через суффиксный массив)
// =============================================================
// Полная реализация Укконена слишком велика; здесь — обёртка через SA.

struct SuffixTreeLite {
    vector<int> sa;
    vector<int> lcp;
    string s;

    SuffixTreeLite(const string& str, const StringAutomata& sa_obj) : s(str) {
        sa = sa_obj.suffix_array(str);
        lcp = sa_obj.lcp_array(str, sa);
    }

    // Longest common substring двух строк (через конкатенацию)
    static string longest_common_substring(const string& a, const string& b,
                                           const StringAutomata& sa_obj) {
        string combined = a + "#" + b;
        auto sa = sa_obj.suffix_array(combined);
        auto lcp = sa_obj.lcp_array(combined, sa);
        int na = a.size();
        int best_len = 0, best_pos = 0;
        for (int i = 1; i < (int)lcp.size(); i++) {
            int p1 = sa[i-1], p2 = sa[i];
            // Проверяем, что суффиксы из разных строк
            bool p1_in_a = (p1 < na), p2_in_a = (p2 < na);
            if (p1_in_a != p2_in_a && lcp[i] > best_len) {
                best_len = lcp[i];
                best_pos = min(p1, p2);
            }
        }
        return combined.substr(best_pos, best_len);
    }
};

// =============================================================
// 7. ДЕКОМПОЗИЦИЯ ЛИНДОНА (Duval's Algorithm)
// =============================================================

// Наиболее повторяющаяся подстрока: max(lcp[i]) в LCP массиве.
// O(n) время после построения SA + LCP.
string longest_repeating_substring(const string& s) {
    auto sa = suffix_array(s);
    auto lcp = lcp_array(s, sa);
    int best_len = 0, best_pos = 0;
    for (int i = 0; i < (int)lcp.size(); i++) {
        if (lcp[i] > best_len) {
            best_len = lcp[i];
            best_pos = sa[i];
        }
    }
    return s.substr(best_pos, best_len);
}

// Декомпозиция Линдона (Duval's Algorithm).
// Разбиение строки на линдон-множители за O(n).
// Линдон строка: лексикографически наименьший суффикс.
vector<string> lyndon_decomposition(const string& s) {
    vector<string> result;
    int i = 0;
    int n = s.size();
    while (i < n) {
        int j = i + 1, k = i;
        while (j < n && s[k] <= s[j]) {
            if (s[k] < s[j]) k = i;
            else k++;
            j++;
        }
        while (i <= k) {
            result.push_back(s.substr(i, j - k));
            i += j - k;
        }
    }
    return result;
}

// Наименьший циклический сдвиг через линдон-декомпозицию.
string min_cyclic_shift(const string& s) {
    string doubled = s + s;
    auto decomp = lyndon_decomposition(doubled);
    // Первый линдон-множитель начинается в позиции наименьшего сдвига
    int pos = 0;
    for (auto& part : decomp) {
        if (part.size() == s.size()) return part;
        pos += part.size();
    }
    return s;  // fallback
}

// =============================================================
// 8. ТЕРНАРНЫЙ ПОИСКОВЫЙ TRIE (Ternary Search Trie)
// =============================================================
// Экономия памяти vs обычный Trie; каждый узел — 3 указателя.
struct TernarySearchTrie {
    struct Node {
        char ch;
        bool is_word = false;
        Node *left = nullptr, *mid = nullptr, *right = nullptr;
        Node(char c = 0) : ch(c) {}
    };
    Node* root = nullptr;

    void insert(const string& s) { root = insert(root, s, 0); }

    Node* insert(Node* node, const string& s, int d) {
        if (!node) node = new Node(s[d]);
        if (s[d] < node->ch) node->left = insert(node->left, s, d);
        else if (s[d] > node->ch) node->right = insert(node->right, s, d);
        else if (d + 1 < (int)s.size()) node->mid = insert(node->mid, s, d + 1);
        else node->is_word = true;
        return node;
    }

    bool search(const string& s) const { return search(root, s, 0); }

    bool search(Node* node, const string& s, int d) const {
        if (!node) return false;
        if (s[d] < node->ch) return search(node->left, s, d);
        if (s[d] > node->ch) return search(node->right, s, d);
        if (d + 1 == (int)s.size()) return node->is_word;
        return search(node->mid, s, d + 1);
    }

    bool starts_with(const string& prefix) const { return starts_with(root, prefix, 0); }

    bool starts_with(Node* node, const string& prefix, int d) const {
        if (!node) return false;
        if (prefix[d] < node->ch) return starts_with(node->left, prefix, d);
        if (prefix[d] > node->ch) return starts_with(node->right, prefix, d);
        if (d + 1 == (int)prefix.size()) return true;
        return starts_with(node->mid, prefix, d + 1);
    }
};

}; // struct StringAutomata

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_E_MAIN
int main() {
    StringAutomata sa;

    cout << "=== Trie ===" << endl;
    StringAutomata::Trie trie;
    trie.insert("apple");
    trie.insert("app");
    trie.insert("application");
    trie.insert("bat");
    cout << "search 'app': " << trie.search("app") << endl;
    cout << "search 'ap': " << trie.search("ap") << endl;
    cout << "starts_with 'app': " << trie.starts_with("app") << endl;
    cout << "words with prefix 'app': " << trie.count_words_with_prefix("app") << endl;

    cout << "\n=== Aho-Corasick ===" << endl;
    StringAutomata::AhoCorasick ac;
    ac.add_pattern("he", 0);
    ac.add_pattern("she", 1);
    ac.add_pattern("his", 2);
    ac.add_pattern("hers", 3);
    ac.build();
    auto matches = ac.search("ahishers");
    for (auto& [pos, pid] : matches)
        cout << "  pattern " << pid << " ends at " << pos << endl;

    cout << "\n=== Suffix Array ===" << endl;
    string s = "abracadabra";
    auto suf = sa.suffix_array(s);
    cout << "SA: ";
    for (int x : suf) cout << x << " "; cout << endl;
    auto lcp = sa.lcp_array(s, suf);
    cout << "LCP: ";
    for (int x : lcp) cout << x << " "; cout << endl;
    cout << "distinct substrings: " << sa.count_distinct_substrings_sa(s) << endl;

    cout << "\n=== Suffix Automaton ===" << endl;
    StringAutomata::SuffixAutomaton sam;
    for (char c : s) sam.extend(c);
    cout << "distinct substrings: " << sam.count_distinct_substrings() << endl;

    cout << "\n=== Palindromic Tree ===" << endl;
    StringAutomata::PalindromicTree pt;
    for (char c : s) pt.extend(c);
    cout << "distinct palindromes: " << pt.count_distinct_palindromes() << endl;

    cout << "\n=== SuffixTreeLite ===" << endl;
    cout << "LCS('abcde', 'abfde'): "
         << StringAutomata::SuffixTreeLite::longest_common_substring("abcde", "abfde", sa) << endl;

    return 0;
}
#endif

#endif // STRING_E_CPP
