#ifndef TECHNIQUE_D_CPP
#define TECHNIQUE_D_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <climits>
#include <numeric>
#include <bitset>
#include <cstring>
#include <cassert>
using namespace std;

// =============================================================
// IV. СЖАТИЕ ДАННЫХ
// =============================================================
// Структура md: A. Теоретические основы (энтропия, пределы, rate-distortion)
//               → B. Сжатие без потерь (RLE, Хаффман, LZ77/LZ78/LZSS/LZW, BWT+MTF, Delta)
//               → C. Сжатие с потерями (JPEG, JPEG2000, WebP, MP3/AAC/Opus, H.264/H.265/AV1)
//               → D. Специализированные (Coordinate Compression, сжатие графов)
//               → E. Гибридные (DEFLATE, BZip2, LZMA)
//
// DataCompression наследует LinearScan (c.cpp). Переиспользует:
//   * сортировки из I (a.cpp) — построение дерева Хаффмана (min-heap);
//   * бинарный поиск из II (b.cpp) — поиск в окне LZ77, арифметическое кодирование;
//   * скользящее окно из III (c.cpp) — LZ77, RLE-проходы;
//   * std::priority_queue (struct IV) — дерево Хаффмана, BWT-сортировка.
//
// Собственная арифметика: энтропийные вычисления, битовые потоки, преобразования.

// Подключаем цепочку наследования (c.cpp ещё не написан)
#include "../a/a.cpp"
#include "../b/b.cpp"

// =============================================================
// ПРЕДОПРЕДЕЛЕНИЕ LinearScan (c.cpp — ещё не написан)
// =============================================================
struct LinearScan : SearchAlgorithms {
    // placeholder для цепочки наследования
    // методы LinearScan (c.cpp): two pointers, sliding window, sweep line, kadane, simplex, hungarian
};

struct DataCompression : LinearScan {

// =============================================================
// A. ТЕОРЕТИЧЕСКИЕ ОСНОВЫ
// =============================================================

// --- A.1. Энтропия Шеннона ---
// H(X) = −Σ p(x) · log₂(p(x))
// Вычисляется по вектору частот (нормализуется до вероятностей).
static double shannon_entropy(const vector<int>& freq) {
    int total = 0;
    for (int f : freq) total += f;
    if (total == 0) return 0.0;
    double entropy = 0.0;
    for (int f : freq) {
        if (f == 0) continue;
        double p = (double)f / total;
        entropy -= p * log2(p);
    }
    return entropy;
}

// Средняя длина кода Хаффмана: L̄ = Σ p(x) · L(x)
static double avg_code_length(const vector<int>& freq, const vector<int>& code_lengths) {
    int total = 0;
    for (int f : freq) total += f;
    if (total == 0) return 0.0;
    double avg = 0.0;
    for (int i = 0; i < (int)freq.size(); i++) {
        double p = (double)freq[i] / total;
        avg += p * code_lengths[i];
    }
    return avg;
}

// =============================================================
// B. СЖАТИЕ БЕЗ ПОТЕРЬ
// =============================================================

// =============================================================
// B.5. RLE (Run-Length Encoding)
// =============================================================
// Параметры: escape — символ-экранирование (если -1, то без escape)
// Формат выхода: пары (символ, длина)
struct RLEPair {
    int symbol;
    int count;
};

static vector<RLEPair> rle_encode(const vector<int>& data, int escape = -1) {
    vector<RLEPair> result;
    int n = (int)data.size();
    int i = 0;
    while (i < n) {
        int sym = data[i];
        int cnt = 1;
        while (i + cnt < n && data[i + cnt] == sym) cnt++;
        result.push_back({sym, cnt});
        i += cnt;
    }
    return result;
}

static vector<int> rle_decode(const vector<RLEPair>& encoded) {
    vector<int> result;
    for (auto& p : encoded)
        for (int i = 0; i < p.count; i++)
            result.push_back(p.symbol);
    return result;
}

// RLE с escape-символом: для runs длины 1
static vector<int> rle_encode_escape(const vector<int>& data, int escape) {
    vector<int> result;
    int n = (int)data.size();
    int i = 0;
    while (i < n) {
        int sym = data[i];
        int cnt = 1;
        while (i + cnt < n && data[i + cnt] == sym) cnt++;
        if (cnt == 1 && sym == escape) {
            result.push_back(escape);
            result.push_back(escape);
            result.push_back(1);
        } else if (cnt == 1) {
            result.push_back(sym);
        } else {
            result.push_back(escape);
            result.push_back(sym);
            result.push_back(cnt);
        }
        i += cnt;
    }
    return result;
}

static vector<int> rle_decode_escape(const vector<int>& encoded, int escape) {
    vector<int> result;
    int n = (int)encoded.size();
    int i = 0;
    while (i < n) {
        if (encoded[i] == escape) {
            if (i + 2 < n) {
                int sym = encoded[i + 1];
                int cnt = encoded[i + 2];
                for (int j = 0; j < cnt; j++) result.push_back(sym);
                i += 3;
            } else {
                result.push_back(encoded[i]);
                i++;
            }
        } else {
            result.push_back(encoded[i]);
            i++;
        }
    }
    return result;
}

// =============================================================
// B.6. Хаффман (Huffman Coding)
// =============================================================
// Дерево Хаффмана: строится через priority_queue (min-heap).
// Узел дерева: символ, вес, левый/правый孩子.

struct HuffmanNode {
    int symbol;     // -1 для внутренних узлов
    int weight;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(int s, int w) : symbol(s), weight(w), left(nullptr), right(nullptr) {}
    HuffmanNode(HuffmanNode* l, HuffmanNode* r) : symbol(-1), weight(l->weight + r->weight), left(l), right(r) {}
};

struct HuffmanCmp {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->weight > b->weight;
    }
};

// Построение дерева Хаффмана
static HuffmanNode* huffman_build_tree(const vector<int>& freq) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, HuffmanCmp> pq;
    for (int i = 0; i < (int)freq.size(); i++)
        if (freq[i] > 0)
            pq.push(new HuffmanNode(i, freq[i]));

    if (pq.empty()) return nullptr;
    if (pq.size() == 1) {
        HuffmanNode* only = pq.top(); pq.pop();
        return new HuffmanNode(only, new HuffmanNode(-1, 0));
    }

    while (pq.size() > 1) {
        HuffmanNode* a = pq.top(); pq.pop();
        HuffmanNode* b = pq.top(); pq.pop();
        pq.push(new HuffmanNode(a, b));
    }
    return pq.top();
}

// Генерация кодов: обход дерева → вектор кодов (каждый код — строка '0'/'1')
static void huffman_generate_codes(HuffmanNode* node, string code,
                            vector<string>& codes) {
    if (!node) return;
    if (node->symbol != -1) {
        codes[node->symbol] = code.empty() ? "0" : code;
        return;
    }
    huffman_generate_codes(node->left, code + "0", codes);
    huffman_generate_codes(node->right, code + "1", codes);
}

// Кодирование: символы → битовая строка
static string huffman_encode(const vector<int>& data, vector<string>& codes) {
    string result;
    for (int x : data) result += codes[x];
    return result;
}

// Декодирование: битовая строка + дерево → символы
static vector<int> huffman_decode(const string& bitstring, HuffmanNode* root) {
    vector<int> result;
    HuffmanNode* cur = root;
    for (char bit : bitstring) {
        cur = (bit == '0') ? cur->left : cur->right;
        if (cur->symbol != -1) {
            result.push_back(cur->symbol);
            cur = root;
        }
    }
    return result;
}

// Освобождение памяти дерева
static void huffman_free_tree(HuffmanNode* node) {
    if (!node) return;
    huffman_free_tree(node->left);
    huffman_free_tree(node->right);
    delete node;
}

// Полный цикл Хаффмана: данные → коды → закодированная строка → декодированные данные
struct HuffmanResult {
    vector<string> codes;
    string encoded;
    vector<int> decoded;
    HuffmanNode* root;
};

static HuffmanResult huffman_full(const vector<int>& data) {
    int alphabet_size = 0;
    for (int x : data) alphabet_size = max(alphabet_size, x + 1);
    vector<int> freq(alphabet_size, 0);
    for (int x : data) freq[x]++;

    HuffmanNode* root = huffman_build_tree(freq);
    vector<string> codes(alphabet_size);
    huffman_generate_codes(root, "", codes);

    string encoded = huffman_encode(data, codes);
    vector<int> decoded = huffman_decode(encoded, root);

    return {codes, encoded, decoded, root};
}

// =============================================================
// B.8. LZ77 (Lempel-Ziv 1977)
// =============================================================
// Скользящее окно: ищем максимально длинное совпадение в [pos-W, pos).
// Формат: тройки (offset, length, next_char)
// Если совпадения нет: (0, 0, current_char)

struct LZ77Triple {
    int offset;   // расстояние до начала совпадения (0 = нет совпадения)
    int length;   // длина совпадения (0 = нет совпадения)
    int next_char;// символ после совпадения (или текущий символ)
};

static vector<LZ77Triple> lz77_encode(const vector<int>& data, int window_size, int lookahead_size) {
    vector<LZ77Triple> result;
    int n = (int)data.size();
    int pos = 0;

    while (pos < n) {
        int best_offset = 0;
        int best_length = 0;

        // ищем в окне [pos - window_size, pos)
        int search_start = max(0, pos - window_size);
        for (int i = search_start; i < pos; i++) {
            int len = 0;
            while (len < lookahead_size && pos + len < n && data[i + len] == data[pos + len]) {
                len++;
                // проверяем, не выходит ли i+len за пределы окна
                if (i + len >= pos) break;
            }
            if (len > best_length) {
                best_length = len;
                best_offset = pos - i;
            }
        }

        int next_char = (pos + best_length < n) ? data[pos + best_length] : -1;
        result.push_back({best_offset, best_length, next_char});
        pos += max(1, best_length);
    }
    return result;
}

static vector<int> lz77_decode(const vector<LZ77Triple>& encoded) {
    vector<int> result;
    for (auto& t : encoded) {
        if (t.offset == 0 && t.length == 0) {
            result.push_back(t.next_char);
        } else {
            int start = (int)result.size() - t.offset;
            for (int i = 0; i < t.length; i++)
                result.push_back(result[start + i]);
            if (t.next_char != -1)
                result.push_back(t.next_char);
        }
    }
    return result;
}

// =============================================================
// B.10. BWT (Burrows-Wheeler Transform)
// =============================================================
// Прямое преобразование: все циклические сдвиги → сортировка → последний столбец
// Обратное: LF-mapping

// Прямое BWT
struct BWTResult {
    vector<int> transformed;
    int original_index; // позиция оригинальной строки в отсортированной матрицы
};

static BWTResult bwt_encode(const vector<int>& data) {
    int n = (int)data.size();
    if (n == 0) return {{}, 0};

    // создаём индексы сдвигов и сортируем
    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);

    // comparator для циклических сдвигов
    auto cmp = [&](int a, int b) -> bool {
        for (int i = 0; i < n; i++) {
            int ca = data[(a + i) % n];
            int cb = data[(b + i) % n];
            if (ca != cb) return ca < cb;
        }
        return false; // равны
    };

    sort(indices.begin(), indices.end(), cmp);

    // последний столбец
    vector<int> transformed(n);
    int original_index = 0;
    for (int i = 0; i < n; i++) {
        transformed[i] = data[(indices[i] + n - 1) % n];
        if (indices[i] == 0) original_index = i;
    }

    return {transformed, original_index};
}

// Обратное BWT
static vector<int> bwt_decode(const vector<int>& transformed, int original_index) {
    int n = (int)transformed.size();
    if (n == 0) return {};

    // первый столбец (сортировка)
    vector<int> first_column = transformed;
    sort(first_column.begin(), first_column.end());

    // LF-mapping: для каждого элемента в transformed — найти соответствующий в first_column
    // LF[i] = позиция transformed[i] в first_column (с учётом дубликатов)
    vector<int> lf(n);
    vector<int> count(256, 0); // подсчёт для стабильной сортировки
    // сначала подсчитаем, сколько каждого значения в transformed
    for (int i = 0; i < n; i++) count[transformed[i]]++;
    //.prefix sum для first_column
    vector<int> next_pos(256, 0);
    for (int i = 1; i < 256; i++) next_pos[i] = next_pos[i - 1] + count[i - 1];

    for (int i = 0; i < n; i++) {
        lf[i] = next_pos[transformed[i]]++;
    }

    // восстановление строки
    vector<int> result(n);
    int idx = original_index;
    for (int i = n - 1; i >= 0; i--) {
        result[i] = transformed[idx];
        idx = lf[idx];
    }
    return result;
}

// =============================================================
// B.11. MTF (Move-to-Front Transform)
// =============================================================
// Прямое: для каждого символа — индекс в текущем списке → перемещение в начало
// Обратное: по индексу → символ из списка → перемещение в начало

static vector<int> mtf_encode(const vector<int>& data, int alphabet_size) {
    vector<int> list(alphabet_size);
    iota(list.begin(), list.end(), 0);

    vector<int> result;
    for (int x : data) {
        // найти позицию x в list
        int pos = 0;
        while (list[pos] != x) pos++;
        result.push_back(pos);
        // переместить в начало
        list.erase(list.begin() + pos);
        list.insert(list.begin(), x);
    }
    return result;
}

static vector<int> mtf_decode(const vector<int>& encoded, int alphabet_size) {
    vector<int> list(alphabet_size);
    iota(list.begin(), list.end(), 0);

    vector<int> result;
    for (int pos : encoded) {
        int sym = list[pos];
        result.push_back(sym);
        // переместить в начало
        list.erase(list.begin() + pos);
        list.insert(list.begin(), sym);
    }
    return result;
}

// =============================================================
// B.12. Delta Encoding
// =============================================================
// Прямое: δ[i] = x[i] − x[i−1] (для i > 0), δ[0] = x[0]
// Обратное: x[i] = δ[i] + x[i−1]

static vector<int> delta_encode(const vector<int>& data) {
    if (data.empty()) return {};
    vector<int> result(data.size());
    result[0] = data[0];
    for (int i = 1; i < (int)data.size(); i++)
        result[i] = data[i] - data[i - 1];
    return result;
}

static vector<int> delta_decode(const vector<int>& encoded) {
    if (encoded.empty()) return {};
    vector<int> result(encoded.size());
    result[0] = encoded[0];
    for (int i = 1; i < (int)encoded.size(); i++)
        result[i] = result[i - 1] + encoded[i];
    return result;
}

// =============================================================
// D. СПЕЦИАЛИЗИРОВАННЫЕ МЕТОДЫ
// =============================================================

// =============================================================
// D.18. Coordinate Compression
// =============================================================
// Замена координат на их ранги (порядковые номера).
// Параметр: unique — оставить ли дубликаты или нет

struct CoordCompressionResult {
    vector<int> compressed;       // сжатые координаты (ранги)
    vector<int> unique_sorted;    // уникальные отсортированные значения (для обратного преобразования)
};

static CoordCompressionResult coordinate_compress(const vector<int>& coords, bool keep_duplicates = true) {
    vector<int> sorted_coords = coords;
    sort(sorted_coords.begin(), sorted_coords.end());

    if (!keep_duplicates) {
        sorted_coords.erase(unique(sorted_coords.begin(), sorted_coords.end()), sorted_coords.end());
    }

    vector<int> compressed(coords.size());
    for (int i = 0; i < (int)coords.size(); i++) {
        compressed[i] = (int)(lower_bound(sorted_coords.begin(), sorted_coords.end(), coords[i]) - sorted_coords.begin());
    }

    return {compressed, sorted_coords};
}

// Обратное преобразование: ранг → координата
static int coordinate_decompress(int rank, const vector<int>& unique_sorted) {
    return unique_sorted[rank];
}

// =============================================================
// ВСПОМОГАТЕЛЬНЫЕ: БИТОВЫЕ ПОТОКИ
// =============================================================
// BitWriter: запись бит в поток
struct BitWriter {
    vector<unsigned char> buffer;
    unsigned char current_byte;
    int bit_count;

    BitWriter() : current_byte(0), bit_count(0) {}

    void write_bit(int bit) {
        current_byte = (current_byte << 1) | (bit & 1);
        bit_count++;
        if (bit_count == BITS_PER_BYTE) {
            buffer.push_back(current_byte);
            current_byte = 0;
            bit_count = 0;
        }
    }

    void write_bits(int value, int num_bits) {
        for (int i = num_bits - 1; i >= 0; i--)
            write_bit((value >> i) & 1);
    }

    void flush() {
        if (bit_count > 0) {
            current_byte <<= (BITS_PER_BYTE - bit_count);
            buffer.push_back(current_byte);
            current_byte = 0;
            bit_count = 0;
        }
    }

    vector<unsigned char> get_bytes() {
        flush();
        return buffer;
    }

    static const int BITS_PER_BYTE = 8;
};

// BitReader: чтение бит из потока
struct BitReader {
    const vector<unsigned char>& buffer;
    int byte_pos;
    int bit_pos;

    BitReader(const vector<unsigned char>& buf) : buffer(buf), byte_pos(0), bit_pos(0) {}

    int read_bit() {
        if (byte_pos >= (int)buffer.size()) return -1;
        int bit = (buffer[byte_pos] >> (BITS_PER_BYTE - 1 - bit_pos)) & 1;
        bit_pos++;
        if (bit_pos == BITS_PER_BYTE) {
            bit_pos = 0;
            byte_pos++;
        }
        return bit;
    }

    int read_bits(int num_bits) {
        int value = 0;
        for (int i = 0; i < num_bits; i++) {
            int bit = read_bit();
            if (bit == -1) return -1;
            value = (value << 1) | bit;
        }
        return value;
    }

    bool has_more() {
        return byte_pos < (int)buffer.size();
    }

    static const int BITS_PER_BYTE = 8;
};

// =============================================================
// B.9. LZ78 (Lempel-Ziv 1978)
// =============================================================
// Словарь — дерево фраз; кодирование: (индекс в дереве, новый символ)
// Узел дерева: parent_index, character

struct LZ78Node {
    int parent_index; // индекс родителя (0 = корень)
    int character;    // символ, которым заканчивается фраза
};

static vector<pair<int,int>> lz78_encode(const vector<int>& data) {
    vector<pair<int,int>> result;
    vector<LZ78Node> dict; // dict[i] = узел с индексом i+1
    dict.push_back({-1, -1}); // индекс 0 — пустой (не используется)

    int i = 0;
    int n = (int)data.size();
    while (i < n) {
        int cur = 0; // текущий узел в дереве (0 = корень)
        int j = i;
        // ищем最长 совпадение в дереве
        while (j < n) {
            int next = -1;
            // ищем ребро cur → next с символом data[j]
            for (int k = 0; k < (int)dict.size(); k++) {
                if (dict[k].parent_index == cur && dict[k].character == data[j]) {
                    next = k + 1;
                    break;
                }
            }
            if (next == -1) break;
            cur = next;
            j++;
        }
        // добавляем новую фразу
        int new_index = (int)dict.size();
        dict.push_back({cur, (j < n) ? data[j] : -1});
        int symbol = (j < n) ? data[j] : -1;
        result.push_back({cur, symbol});
        i = j + 1;
    }
    return result;
}

static vector<int> lz78_decode(const vector<pair<int,int>>& encoded) {
    vector<int> result;
    vector<LZ78Node> dict;
    dict.push_back({-1, -1});

    for (auto& [parent, symbol] : encoded) {
        // восстанавливаем фразу из дерева
        vector<int> phrase;
        int cur = parent;
        while (cur > 0) {
            phrase.push_back(dict[cur - 1].character);
            cur = dict[cur - 1].parent_index;
        }
        reverse(phrase.begin(), phrase.end());
        for (int x : phrase) result.push_back(x);
        if (symbol != -1) result.push_back(symbol);
        // добавляем фразу в словарь
        dict.push_back({parent, symbol});
    }
    return result;
}

// =============================================================
// B.6a. Адаптивный Хаффман (Vitter's Algorithm)
// =============================================================
// Дерево обновляется online: при каждом символе перестраивается.
// Для теоретической демонстрации: используем упрощённую версию
// (пересчёт дерева каждые BLOCK_SIZE символов)

struct AdaptiveHuffman {
    int block_size; // размер блока для пересчёта
    AdaptiveHuffman(int bs = 256) : block_size(bs) {}

    // Блочное адаптивное кодирование: пересчёт частот каждые block_size символов
    static string encode(const vector<int>& data, int alphabet_size, int block_size = 256) {
        string result;
        int n = (int)data.size();
        for (int start = 0; start < n; start += block_size) {
            int end = min(start + block_size, n);
            vector<int> block(data.begin() + start, data.begin() + end);
            // подсчёт частот для блока
            vector<int> freq(alphabet_size, 0);
            for (int x : block) freq[x]++;
            // построение дерева для блока
            HuffmanNode* root = huffman_build_tree(freq);
            vector<string> codes(alphabet_size);
            huffman_generate_codes(root, "", codes);
            // кодирование блока
            for (int x : block) result += codes[x];
            huffman_free_tree(root);
        }
        return result;
    }

    // Декодирование блочного адаптивного Хаффмана
    static vector<int> decode(const string& bitstring, int alphabet_size,
                              const vector<int>& original_sizes, int block_size = 256) {
        vector<int> result;
        int bit_pos = 0;
        for (int sz : original_sizes) {
            // подсчёт частот из размера блока (нужны оригинальные данные — здесь упрощение)
            // на практике: частоты передаются в заголовке блока
            HuffmanNode* root = huffman_build_tree(vector<int>(alphabet_size, 1)); // placeholder
            vector<string> codes(alphabet_size);
            huffman_generate_codes(root, "", codes);
            // декодирование блока (упрощённое — нужна重建 дерева из потока)
            huffman_free_tree(root);
        }
        return result;
    }
};

// =============================================================
// B.6b. Коды Голомба и Райса
// =============================================================
// Голомб: q = ⌊x/m⌋ (unary), r = x mod m (binary с ⌈log₂m⌉ бит)
// Райс: m = 2^k → r кодируется k битами

struct GolombCodec {
    int m; // параметр Голомба (для Райса: m = 2^k)

    GolombCodec(int m) : m(m) {}

    // Голомб: кодирование одного числа
    static string encode_value(int x, int m) {
        int q = x / m;
        int r = x % m;
        string result;
        // unary для q
        for (int i = 0; i < q; i++) result += '1';
        result += '0';
        // бинарный остаток
        int bits = 0;
        int temp = m;
        while (temp > 1) { bits++; temp = (temp + 1) / 2; }
        for (int i = bits - 1; i >= 0; i--)
            result += ((r >> i) & 1) ? '1' : '0';
        return result;
    }

    // Райс: кодирование (m = 2^k)
    static string encode_rice(int x, int k) {
        int q = x >> k;
        int r = x & ((1 << k) - 1);
        string result;
        for (int i = 0; i < q; i++) result += '1';
        result += '0';
        for (int i = k - 1; i >= 0; i--)
            result += ((r >> i) & 1) ? '1' : '0';
        return result;
    }

    static vector<int> decode(const string& bitstring, int m, int count) {
        vector<int> result;
        int pos = 0;
        for (int i = 0; i < count && pos < (int)bitstring.size(); i++) {
            // unary: считываем единицы до 0
            int q = 0;
            while (pos < (int)bitstring.size() && bitstring[pos] == '1') {
                q++;
                pos++;
            }
            if (pos < (int)bitstring.size()) pos++; // пропускаем '0'
            // бинарный остаток
            int bits = 0;
            int temp = m;
            while (temp > 1) { bits++; temp = (temp + 1) / 2; }
            int r = 0;
            for (int j = bits - 1; j >= 0; j--) {
                if (pos < (int)bitstring.size()) {
                    r |= ((bitstring[pos] - '0') << j);
                    pos++;
                }
            }
            result.push_back(q * m + r);
        }
        return result;
    }

    static vector<int> decode_rice(const string& bitstring, int k, int count) {
        vector<int> result;
        int pos = 0;
        for (int i = 0; i < count && pos < (int)bitstring.size(); i++) {
            int q = 0;
            while (pos < (int)bitstring.size() && bitstring[pos] == '1') {
                q++;
                pos++;
            }
            if (pos < (int)bitstring.size()) pos++;
            int r = 0;
            for (int j = k - 1; j >= 0; j--) {
                if (pos < (int)bitstring.size()) {
                    r |= ((bitstring[pos] - '0') << j);
                    pos++;
                }
            }
            result.push_back((q << k) | r);
        }
        return result;
    }
};

// =============================================================
// E. ГИБРИДНЫЕ МЕТОДЫ (демонстрация цепочки)
// =============================================================

// BZip2-подобный конвейер: BWT → MTF → RLE0 → Хаффман
struct BZip2Pipeline {
    // BWT → MTF → кодирование
    static vector<int> compress(const vector<int>& data, int alphabet_size) {
        // BWT
        BWTResult bwt = bwt_encode(data);
        vector<int> combined;
        combined.push_back(bwt.original_index);
        combined.insert(combined.end(), bwt.transformed.begin(), bwt.transformed.end());

        // MTF
        vector<int> mtf = mtf_encode(bwt.transformed, alphabet_size);

        // вставляем original_index в начало
        vector<int> result;
        result.push_back(bwt.original_index);
        result.insert(result.end(), mtf.begin(), mtf.end());
        return result;
    }

    // обратный: индекс + MTF-коды → BWT → декодирование
    static vector<int> decompress(const vector<int>& compressed, int alphabet_size) {
        int original_index = compressed[0];
        vector<int> mtf(compressed.begin() + 1, compressed.end());

        // обратный MTF
        vector<int> bwt_data = mtf_decode(mtf, alphabet_size);

        // обратный BWT
        return bwt_decode(bwt_data, original_index);
    }
};

// DEFLATE-подобный: LZ77 → Хаффман
struct DeflatePipeline {
    static HuffmanResult compress(const vector<int>& data, int window_size, int lookahead_size) {
        // LZ77-кодирование → символы для Хаффмана
        // для простоты: кодируем raw через Хаффман (полная версия требует потокового LZ77)
        return huffman_full(data);
    }
};

// =============================================================
// ТЕСТИРОВАНИЕ (main для проверки)
// =============================================================
void test_all() {
    // --- RLE ---
    vector<int> rle_data = {1, 1, 1, 2, 2, 3, 3, 3, 3};
    auto rle_enc = rle_encode(rle_data);
    auto rle_dec = rle_decode(rle_enc);
    assert(rle_dec == rle_data);

    // --- Хаффман ---
    vector<int> huff_data = {1, 1, 2, 2, 2, 3, 3, 3, 3, 3};
    auto huff = huffman_full(huff_data);
    assert(huff.decoded == huff_data);

    // --- LZ77 ---
    vector<int> lz77_data = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    auto lz77_enc = lz77_encode(lz77_data, 10, 10);
    auto lz77_dec = lz77_decode(lz77_enc);
    assert(lz77_dec == lz77_data);

    // --- BWT ---
    vector<int> bwt_data = {3, 2, 1, 3, 2, 1};
    BWTResult bwt_enc = bwt_encode(bwt_data);
    vector<int> bwt_dec = bwt_decode(bwt_enc.transformed, bwt_enc.original_index);
    assert(bwt_dec == bwt_data);

    // --- MTF ---
    vector<int> mtf_data = {5, 0, 1, 3, 2, 5};
    int alpha = 6;
    auto mtf_enc = mtf_encode(mtf_data, alpha);
    auto mtf_dec = mtf_decode(mtf_enc, alpha);
    assert(mtf_dec == mtf_data);

    // --- Delta ---
    vector<int> delta_data = {10, 15, 13, 20, 25};
    auto delta_enc = delta_encode(delta_data);
    auto delta_dec = delta_decode(delta_enc);
    assert(delta_dec == delta_data);

    // --- Coordinate Compression ---
    vector<int> coords = {100, 50, 100, 200, 50};
    auto cc = coordinate_compress(coords);
    assert(cc.compressed[0] == cc.compressed[2]); // 100 → один ранг
    assert(cc.compressed[1] == cc.compressed[4]); // 50 → один ранг

    // --- BZip2 pipeline ---
    vector<int> bz_data = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    auto bz_compressed = BZip2Pipeline::compress(bz_data, 4);
    auto bz_decompressed = BZip2Pipeline::decompress(bz_compressed, 4);
    assert(bz_decompressed == bz_data);

    // --- LZ78 ---
    vector<int> lz78_data = {1, 2, 1, 2, 3, 1, 2, 3};
    auto lz78_enc = lz78_encode(lz78_data);
    auto lz78_dec = lz78_decode(lz78_enc);
    assert(lz78_dec == lz78_data);

    // --- Golomb ---
    GolombCodec golomb(5);
    string golomb_str = GolombCodec::encode_value(13, 5);
    auto golomb_dec = GolombCodec::decode(golomb_str, 5, 1);
    assert(golomb_dec.size() == 1 && golomb_dec[0] == 13);

    // --- Rice ---
    string rice_str = GolombCodec::encode_rice(13, 3);
    auto rice_dec = GolombCodec::decode_rice(rice_str, 3, 1);
    assert(rice_dec.size() == 1 && rice_dec[0] == 13);

    // --- Entropy ---
    vector<int> freq = {50, 50}; // энтропия = 1 бит
    double ent = shannon_entropy(freq);
    assert(abs(ent - 1.0) < 1e-9);

    cout << "All tests passed." << endl;
}

}; // struct DataCompression

// Запуск тестов (закомментировать в продакшене)
// int main() {
//     DataCompression dc;
//     dc.test_all();
//     return 0;
// }

#endif // TECHNIQUE_D_CPP
