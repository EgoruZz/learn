#ifndef HASHING_G_CPP
#define HASHING_G_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cstdint>
#include <thread>
#include <mutex>
#include <atomic>
#include <numeric>
#include <chrono>
#include <map>
#include <set>
using namespace std;

// =============================================================
// G. ОПТИМИЗАЦИЯ И ВЫСОКОПРОИЗВОДИТЕЛЬНЫЕ РЕАЛИЗАЦИИ
// =============================================================
// Структура md: A. Алгоритмические оптимизации
//               → B. Аппаратные оптимизации
//               → C. Распределённые реализации
//
// HashOptimization — наследует CryptoHashing (f.cpp).
// Lookup Tables, Loop Unrolling, Branchless,
// SIMD-моделирование, консистентное хеширование.

#ifndef INSIDE_HASHING_G
#define INSIDE_HASHING_G
#include "../f/f.cpp"
#endif

struct HashOptimization : CryptoHashing {

// =============================================================
// A. АЛГОРИТМИЧЕСКИЕ ОПТИМИЗАЦИИ
// =============================================================

// --- A.1. Lookup Table для CRC32 ---
// Таблица из 256 полиномов.
static uint32_t crc32_table[];
static bool crc32_table_ready;

static void init_crc32_table() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        crc32_table[i] = crc;
    }
    crc32_table_ready = true;
}

uint32_t crc32_lut(const string& s) {
    uint32_t crc = 0xFFFFFFFF;
    for (char c : s) {
        crc = crc32_table[(crc ^ (unsigned char)c) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

// --- A.2. CRC32 без таблицы (наивный) ---
uint32_t crc32_naive(const string& s) {
    uint32_t crc = 0xFFFFFFFF;
    for (char c : s) {
        crc ^= (unsigned char)c;
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
    }
    return crc ^ 0xFFFFFFFF;
}

// --- A.3. Branchless хеш (conditional) ---
// branchless_abs: |x| без ветвления.
int branchless_abs(int x) {
    int mask = x >> 31;
    return (x ^ mask) - mask;
}

// --- A.4. Loop Unrolling: сумма квадратов ---
// Развёрнутый цикл (4 итерации за шаг).
long long sum_squares_unrolled(const vector<int>& v) {
    long long sum = 0;
    int n = (int)v.size();
    int i = 0;
    for (; i + 3 < n; i += 4) {
        sum += (long long)v[i] * v[i];
        sum += (long long)v[i+1] * v[i+1];
        sum += (long long)v[i+2] * v[i+2];
        sum += (long long)v[i+3] * v[i+3];
    }
    for (; i < n; i++) sum += (long long)v[i] * v[i];
    return sum;
}

// =============================================================
// B. МОДЕЛИРОВАНИЕ АППАРАТНЫХ ОПТИМИЗАЦИЙ
// =============================================================

// --- B.1. SIMD-модель: параллельное хеширование 4 строк ---
// Обрабатываем 4 строки за проход (имитация AVX2).
vector<long long> simd_hash4(const vector<string>& strings,
                              function<long long(const string&)> hash_fn) {
    vector<long long> results(strings.size());
    int i = 0;
    for (; i + 3 < (int)strings.size(); i += 4) {
        results[i] = hash_fn(strings[i]);
        results[i+1] = hash_fn(strings[i+1]);
        results[i+2] = hash_fn(strings[i+2]);
        results[i+3] = hash_fn(strings[i+3]);
    }
    for (; i < (int)strings.size(); i++)
        results[i] = hash_fn(strings[i]);
    return results;
}

// =============================================================
// C. КОНСИСТЕНТНОЕ ХЕШИРОВАНИЕ
// =============================================================

struct ConsistentHash {
    map<unsigned int, int> ring;  // hash → server_id
    int num_replicas;

    ConsistentHash(int servers, int replicas = 150)
        : num_replicas(replicas) {
        for (int i = 0; i < servers; i++)
            add_server(i);
    }

    void add_server(int id) {
        for (int i = 0; i < num_replicas; i++) {
            unsigned int h = (unsigned int)((long long)(id * 1000 + i) * 2654435761ULL);
            ring[h] = id;
        }
    }

    void remove_server(int id) {
        for (int i = 0; i < num_replicas; i++) {
            unsigned int h = (unsigned int)((long long)(id * 1000 + i) * 2654435761ULL);
            ring.erase(h);
        }
    }

    int get_server(const string& key) const {
        if (ring.empty()) return -1;
        unsigned int h = (unsigned int)((long long)(hash<string>{}(key)) * 2654435761ULL);
        auto it = ring.lower_bound(h);
        if (it == ring.end()) it = ring.begin();
        return it->second;
    }

    // Подсчёт перераспределения ключей
    int count_affected_keys(const vector<string>& keys,
                             int old_server, int new_server) {
        int affected = 0;
        for (auto& k : keys) {
            int old_s = get_server(k);
            // Temporarily add new server and check
            const_cast<ConsistentHash*>(this)->add_server(new_server);
            int new_s = get_server(k);
            const_cast<ConsistentHash*>(this)->remove_server(new_server);
            if (old_s != new_s) affected++;
        }
        return affected;
    }
};

// =============================================================
// D. PARALLEL HASHING
// =============================================================

// --- D.1. Многопоточное хеширование ---
// Разбиваем массив на потоки, каждый хеширует свою часть.
vector<long long> parallel_hash(const vector<string>& data,
                                 function<long long(const string&)> hash_fn,
                                 int num_threads = 4) {
    int n = (int)data.size();
    vector<long long> results(n);
    vector<thread> threads;
    int chunk = (n + num_threads - 1) / num_threads;

    for (int t = 0; t < num_threads; t++) {
        int start = t * chunk;
        int end = min(start + chunk, n);
        threads.emplace_back([&](int s, int e) {
            for (int i = s; i < e; i++)
                results[i] = hash_fn(data[i]);
        }, start, end);
    }
    for (auto& th : threads) th.join();
    return results;
}

// --- D.2. Sharded хеш-таблица ---
struct ShardedHashTable {
    int num_shards;
    vector<unordered_map<long long, int>> shards;
    vector<mutex> shard_mutexes;

    ShardedHashTable(int n = 8) : num_shards(n), shards(n), shard_mutexes(n) {}

    int get_shard(long long key) {
        return ((key % num_shards) + num_shards) % num_shards;
    }

    void insert(long long key, int value) {
        int s = get_shard(key);
        lock_guard<mutex> lock(shard_mutexes[s]);
        shards[s][key] = value;
    }

    bool search(long long key, int& value) {
        int s = get_shard(key);
        lock_guard<mutex> lock(shard_mutexes[s]);
        if (shards[s].count(key)) { value = shards[s][key]; return true; }
        return false;
    }

    int size() {
        int total = 0;
        for (auto& s : shards) total += s.size();
        return total;
    }
};

}; // struct HashOptimization

uint32_t HashOptimization::crc32_table[256];
bool HashOptimization::crc32_table_ready = false;

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_G_MAIN
int main() {
    HashOptimization ho;
    srand(42);

    cout << "=== A. АЛГОРИТМИЧЕСКИЕ ОПТИМИЗАЦИИ ===" << endl;

    cout << "--- CRC32: Lookup Table vs Naive ---" << endl;
    {
        string test = "Hello, World! This is a test string for CRC32.";
        auto start1 = chrono::high_resolution_clock::now();
        uint32_t crc1 = ho.crc32_naive(test);
        auto end1 = chrono::high_resolution_clock::now();
        auto start2 = chrono::high_resolution_clock::now();
        uint32_t crc2 = ho.crc32_lut(test);
        auto end2 = chrono::high_resolution_clock::now();
        auto t1 = chrono::duration_cast<chrono::nanoseconds>(end1 - start1).count();
        auto t2 = chrono::duration_cast<chrono::nanoseconds>(end2 - start2).count();
        cout << "  Naive:  " << crc1 << " (" << t1 << " ns)" << endl;
        cout << "  LUT:    " << crc2 << " (" << t2 << " ns)" << endl;
        cout << "  Speedup: " << (t1 > 0 ? (double)t1/t2 : 0) << "x" << endl;
    }

    cout << "\n--- Branchless abs ---" << endl;
    for (int x : {-5, -1, 0, 3, 7}) {
        cout << "  |" << x << "| = " << ho.branchless_abs(x)
             << " (expected " << abs(x) << ")" << endl;
    }

    cout << "\n--- Loop Unrolling: sum of squares ---" << endl;
    {
        vector<int> v(1000);
        for (int i = 0; i < 1000; i++) v[i] = i;
        long long result = ho.sum_squares_unrolled(v);
        long long expected = 0;
        for (int i = 0; i < 1000; i++) expected += (long long)i * i;
        cout << "  sum_squares(0..999) = " << result << " (expected " << expected << ")" << endl;
    }

    cout << "\n=== B. SIMD-МОДЕЛИРОВАНИЕ ===" << endl;
    {
        vector<string> strs = {"hello", "world", "hash", "test", "foo", "bar", "baz", "qux"};
        auto results = ho.simd_hash4(strs, [&ho](const string& s) -> long long {
            return ho.simple_polynomial_hash(s);
        });
        cout << "  SIMD hash4:" << endl;
        for (int i = 0; i < (int)strs.size(); i++)
            cout << "    \"" << strs[i] << "\" = " << results[i] << endl;
    }

    cout << "\n=== C. КОНСИСТЕНТНОЕ ХЕШИРОВАНИЕ ===" << endl;
    {
        HashOptimization::ConsistentHash ch(3);  // 3 сервера
        vector<string> keys = {"key1", "key2", "key3", "key4", "key5",
                               "key6", "key7", "key8", "key9", "key10"};
        cout << "  Распределение по 3 серверам:" << endl;
        map<int, int> counts;
        for (auto& k : keys) {
            int s = ch.get_server(k);
            counts[s]++;
        }
        for (auto& [s, c] : counts) cout << "    Сервер " << s << ": " << c << " ключей" << endl;
        cout << "  Добавляем сервер 3..." << endl;
        ch.add_server(3);
        counts.clear();
        for (auto& k : keys) counts[ch.get_server(k)]++;
        for (auto& [s, c] : counts) cout << "    Сервер " << s << ": " << c << " ключей" << endl;
    }

    cout << "\n=== D. ПАРАЛЛЕЛЬНОЕ ХЕШИРОВАНИЕ ===" << endl;
    {
        vector<string> data(10000);
        for (int i = 0; i < 10000; i++) data[i] = "key_" + to_string(i);
        auto hash_fn = [&ho](const string& s) -> long long {
            return ho.simple_polynomial_hash(s);
        };

        auto start1 = chrono::high_resolution_clock::now();
        vector<long long> seq_results(data.size());
        for (int i = 0; i < (int)data.size(); i++) seq_results[i] = hash_fn(data[i]);
        auto end1 = chrono::high_resolution_clock::now();

        auto start2 = chrono::high_resolution_clock::now();
        auto par_results = ho.parallel_hash(data, hash_fn, 4);
        auto end2 = chrono::high_resolution_clock::now();

        auto t1 = chrono::duration_cast<chrono::microseconds>(end1 - start1).count();
        auto t2 = chrono::duration_cast<chrono::microseconds>(end2 - start2).count();
        cout << "  Sequential: " << t1 << " μs" << endl;
        cout << "  Parallel (4 threads): " << t2 << " μs" << endl;
        cout << "  Speedup: " << (t2 > 0 ? (double)t1/t2 : 0) << "x" << endl;
        cout << "  Results match: " << (seq_results == par_results ? "OK" : "FAIL") << endl;
    }

    cout << "\n--- Sharded Hash Table ---" << endl;
    {
        HashOptimization::ShardedHashTable sht(4);
        for (int i = 0; i < 1000; i++) sht.insert(i, i * 10);
        cout << "  Inserted 1000 elements, total size: " << sht.size() << endl;
        int v;
        cout << "  search(42): " << (sht.search(42, v) ? to_string(v) : "not found") << endl;
        cout << "  search(999): " << (sht.search(999, v) ? to_string(v) : "not found") << endl;
    }

    return 0;
}
#endif

#endif // HASHING_G_CPP
