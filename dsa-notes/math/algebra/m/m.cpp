#ifndef ALGEBRA_M_CPP
#define ALGEBRA_M_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <cmath>
using namespace std;

// =============================================================
// XIII. ШИФРЫ
// =============================================================
// Структура md: A. Классические подстановочные шифры
//               B. Транспозиционные шифры
//               C. Кодировки
//               D. Асимметричные шифры
//               E. Симметричные шифры
//
// Ciphers — раздел ветки algebra. Классические шифры: Caesar,
// Affine, Vigenère, Playfair, Hill. Транспозиционные: Rail Fence.
// Кодировки: Base64, Morse. Асимметричные: RSA, Diffie-Hellman,
// ElGamal. Симметричные: OTP, XOR. Опирается на number-theory
// (обратные элементы, возведение в степень, простые) и
// long arithmetic (раздел XII).

struct Ciphers {

// =============================================================
// A. КЛАССИЧЕСКИЕ ПОДСТАНОВОЧНЫЕ ШИФРЫ
// =============================================================

// --- A.1. Шифр Цезаря ---

// Шифрование: сдвиг каждого символа на k позиций по модулю m
static string caesarEncrypt(const string& text, int k, int m = 26) {
    string res;
    for (char c : text) {
        if (c >= 'A' && c <= 'Z')
            res += (char)('A' + (c - 'A' + k) % m);
        else if (c >= 'a' && c <= 'z')
            res += (char)('a' + (c - 'a' + k) % m);
        else
            res += c;
    }
    return res;
}

// Дешифрование: обратный сдвиг
static string caesarDecrypt(const string& cipher, int k, int m = 26) {
    return caesarEncrypt(cipher, m - (k % m), m);
}

// --- A.2. Аффинный шифр ---

// НОД (расширенный алгоритм Евклида)
static int extGcd(int a, int b, int& x, int& y) {
    if (b == 0) { x = 1; y = 0; return a; }
    int g = extGcd(b, a % b, y, x);
    y -= (a / b) * x;
    return g;
}

// Обратный элемент по модулю: a⁻¹ mod m
static int modInverse(int a, int m) {
    int x, y;
    int g = extGcd(a, m, x, y);
    if (g != 1) return -1; // обратного нет
    return (x % m + m) % m;
}

// Аффинное шифрование: C = (a*P + b) mod m
static string affineEncrypt(const string& text, int a, int b, int m = 26) {
    string res;
    for (char c : text) {
        if (c >= 'A' && c <= 'Z')
            res += (char)('A' + (a * (c - 'A') + b) % m);
        else if (c >= 'a' && c <= 'z')
            res += (char)('a' + (a * (c - 'a') + b) % m);
        else
            res += c;
    }
    return res;
}

// Аффинное расшифрование: P = a⁻¹*(C - b) mod m
static string affineDecrypt(const string& cipher, int a, int b, int m = 26) {
    int aInv = modInverse(a, m);
    if (aInv == -1) return "";
    string res;
    for (char c : cipher) {
        if (c >= 'A' && c <= 'Z')
            res += (char)('A' + (aInv * ((c - 'A') - b + m)) % m);
        else if (c >= 'a' && c <= 'z')
            res += (char)('a' + (aInv * ((c - 'a') - b + m)) % m);
        else
            res += c;
    }
    return res;
}

// --- A.3. Шифр Виженера ---

// Шифрование: C[i] = (P[i] + K[i mod t]) mod m
static string vigenereEncrypt(const string& text, const string& key, int m = 26) {
    string res;
    int t = (int)key.size();
    int ki = 0;
    for (char c : text) {
        if (c >= 'A' && c <= 'Z') {
            int k = toupper(key[ki % t]) - 'A';
            res += (char)('A' + (c - 'A' + k) % m);
            ki++;
        } else if (c >= 'a' && c <= 'z') {
            int k = toupper(key[ki % t]) - 'A';
            res += (char)('a' + (c - 'a' + k) % m);
            ki++;
        } else {
            res += c;
        }
    }
    return res;
}

// Дешифрование: P[i] = (C[i] - K[i mod t]) mod m
static string vigenereDecrypt(const string& cipher, const string& key, int m = 26) {
    string res;
    int t = (int)key.size();
    int ki = 0;
    for (char c : cipher) {
        if (c >= 'A' && c <= 'Z') {
            int k = toupper(key[ki % t]) - 'A';
            res += (char)('A' + (c - 'A' - k + m) % m);
            ki++;
        } else if (c >= 'a' && c <= 'z') {
            int k = toupper(key[ki % t]) - 'A';
            res += (char)('a' + (c - 'a' - k + m) % m);
            ki++;
        } else {
            res += c;
        }
    }
    return res;
}

// --- A.5. Шифр Хилла ---

// Умножение матрицы (n×n) на вектор (n) по модулю m
static vector<int> matMulMod(const vector<vector<int>>& mat,
                             const vector<int>& vec, int n, int m) {
    vector<int> res(n, 0);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            res[i] = (res[i] + mat[i][j] * vec[j]) % m;
        }
    }
    return res;
}

// Hill-шифрование блока
static vector<int> hillEncryptBlock(const vector<vector<int>>& key,
                                     const vector<int>& block, int n, int m) {
    return matMulMod(key, block, n, m);
}

// НОД матрицы по модулю m (для дешифрования)
static vector<vector<int>> matInverseMod(const vector<vector<int>>& mat, int n, int m) {
    // Расширенная матрица [A | I]
    vector<vector<int>> aug(n, vector<int>(2 * n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) aug[i][j] = ((mat[i][j] % m) + m) % m;
        aug[i][n + i] = 1;
    }
    // Приведение к ступенчатому виду
    for (int col = 0; col < n; col++) {
        // Поиск опорного элемента
        int pivot = -1;
        for (int row = col; row < n; row++) {
            if (aug[row][col] != 0) { pivot = row; break; }
        }
        if (pivot == -1) return {}; // необратима
        swap(aug[col], aug[pivot]);
        int inv = modInverse(aug[col][col], m);
        if (inv == -1) return {};
        // Нормализация строки
        for (int j = 0; j < 2 * n; j++)
            aug[col][j] = (int)((long long)aug[col][j] * inv % m);
        // Обнуление столбца
        for (int row = 0; row < n; row++) {
            if (row == col) continue;
            int factor = aug[row][col];
            for (int j = 0; j < 2 * n; j++)
                aug[row][j] = (aug[row][j] - (int)((long long)factor * aug[col][j] % m) + m * m) % m;
        }
    }
    // Извлечение обратной матрицы
    vector<vector<int>> inv(n, vector<int>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            inv[i][j] = aug[i][n + j];
    return inv;
}

// =============================================================
// B. ТРАНСПОЗИЦИОННЫЕ ШИФРЫ
// =============================================================

// --- B.6. Рельсовая ограда (Rail Fence) ---

static string railFenceEncrypt(const string& text, int rails) {
    if (rails <= 1) return text;
    int n = (int)text.size();
    vector<string> fence(rails);
    int row = 0, dir = 1;
    for (int i = 0; i < n; i++) {
        fence[row] += text[i];
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
    }
    string res;
    for (auto& s : fence) res += s;
    return res;
}

static string railFenceDecrypt(const string& cipher, int rails) {
    if (rails <= 1) return cipher;
    int n = (int)cipher.size();
    // Определяем длину каждой строки
    vector<int> len(rails, 0);
    int row = 0, dir = 1;
    for (int i = 0; i < n; i++) {
        len[row]++;
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
    }
    // Заполняем строки
    vector<string> fence(rails);
    int idx = 0;
    for (int r = 0; r < rails; r++) {
        fence[r] = cipher.substr(idx, len[r]);
        idx += len[r];
    }
    // Читаем зигзагом
    string res;
    row = 0; dir = 1;
    vector<int> pos(rails, 0);
    for (int i = 0; i < n; i++) {
        res += fence[row][pos[row]++];
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
    }
    return res;
}

// =============================================================
// C. КОДИРОВКИ
// =============================================================

// --- C.8. Base64 ---

static string base64Encode(const string& input) {
    const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string res;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            res += table[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) res += table[((val << 8) >> (valb + 8)) & 0x3F];
    while (res.size() % 4) res += '=';
    return res;
}

// --- C.9. Код Морзе ---

static string morseEncode(const string& text) {
    const string morse[] = {
        ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
        "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
        "..-", "...-", ".--", "-..-", "-.--", "--.."
    };
    string res;
    for (int i = 0; i < (int)text.size(); i++) {
        char c = toupper(text[i]);
        if (c >= 'A' && c <= 'Z') {
            if (!res.empty()) res += ' ';
            res += morse[c - 'A'];
        } else if (c == ' ') {
            res += " / ";
        }
    }
    return res;
}

// =============================================================
// D. АСИММЕТРИЧНЫЕ ШИФРЫ (концепции)
// =============================================================

// --- D.10. RSA (упрощённый для демонстрации) ---

// Быстрое возведение в степень по модулю: base^exp mod m
static long long modPow(long long base, long long exp, long long m) {
    base %= m;
    long long result = 1;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % m;
        base = (base * base) % m;
        exp >>= 1;
    }
    return result;
}

// RSA-шифрование: C = P^e mod n
static long long rsaEncrypt(long long plain, long long e, long long n) {
    return modPow(plain, e, n);
}

// RSA-дешифрование: P = C^d mod n
static long long rsaDecrypt(long long cipher, long long d, long long n) {
    return modPow(cipher, d, n);
}

// =============================================================
// E. СИММЕТРИЧНЫЕ ШИФРЫ
// =============================================================

// --- E.13. XOR-шифр (с повторяющимся ключом) ---

static string xorEncrypt(const string& text, const string& key) {
    string res;
    int t = (int)key.size();
    for (int i = 0; i < (int)text.size(); i++)
        res += (char)(text[i] ^ key[i % t]);
    return res;
}

// XOR-шифрование и дешифрование — одна операция
static string xorDecrypt(const string& cipher, const string& key) {
    return xorEncrypt(cipher, key);
}

// =============================================================
// ПРИМЕРЫ И ТЕСТЫ
// =============================================================
static void runTests() {
    // Caesar
    assert(caesarEncrypt("HELLO", 3) == "KHOOR");
    assert(caesarDecrypt("KHOOR", 3) == "HELLO");

    // Affine
    string enc = affineEncrypt("HELLO", 5, 8);
    string dec = affineDecrypt(enc, 5, 8);
    assert(dec == "HELLO");

    // Vigenere
    assert(vigenereEncrypt("HELLO", "KEY") == "RIJVS");
    assert(vigenereDecrypt("RIJVS", "KEY") == "HELLO");

    // Rail Fence
    assert(railFenceEncrypt("WEAREDISCOVEREDFLEEATONCE", 3) == "WECRLTEERDSOEEFEAOCAIVDEN");
    assert(railFenceDecrypt("WECRLTEERDSOEEFEAOCAIVDEN", 3) == "WEAREDISCOVEREDFLEEATONCE");

    // Base64
    assert(base64Encode("Hello, World!") == "SGVsbG8sIFdvcmxkIQ==");

    // Morse
    assert(morseEncode("SOS") == "... --- ...");

    // XOR
    assert(xorDecrypt(xorEncrypt("HELLO", "KEY"), "KEY") == "HELLO");

    // RSA
    long long p = 61, q = 53, n = p * q, e = 17;
    long long phi = (p - 1) * (q - 1);
    long long d = modInverse(e, phi);
    long long msg = 65;
    long long cipher = rsaEncrypt(msg, e, n);
    long long plain = rsaDecrypt(cipher, d, n);
    assert(plain == msg);

    cout << "Ciphers: all tests passed" << endl;
}

};

#endif // ALGEBRA_M_CPP
