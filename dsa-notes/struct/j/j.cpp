#ifndef STRUCT_J_CPP
#define STRUCT_J_CPP

#include <list>
#include <queue>
#include <unordered_set>
#include <random>

// =============================================================
// X. СТРУКТУРЫ ДЛЯ КЭШИРОВАНИЯ
// =============================================================
// Структура md: A. Постановка задачи и теория
//               → B. LRU Cache
//               → C. LFU Cache
//               → D. ARC Cache
//               → E. LIRS Cache
//               → F. Другие политики и применения
//
// CachingStructures наследует SyntaxStructures (i.cpp). Ось X —
// ограниченное хранилище с политикой замены: модель кэша (A),
// оффлайн-оптимум Белади и конкурентный анализ (A), LRU (B),
// LFU (C), ARC (D), LIRS (E), прочие политики и применения (F).
//
// Переиспользование из базы:
//   * DList / DNode (I.B) — двусвязный список LRU (B.1) и
//     Write-Back (F.6): erase/push_back/pop_front — перенос и
//     вытеснение с концов за O(1);
//   * MemoizedFn (I.F) — мемоизация без ограничения памяти (F.7);
//   * knapsack_bitset / DynamicBitset (III.B) — достижимые суммы
//     размеров размерного кэша (F.5);
//   * хеш-таблицы (hashing.md) — pos: ключ → узел/итератор во
//     всех кэшах;
//   * Fenwick (V.B), кучи (IV) — упоминаются в md как альтернативы
//     (счётчики частот, heap+позиции); кода нет.
//
// Порядок структур строго соответствует порядку md (A → F).
// B.3 (k-LRU) — концепт, реализация — 2Q (F.3). Все политики —
// объекты с методом bool access(int x): обработать запрос, вернуть
// попадание; при промахе элемент загружается (при переполнении —
// вытесняется по правилу). Прогон потока — hit_count (A.1).
//
// ВНИМАНИЕ (скрытие имён): методы get, put, access, touch, halve,
// replace, size здесь локальные; одноимённые из других веток не
// подключаются.

#define STRUCT_I_MAIN
#include "../i/i.cpp"
#undef STRUCT_I_MAIN

struct CachingStructures : SyntaxStructures {

// =============================================================
// A. ПОСТАНОВКА ЗАДАЧИ И ТЕОРИЯ
// =============================================================

// --- A.1. Прогон потока через политику ---
// Политика — объект с методом bool access(int x). Возвращает число
// попаданий за поток. Любой класс-кэш раздела X — такая политика.
template <class Policy>
static long long hit_count(Policy& pol, const vector<int>& req) {
    long long h = 0;
    for (int x : req) if (pol.access(x)) ++h;
    return h;
}

// --- A.3. Belady (оффлайн-оптимум, MIN) ---
// Предвычисление nxt[i] — позиция следующего вхождения req[i] в
// потоке (обратный проход с картой последнего вхождения). fa[e] —
// текущее следующее вхождение элемента e (обновляется после каждого
// запроса). При промахе вытесняется элемент кэша с максимальным
// fa[e] — его следующий запрос дальше всего; не востребованный
// больше элемент (fa = n) вытесняется в первую очередь.
struct BeladyCache {
    int k, n;
    vector<int> nxt;
    unordered_map<int, int> fa;
    unordered_set<int> cache;
    int pos = 0;

    BeladyCache(int cap, const vector<int>& req) : k(cap), n((int)req.size()) {
        nxt.assign(n, n);
        unordered_map<int, int> last;
        for (int i = n - 1; i >= 0; --i) {
            auto it = last.find(req[i]);
            if (it != last.end()) nxt[i] = it->second;
            last[req[i]] = i;
        }
        unordered_set<int> seen;
        for (int i = 0; i < n; ++i) {
            if (!seen.count(req[i])) { fa[req[i]] = nxt[i]; seen.insert(req[i]); }
        }
    }

    bool access(int x) {
        int i = pos++;
        bool hit = cache.count(x) != 0;
        if (!hit) {
            if ((int)cache.size() < k) {
                cache.insert(x);
            } else {
                int ev = -1, far = -1;
                for (int e : cache) if (fa[e] > far) { far = fa[e]; ev = e; }
                cache.erase(ev);
                cache.insert(x);
            }
        }
        fa[x] = nxt[i];
        return hit;
    }
};

// --- A.4. Конструкции потоков для конкурентного анализа ---
// round_robin(k, cycles): k+1 различных элементов по кругу — худший
// случай для онлайн-политик: LRU/FIFO промахиваются на каждом запросе,
// оффлайн-оптимум держит (k−1)/k попаданий.
static vector<int> round_robin(int k, int cycles) {
    vector<int> seq;
    seq.reserve((size_t)(k + 1) * cycles);
    for (int c = 0; c < cycles; ++c)
        for (int x = 0; x <= k; ++x) seq.push_back(x);
    return seq;
}

// --- A.4. Равномерный случайный поток из U элементов ---
// Для |U| = k+1 доля попаданий ограничена k/(k+1) (A.4 в md).
static vector<int> uniform_stream(int u, int len, unsigned seed = 12345u) {
    mt19937 rng(seed);
    uniform_int_distribution<int> d(0, u - 1);
    vector<int> seq;
    seq.reserve(len);
    for (int i = 0; i < len; ++i) seq.push_back(d(rng));
    return seq;
}

// =============================================================
// B. LRU CACHE
// =============================================================

// --- B.1. LRU Cache: хеш-таблица + двусвязный список ---
// Список DList (I.B) хранит ключи (хвост — MRU); pos: ключ → узел;
// value: ключ → значение. Попадание — перенос узла в хвост
// (erase + push_back, обе O(1)); вытеснение — pop_front (голова =
// LRU). Единый интерфейс политик — access(x) (get по ключу).
struct LRUCache {
    int cap;
    DList list;
    unordered_map<int, DNode*> pos;
    unordered_map<int, int> value;

    explicit LRUCache(int c) : cap(c) {}

    bool get(int key, int& out) {
        auto it = pos.find(key);
        if (it == pos.end()) return false;
        touch(it->second);
        out = value[key];
        return true;
    }

    bool access(int x) {
        int v;
        if (get(x, v)) return true;
        put(x, 0);                  // загрузка при промахе
        return false;
    }

    void touch(DNode* u) {
        int v = u->val;
        list.erase(u);
        list.push_back(v);
        pos[v] = list.tail;
    }

    void put(int key, int v) {
        auto it = pos.find(key);
        if (it != pos.end()) { value[key] = v; touch(it->second); return; }
        if ((int)pos.size() == cap) {
            int old = 0;
            list.pop_front(old);
            pos.erase(old);
            value.erase(old);
        }
        list.push_back(key);
        pos[key] = list.tail;
        value[key] = v;
    }

    int size() const { return (int)pos.size(); }
};

// --- B.2. LRU с батчами ---
// Пачка обрабатывается как единое «время»: первое обращение к элементу
// в пачке переносит его в хвост (недавность обновляется раз на пачку),
// повторные — только попадание. Возвращает попадания пачки.
struct BatchLRU {
    int cap;
    DList list;
    unordered_map<int, DNode*> pos;

    explicit BatchLRU(int c) : cap(c) {}

    int batch(const vector<int>& req) {
        int hits = 0;
        unordered_set<int> seen;
        for (int x : req) {
            auto it = pos.find(x);
            if (it != pos.end()) {
                ++hits;
                if (!seen.count(x)) { seen.insert(x); touch(it->second); }
            } else {
        if ((int)pos.size() == cap) {
            int old = 0;
            list.pop_front(old);
            pos.erase(old);
        }
        list.push_back(x);
        pos[x] = list.tail;
            }
        }
        return hits;
    }

    void touch(DNode* u) {
        int v = u->val;
        list.erase(u);
        list.push_back(v);
        pos[v] = list.tail;
    }
};

// =============================================================
// C. LFU CACHE
// =============================================================

// --- C.1. LFU: бакеты частот ---
// buckets[f] — список ключей с частотой f (фронт — свежий, ти-брейк
// LRU внутри частоты); freq: ключ → частота; pos: ключ → итератор в
// бакете; min_f — минимальная непустая частота. Попадание: перенос в
// следующий бакет; промах: вытеснение хвоста buckets[min_f], вставка
// с частотой 1. Все операции — O(1) ожидаемо.
struct LFUCache {
    int cap;
    vector<list<int>> buckets;
    unordered_map<int, int> freq;
    unordered_map<int, list<int>::iterator> pos;
    int min_f = 0;

    explicit LFUCache(int c) : cap(c) { buckets.assign(2, list<int>()); }

    bool access(int x) {
        auto it = freq.find(x);
        if (it != freq.end()) {
            int f = it->second;
            buckets[f].erase(pos[x]);
            int nf = f + 1;
            if (nf == (int)buckets.size()) buckets.push_back(list<int>());
            buckets[nf].push_front(x);
            pos[x] = buckets[nf].begin();
            it->second = nf;
            if (f == min_f && buckets[f].empty()) ++min_f;
            return true;
        }
        if (cap == 0) return false;
        if ((int)freq.size() == cap) {
            int ev = buckets[min_f].back();
            buckets[min_f].pop_back();
            pos.erase(ev);
            freq.erase(ev);
            while (min_f < (int)buckets.size() && buckets[min_f].empty()) ++min_f;
        }
        freq[x] = 1;
        buckets[1].push_front(x);
        pos[x] = buckets[1].begin();
        min_f = 1;
        return false;
    }

    // --- C.2. Полуделение всех частот (устаревание) ---
    // Все freq /= 2 (минимум 1), бакеты перестраиваются. Вызывается
    // обёрткой LFUDecay раз в b обращений.
    void halve() {
        if (freq.empty()) return;
        vector<pair<int, int>> kv;
        kv.reserve(freq.size());
        int maxf = 0;
        for (auto& p : freq) {
            int nf = max(1, p.second >> 1);
            kv.push_back({p.first, nf});
            maxf = max(maxf, nf);
        }
        buckets.assign(maxf + 1, list<int>());
        pos.clear();
        min_f = maxf;
        for (auto& p : kv) {
            int k = p.first, f = p.second;
            buckets[f].push_front(k);
            pos[k] = buckets[f].begin();
            freq[k] = f;
            min_f = min(min_f, f);
        }
    }

    int size() const { return (int)freq.size(); }
};

// --- C.2. LFU с устареванием частот ---
// Раз в b обращений вызывает halve: старые частоты затухают, «горячий
// сейчас» элемент копит частоту быстрее, чем «горячий в прошлом».
struct LFUDecay {
    LFUCache base;
    int bound;
    int acc = 0;

    LFUDecay(int c, int b) : base(c), bound(b) {}

    bool access(int x) {
        if (++acc >= bound) { base.halve(); acc = 0; }
        return base.access(x);
    }
};

// =============================================================
// D. ARC CACHE
// =============================================================

// --- D. ARC: Adaptive Replacement Cache ---
// Четыре LRU-списка (фронт = MRU): T1 — недавно «одно обращение»,
// T2 — недавно «≥ 2 обращений», B1/B2 — история (ключи без данных).
// Инвариант |T1| + |T2| = k; адаптивный параметр p — целевой размер
// T1 (p = 0 — LFU-подобный режим, p = k — LRU-подобный). access:
// попадания T1 → T2, T2 → хвост T2; попадания B1/B2 адаптируют p и
// возвращают элемент в T2 через REPLACE; промах — удаление из B1/T1
// или B2 (по размеру L1 = T1+B1) и вставка в T1.
struct ARCCache {
    int c, p = 0;
    list<int> T1, T2, B1, B2;
    unordered_map<int, int> type;              // 0 нет, 1 T1, 2 T2, 3 B1, 4 B2
    unordered_map<int, list<int>::iterator> pos;
    int s1 = 0, s2 = 0, b1 = 0, b2 = 0;

    explicit ARCCache(int cap) : c(cap) {}

    bool access(int x) {
        int t = type[x];
        if (t == 1) {
            T1.erase(pos[x]); --s1;
            T2.push_front(x); pos[x] = T2.begin(); ++s2;
            type[x] = 2;
            return true;
        }
        if (t == 2) {
            T2.erase(pos[x]);
            T2.push_front(x); pos[x] = T2.begin();
            return true;
        }
        if (t == 3) {
            p = min(c, p + max(1, b2 / max(1, b1)));
            replace(x);
            B1.erase(pos[x]); --b1;
            T2.push_front(x); pos[x] = T2.begin(); ++s2;
            type[x] = 2;
            return false;
        }
        if (t == 4) {
            p = max(0, p - max(1, b1 / max(1, b2)));
            replace(x);
            B2.erase(pos[x]); --b2;
            T2.push_front(x); pos[x] = T2.begin(); ++s2;
            type[x] = 2;
            return false;
        }
        if (c == 0) return false;
        if (s1 + s2 == c) {
            if (s1 < c) {
                if (b1) { B1.pop_back(); --b1; }
                replace(x);
            } else {
                int ev = T1.back(); T1.pop_back(); --s1;
                type[ev] = 0; pos.erase(ev);
            }
        } else if (b1 + s1 + b2 + s2 >= c) {
            if (b1 + s1 + b2 + s2 == 2 * c) { B2.pop_back(); --b2; }
            replace(x);
        }
        T1.push_front(x); pos[x] = T1.begin(); ++s1;
        type[x] = 1;
        return false;
    }

    // --- D.2. REPLACE(x): освобождение места под x ---
    // Если |T1| > p (или |T1| = p и x ∈ B2) — вытеснить LRU из T1 в
    // B1, иначе LRU из T2 в B2. Жертва уходит в историю; переполнение
    // истории (b1 == c / b2 == c) вытесняет LRU истории.
    void replace(int x) {
        if (s1 >= 1 && (s1 > p || (s1 == p && type[x] == 4))) {
            int ev = T1.back(); T1.pop_back(); --s1;
            if (b1 == c) { B1.pop_back(); --b1; }
            B1.push_front(ev); ++b1;
            pos[ev] = B1.begin(); type[ev] = 3;
        } else {
            int ev = T2.back(); T2.pop_back(); --s2;
            if (b2 == c) { B2.pop_back(); --b2; }
            B2.push_front(ev); ++b2;
            pos[ev] = B2.begin(); type[ev] = 4;
        }
    }
};

// =============================================================
// E. LIRS CACHE
// =============================================================

// --- E. LIRS: Low Inter-Reference Recency ---
// LIR-блоки (долгожители, лимит L_LIR) и HIR-блоки (одноразовые,
// лимит резидентных L_HIR). Стек S (верх = MRU) хранит все LIR,
// резидентные HIR и нерезидентную историю HIR (лимит SL). Второе
// обращение к резидентному HIR повышает его до LIR и понижает нижний
// LIR стека до HIR; переполнение L_HIR вытесняет нижний резидентный
// HIR (он остаётся в стеке как история до trim_stack).
struct LIRSCache {
    enum Type { LIR, HIR };
    int L_LIR, L_HIR, SL;
    list<int> S;
    unordered_map<int, list<int>::iterator> pos;
    unordered_map<int, Type> type;
    unordered_map<int, bool> resident;
    int n_lir = 0, n_hir_res = 0;

    explicit LIRSCache(int cap, int lir_permille = 900) {
        L_HIR = max(1, cap * (1000 - lir_permille) / 1000);
        L_LIR = max(0, cap - L_HIR);
        SL = 2 * cap + 2;
    }

    bool access(int x) {
        auto it = pos.find(x);
        if (it != pos.end()) {
            if (type[x] == LIR) {
                move_to_top(x);
                return true;
            }
            if (resident[x]) {
                move_to_top(x);
                promote_to_lir(x);
                return true;
            }
            return false;                // нерезидентная история HIR — промах
        }
        load_miss(x);
        return false;
    }

    void move_to_top(int x) {
        S.erase(pos[x]);
        S.push_front(x);
        pos[x] = S.begin();
    }

    // --- E.1. Повышение HIR → LIR; при переполнении L_LIR — демоция ---
    void promote_to_lir(int x) {
        type[x] = LIR;
        --n_hir_res;
        ++n_lir;
        if (n_lir > L_LIR) {
            int key;
            if (bottom_scan([&](int k) { return type[k] == LIR; }, key)) demote(key);
        }
        if (n_hir_res > L_HIR) {
            int key;
            if (bottom_scan([&](int k) { return type[k] == HIR && resident[k]; }, key)) evict_hir(key);
        }
        trim_stack();
    }

    void demote(int x) {
        type[x] = HIR;              // остаётся резидентным
        --n_lir;
        ++n_hir_res;
    }

    void evict_hir(int x) {
        resident[x] = false;        // остаётся в стеке как история
        --n_hir_res;
    }

    void load_miss(int x) {
        S.push_front(x);
        pos[x] = S.begin();
        type[x] = HIR;
        resident[x] = true;
        ++n_hir_res;
        if (n_hir_res > L_HIR) {
            int key;
            if (bottom_scan([&](int k) { return type[k] == HIR && resident[k]; }, key)) evict_hir(key);
        }
        trim_stack();
    }

    // --- E.2. Удаление нерезидентной истории со дна стека (лимит SL) ---
    void trim_stack() {
        int key;
        while ((int)S.size() > SL &&
               bottom_scan([&](int k) { return type[k] == HIR && !resident[k]; }, key)) {
            S.erase(pos[key]);
            pos.erase(key);
        }
    }

    // Поиск снизу стека первого ключа, удовлетворяющего предикату.
    template <class Pred>
    bool bottom_scan(Pred pred, int& out) {
        if (S.empty()) return false;
        auto it = S.end();
        do {
            --it;
            if (pred(*it)) { out = *it; return true; }
        } while (it != S.begin());
        return false;
    }
};

// =============================================================
// F. ДРУГИЕ ПОЛИТИКИ И ПРИМЕНЕНИЯ
// =============================================================

// --- F.1. FIFO: вытеснение по времени прихода ---
struct FIFOCache {
    int cap;
    queue<int> q;
    unordered_set<int> in;

    explicit FIFOCache(int c) : cap(c) {}

    bool access(int x) {
        if (in.count(x)) return true;
        if ((int)in.size() == cap) {
            int ev = q.front(); q.pop();
            in.erase(ev);
        }
        q.push(x);
        in.insert(x);
        return false;
    }
};

// --- F.1. MRU: вытеснение самого свежего ---
struct MRUCache {
    int cap;
    list<int> recent;
    unordered_map<int, list<int>::iterator> pos;

    explicit MRUCache(int c) : cap(c) {}

    bool access(int x) {
        auto it = pos.find(x);
        if (it != pos.end()) {
            recent.erase(it->second);
            recent.push_front(x);
            pos[x] = recent.begin();
            return true;
        }
        if ((int)pos.size() == cap) {
            int ev = recent.front();       // MRU вытесняется первым
            recent.pop_front();
            pos.erase(ev);
        }
        recent.push_front(x);
        pos[x] = recent.begin();
        return false;
    }
};

// --- F.2. Clock / Second-Chance: кольцо с битом обращения ---
// Слот: ключ + бит ref. Попадание — установка бита. Промах: рука
// движется по кругу, сбрасывая биты 1 → 0 (второй шанс), и вытесняет
// слот с битом 0 (или пустой слот, пока кэш не заполнен).
struct ClockCache {
    int cap;
    vector<int> key;
    vector<char> ref;
    int hand = 0;
    unordered_map<int, int> slot;

    explicit ClockCache(int c) : cap(c), key(c, -1), ref(c, 0) {}

    bool access(int x) {
        auto it = slot.find(x);
        if (it != slot.end()) { ref[it->second] = 1; return true; }
        if (cap == 0) return false;
        if ((int)slot.size() == cap) {
            while (ref[hand]) { ref[hand] = 0; hand = (hand + 1) % cap; }
            slot.erase(key[hand]);
        } else {
            while (key[hand] != -1) hand = (hand + 1) % cap;
        }
        key[hand] = x;
        ref[hand] = 1;
        slot[x] = hand;
        hand = (hand + 1) % cap;
        return false;
    }
};

// --- F.3. 2Q: A1in (карантин) / A1out (история) / Am (долгожители) ---
// Первый запрос → A1in; повторный из A1in или A1out → Am (MRU);
// попадание — из Am и A1in. Одноразовые застревают в A1in и
// вытесняются, частые получают «пропуск» в Am через историю.
struct TwoQueueCache {
    int kin, kout, km;
    list<int> Ain, Aout, Am;
    unordered_map<int, list<int>::iterator> pos;
    unordered_map<int, int> where;           // 1 Ain, 2 Aout, 3 Am

    TwoQueueCache(int kin_, int kout_, int km_)
        : kin(kin_), kout(kout_), km(km_) {}

    bool access(int x) {
        int w = where[x];
        if (w == 3) {
            Am.erase(pos[x]);
            Am.push_front(x); pos[x] = Am.begin();
            return true;
        }
        if (w == 1) {
            Ain.erase(pos[x]);
            to_am(x);
            return true;
        }
        if (w == 2) {
            Aout.erase(pos[x]);
            to_am(x);
            return false;                    // история — промах по данным
        }
        if ((int)Ain.size() == kin) {
            int ev = Ain.back(); Ain.pop_back();
            if ((int)Aout.size() == kout) Aout.pop_back();
            Aout.push_front(ev); pos[ev] = Aout.begin(); where[ev] = 2;
        }
        Ain.push_front(x); pos[x] = Ain.begin(); where[x] = 1;
        return false;
    }

    void to_am(int x) {
        if ((int)Am.size() == km) {
            int ev = Am.back(); Am.pop_back();
            where[ev] = 0; pos.erase(ev);
        }
        Am.push_front(x); pos[x] = Am.begin(); where[x] = 3;
    }
};

// --- F.4. TTL-кэш: срок жизни записи ---
// Время — счётчик обращений. Запись с deadline ≤ now при доступе
// удаляется (ленивая инвалидация) и считается промахом.
struct TTLCache {
    int cap;
    long long ttl;
    long long now = 0;
    struct Entry { int key; int value; long long deadline; };
    list<Entry> entries;
    unordered_map<int, list<Entry>::iterator> pos;

    TTLCache(int c, long long t) : cap(c), ttl(t) {}

    bool get(int key, int& out) {
        ++now;
        auto it = pos.find(key);
        if (it == pos.end()) return false;
        if (it->second->deadline <= now) {
            entries.erase(it->second);
            pos.erase(key);
            return false;
        }
        out = it->second->value;
        return true;
    }

    bool access(int x) {
        int v;
        if (get(x, v)) return true;
        put(x, 0);                  // загрузка при промахе
        return false;
    }

    void put(int key, int value_) {
        auto it = pos.find(key);
        if (it != pos.end()) {
            it->second->value = value_;
            it->second->deadline = now + ttl;
            return;
        }
        if ((int)pos.size() == cap) {
            int ev = entries.back().key;
            entries.pop_back();
            pos.erase(ev);
        }
        entries.push_front({key, value_, now + ttl});
        pos[key] = entries.begin();
    }
};

// --- F.5. Размерные кэши: вместимость против ценности ---
// Элементы с размерами и ценностями; кэш — вместимость S. Online:
// при переполнении вытесняется элемент с минимальной плотностью
// ценности (ценность/размер). Offline: достижимые суммы размеров —
// knapsack_bitset (III.B).
struct SizeCache {
    long long S;
    long long used = 0;
    list<pair<int, long long>> items;
    unordered_map<int, list<pair<int, long long>>::iterator> pos;
    unordered_map<int, long long> size_of;
    unordered_map<int, long long> value_of;

    explicit SizeCache(long long cap) : S(cap) {}

    void put(int key, long long sz, long long val) {
        size_of[key] = sz;
        value_of[key] = val;
    }

    bool access(int x) {
        if (pos.count(x)) return true;
        long long sz = size_of[x];
        if (sz > S) return false;                       // не помещается вовсе
        while (used + sz > S) {
            auto best = evict_min_density();
            if (best == items.end()) break;
            used -= best->second;
            pos.erase(best->first);
            items.erase(best);
        }
        if (used + sz > S) return false;                // не смогли освободить
        items.push_front({x, sz});
        pos[x] = items.begin();
        used += sz;
        return false;
    }

    list<pair<int, long long>>::iterator evict_min_density() {
        auto best = items.end();
        double best_ratio = 1e18;
        for (auto it = items.begin(); it != items.end(); ++it) {
            double r = (double)value_of[it->first] / max(1LL, it->second);
            if (r < best_ratio) { best_ratio = r; best = it; }
        }
        return best;
    }

    // --- Offline: наибольшая достижимая сумма размеров ≤ S ---
    // Переиспользуется knapsack_bitset (III.B): битсет достижимых сумм.
    static int max_reachable(const vector<int>& sizes, int S_) {
        DynamicBitset dp = knapsack_bitset(sizes, S_);
        int best = 0;
        for (int s = 0; s <= S_; ++s) if (dp.test(s)) best = s;
        return best;
    }
};

// --- F.6. Write-back / write-through ---
// Поверх LRU (B.1) с битом «грязно»: write-through — каждый put пишет
// в «медленную память»; write-back — только при вытеснении грязной
// записи. Метрика — writes_backend (число обращений к памяти).
struct WriteBackCache {
    int cap;
    DList list;
    unordered_map<int, DNode*> pos;
    unordered_map<int, int> value;
    unordered_map<int, bool> dirty;
    long long writes_backend = 0;

    explicit WriteBackCache(int c) : cap(c) {}

    bool get(int key, int& out) {
        auto it = pos.find(key);
        if (it == pos.end()) return false;
        out = value[key];
        touch(it->second);
        return true;
    }

    void touch(DNode* u) {
        int v = u->val;
        list.erase(u);
        list.push_back(v);
        pos[v] = list.tail;
    }

    void put(int key, int v, bool through) {
        auto it = pos.find(key);
        if (it != pos.end()) {
            value[key] = v;
            if (through) { ++writes_backend; dirty[key] = false; }
            else dirty[key] = true;
            touch(it->second);
            return;
        }
        if ((int)pos.size() == cap) {
            int ev = 0;
            list.pop_front(ev);
            if (dirty[ev]) ++writes_backend;            // сброс грязной записи
            dirty.erase(ev);
            pos.erase(ev);
            value.erase(ev);
        }
        list.push_back(key);
        pos[key] = list.tail;
        value[key] = v;
        if (through) { ++writes_backend; dirty[key] = false; }
        else dirty[key] = true;
    }
};

// --- F.7. Мемоизация с ограниченной памятью (LRU-вытеснение) ---
// MemoizedFn (I.F) кэширует без ограничений; этот вариант — поверх
// LRUCache (B.1): кэш фиксированного размера, память не растёт.
struct MemoLRU {
    LRUCache cache;
    std::function<long long(long long)> f;

    MemoLRU(int cap, std::function<long long(long long)> func)
        : cache(cap), f(func) {}

    long long operator()(long long x) {
        int v;
        if (cache.get((int)x, v)) return v;
        long long res = f(x);
        cache.put((int)x, (int)res);
        return res;
    }

    int size() const { return cache.size(); }
};

};

#ifndef STRUCT_J_MAIN
#define STRUCT_J_MAIN

int main() {
    using H = CachingStructures;
    cout << "=== X. СТРУКТУРЫ ДЛЯ КЭШИРОВАНИЯ ===" << endl;

    // ---------- Мосты из I: DList (I.B), MemoizedFn (I.F), knapsack_bitset (III.B) ----------
    {
        H::DList dl;
        dl.push_back(1); dl.push_back(2); dl.push_back(3);
        int v = 0;
        dl.pop_front(v);
        cout << "I.B DList pop_front = " << v << " (ожидаем 1)" << endl;
    }

    // ---------- A.1 Модель: прогон потока через политику ----------
    {
        H::LRUCache lru(2);
        long long h = H::hit_count(lru, {1, 2, 1, 3, 1, 2, 3});
        cout << "A.1 LRU hits on {1,2,1,3,1,2,3} = " << h << " (ожидаем 2)" << endl;
    }

    // ---------- A.3 Belady ----------
    {
        H::BeladyCache b(2, {1, 2, 1, 3, 1, 2, 3});
        long long h = H::hit_count(b, {1, 2, 1, 3, 1, 2, 3});
        cout << "A.3 Belady hits on {1,2,1,3,1,2,3} = " << h << " (ожидаем 3)" << endl;
    }

    // ---------- A.4 Конкурентный анализ ----------
    {
        int k = 2, cycles = 6;
        vector<int> rr = H::round_robin(k, cycles);
        H::BeladyCache bel(k, rr);
        H::LRUCache lru(k);
        H::FIFOCache fifo(k);
        long long hb = H::hit_count(bel, rr);
        long long hl = H::hit_count(lru, rr);
        long long hf = H::hit_count(fifo, rr);
        cout << "A.4 round_robin(2,6) len = " << rr.size()
             << ", Belady hits = " << hb << " (ожидаем 8), LRU = " << hl
             << ", FIFO = " << hf << " (ожидаем 0/0)" << endl;
        cout << "A.4 Belady hit rate = " << hb << "/" << rr.size()
             << " = " << (double)hb / rr.size()
             << " (ожидаем 1/2 = (k-1)/k при k=2)" << endl;

        vector<int> uni = H::uniform_stream(3, 12);
        H::LRUCache lru2(2);
        long long hu = H::hit_count(lru2, uni);
        cout << "A.4 uniform over U=k+1=3: LRU hits = " << hu << "/12"
             << " (ожидаем ~8: доля k/(k+1)=2/3)" << endl;
    }

    // ---------- B.1 LRU Cache ----------
    {
        H::LRUCache c(2);
        int v = 0;
        c.put(1, 10);
        c.put(2, 20);
        cout << "B.1 get(1) = " << c.get(1, v) << " (ожидаем 1)" << endl;
        c.put(3, 30);                        // вытеснит 2 (LRU)
        cout << "B.1 get(2) after put(3) = " << c.get(2, v)
             << " (ожидаем 0: 2 вытеснен как LRU)" << endl;
        int g1 = c.get(1, v), g3 = c.get(3, v);
        cout << "B.1 get(1)/get(3) = " << g1 << "/" << g3
             << " (ожидаем 1/1), value(1) = " << v << " (ожидаем 30)" << endl;
    }

    // ---------- B.2 LRU с батчами ----------
    {
        H::BatchLRU c(2);
        int h1 = c.batch({1, 2, 3, 1});      // пачка: 1,2 грузятся, 3 вытесняет 1, 1 промах
        cout << "B.2 batch1 hits = " << h1 << " (ожидаем 0)" << endl;
        int h2 = c.batch({1, 2, 3, 1, 2});   // 1,2 уже «недавние» из прошлой пачки
        cout << "B.2 batch2 hits = " << h2 << " (ожидаем 1)" << endl;
    }

    // ---------- C.1 LFU ----------
    {
        H::LFUCache c(2);
        long long h = H::hit_count(c, {1, 1, 2, 3, 1, 1});
        cout << "C.1 LFU hits on {1,1,2,3,1,1} = " << h << " (ожидаем 3)" << endl;
    }

    // ---------- C.2 LFU с устареванием ----------
    {
        // 1 стал горячим в начале и без устаревания навсегда занимает
        // слот (LFU-голодание): 2 и 3 вытесняют друг друга, попаданий
        // нет. Полуделение частот раз в 5 обращений сбрасывает счётчик
        // 1, при приходе 2 он — старейший среди равных частот и уходит,
        // после чего 2 и 3 сосуществуют и дают почти все попадания.
        vector<int> seq = {1, 1, 1, 1, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3};
        H::LFUCache plain(2);
        H::LFUDecay aged(2, 5);
        long long hp = H::hit_count(plain, seq);
        long long ha = H::hit_count(aged, seq);
        cout << "C.2 phase-change: LFU = " << hp << ", LFU+decay = " << ha
             << " (ожидаем 4 < 13: устаревание ловит попадания в новой фазе)" << endl;
    }

    // ---------- D ARC ----------
    {
        H::ARCCache c(2);
        long long h = H::hit_count(c, {1, 2, 1, 3, 2, 1, 3});
        cout << "D ARC hits on {1,2,1,3,2,1,3} = " << h << " (ожидаем 1)" << endl;

        H::ARCCache c2(2);
        long long h2 = H::hit_count(c2, {1, 2, 1, 3, 1, 2, 3});
        cout << "D ARC hits on {1,2,1,3,1,2,3} = " << h2
             << " (ожидаем 3, как оффлайн-оптимум Belady)" << endl;
    }

    // ---------- E LIRS ----------
    {
        // Горячий элемент a + скан 1..6 + снова a: LIRS сохраняет a.
        vector<int> seq = {1, 1, 2, 3, 4, 5, 6, 1, 1, 1};
        H::LIRSCache lirs(2);
        H::LRUCache lru(2);
        long long hl = H::hit_count(lirs, seq);
        long long hr = H::hit_count(lru, seq);
        cout << "E.2 scan-resilience: LIRS = " << hl << ", LRU = " << hr
             << " (ожидаем LIRS > LRU: 4 > 3)" << endl;

        H::LIRSCache lirs2(2);
        long long h2 = H::hit_count(lirs2, {1, 1, 2, 3, 2, 1, 3});
        cout << "E.1 LIRS hits on {1,1,2,3,2,1,3} = " << h2 << " (ожидаем 3)" << endl;
    }

    // ---------- F.1 FIFO / MRU ----------
    {
        H::FIFOCache fifo(2);
        long long hf = H::hit_count(fifo, {1, 2, 3, 1, 2, 3});
        cout << "F.1 FIFO hits on {1,2,3,1,2,3} = " << hf << " (ожидаем 0)" << endl;
        H::MRUCache mru(2);
        long long hm = H::hit_count(mru, {1, 2, 3, 1, 2, 3});
        cout << "F.1 MRU hits on {1,2,3,1,2,3} = " << hm
             << " (ожидаем 2: MRU вытесняет самый свежий — контраст с LRU=0)" << endl;
    }

    // ---------- F.2 Clock ----------
    {
        H::ClockCache c(2);
        long long h = H::hit_count(c, {1, 2, 1, 3, 1, 2, 3});
        cout << "F.2 Clock hits on {1,2,1,3,1,2,3} = " << h
             << " (ожидаем 1: бит обращения даёт второй шанс 1 и 2)" << endl;
    }

    // ---------- F.3 2Q ----------
    {
        H::TwoQueueCache c(2, 2, 2);
        long long h = H::hit_count(c, {1, 2, 1, 3, 1, 2, 3});
        cout << "F.3 2Q hits on {1,2,1,3,1,2,3} = " << h
             << " (ожидаем 4: повторные из Ain переходят в Am)" << endl;
    }

    // ---------- F.4 TTL ----------
    {
        H::TTLCache c(1, 2);
        int v = 0;
        c.put(1, 10);
        bool h1 = c.get(1, v);      // now=1 < deadline=2 → попадание
        bool h2 = c.get(1, v);      // now=2, срок истёк → промах
        cout << "F.4 TTL get(1) = " << h1 << " (ожидаем 1)" << endl;
        cout << "F.4 TTL get(1) после истечения = " << h2 << " (ожидаем 0)" << endl;
    }

    // ---------- F.5 Размерные кэши ----------
    {
        H::SizeCache c(5);
        c.put(1, 2, 8);
        c.put(2, 2, 6);
        c.put(3, 3, 100);
        bool a1 = c.access(1);
        bool a2 = c.access(2);
        bool a3 = c.access(3);      // 3 > свободно 0: вытесняется мин. плотность (2: 6/2=3 < 1: 8/2=4)
        bool a1b = c.access(1);     // 1 уцелел — попадание
        cout << "F.5 SizeCache: 1->" << a1 << ", 2->" << a2 << ", 3->" << a3
             << ", 1->" << a1b << " (ожидаем 0 0 0 1)" << endl;

        cout << "F.5 knapsack max_reachable({2,3,5}, 11) = "
             << H::SizeCache::max_reachable({2, 3, 5}, 11)
             << " (ожидаем 10: 0/1-рюкзак, 2+3+5=10 ≤ 11)" << endl;
    }

    // ---------- F.6 Write-back / write-through ----------
    {
        H::WriteBackCache wt(2), wb(2);
        for (int k = 1; k <= 4; ++k) {
            wt.put(k, k * 10, true);     // write-through: каждая запись — в память
            wb.put(k, k * 10, false);    // write-back: в память только при вытеснении грязной
        }
        cout << "F.6 write-through backend writes = " << wt.writes_backend
             << " (ожидаем 4)" << endl;
        cout << "F.6 write-back backend writes = " << wb.writes_backend
             << " (ожидаем 2: при put 3 вытесняется грязная 1, при put 4 — грязная 2)" << endl;
    }

    // ---------- F.7 Мемоизация ----------
    {
        std::function<long long(long long)> fib;
        H::MemoLRU memo(8, [&](long long n) {
            return n <= 1 ? n : fib(n - 1) + fib(n - 2);
        });
        fib = [&](long long n) { return memo(n); };
        long long f = fib(30);
        cout << "F.7 memo fib(30) = " << f << ", cache size = " << memo.size()
             << " (ожидаем 832040 / 8)" << endl;

        H::MemoizedFn mf([](int x) { return x * x; });
        int r1 = mf(5), r2 = mf(5), r3 = mf(7);
        cout << "F.7 MemoizedFn (I.F) 5*5 twice = " << r1 << "/" << r2
             << ", 7*7 = " << r3 << ", cache size = " << mf.cacheSize()
             << " (ожидаем 25/25/49/2)" << endl;
    }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_J_MAIN

#endif // STRUCT_J_CPP
