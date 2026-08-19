#ifndef HASHING_J_CPP
#define HASHING_J_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cstdint>
#include <numeric>
using namespace std;

// =============================================================
// J. БУДУЩИЕ НАПРАВЛЕНИЯ
// =============================================================
// Структура md: A. Постквантовая криптография
//               → B. Новые парадигмы
//               → C. Экзотические применения
//
// HashFuture — наследует HashVerification (i.cpp).
// Grover-подобные оценки, lattice-based хеш,
// гомоморфное хеширование, perceptual hashing.

#ifndef INSIDE_HASHING_J
#define INSIDE_HASHING_J
#include "../i/i.cpp"
#endif

struct HashFuture : HashVerification {

// =============================================================
// A. ПОСТКВАНТОВАЯ КРИПТОГРАФИЯ
// =============================================================

// --- A.1. Оценка безопасности при атаке Гровера ---
pair<double, double> grover_security(int hash_bits) {
    double classical = pow(2.0, hash_bits);
    double quantum = pow(2.0, hash_bits / 2.0);
    return {classical, quantum};
}

// --- A.2. Оценка collision resistance ---
pair<double, double> collision_quantum_security(int hash_bits) {
    double classical = pow(2.0, hash_bits / 2.0);
    double quantum = pow(2.0, hash_bits / 3.0);
    return {classical, quantum};
}

// --- A.3. Рекомендация по размеру хеша ---
// Для post-quantum: m >= 256 bits.
int recommended_hash_bits(bool quantum_threat) {
    return quantum_threat ? 256 : 128;
}

// =============================================================
// B. ГОМОМОРФНОЕ ХЕШИРОВАНИЕ
// =============================================================

// --- B.1. Простое гомоморфное хеширование ---
// h(a * b) = h(a) * h(b) mod p (для мультипликативной группы).
// Работает для: h(x) = g^x mod p.
struct HomomorphicHash {
    long long p, g;

    HomomorphicHash(long long prime = 1000003, long long gen = 3)
        : p(prime), g(gen) {}

    // h(x) = g^x mod p
    long long hash(long long x) {
        long long result = 1;
        long long base = g % p;
        long long exp = ((x % (p - 1)) + (p - 1)) % (p - 1);
        while (exp > 0) {
            if (exp & 1) result = result * base % p;
            base = base * base % p;
            exp >>= 1;
        }
        return result;
    }

    // h(a * b) = h(a) * h(b) mod p
    long long hash_product(long long ha, long long hb) {
        return ha * hb % p;
    }

    // h(a + b) = h(a) * h(b) mod p (аддитивный вариант через g^{a+b})
    long long hash_sum(long long ha, long long hb) {
        return ha * hb % p;
    }
};

// =============================================================
// C. PERCEPTUAL HASHING (МОДЕЛЬ)
// =============================================================

// --- C.1. Простой dHash (difference hash) ---
// Сравнивает смежные пиксели: h[i] = (pixel[i] > pixel[i+1]) ? 1 : 0.
// Устойчив к масштабированию и небольшим поворотам.
long long simple_dhash(const vector<int>& pixels) {
    long long hash = 0;
    for (int i = 0; i + 1 < (int)pixels.size(); i++) {
        hash <<= 1;
        if (pixels[i] > pixels[i + 1]) hash |= 1;
    }
    return hash;
}

// --- C.2. Расстояние Хэмминга между perceptual хешами ---
int perceptual_distance(long long h1, long long h2) {
    return __builtin_popcountll(h1 ^ h2);
}

// =============================================================
// D. МОДЕЛЬ LATTICE-BASED ХЕША
// =============================================================

// --- D.1. Простой lattice hash ---
// h(x) = (A*x + e) mod q, где A — случайная матрица, e — малый шум.
// Для демонстрации: A = фиксированная матрица, e = 0.
struct LatticeHash {
    int n, q;
    vector<vector<int>> A;

    LatticeHash(int dim = 8, int modulus = 97) : n(dim), q(modulus) {
        A.resize(n, vector<int>(n));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                A[i][j] = rand() % q;
    }

    vector<int> hash(const vector<int>& x) {
        vector<int> result(n, 0);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++)
                result[i] = (result[i] + A[i][j] * x[j]) % q;
        }
        return result;
    }
};

}; // struct HashFuture

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_J_MAIN
int main() {
    HashFuture hf;
    srand(42);

    cout << "=== A. ПОСТКВАНТОВАЯ КРИПТОГРАФИЯ ===" << endl;

    cout << "--- Безопасность при атаке Гровера ---" << endl;
    for (int bits : {128, 192, 256, 512}) {
        auto [cl, qu] = hf.grover_security(bits);
        auto [cl_c, qu_c] = hf.collision_quantum_security(bits);
        cout << "  " << bits << "-битный хеш:" << endl;
        cout << "    Preimage: classical=" << cl << " quantum=" << qu << endl;
        cout << "    Collision: classical=" << cl_c << " quantum=" << qu_c << endl;
    }

    cout << "\n--- Рекомендации по размеру ---" << endl;
    cout << "  Классическая угроза: " << hf.recommended_hash_bits(false) << " бит" << endl;
    cout << "  Постквантовая угроза: " << hf.recommended_hash_bits(true) << " бит" << endl;

    cout << "\n=== B. ГОМОМОРФНОЕ ХЕШИРОВАНИЕ ===" << endl;
    {
        HashFuture::HomomorphicHash hh(1000003, 3);
        long long a = 123, b = 456;
        long long ha = hh.hash(a);
        long long hb = hh.hash(b);

        // Аддитивная гомоморфность: h(a+b) = h(a) * h(b) mod p
        long long h_sum = hh.hash(a + b);
        long long h_sum_prod = hh.hash_sum(ha, hb);
        cout << "  h(" << a << ") = " << ha << endl;
        cout << "  h(" << b << ") = " << hb << endl;
        cout << "  h(" << a << " + " << b << ") = " << h_sum << endl;
        cout << "  h(a) * h(b) mod p = " << h_sum_prod << endl;
        cout << "  Аддитивная гомоморфность: " << (h_sum == h_sum_prod ? "OK" : "FAIL") << endl;

        // Применение: хеш суммы без знания слагаемых
        long long h_unknown = hh.hash(789);
        cout << "  h(789) = " << h_unknown << endl;
        cout << "  h(123 + 456) = h(579) = " << hh.hash(579) << endl;
    }

    cout << "\n=== C. PERCEPTUAL HASHING ===" << endl;
    {
        vector<int> img1 = {10, 20, 30, 40, 50, 60, 70, 80};
        vector<int> img2 = {11, 21, 31, 41, 51, 61, 71, 81};  // похожее (+1 к каждому)
        vector<int> img3 = {80, 70, 60, 50, 40, 30, 20, 10};  // перевёрнутое

        auto h1 = hf.simple_dhash(img1);
        auto h2 = hf.simple_dhash(img2);
        auto h3 = hf.simple_dhash(img3);

        cout << "  dhash(img1) = " << h1 << endl;
        cout << "  dhash(img2) = " << h2 << " (похожее)" << endl;
        cout << "  dhash(img3) = " << h3 << " (перевёрнутое)" << endl;
        cout << "  dist(img1, img2) = " << hf.perceptual_distance(h1, h2)
             << " (маленькое = похожие)" << endl;
        cout << "  dist(img1, img3) = " << hf.perceptual_distance(h1, h3)
             << " (большее = непохожие)" << endl;
    }

    cout << "\n=== D. LATTICE-BASED HASH ===" << endl;
    {
        HashFuture::LatticeHash lh(4, 97);
        vector<int> x1 = {1, 2, 3, 4};
        vector<int> x2 = {1, 2, 3, 5};
        vector<int> x3 = {5, 4, 3, 2};

        auto h1 = lh.hash(x1);
        auto h2 = lh.hash(x2);
        auto h3 = lh.hash(x3);

        cout << "  h(1,2,3,4) = [";
        for (int i = 0; i < 4; i++) { if (i) cout << ","; cout << h1[i]; }
        cout << "]" << endl;
        cout << "  h(1,2,3,5) = [";
        for (int i = 0; i < 4; i++) { if (i) cout << ","; cout << h2[i]; }
        cout << "]" << endl;
        cout << "  h(5,4,3,2) = [";
        for (int i = 0; i < 4; i++) { if (i) cout << ","; cout << h3[i]; }
        cout << "]" << endl;

        // Проверка: похожие входы → разные хеши
        int diff12 = 0, diff13 = 0;
        for (int i = 0; i < 4; i++) {
            if (h1[i] != h2[i]) diff12++;
            if (h1[i] != h3[i]) diff13++;
        }
        cout << "  Различий h1 vs h2: " << diff12 << "/4" << endl;
        cout << "  Различий h1 vs h3: " << diff13 << "/4" << endl;
    }

    return 0;
}
#endif

#endif // HASHING_J_CPP
