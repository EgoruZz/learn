#ifndef STRING_A_CPP
#define STRING_A_CPP

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <cstring>
using namespace std;

// =============================================================
// A. СТРОКОВЫЕ КОНТЕЙНЕРЫ И ПРЕДСТАВЛЕНИЯ
// =============================================================
// Структура md: 1. Стандартные контейнеры (string, string_view, stringstream)
//               → 2. Продвинутые структуры (Rope, Gap Buffer, Piece Table)
//               → 3. Память и оптимизации (SSO, interning, COW)
//
// StringContainers — базовый класс всей ветки string.
// Собственной арифметики не имеет. Вводит контейнеры и структуры,
// на которых строятся все последующие алгоритмы.

struct StringContainers {

// =============================================================
// 1. СТАНДАРТНЫЕ КОНТЕЙНЕРЫ (демонстрация)
// =============================================================

// --- 1.1. Демонстрация std::string ---
string demo_string_ops() {
    string s = "Hello";
    s += " World";           // append O(6)
    s.push_back('!');        // push_back O(1) амортизированно
    string sub = s.substr(0, 5);  // "Hello" O(5)
    size_t pos = s.find("World"); // O(n·m) худший
    s.replace(6, 5, "tldraw");    // O(n) сдвиг
    return s;
}

// --- 1.2. Демонстрация string_view ---
string_view demo_string_view(string_view input) {
    // O(1) копирование, без аллокации
    auto pos = input.find(' ');
    if (pos != string_view::npos)
        return input.substr(0, pos);  // O(1)
    return input;
}

// --- 1.3. Демонстрация stringstream ---
string demo_stringstream(int n) {
    ostringstream oss;
    for (int i = 0; i < n; i++)
        oss << i << " ";
    return oss.str();
}

// =============================================================
// 2. ПРОДВИНУТЫЕ СТРУКТУРЫ
// =============================================================

// --- 2.1. Rope (канатная структура) ---
// Сбалансированное бинарное дерево строк.
// Листья — подстроки; внутренние узлы — объединение.
// insert/erase/substr за O(log n + m).
struct Rope {
    struct Node {
        string data;          // только для листьев
        int weight;           // размер левого поддерева (суммарная длина левой части)
        Node* left = nullptr;
        Node* right = nullptr;
        bool is_leaf;

        Node(const string& s) : data(s), weight(s.size()), is_leaf(true) {}
        Node(Node* l, Node* r) : left(l), right(r), is_leaf(false) {
            weight = (l ? l->total_size() : 0);
        }

        int total_size() const {
            if (is_leaf) return data.size();
            return weight + (right ? right->total_size() : 0);
        }
    };

    Node* root = nullptr;

    Rope() = default;
    Rope(const string& s) { if (!s.empty()) root = new Node(s); }
    ~Rope() { destroy(root); }

    void destroy(Node* n) {
        if (!n) return;
        if (!n->is_leaf) { destroy(n->left); destroy(n->right); }
        delete n;
    }

    int size() const { return root ? root->total_size() : 0; }

    // Разрезание дерева в позиции pos → (left, right)
    static pair<Node*, Node*> split(Node* n, int pos) {
        if (!n) return {nullptr, nullptr};
        if (n->is_leaf) {
            string l = n->data.substr(0, pos);
            string r = n->data.substr(pos);
            delete n;
            return {l.empty() ? nullptr : new Node(l),
                    r.empty() ? nullptr : new Node(r)};
        }
        int left_size = n->left ? n->left->total_size() : 0;
        if (pos <= left_size) {
            auto [ll, lr] = split(n->left, pos);
            Node* nr = lr ? new Node(lr, n->right) : n->right;
            n->left = nullptr; n->right = nullptr; delete n;
            return {ll, nr};
        } else {
            auto [rl, rr] = split(n->right, pos - left_size);
            Node* nl = n->left ? new Node(n->left, rl) : rl;
            n->left = nullptr; n->right = nullptr; delete n;
            return {nl, rr};
        }
    }

    // Вставка строки s в позицию pos
    void insert(int pos, const string& s) {
        if (s.empty()) return;
        Node* mid = new Node(s);
        if (!root) { root = mid; return; }
        auto [l, r] = split(root, pos);
        root = new Node(new Node(l, mid), r);
    }

    // Удаление len символов начиная с pos
    void erase(int pos, int len) {
        auto [l, tmp] = split(root, pos);
        auto [mid, r] = split(tmp, len);
        destroy(mid);
        root = new Node(l, r);
    }

    // Получение символа по индексу — O(log n)
    char at(int idx) const {
        Node* cur = root;
        while (cur && !cur->is_leaf) {
            int left_size = cur->left ? cur->left->total_size() : 0;
            if (idx < left_size) cur = cur->left;
            else { idx -= left_size; cur = cur->right; }
        }
        return cur ? cur->data[idx] : '\0';
    }

    // Преобразование в std::string — O(n)
    string to_string() const {
        string result;
        result.reserve(size());
        function<void(Node*)> collect = [&](Node* n) {
            if (!n) return;
            if (n->is_leaf) { result += n->data; return; }
            collect(n->left); collect(n->right);
        };
        collect(root);
        return result;
    }
};

// --- 2.2. Gap Buffer ---
// Массив с «разрывом» в позиции курсора.
// Вставка/удаление в позиции курсора O(1).
struct GapBuffer {
    vector<char> buf;
    int gap_start = 0;
    int gap_end = 0;
    int gap_size;

    GapBuffer(int initial_gap = 64) : gap_size(initial_gap) {
        buf.resize(gap_size);
        gap_end = gap_size;
    }

    GapBuffer(const string& s) : gap_size(max(64, (int)s.size() * 2)) {
        int total = (int)s.size() + gap_size;
        buf.resize(total);
        // Данные в начале, gap после них
        for (int i = 0; i < (int)s.size(); i++)
            buf[i] = s[i];
        gap_start = (int)s.size();
        gap_end = total;
    }

    // Перемещение gap в позицию pos
    void move_gap(int pos) {
        if (pos < gap_start) {
            int len = gap_start - pos;
            copy(buf.begin() + pos, buf.begin() + gap_start,
                 buf.begin() + gap_end - len);
            gap_start -= len;
            gap_end -= len;
        } else if (pos > gap_start && pos < gap_end) {
            int len = pos - gap_start;
            copy(buf.begin() + gap_end, buf.begin() + gap_end + len,
                 buf.begin() + gap_start);
            gap_start += len;
            gap_end += len;
        }
    }

    void grow() {
        int data_before = gap_start;
        int data_after = (int)buf.size() - gap_end;
        int new_gap_size = max(gap_size, (int)buf.size());
        int new_total = data_before + new_gap_size + data_after;
        vector<char> new_buf(new_total);
        copy(buf.begin(), buf.begin() + gap_start, new_buf.begin());
        copy(buf.begin() + gap_end, buf.end(), new_buf.begin() + data_before + new_gap_size);
        buf = std::move(new_buf);
        gap_start = data_before;
        gap_end = data_before + new_gap_size;
    }

    // Вставка символа в текущую позицию курсора
    void insert(char ch) {
        if (gap_start == gap_end) grow();
        buf[gap_start++] = ch;
    }

    // Вставка строки
    void insert_string(const string& s) {
        for (char c : s) insert(c);
    }

    // Удаление символа перед курсором
    char erase_back() {
        if (gap_start == 0) return '\0';
        return buf[--gap_start];
    }

    // Удаление символа после курсора
    char erase_forward() {
        if (gap_end >= (int)buf.size()) return '\0';
        return buf[gap_end++];
    }

    int cursor_pos() const { return gap_start; }

    string to_string() const {
        string result;
        result.reserve(buf.size() - (gap_end - gap_start));
        for (int i = 0; i < gap_start; i++) result += buf[i];
        for (int i = gap_end; i < (int)buf.size(); i++) result += buf[i];
        return result;
    }
};

// --- 2.3. Piece Table ---
// Два буфера (original + modified) + таблица кусков.
struct PieceTable {
    string original;
    string modified;
    struct Piece { int start; int length; bool is_modified; };
    vector<Piece> pieces;

    PieceTable(const string& s) : original(s) {
        if (!s.empty()) pieces.push_back({0, (int)s.size(), false});
    }

    // Вставка строки s в позицию pos
    void insert(int pos, const string& s) {
        if (s.empty()) return;
        int mod_start = modified.size();
        modified += s;
        int offset = 0;
        for (int i = 0; i < (int)pieces.size(); i++) {
            int p_size = pieces[i].length;
            if (offset + p_size >= pos) {
                int local = pos - offset;
                // Разбиваем текущий кусок: [start, start+local) | [start+local, start+length)
                Piece left = {pieces[i].start, local, pieces[i].is_modified};
                Piece mid = {mod_start, (int)s.size(), true};
                Piece right = {pieces[i].start + local, p_size - local, pieces[i].is_modified};
                pieces.erase(pieces.begin() + i);
                if (left.length > 0) pieces.insert(pieces.begin() + i, left);
                int insert_pos = (left.length > 0) ? i + 1 : i;
                pieces.insert(pieces.begin() + insert_pos, mid);
                if (right.length > 0) pieces.insert(pieces.begin() + insert_pos + 1, right);
                return;
            }
            offset += p_size;
        }
        pieces.push_back({mod_start, (int)s.size(), true});
    }

    string to_string() const {
        string result;
        for (auto& p : pieces) {
            const string& src = p.is_modified ? modified : original;
            result += src.substr(p.start, p.length);
        }
        return result;
    }
};

// =============================================================
// 3. ПАМЯТЬ И ОПТИМИЗАЦИИ
// =============================================================

// --- 3.1. String Interning ---
// Пул уникальных строк; O(1) сравнение через указатель.
struct StringInterner {
    unordered_map<string, const string*> pool;

    const string& intern(const string& s) {
        auto it = pool.find(s);
        if (it != pool.end()) return *it->second;
        auto [ins, _] = pool.insert({s, nullptr});
        ins->second = &ins->first;
        return *ins->second;
    }

    bool is_interned(const string& s) const {
        return pool.count(s) > 0;
    }

    int unique_count() const { return pool.size(); }
};

}; // struct StringContainers

// =============================================================
// MAIN
// =============================================================
#ifdef STRING_A_MAIN
int main() {
    StringContainers sc;

    cout << "=== Rope ===" << endl;
    StringContainers::Rope rope("Hello");
    rope.insert(5, " World");
    rope.insert(12, "!");
    cout << rope.to_string() << " (size=" << rope.size() << ")" << endl;
    cout << "char[0]=" << rope.at(0) << " char[6]=" << rope.at(6) << endl;
    rope.erase(5, 6);
    cout << "after erase: " << rope.to_string() << endl;

    cout << "\n=== Gap Buffer ===" << endl;
    StringContainers::GapBuffer gb("Hello World");
    gb.move_gap(5);
    gb.insert('-');
    gb.insert('-');
    cout << gb.to_string() << endl;
    gb.move_gap(7);
    gb.erase_back();
    cout << "after erase: " << gb.to_string() << endl;

    cout << "\n=== Piece Table ===" << endl;
    StringContainers::PieceTable pt("Hello World");
    pt.insert(5, ",");
    pt.insert(7, " Beautiful");
    cout << pt.to_string() << endl;

    cout << "\n=== String Interning ===" << endl;
    StringContainers::StringInterner interner;
    const string& s1 = interner.intern("hello");
    const string& s2 = interner.intern("hello");
    const string& s3 = interner.intern("world");
    cout << "s1 == s2 (same ptr): " << (&s1 == &s2) << endl;
    cout << "s1 == s3 (same ptr): " << (&s1 == &s3) << endl;
    cout << "unique: " << interner.unique_count() << endl;

    cout << "\n=== string_view ===" << endl;
    string input = "Hello World from tldraw";
    string_view first_word = sc.demo_string_view(input);
    cout << "first word: " << first_word << endl;

    cout << "\n=== stringstream ===" << endl;
    string nums = sc.demo_stringstream(5);
    cout << "numbers: " << nums << endl;

    return 0;
}
#endif

#endif // STRING_A_CPP
