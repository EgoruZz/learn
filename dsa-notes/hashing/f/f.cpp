#ifndef HASHING_F_CPP
#define HASHING_F_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <iomanip>
using namespace std;

// =============================================================
// F. КРИПТОГРАФИЧЕСКИЕ АСПЕКТЫ
// =============================================================
// Структура md: A. Криптоанализ
//               → B. Современные криптографические хеши
//               → C. Специализированные примитивы
//
// CryptoHashing — наследует HashFunctions (e.cpp).
// Дифференциальный/линейный криптоанализ (модели),
// SHA-256 (упрощённый), HMAC, HKDF.

#ifndef INSIDE_HASHING_F
#define INSIDE_HASHING_F
#include "../e/e.cpp"
#endif

struct CryptoHashing : HashFunctions {

// =============================================================
// A. КРИПТОАНАЛИЗ (МОДЕЛИ)
// =============================================================

// --- A.1. Дифференциальная вероятность S-блока ---
// P(S(x) ⊕ S(x ⊕ Δx) = Δy) для заданных Δx, Δy.
// model[i] = вероятность для i-го входа.
// Возвращает P(Δy | Δx) — сумма по всем x, дающим Δy.
double differential_probability(
    const vector<int>& input_diffs,
    const vector<int>& output_diffs,
    int block_size) {
    int total = 1 << block_size;
    int favorable = 0;
    for (int x = 0; x < total; x++) {
        for (int dx : input_diffs) {
            int sx = x;  // placeholder for S-box
            int sxdx = x ^ dx;
            int out = sx ^ sxdx;
            for (int dy : output_diffs) {
                if (out == dy) favorable++;
            }
        }
    }
    return (double)favorable / (total * input_diffs.size());
}

// --- A.2. Линейная аппроксимация S-блока ---
// P(S(x) · mask_out = x · mask_in) - 1/2 = bias.
// Возвращает |bias|.
double linear_approximation_bias(int mask_in, int mask_out,
                                  int block_size) {
    int total = 1 << block_size;
    int agree = 0;
    for (int x = 0; x < total; x++) {
        int sx = x;  // placeholder for S-box
        if (__builtin_popcount(x & mask_in) % 2 ==
            __builtin_popcount(sx & mask_out) % 2)
            agree++;
    }
    return fabs((double)agree / total - 0.5);
}

// =============================================================
// B. SHA-256 (УПРОЩЁННАЯ РЕАЛИЗАЦИЯ)
// =============================================================

struct SHA256 {
    uint32_t H[8];
    static uint32_t get_K(int i) {
        static const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };
        return k[i];
    }

    SHA256() {
        H[0] = 0x6a09e667; H[1] = 0xbb67ae85;
        H[2] = 0x3c6ef372; H[3] = 0xa54ff53a;
        H[4] = 0x510e527f; H[5] = 0x9b05688c;
        H[6] = 0x1f83d9ab; H[7] = 0x5be0cd19;
    }

    static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
    static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static uint32_t Sigma0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static uint32_t Sigma1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static uint32_t sigma0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static uint32_t sigma1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    void process_block(const uint8_t block[64]) {
        uint32_t W[64];
        for (int i = 0; i < 16; i++)
            W[i] = (uint32_t)block[4*i] << 24 | (uint32_t)block[4*i+1] << 16 |
                   (uint32_t)block[4*i+2] << 8 | (uint32_t)block[4*i+3];
        for (int i = 16; i < 64; i++)
            W[i] = sigma1(W[i-2]) + W[i-7] + sigma0(W[i-15]) + W[i-16];

        uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
        uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

        for (int i = 0; i < 64; i++) {
            uint32_t T1 = h + Sigma1(e) + Ch(e, f, g) + get_K(i) + W[i];
            uint32_t T2 = Sigma0(a) + Maj(a, b, c);
            h = g; g = f; f = e; e = d + T1;
            d = c; c = b; b = a; a = T1 + T2;
        }
        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }

    string hash(const string& input) {
        uint64_t bit_len = input.size() * 8;
        vector<uint8_t> msg(input.begin(), input.end());
        msg.push_back(0x80);
        while (msg.size() % 64 != 56) msg.push_back(0);
        for (int i = 0; i < 8; i++)
            msg.push_back((bit_len >> (56 - 8 * i)) & 0xFF);

        for (size_t i = 0; i < msg.size(); i += 64)
            process_block(msg.data() + i);

        stringstream ss;
        for (int i = 0; i < 8; i++)
            ss << hex << setfill('0') << setw(8) << H[i];
        return ss.str();
    }
};


// =============================================================
// C. HMAC
// =============================================================

// HMAC(K, m) = H((K' ⊕ opad) ‖ H((K' ⊕ ipad) ‖ m)).
// Упрощённая версия для демонстрации.
string hmac_sha256(const string& key, const string& message) {
    const int blockSize = 64;
    string K = key;
    if ((int)K.size() > blockSize) {
        SHA256 sha;
        K = sha.hash(key);
    }
    K += string(blockSize - K.size(), '\0');

    string ipad(blockSize, 0x36);
    string opad(blockSize, 0x5c);
    for (int i = 0; i < blockSize; i++) {
        ipad[i] ^= (unsigned char)K[i];
        opad[i] ^= (unsigned char)K[i];
    }

    SHA256 inner;
    string inner_hash = inner.hash(ipad + message);

    SHA256 outer;
    // Need to pad inner_hash to blockSize for concatenation
    string inner_padded = inner_hash;
    while ((int)inner_padded.size() < blockSize) inner_padded += '\0';
    return outer.hash(opad + inner_padded);
}

// =============================================================
// D. HKDF
// =============================================================

// HKDF-Extract: PRK = HMAC(salt, IKM).
// HKDF-Expand: OKM = T(1) ‖ T(2) ‖ ... where T(i) = HMAC(PRK, T(i-1) ‖ info ‖ i).
string hkdf_extract(const string& salt, const string& ikm) {
    return hmac_sha256(salt, ikm);
}

string hkdf_expand(const string& prk, const string& info, int length) {
    string result;
    string prev;
    int iterations = (length + 31) / 32;
    for (int i = 1; i <= iterations; i++) {
        string input = prev + info + (char)i;
        prev = hmac_sha256(prk, input);
        result += prev;
    }
    return result.substr(0, length);
}

}; // struct CryptoHashing

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_F_MAIN
int main() {
    CryptoHashing ch;
    srand(42);

    cout << "=== A. КРИПТОАНАЛИЗ ===" << endl;

    cout << "--- Дифференциальная вероятность (модель) ---" << endl;
    // Простая модель:.identity S-box, 4-битный блок
    {
        // Δx=0x1 → Δy=0x1 с вероятностью 1 (identity)
        vector<int> dx = {0x1};
        vector<int> dy = {0x1};
        double dp = ch.differential_probability(dx, dy, 4);
        cout << "  Identity S-box (4-bit): P(Δx=1→Δy=1) = " << dp << endl;
    }

    cout << "\n--- Линейная аппроксимация (модель) ---" << endl;
    {
        double bias = ch.linear_approximation_bias(0x1, 0x1, 4);
        cout << "  Identity S-box (4-bit): bias(|mask_in=1, mask_out=1|) = " << bias << endl;
    }

    cout << "\n=== B. SHA-256 ===" << endl;
    {
        CryptoHashing::SHA256 sha;
        for (auto& msg : {"hello", "world", "SHA-256 test", ""}) {
            string h = sha.hash(msg);
            cout << "  SHA256(\"" << msg << "\") = " << h.substr(0, 16) << "..." << endl;
        }
        // Проверка: SHA256("") = e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
        string h_empty = sha.hash("");
        cout << "  SHA256(\"\") = " << h_empty << endl;
        cout << "  Ожидается:    e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" << endl;
    }

    cout << "\n=== C. HMAC ===" << endl;
    {
        string key = "secret_key";
        string message = "Hello, World!";
        string mac = ch.hmac_sha256(key, message);
        cout << "  HMAC-SHA256(\"" << key << "\", \"" << message << "\") = " << mac.substr(0, 16) << "..." << endl;
        // Проверка: одинаковый ключ → одинаковый MAC
        string mac2 = ch.hmac_sha256(key, message);
        cout << "  Детерминированность: " << (mac == mac2 ? "OK" : "FAIL") << endl;
        // Разный ключ → разный MAC
        string mac3 = ch.hmac_sha256("other_key", message);
        cout << "  Разные ключи → разные MAC: " << (mac != mac3 ? "OK" : "FAIL") << endl;
    }

    cout << "\n=== D. HKDF ===" << endl;
    {
        string salt = "random_salt";
        string ikm = "input_key_material";
        string prk = ch.hkdf_extract(salt, ikm);
        cout << "  PRK = " << prk.substr(0, 16) << "..." << endl;
        string okm = ch.hkdf_expand(prk, "info_context", 64);
        cout << "  OKM (64 bytes) = " << okm.substr(0, 16) << "..." << endl;
        // Проверка: одинаковый input → одинаковый output
        string prk2 = ch.hkdf_extract(salt, ikm);
        cout << "  Детерминированность Extract: " << (prk == prk2 ? "OK" : "FAIL") << endl;
    }

    return 0;
}
#endif

#endif // HASHING_F_CPP
