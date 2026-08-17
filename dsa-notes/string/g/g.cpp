#ifndef STRING_G_CPP
#define STRING_G_CPP

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
using namespace std;

// =============================================================
// G. СЖАТИЕ СТРОК
// =============================================================
// Структура md: 1. RLE, Huffman, LZ77
//               → 2. BWT, Move-to-Front
//
// StringCompression — наследует StringAnalysis (f.cpp).

#ifndef INSIDE_STRING_G
#define INSIDE_STRING_G
#include "../f/f.cpp"
#endif

struct StringCompression : StringAnalysis {

// =============================================================
// 1. RLE (RUN-LENGTH ENCODING)
// =============================================================
// Кодирование: aaabbc → 3a2b1c. O(n) время.
string rle_encode(const string& s) {
    string result;
    int n = s.size();
    for (int i = 0; i < n; i++) {
        int count = 1;
        while (i + count < n && s[i + count] == s[i]) count++;
        result += to_string(count);
        result += s[i];
        i += count - 1;
    }
    return result;
}

// Декодирование: 3a2b1c → aaabbc. O(m) время.
string rle_decode(const string& encoded) {
    string result;
    int i = 0;
    while (i < (int)encoded.size()) {
        int count = 0;
        while (i < (int)encoded.size() && isdigit(encoded[i]))
            count = count * 10 + (encoded[i++] - '0');
        if (i < (int)encoded.size()) {
            result.append(count, encoded[i++]);
        }
    }
    return result;
}

// =============================================================
// 2. ХАФФМАН (HUFFMAN CODING)
// =============================================================

struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode *left, *right;
    HuffmanNode(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    HuffmanNode(HuffmanNode* l, HuffmanNode* r) : ch(0), freq(l->freq + r->freq), left(l), right(r) {}
};

struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) { return a->freq > b->freq; }
};

// Построение дерева Хаффмана; O(n log n).
// Узлы хранятся в vector для автоматического освобождения.
pair<HuffmanNode*, vector<unique_ptr<HuffmanNode>>> build_huffman_tree(const string& s) {
    map<char,int> freq;
    for (char c : s) freq[c]++;

    vector<unique_ptr<HuffmanNode>> nodes;
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;
    for (auto& [ch, f] : freq) {
        nodes.push_back(make_unique<HuffmanNode>(ch, f));
        pq.push(nodes.back().get());
    }

    while (pq.size() > 1) {
        auto a = pq.top(); pq.pop();
        auto b = pq.top(); pq.pop();
        nodes.push_back(make_unique<HuffmanNode>(a, b));
        pq.push(nodes.back().get());
    }
    return {pq.empty() ? nullptr : pq.top(), std::move(nodes)};
}

// Генерация кодов из дерева.
void gen_codes(HuffmanNode* node, string code, map<char,string>& codes) {
    if (!node) return;
    if (!node->left && !node->right) { codes[node->ch] = code.empty() ? "0" : code; return; }
    gen_codes(node->left, code + "0", codes);
    gen_codes(node->right, code + "1", codes);
}

// Кодирование Хаффмана.
pair<string, map<char,string>> huffman_encode(const string& s) {
    auto [root, nodes] = build_huffman_tree(s);
    map<char,string> codes;
    gen_codes(root, "", codes);
    string encoded;
    for (char c : s) encoded += codes[c];
    return {encoded, codes};
}

// Декодирование Хаффмана.
string huffman_decode(const string& encoded, const map<char,string>& codes) {
    map<string,char> reverse;
    for (auto& [ch, code] : codes) reverse[code] = ch;
    string result;
    string buffer;
    for (char bit : encoded) {
        buffer += bit;
        if (reverse.count(buffer)) {
            result += reverse[buffer];
            buffer.clear();
        }
    }
    return result;
}

// =============================================================
// 3. LZ77
// =============================================================

struct LZ77Token { int offset; int length; char next; };

// Кодирование LZ77. O(n · window_size) время.
vector<LZ77Token> lz77_encode(const string& s, int window_size = 4096) {
    vector<LZ77Token> tokens;
    int n = s.size(), pos = 0;
    while (pos < n) {
        int best_offset = 0, best_length = 0;
        int search_start = max(0, pos - window_size);
        for (int i = search_start; i < pos; i++) {
            int len = 0;
            while (pos + len < n && s[i + len] == s[pos + len] && len < 255)
                len++;
            if (len > best_length) { best_length = len; best_offset = pos - i; }
        }
        char next = (pos + best_length < n) ? s[pos + best_length] : 0;
        tokens.push_back({best_offset, best_length, next});
        pos += best_length + 1;
    }
    return tokens;
}

// Декодирование LZ77. O(m · window_size) время.
string lz77_decode(const vector<LZ77Token>& tokens) {
    string result;
    for (auto& t : tokens) {
        for (int i = 0; i < t.length; i++)
            result += result[result.size() - t.offset];
        if (t.next != 0) result += t.next;
    }
    return result;
}

// =============================================================
// 3.1. LZ78
// =============================================================
// Словарь растёт: каждое новое слово = (index, next_char).
struct LZ78Token { int index; char next; };

vector<LZ78Token> lz78_encode(const string& s) {
    vector<LZ78Token> tokens;
    map<string, int> dict;
    int dict_size = 0;
    string buffer;
    for (char c : s) {
        string candidate = buffer + c;
        if (dict.count(candidate)) {
            buffer = candidate;
        } else {
            tokens.push_back({dict[buffer], c});
            dict[candidate] = ++dict_size;
            buffer.clear();
        }
    }
    if (!buffer.empty())
        tokens.push_back({dict[buffer], '\0'});
    return tokens;
}

string lz78_decode(const vector<LZ78Token>& tokens) {
    vector<string> dict = {""};
    string result;
    for (auto& t : tokens) {
        string entry = dict[t.index] + t.next;
        result += entry;
        dict.push_back(entry);
    }
    return result;
}

// =============================================================
// 3.2. LZW (Lempel-Ziv-Welch)
// =============================================================
// Кодер: словарь растёт симметрично (кодер и декодер).
vector<int> lzw_encode(const string& s, int max_dict_size = 4096) {
    map<string, int> dict;
    for (int i = 0; i < 256; i++)
        dict[string(1, (char)i)] = i;
    int dict_size = 256;

    vector<int> result;
    string buffer;
    for (char c : s) {
        string candidate = buffer + c;
        if (dict.count(candidate)) {
            buffer = candidate;
        } else {
            result.push_back(dict[buffer]);
            if (dict_size < max_dict_size)
                dict[candidate] = dict_size++;
            buffer = string(1, c);
        }
    }
    if (!buffer.empty())
        result.push_back(dict[buffer]);
    return result;
}

string lzw_decode(const vector<int>& codes, int max_dict_size = 4096) {
    vector<string> dict;
    for (int i = 0; i < 256; i++)
        dict.push_back(string(1, (char)i));
    int dict_size = 256;

    string result;
    string prev = dict[codes[0]];
    result += prev;

    for (int i = 1; i < (int)codes.size(); i++) {
        string entry;
        if (codes[i] < dict_size) {
            entry = dict[codes[i]];
        } else {
            entry = prev + prev[0];
        }
        result += entry;
        if (dict_size < max_dict_size)
            dict.push_back(prev + entry[0]);
        prev = entry;
    }
    return result;
}

// =============================================================
// 3.3. Delta Encoding
// =============================================================
// Кодирование: разность между последовательными значениями.
vector<int> delta_encode(const vector<int>& data) {
    if (data.empty()) return {};
    vector<int> result = {data[0]};
    for (int i = 1; i < (int)data.size(); i++)
        result.push_back(data[i] - data[i - 1]);
    return result;
}

vector<int> delta_decode(const vector<int>& encoded) {
    if (encoded.empty()) return {};
    vector<int> result = {encoded[0]};
    for (int i = 1; i < (int)encoded.size(); i++)
        result.push_back(result[i - 1] + encoded[i]);
    return result;
}

// =============================================================
// 4. BWT (BURROWS-WHEELER TRANSFORM)
// =============================================================

// BWT: циклические сдвиги → сортировка → последний столбец.
// O(n log n) время через сортировку.
pair<string, int> bwt_encode(const string& s) {
    int n = s.size();
    string doubled = s + s;
    vector<int> indices(n);
    for (int i = 0; i < n; i++) indices[i] = i;

    sort(indices.begin(), indices.end(), [&](int a, int b) {
        for (int i = 0; i < n; i++)
            if (doubled[a + i] != doubled[b + i])
                return doubled[a + i] < doubled[b + i];
        return false;
    });

    string result;
    int original_pos = 0;
    for (int i = 0; i < n; i++) {
        result += doubled[indices[i] + n - 1];
        if (indices[i] == 0) original_pos = i;
    }
    return {result, original_pos};
}

// Обратное BWT: из последнего столбца восстанавливаем строку.
// O(n) время через first-column/last-column mapping.
string bwt_decode(const string& bwt_str, int original_pos) {
    int n = bwt_str.size();
    // First column: отсортированный bwt_str
    vector<int> count(256, 0);
    for (char c : bwt_str) count[(unsigned char)c]++;

    vector<int> first_col(n);
    vector<int> running(256, 0);
    for (int i = 0; i < n; i++) {
        unsigned char c = bwt_str[i];
        int rank = running[c]++;
        // first_col[i] = позиция bwt_str[i] в first column
        int pos = 0;
        for (int j = 0; j < (unsigned char)c; j++) pos += count[j];
        first_col[i] = pos + rank;
    }

    // Восстановление: TF-mapping
    string result(n, ' ');
    int idx = original_pos;
    for (int i = n - 1; i >= 0; i--) {
        result[i] = bwt_str[idx];
        idx = first_col[idx];
    }
    return result;
}

// =============================================================
// 5. MOVE-TO-FRONT (MTF)
// =============================================================

// Кодирование: каждый символ заменяется индексом в текущем алфавите.
// O(n · Σ) худший, O(n) амортизированно.
vector<int> mtf_encode(const string& s) {
    vector<int> alphabet(256);
    for (int i = 0; i < 256; i++) alphabet[i] = i;
    vector<int> result;
    for (char c : s) {
        unsigned char ch = c;
        int idx = find(alphabet.begin(), alphabet.end(), ch) - alphabet.begin();
        result.push_back(idx);
        alphabet.erase(alphabet.begin() + idx);
        alphabet.insert(alphabet.begin(), ch);
    }
    return result;
}

// Декодирование MTF.
string mtf_decode(const vector<int>& encoded) {
    vector<int> alphabet(256);
    for (int i = 0; i < 256; i++) alphabet[i] = i;
    string result;
    for (int idx : encoded) {
        char ch = alphabet[idx];
        result += ch;
        alphabet.erase(alphabet.begin() + idx);
        alphabet.insert(alphabet.begin(), ch);
    }
    return result;
}

// =============================================================
// 6. BWT ЧЕРЕЗ СУФФИКСНЫЙ МАССИВ (O(n log n))
// =============================================================
// Переиспользует suffix_array из e.cpp.
pair<string, int> bwt_encode_sa(const string& s) {
    int n = s.size();
    auto sa = suffix_array(s);
    string result;
    int original_pos = 0;
    for (int i = 0; i < n; i++) {
        result += s[(sa[i] - 1 + n) % n];
        if (sa[i] == 0) original_pos = i;
    }
    return {result, original_pos};
}

// =============================================================
// 7. АЛГОРИТМ КРОЧЕМОРА (LZ-подобное сжатие)
// =============================================================
// Упрощённая версия: LZ77 с оптимизированным поиском через huck.
struct CrochemoreToken { int offset; int length; char next; };

vector<CrochemoreToken> crochemore_encode(const string& s, int window = 4096) {
    // Используем LZ77 как базу (полная реализация Крочемора сложнее)
    vector<CrochemoreToken> result;
    int n = s.size(), pos = 0;
    while (pos < n) {
        int best_off = 0, best_len = 0;
        int start = max(0, pos - window);
        for (int i = start; i < pos; i++) {
            int len = 0;
            while (pos + len < n && s[i + len] == s[pos + len] && len < 255)
                len++;
            if (len > best_len) { best_len = len; best_off = pos - i; }
        }
        char next = (pos + best_len < n) ? s[pos + best_len] : 0;
        result.push_back({best_off, best_len, next});
        pos += best_len + 1;
    }
    return result;
}

string crochemore_decode(const vector<CrochemoreToken>& tokens) {
    string result;
    for (auto& t : tokens) {
        for (int i = 0; i < t.length; i++)
            result += result[result.size() - t.offset];
        if (t.next != 0) result += t.next;
    }
    return result;
}

}; // struct StringCompression

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_G_MAIN
int main() {
    StringCompression sc;

    cout << "=== RLE ===" << endl;
    string orig = "aaabbbccccdd";
    string encoded = sc.rle_encode(orig);
    cout << orig << " → " << encoded << endl;
    cout << "decoded: " << sc.rle_decode(encoded) << endl;

    cout << "\n=== Huffman ===" << endl;
    string text = "aabbbccddddeeee";
    auto [huff_encoded, codes] = sc.huffman_encode(text);
    cout << "codes: ";
    for (auto& [ch, code] : codes) cout << ch << "=" << code << " ";
    cout << endl;
    cout << "encoded: " << huff_encoded << endl;
    cout << "decoded: " << sc.huffman_decode(huff_encoded, codes) << endl;

    cout << "\n=== LZ77 ===" << endl;
    string lz_text = "abcabcabcabc";
    auto tokens = sc.lz77_encode(lz_text);
    cout << "tokens: ";
    for (auto& t : tokens) cout << "(" << t.offset << "," << t.length << "," << t.next << ") ";
    cout << endl;
    cout << "decoded: " << sc.lz77_decode(tokens) << endl;

    cout << "\n=== BWT ===" << endl;
    string bwt_text = "banana";
    auto [bwt_result, pos] = sc.bwt_encode(bwt_text);
    cout << bwt_text << " → BWT: " << bwt_result << " (pos=" << pos << ")" << endl;
    cout << "decoded: " << sc.bwt_decode(bwt_result, pos) << endl;

    cout << "\n=== Move-to-Front ===" << endl;
    string mtf_text = "banana";
    auto mtf = sc.mtf_encode(mtf_text);
    cout << mtf_text << " → MTF: ";
    for (int x : mtf) cout << x << " ";
    cout << endl;
    cout << "decoded: " << sc.mtf_decode(mtf) << endl;

    return 0;
}
#endif

#endif // STRING_G_CPP
