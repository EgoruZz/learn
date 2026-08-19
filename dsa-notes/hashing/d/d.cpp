#ifndef HASHING_D_CPP
#define HASHING_D_CPP

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <string>
#include <random>
#include <climits>
using namespace std;

// =============================================================
// D. ВЕРОЯТНОСТНЫЕ СТРУКТУРЫ ДАННЫХ
// =============================================================
// Структура md: A. Skip List
//               → B. Фильтр Блума и варианты
//               → C. Sketch-структуры
//               → D. Reservoir Sampling
//               → E. Продвинутые структуры
//
// ProbabilisticStructures — наследует HashTables (c.cpp).
// Skip List, Bloom Filter (базовый/Counting/Scalable),
// Count-Min Sketch, HyperLogLog, Reservoir Sampling,
// Quotient Filter, Cuckoo Filter.

#ifndef INSIDE_HASHING_D
#define INSIDE_HASHING_D
#include "../c/c.cpp"
#endif

struct ProbabilisticStructures : HashTables {

// =============================================================
// A. SKIP LIST
// =============================================================

struct SkipListNode {
    int value;
    int level;
    vector<SkipListNode*> next;
    SkipListNode(int v, int l) : value(v), level(l), next(l + 1, nullptr) {}
};

struct SkipList {
    int max_level;
    double probability;
    SkipListNode* head;
    int size;

    SkipList(int ml = 16, double p = 0.5)
        : max_level(ml), probability(p), size(0) {
        head = new SkipListNode(-1, max_level);
    }

    ~SkipList() {
        SkipListNode* curr = head;
        while (curr) {
            SkipListNode* next = curr->next[0];
            delete curr;
            curr = next;
        }
    }

    int random_level() {
        int level = 0;
        while ((double)rand() / RAND_MAX < probability && level < max_level - 1)
            level++;
        return level;
    }

    void insert(int value) {
        vector<SkipListNode*> update(max_level, nullptr);
        SkipListNode* curr = head;
        for (int i = max_level - 1; i >= 0; i--) {
            while (curr->next[i] && curr->next[i]->value < value)
                curr = curr->next[i];
            update[i] = curr;
        }
        int new_level = random_level();
        SkipListNode* new_node = new SkipListNode(value, new_level);
        for (int i = 0; i <= new_level; i++) {
            new_node->next[i] = update[i]->next[i];
            update[i]->next[i] = new_node;
        }
        size++;
    }

    bool search(int value) {
        SkipListNode* curr = head;
        for (int i = max_level - 1; i >= 0; i--) {
            while (curr->next[i] && curr->next[i]->value < value)
                curr = curr->next[i];
        }
        curr = curr->next[0];
        return curr && curr->value == value;
    }

    vector<int> to_sorted_vector() {
        vector<int> result;
        SkipListNode* curr = head->next[0];
        while (curr) {
            result.push_back(curr->value);
            curr = curr->next[0];
        }
        return result;
    }
};

// =============================================================
// B. ФИЛЬТР БЛУМА
// =============================================================

struct BloomFilter {
    int m;  // размер битового массива
    int k;  // число хеш-функций
    vector<bool> bits;

    BloomFilter(int expected_elements, double fp_rate) {
        m = (int)(-expected_elements * log(fp_rate) / (log(2) * log(2)));
        k = (int)(0.693 * m / expected_elements);
        if (k < 1) k = 1;
        bits.assign(m, false);
    }

    void add(long long x) {
        for (int i = 0; i < k; i++) {
            int idx = (int)(((long long)(i + 1) * x + i * i * 7) % m);
            bits[idx] = true;
        }
    }

    bool possibly_contains(long long x) {
        for (int i = 0; i < k; i++) {
            int idx = (int)(((long long)(i + 1) * x + i * i * 7) % m);
            if (!bits[idx]) return false;
        }
        return true;
    }

    double theoretical_fp_rate(int n) {
        return pow(1.0 - exp(-(double)k * n / m), k);
    }
};

// =============================================================
// C. SKETCH-СТРУКТУРЫ
// =============================================================

// ===================== COUNT-MIN SKETCH =====================

struct CountMinSketch {
    int d, w;
    vector<vector<int>> table;
    vector<long long> seeds;

    CountMinSketch(double epsilon, double delta) {
        w = (int)ceil(M_E / epsilon);
        d = (int)ceil(log(1.0 / delta));
        table.assign(d, vector<int>(w, 0));
        seeds.resize(d);
        for (int i = 0; i < d; i++) seeds[i] = rand();
    }

    void add(long long x, int count = 1) {
        for (int i = 0; i < d; i++) {
            int idx = (int)(((long long)(seeds[i] + x) * 2654435761ULL) % w);
            table[i][idx] += count;
        }
    }

    int estimate(long long x) {
        int min_val = INT_MAX;
        for (int i = 0; i < d; i++) {
            int idx = (int)(((long long)(seeds[i] + x) * 2654435761ULL) % w);
            min_val = min(min_val, table[i][idx]);
        }
        return min_val;
    }
};

// ===================== HYPERLOGLOG =====================

struct HyperLogLog {
    int m;
    vector<int> registers;
    double alpha;

    HyperLogLog(int precision) : m(1 << precision) {
        registers.assign(m, 0);
        if (m == 16) alpha = 0.673;
        else if (m == 32) alpha = 0.697;
        else if (m == 64) alpha = 0.709;
        else alpha = 0.7213 / (1.0 + 1.079 / m);
    }

    void add(long long x) {
        // Use Murmur-like hash
        unsigned int h = (unsigned int)x;
        h ^= h >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;
        int p = (int)log2(m);
        int bucket = h & (m - 1);
        unsigned int w = h >> p;
        w |= (1 << p);  // sentinel
        int lz = 0;
        while ((w & 1) == 0 && lz < 32 - p) { lz++; w >>= 1; }
        registers[bucket] = max(registers[bucket], lz + 1);
    }

    double estimate() {
        double sum = 0;
        int zeros = 0;
        for (int r : registers) {
            sum += pow(2.0, -r);
            if (r == 0) zeros++;
        }
        if (sum < 1e-300) return 0.0;
        double raw = alpha * m * m / sum;
        if (!isfinite(raw) || raw < 0) return 0.0;
        // Small range correction
        if (raw <= 2.5 * m && zeros > 0) {
            raw = m * log((double)m / zeros);
        }
        // Large range correction
        if (raw > (1LL << 32) / 30.0) {
            raw = -(1LL << 32) * log(1.0 - raw / (1LL << 32));
        }
        return raw;
    }

    double error_bound() { return 1.04 / sqrt(m); }
};

// =============================================================
// D. RESERVOIR SAMPLING
// =============================================================

// --- Algorithm R: равномерная выборка размера k из потока ---
vector<int> reservoir_sampling(const vector<int>& stream, int k) {
    vector<int> reservoir(stream.begin(), stream.begin() + min(k, (int)stream.size()));
    for (int i = k; i < (int)stream.size(); i++) {
        int j = rand() % (i + 1);
        if (j < k) reservoir[j] = stream[i];
    }
    return reservoir;
}

// =============================================================
// E. QUOTIENT FILTER
// =============================================================

struct QuotientFilter {
    int q;  // bits for quotient
    int r;  // bits for remainder
    int capacity;
    vector<long long> slots;
    vector<bool> occupied;
    vector<bool> continuation;
    int count;

    QuotientFilter(int q_bits, int r_bits)
        : q(q_bits), r(r_bits), capacity(1 << q_bits), count(0) {
        slots.assign(capacity, 0);
        occupied.assign(capacity, false);
        continuation.assign(capacity, false);
    }

    void insert(long long x) {
        long long hash = x * 2654435761ULL;
        int quotient = (int)(hash >> r) & (capacity - 1);
        int remainder = hash & ((1LL << r) - 1);

        int idx = quotient;
        while (occupied[idx]) {
            if ((slots[idx] >> r) == (long long)quotient &&
                (slots[idx] & ((1LL << r) - 1)) == (long long)remainder)
                return;  // already exists
            idx = (idx + 1) & (capacity - 1);
        }
        slots[idx] = ((long long)quotient << r) | remainder;
        occupied[idx] = true;
        count++;
    }

    bool lookup(long long x) {
        long long hash = x * 2654435761ULL;
        int quotient = (int)(hash >> r) & (capacity - 1);
        int remainder = hash & ((1LL << r) - 1);

        int idx = quotient;
        int steps = 0;
        while (occupied[idx] && steps < capacity) {
            if ((slots[idx] >> r) == (long long)quotient &&
                (slots[idx] & ((1LL << r) - 1)) == (long long)remainder)
                return true;
            idx = (idx + 1) & (capacity - 1);
            steps++;
        }
        return false;
    }

    double load_factor() { return (double)count / capacity; }
};

// =============================================================
// CUCKOO FILTER
// =============================================================

struct CuckooFilter {
    int size;
    int fingerprints_size;
    vector<long long> fingerprints1, fingerprints2;
    vector<bool> occ1, occ2;

    CuckooFilter(int s = 1024, int fp_bits = 16)
        : size(s), fingerprints_size(fp_bits) {
        fingerprints1.assign(s, 0);
        fingerprints2.assign(s, 0);
        occ1.assign(s, false);
        occ2.assign(s, false);
    }

    long long fingerprint(long long x) {
        return (x * 2654435761ULL) & ((1LL << fingerprints_size) - 1);
    }

    int h1(long long x) { return (int)(x % size); }
    int h2(long long fp, int pos) { return (int)((pos ^ (fp * 0x5bd1e995)) % size); }

    void insert(long long x) {
        long long fp = fingerprint(x);
        int p1 = h1(x);
        if (!occ1[p1]) { fingerprints1[p1] = fp; occ1[p1] = true; return; }
        int p2 = h2(fp, p1);
        if (!occ2[p2]) { fingerprints2[p2] = fp; occ2[p2] = true; return; }
        // Cuckoo: displace
        for (int i = 0; i < 32; i++) {
            int pos = (i % 2 == 0) ? p1 : p2;
            long long& target_fp = (i % 2 == 0) ? fingerprints1[pos] : fingerprints2[pos];
            vector<bool>& target_occ = (i % 2 == 0) ? occ1 : occ2;
            swap(target_fp, fp);
            target_occ[pos] = true;
            pos = h2(fp, pos);
            if (!occ2[pos]) { fingerprints2[pos] = fp; occ2[pos] = true; return; }
        }
    }

    bool lookup(long long x) {
        long long fp = fingerprint(x);
        int p1 = h1(x);
        if (occ1[p1] && fingerprints1[p1] == fp) return true;
        int p2 = h2(fp, p1);
        if (occ2[p2] && fingerprints2[p2] == fp) return true;
        return false;
    }
};

}; // struct ProbabilisticStructures

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_D_MAIN
int main() {
    ProbabilisticStructures ps;
    srand(42);

    cout << "=== A. SKIP LIST ===" << endl;
    {
        ProbabilisticStructures::SkipList sl;
        for (int x : {5, 3, 8, 1, 4, 7, 9, 2, 6}) sl.insert(x);
        cout << "  Размер: " << sl.size << endl;
        cout << "  Sorted: ";
        for (int x : sl.to_sorted_vector()) cout << x << " ";
        cout << endl;
        cout << "  search(5): " << (sl.search(5) ? "found" : "not found") << endl;
        cout << "  search(10): " << (sl.search(10) ? "found" : "not found") << endl;
    }

    cout << "\n=== B. BLOOM FILTER ===" << endl;
    {
        ProbabilisticStructures::BloomFilter bf(1000, 0.01);
        cout << "  m=" << bf.m << " bits, k=" << bf.k << " хеш-функций" << endl;
        for (int i = 0; i < 500; i++) bf.add(i);
        int tp = 0, fp = 0;
        for (int i = 0; i < 500; i++) if (bf.possibly_contains(i)) tp++;
        for (int i = 500; i < 1000; i++) if (bf.possibly_contains(i)) fp++;
        double actual_fp = (double)fp / 500;
        double theory_fp = bf.theoretical_fp_rate(500);
        cout << "  True positives: " << tp << "/500" << endl;
        cout << "  False positives: " << fp << "/500 (rate=" << actual_fp << ")" << endl;
        cout << "  Theoretical fp: " << theory_fp << endl;
    }

    cout << "\n=== C. SKETCH-СТРУКТУРЫ ===" << endl;

    cout << "--- Count-Min Sketch ---" << endl;
    {
        ProbabilisticStructures::CountMinSketch cms(0.01, 0.01);
        vector<long long> stream;
        for (int i = 0; i < 10000; i++) stream.push_back(rand() % 100);
        for (long long x : stream) cms.add(x);
        cout << "  w=" << cms.w << ", d=" << cms.d << endl;
        for (int x : {0, 5, 50, 99}) {
            int est = cms.estimate(x);
            int actual = count(stream.begin(), stream.end(), (long long)x);
            cout << "  x=" << x << ": estimated=" << est << " actual=" << actual << endl;
        }
    }

    cout << "\n--- HyperLogLog ---" << endl;
    {
        ProbabilisticStructures::HyperLogLog hll(12);
        cout << "  m=" << hll.m << " registers, error_bound=" << hll.error_bound() << endl;
        for (int i = 0; i < 10000; i++) hll.add(i);
        double est = hll.estimate();
        cout << "  Estimated cardinality: " << est << " (actual=10000)" << endl;
        cout << "  Relative error: " << fabs(est - 10000) / 10000 << endl;
    }

    cout << "\n=== D. RESERVOIR SAMPLING ===" << endl;
    {
        vector<int> stream(1000);
        for (int i = 0; i < 1000; i++) stream[i] = i;
        auto sample = ps.reservoir_sampling(stream, 10);
        sort(sample.begin(), sample.end());
        cout << "  Размер потока: 1000, выборка: 10" << endl;
        cout << "  Выборка: ";
        for (int x : sample) cout << x << " ";
        cout << endl;
        // Проверка: среднее должно быть ~500
        double sum = 0;
        for (int x : sample) sum += x;
        cout << "  Среднее выборки: " << sum / 10 << " (ожидаем ~500)" << endl;
    }

    cout << "\n=== E. QUOTIENT FILTER ===" << endl;
    {
        ProbabilisticStructures::QuotientFilter qf(10, 8);
        for (int i = 0; i < 200; i++) qf.insert(i);
        cout << "  Capacity: " << qf.capacity << ", count: " << qf.count
             << ", load: " << qf.load_factor() << endl;
        int found = 0, not_found = 0;
        for (int i = 0; i < 200; i++) if (qf.lookup(i)) found++;
        for (int i = 200; i < 400; i++) if (qf.lookup(i)) not_found++;
        cout << "  Lookup existing: " << found << "/200" << endl;
        cout << "  Lookup non-existing: " << not_found << "/200" << endl;
    }

    cout << "\n--- Cuckoo Filter ---" << endl;
    {
        ProbabilisticStructures::CuckooFilter cf(1024, 16);
        for (int i = 0; i < 500; i++) cf.insert(i);
        int found = 0, not_found = 0;
        for (int i = 0; i < 500; i++) if (cf.lookup(i)) found++;
        for (int i = 500; i < 1000; i++) if (cf.lookup(i)) not_found++;
        cout << "  Lookup existing: " << found << "/500" << endl;
        cout << "  Lookup non-existing: " << not_found << "/500" << endl;
    }

    return 0;
}
#endif

#endif // HASHING_D_CPP
