#ifndef HASHING_E_CPP
#define HASHING_E_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cstring>
using namespace std;

// =============================================================
// E. АЛГОРИТМЫ И КОНСТРУКЦИИ ХЕШ-ФУНКЦИЙ
// =============================================================
// Структура md: A. Универсальные семейства
//               → B. Криптографические конструкции
//               → C. Некриптографические хеш-функции
//               → D. Специализированные алгоритмы
//
// HashFunctions — наследует ProbabilisticStructures (d.cpp).
// Универсальные семейства, полиномиальное хеширование,
// Djb2, Sdbm, Adler32, Fletcher16, Rolling Hash,
// MinHash, SimHash, Luhn, Hamming Code.

#ifndef INSIDE_HASHING_E
#define INSIDE_HASHING_E
#include "../d/d.cpp"
#endif

struct HashFunctions : ProbabilisticStructures {

// =============================================================
// A. УНИВЕРСАЛЬНЫЕ СЕМЕЙСТВА
// =============================================================

// --- A.1. Универсальное хеширование (Carter-Wegman) ---
// h_{a,b}(x) = ((a*x + b) mod p) mod m.
// P(h(x)=h(y)) <= 1/m для x != y.
struct UniversalHash {
    long long p, m, a, b;
    UniversalHash(long long prime = 1000003, long long modulus = 100000)
        : p(prime), m(modulus) {
        a = 1 + rand() % (p - 1);
        b = rand() % p;
    }
    int operator()(long long x) const {
        return (int)(((a * (x % p) + b) % p) % m);
    }
};

// --- A.2. Полиномиальное хеширование ---
// h(s) = (s[0]*p^(n-1) + s[1]*p^(n-2) + ... + s[n-1]) mod m.
long long polynomial_hash(const string& s, long long p = 31,
                           long long m = 1e9 + 7) {
    long long h = 0;
    for (char c : s) h = (h * p + (c - 'a' + 1)) % m;
    return h;
}

// --- A.3. Полиномиальный hash для подстроки (rolling) ---
// h(l..r) через префиксные хеши и степени p.
struct RollingHash {
    long long p, m;
    vector<long long> prefix, power;
    RollingHash(const string& s, long long prime = 31, long long mod = 1e9 + 7)
        : p(prime), m(mod) {
        int n = (int)s.size();
        prefix.resize(n + 1, 0);
        power.resize(n + 1, 1);
        for (int i = 0; i < n; i++) {
            prefix[i + 1] = (prefix[i] * p + (s[i] - 'a' + 1)) % m;
            power[i + 1] = power[i] * p % m;
        }
    }
    long long get_hash(int l, int r) {  // [l, r)
        return (prefix[r] - prefix[l] * power[r - l] % m + m) % m;
    }
};

// --- A.4. Tabulation Hashing ---
// h(x) = T1[x1] ^ T2[x2] ^ ... ^ Tk[xk].
// 3-wise независимое.
struct TabulationHash {
    vector<vector<int>> tables;
    int k;
    TabulationHash(int key_bytes = 8, int table_size = 256)
        : k(key_bytes) {
        tables.resize(k, vector<int>(table_size));
        for (int i = 0; i < k; i++)
            for (int j = 0; j < table_size; j++)
                tables[i][j] = rand();
    }
    int operator()(long long x) const {
        int h = 0;
        for (int i = 0; i < k; i++) {
            h ^= tables[i][x & 0xFF];
            x >>= 8;
        }
        return h;
    }
};

// =============================================================
// B. НЕКРИПТОГРАФИЧЕСКИЕ ХЕШ-ФУНКЦИИ
// =============================================================

// --- B.1. Djb2 ---
long long djb2(const string& s) {
    long long h = 5381;
    for (char c : s) h = ((h << 5) + h) + c;
    return h;
}

// --- B.2. Sdbm ---
long long sdbm(const string& s) {
    long long h = 0;
    for (char c : s) h = c + (h << 6) + (h << 16) - h;
    return h;
}

// --- B.3. Elf ---
long long elf_hash(const string& s) {
    long long h = 0;
    for (char c : s) {
        h = (h << 4) + c;
        long long g = h & 0xF0000000;
        if (g) h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

// --- B.4. Adler32 ---
unsigned int adler32(const string& s) {
    unsigned int a = 1, b = 0;
    for (char c : s) {
        a = (a + (unsigned char)c) % 65521;
        b = (b + a) % 65521;
    }
    return (b << 16) | a;
}

// --- B.5. Fletcher16 ---
unsigned int fletcher16(const string& s) {
    unsigned int sum1 = 0, sum2 = 0;
    for (char c : s) {
        sum1 = (sum1 + (unsigned char)c) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    return (sum2 << 8) | sum1;
}

// =============================================================
// D. СПЕЦИАЛИЗИРОВАННЫЕ АЛГОРИТМЫ
// =============================================================

// --- D.1. Rabin-Karp поиск подстроки ---
// Возвращает все позиции вхождения pattern в text.
vector<int> rabin_karp(const string& text, const string& pattern) {
    int n = (int)text.size(), m = (int)pattern.size();
    if (m > n) return {};
    long long p = 31, mod = 1e9 + 7;
    long long pattern_hash = polynomial_hash(pattern, p, mod);
    RollingHash rh(text, p, mod);
    vector<int> result;
    for (int i = 0; i <= n - m; i++) {
        if (rh.get_hash(i, i + m) == pattern_hash)
            result.push_back(i);
    }
    return result;
}

// --- D.2. MinHash (Jaccard similarity) ---
// Подпись множества: r минимумов хешей.
// Сходство = доля совпадений минимумов.
vector<unsigned int> minhash_signature(const vector<int>& set, int r) {
    vector<unsigned int> sig(r, UINT_MAX);
    for (int x : set) {
        for (int i = 0; i < r; i++) {
            unsigned int h = (unsigned int)((long long)(i + 1) * x * 2654435761ULL);
            sig[i] = min(sig[i], h);
        }
    }
    return sig;
}

double jaccard_estimate(const vector<unsigned int>& sig1,
                        const vector<unsigned int>& sig2) {
    int matches = 0;
    for (int i = 0; i < (int)sig1.size(); i++)
        if (sig1[i] == sig2[i]) matches++;
    return (double)matches / sig1.size();
}

// --- D.3. SimHash (cosine similarity) ---
// Бинарное хеширование векторов.
unsigned int simhash(const vector<int>& vec, int dim = 64) {
    vector<int> v(dim, 0);
    for (int i = 0; i < (int)vec.size(); i++) {
        unsigned int h = (unsigned int)((long long)(i + 1) * vec[i] * 2654435761ULL);
        for (int b = 0; b < dim; b++)
            v[b] += ((h >> b) & 1) ? vec[i] : -vec[i];
    }
    unsigned int hash = 0;
    for (int b = 0; b < dim; b++)
        if (v[b] > 0) hash |= (1u << b);
    return hash;
}

int hamming_distance(unsigned int a, unsigned int b) {
    return __builtin_popcount(a ^ b);
}

// --- D.4. Luhn Algorithm ---
bool luhn_check(const string& card) {
    int n = (int)card.size();
    int sum = 0;
    for (int i = n - 1; i >= 0; i--) {
        int d = card[i] - '0';
        if ((n - 1 - i) % 2 == 1) {
            d *= 2;
            if (d > 9) d -= 9;
        }
        sum += d;
    }
    return sum % 10 == 0;
}

// --- D.5. Hamming Code ---
// Кодирование: добавление контрольных битов на позициях 2^k.
string hamming_encode(const string& data) {
    int m = (int)data.size();
    int r = 0;
    while ((1 << r) < m + r + 1) r++;
    int n = m + r;
    string code(n + 1, '0');  // 1-indexed
    int j = 0;
    for (int i = 1; i <= n; i++) {
        if (i & (i - 1)) {  // not power of 2
            code[i] = data[j++];
        }
    }
    for (int i = 0; i < r; i++) {
        int pos = (1 << i);
        int parity = 0;
        for (int j = 1; j <= n; j++)
            if (j & pos) parity ^= (code[j] - '0');
        code[pos] = parity + '0';
    }
    return code.substr(1);
}

int hamming_detect(const string& code) {
    int n = (int)code.size();
    int pos = 0;
    for (int i = 0; (1 << i) < n; i++) {
        int parity = 0;
        for (int j = 1; j <= n; j++)
            if (j & (1 << i)) parity ^= (code[j - 1] - '0');
        if (parity) pos += (1 << i);
    }
    return pos;
}

}; // struct HashFunctions

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_E_MAIN
int main() {
    HashFunctions hf;
    srand(42);

    cout << "=== A. УНИВЕРСАЛЬНЫЕ СЕМЕЙСТВА ===" << endl;

    cout << "--- Универсальное хеширование ---" << endl;
    {
        HashFunctions::UniversalHash h1(1000003, 100000);
        HashFunctions::UniversalHash h2(1000003, 100000);
        cout << "  h1(42) = " << h1(42) << ", h2(42) = " << h2(42) << endl;
        cout << "  h1(42) = " << h1(42) << " (детерминировано)" << endl;
    }

    cout << "\n--- Полиномиальное хеширование ---" << endl;
    for (auto& s : {"hello", "world", "hash", "test"})
        cout << "  hash(\"" << s << "\") = " << hf.polynomial_hash(s) << endl;

    cout << "\n--- Rolling Hash (подстрока) ---" << endl;
    {
        HashFunctions::RollingHash rh("abcdefghij");
        cout << "  hash(0..3) = " << rh.get_hash(0, 4) << " (\"abcd\")" << endl;
        cout << "  hash(3..7) = " << rh.get_hash(3, 8) << " (\"defgh\")" << endl;
        cout << "  hash(0..9) = " << rh.get_hash(0, 10) << " (\"abcdefghij\")" << endl;
    }

    cout << "\n--- Tabulation Hash ---" << endl;
    {
        HashFunctions::TabulationHash th;
        cout << "  h(42) = " << th(42) << endl;
        cout << "  h(42) = " << th(42) << " (детерминировано)" << endl;
        cout << "  h(43) = " << th(43) << endl;
    }

    cout << "\n=== B. НЕКРИПТОГРАФИЧЕСКИЕ ХЕШ-ФУНКЦИИ ===" << endl;
    for (auto& s : {"hello", "world"}) {
        cout << "  \"" << s << "\":" << endl;
        cout << "    Djb2:   " << hf.djb2(s) << endl;
        cout << "    Sdbm:   " << hf.sdbm(s) << endl;
        cout << "    Elf:    " << hf.elf_hash(s) << endl;
        cout << "    Adler32: " << hf.adler32(s) << endl;
        cout << "    Fletcher16: " << hf.fletcher16(s) << endl;
    }

    cout << "\n=== D. СПЕЦИАЛИЗИРОВАННЫЕ АЛГОРИТМЫ ===" << endl;

    cout << "--- Rabin-Karp ---" << endl;
    {
        string text = "abracadabra";
        string pattern = "abra";
        auto positions = hf.rabin_karp(text, pattern);
        cout << "  text=\"" << text << "\", pattern=\"" << pattern << "\"" << endl;
        cout << "  Найдено на позициях: ";
        for (int p : positions) cout << p << " ";
        cout << endl;
    }

    cout << "\n--- MinHash (Jaccard) ---" << endl;
    {
        vector<int> A = {1, 2, 3, 4, 5};
        vector<int> B = {3, 4, 5, 6, 7};
        vector<int> C = {10, 20, 30};
        auto sigA = hf.minhash_signature(A, 100);
        auto sigB = hf.minhash_signature(B, 100);
        auto sigC = hf.minhash_signature(C, 100);
        cout << "  |A|=" << A.size() << " |B|=" << B.size()
             << " |C|=" << C.size() << endl;
        cout << "  J(A,B) = " << hf.jaccard_estimate(sigA, sigB)
             << " (точное: " << 3.0/7 << ")" << endl;
        cout << "  J(A,C) = " << hf.jaccard_estimate(sigA, sigC)
             << " (точное: 0)" << endl;
    }

    cout << "\n--- SimHash ---" << endl;
    {
        vector<int> v1 = {1, 2, 3, 4, 5};
        vector<int> v2 = {1, 2, 3, 4, 6};
        vector<int> v3 = {10, 20, 30, 40, 50};
        auto h1 = hf.simhash(v1);
        auto h2 = hf.simhash(v2);
        auto h3 = hf.simhash(v3);
        cout << "  h(v1)=" << h1 << " h(v2)=" << h2 << " h(v3)=" << h3 << endl;
        cout << "  dist(v1,v2) = " << hf.hamming_distance(h1, h2) << endl;
        cout << "  dist(v1,v3) = " << hf.hamming_distance(h1, h3) << endl;
    }

    cout << "\n--- Luhn Algorithm ---" << endl;
    {
        cout << "  \"49927398716\" (валидный): "
             << (hf.luhn_check("49927398716") ? "OK" : "FAIL") << endl;
        cout << "  \"49927398717\" (невалидный): "
             << (hf.luhn_check("49927398717") ? "OK" : "FAIL") << endl;
    }

    cout << "\n--- Hamming Code ---" << endl;
    {
        string data = "1011";
        string encoded = hf.hamming_encode(data);
        cout << "  Data: " << data << endl;
        cout << "  Encoded: " << encoded << endl;
        string corrupted = encoded;
        corrupted[2] = (corrupted[2] == '0') ? '1' : '0';  // flip bit
        cout << "  Corrupted: " << corrupted << endl;
        int err_pos = hf.hamming_detect(corrupted);
        cout << "  Error at position: " << err_pos << endl;
    }

    return 0;
}
#endif

#endif // HASHING_E_CPP
