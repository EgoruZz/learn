#ifndef STRUCT_H_CPP
#define STRUCT_H_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <climits>
#include <cmath>
#include <random>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <deque>
#include <array>
#include <set>
#include <cstdint>
using namespace std;

// =============================================================
// VIII. КОНКУРЕНТНЫЕ (ПАРАЛЛЕЛЬНЫЕ) СТРУКТУРЫ
// =============================================================
// Структура md: A. Lock-based
//               → B. Lock-free
//               → C. Wait-free
//               → D. Concurrent Containers
//               → E. Специальные примитивы
//
// ConcurrentStructures наследует PersistentStructures (g.cpp).
// Идея уровней Skip List (hashing.md IV) — основа LockFreeSkipList
// (E.3); mt19937 — приоритеты уровней (E.3); приёмы вероятностных
// структур — hashing.md IV.
//
// Примитивы — стандартная библиотека (C++11/17): mutex,
// condition_variable, atomic, thread.
//
// ВНИМАНИЕ (скрытие имён): push, pop, insert, erase, search,
// contains, update здесь локальные; одноимённые из других веток
// не подключаются. Узлы lock-free структур не освобождаются
// (reclamation — B.5, E.2): в демо/тестах узлы живут до конца.

#define STRUCT_G_MAIN
#include "../g/g.cpp"
#undef STRUCT_G_MAIN

struct ConcurrentStructures : PersistentStructures {

// =============================================================
// A. LOCK-BASED СТРУКТУРЫ
// =============================================================

// --- A.1. Mutex-protected Queue / Stack ---
// Контейнер + std::mutex: каждая операция — короткая критическая
// секция; try_pop возвращает false на пустой структуре (неблокирующий).
struct MutexQueue {
    deque<int> q;
    mutable mutex m;
    void push(int x) { lock_guard<mutex> l(m); q.push_back(x); }
    bool try_pop(int& out) {
        lock_guard<mutex> l(m);
        if (q.empty()) return false;
        out = q.front();
        q.pop_front();
        return true;
    }
    size_t size() const { lock_guard<mutex> l(m); return q.size(); }
};
struct MutexStack {
    vector<int> s;
    mutable mutex m;
    void push(int x) { lock_guard<mutex> l(m); s.push_back(x); }
    bool try_pop(int& out) {
        lock_guard<mutex> l(m);
        if (s.empty()) return false;
        out = s.back();
        s.pop_back();
        return true;
    }
    size_t size() const { lock_guard<mutex> l(m); return s.size(); }
};

// --- A.2. Reader-Writer Lock ---
// Разделяемый доступ: много читателей или один писатель; писатели
// приоритетнее (waiting_writers блокирует новых читателей) — защита
// от голодания. Реализация — mutex + две condition_variable.
struct RWLock {
    mutable mutex m;
    condition_variable cv_r, cv_w;
    int readers = 0, writers = 0, waiting_writers = 0;

    void read_lock() {
        unique_lock<mutex> l(m);
        cv_r.wait(l, [&] { return writers == 0 && waiting_writers == 0; });
        ++readers;
    }
    void read_unlock() {
        unique_lock<mutex> l(m);
        if (--readers == 0) cv_w.notify_one();
    }
    void write_lock() {
        unique_lock<mutex> l(m);
        ++waiting_writers;
        cv_w.wait(l, [&] { return writers == 0 && readers == 0; });
        --waiting_writers;
        ++writers;
    }
    void write_unlock() {
        unique_lock<mutex> l(m);
        --writers;
        if (waiting_writers) cv_w.notify_one();
        else cv_r.notify_all();
    }
};

// --- A.3. Condition Variable очередь (блокирующая) ---
// pop ждёт, пока очередь непуста (wait в цикле — спуриусные
// пробуждения); push уведомляет notify_one; close — завершение.
struct ConditionQueue {
    deque<int> q;
    mutable mutex m;
    condition_variable cv;
    bool closed = false;

    void push(int x) {
        unique_lock<mutex> l(m);
        q.push_back(x);
        cv.notify_one();
    }
    bool pop(int& out) {  // блокирующий; false — очередь закрыта и пуста
        unique_lock<mutex> l(m);
        cv.wait(l, [&] { return !q.empty() || closed; });
        if (q.empty()) return false;
        out = q.front();
        q.pop_front();
        return true;
    }
    void close() {
        unique_lock<mutex> l(m);
        closed = true;
        cv.notify_all();
    }
};

// --- A.4. Спинлок (atomic_flag + backoff) ---
// Активное ожидание с экспоненциальным backoff (yield между
// попытками) — дешевле сна при коротких критических секциях.
struct SpinLock {
    atomic_flag flag = ATOMIC_FLAG_INIT;
    static const int MAX_BACKOFF = 1024;

    void lock() {
        int backoff = 1;
        while (flag.test_and_set(memory_order_acquire)) {
            for (int i = 0; i < backoff; ++i) this_thread::yield();
            backoff = min(backoff * 2, MAX_BACKOFF);
        }
    }
    void unlock() { flag.clear(memory_order_release); }
};

// =============================================================
// B. LOCK-FREE СТРУКТУРЫ
// =============================================================

// --- B.1. Lock-free Stack (Treiber) ---
// Атомарная вершина + CAS-циклы: push — CAS(top, t, n), pop —
// CAS(top, t, t->next); узлы не освобождаются (B.5, E.2).
struct TreiberStack {
    struct Node { int val; Node* next; };
    atomic<Node*> top{nullptr};

    void push(int x) {
        Node* n = new Node{x, nullptr};
        Node* t = top.load(memory_order_relaxed);
        do { n->next = t; } while (!top.compare_exchange_weak(t, n, memory_order_release, memory_order_relaxed));
    }
    bool try_pop(int& out) {
        Node* t = top.load(memory_order_relaxed);
        while (t) {
            if (top.compare_exchange_weak(t, t->next, memory_order_acquire, memory_order_relaxed)) {
                out = t->val;
                return true;
            }
        }
        return false;
    }
};

// --- B.2. Lock-free Queue (Michael-Scott) ---
// head — фиктивный узел, tail — последний; enqueue — CAS на
// t->next (допомогающий: при отставании tail продвигается),
// dequeue — CAS на head.
struct MichaelScottQueue {
    struct Node { int val; atomic<Node*> next; };
    atomic<Node*> head, tail;

    MichaelScottQueue() {
        Node* d = new Node{0, nullptr};
        head.store(d);
        tail.store(d);
    }
    void push(int x) {
        Node* n = new Node{x, nullptr};
        while (true) {
            Node* t = tail.load(memory_order_relaxed);
            Node* next = t->next.load(memory_order_relaxed);
            if (next == nullptr) {
                if (t->next.compare_exchange_weak(next, n, memory_order_release, memory_order_relaxed)) {
                    tail.compare_exchange_weak(t, n, memory_order_release, memory_order_relaxed);
                    return;
                }
            } else {
                tail.compare_exchange_weak(t, next, memory_order_release, memory_order_relaxed);
            }
        }
    }
    bool try_pop(int& out) {
        while (true) {
            Node* h = head.load(memory_order_relaxed);
            Node* next = h->next.load(memory_order_acquire);
            if (next == nullptr) return false;
            if (head.compare_exchange_weak(h, next, memory_order_release, memory_order_relaxed)) {
                out = next->val;
                return true;
            }
        }
    }
};

// --- B.3. Lock-free Deque (ограниченный кольцевой MPMC-буфер) ---
// Слоты с монотонными последовательностями (Vyukov-рукопожатие):
// позиция p пишется, когда seq[p % cap] == p (слот свободен), и
// публикуется seq = p + 1 (данные готовы); pop_front ждёт seq == h+1
// и освобождает слот seq = h + 1 + cap. Позиции строго возрастают —
// дисциплины по отдельности: очередь (push_back/pop_front, MPMC)
// и стек (push_back/pop_back, одиночный доступ к концам); оба конца
// одновременно — сложные алгоритмы (анализ в md).
struct LockFreeDeque {
    struct Slot { atomic<long long> data; atomic<unsigned long long> seq; };
    int cap;
    vector<Slot> slots;
    atomic<unsigned long long> head{0}, tail{0};

    LockFreeDeque(int cap_) : cap(cap_), slots(cap_) {
        for (int i = 0; i < cap; ++i) slots[i].seq.store((unsigned long long)i, memory_order_relaxed);
    }
    bool push_back(int x) {
        while (true) {
            unsigned long long t = tail.load(memory_order_acquire);
            unsigned long long h = head.load(memory_order_acquire);
            if (t - h >= (unsigned long long)cap) return false;  // полная
            if (slots[t % cap].seq.load(memory_order_acquire) != t) return false;  // слот занят
            if (!tail.compare_exchange_weak(t, t + 1, memory_order_release, memory_order_relaxed)) continue;
            slots[t % cap].data.store(x, memory_order_release);
            slots[t % cap].seq.store(t + 1, memory_order_release);
            return true;
        }
    }
    bool pop_front(int& out) {
        while (true) {
            unsigned long long h = head.load(memory_order_acquire);
            if (tail.load(memory_order_acquire) == h) return false;  // пустая
            if (slots[h % cap].seq.load(memory_order_acquire) != h + 1) return false;  // ещё не готово
            if (!head.compare_exchange_weak(h, h + 1, memory_order_release, memory_order_relaxed)) continue;
            out = slots[h % cap].data.load(memory_order_acquire);
            slots[h % cap].seq.store(h + cap, memory_order_release);  // свободен для позиции h + cap
            return true;
        }
    }
    bool pop_back(int& out) {  // стек-дисциплина: без повторного использования позиций
        unsigned long long t = tail.load(memory_order_acquire);
        unsigned long long h = head.load(memory_order_acquire);
        if (t == h) return false;
        if (slots[(t - 1) % cap].seq.load(memory_order_acquire) != t) return false;
        if (!tail.compare_exchange_weak(t, t - 1, memory_order_release, memory_order_relaxed)) return false;
        out = slots[(t - 1) % cap].data.load(memory_order_acquire);
        slots[(t - 1) % cap].seq.store(t - 1 + cap, memory_order_release);
        return true;
    }
};

// --- B.4. Атомика: счётчик (fetch_add и CAS-цикл) ---
// wait-free счётчик — fetch_add; CAS-цикл — идиома «прочитай-
// проверь-запиши» (для нетривиальных обновлений).
struct AtomicCounter {
    atomic<long long> v{0};

    void add_fetch(long long x) { v.fetch_add(x, memory_order_relaxed); }
    void add_cas(long long x) {  // CAS-цикл: прочитай-проверь-запиши
        long long old = v.load(memory_order_relaxed);
        while (!v.compare_exchange_weak(old, old + x, memory_order_release, memory_order_relaxed)) {}
    }
    long long get() const { return v.load(memory_order_acquire); }
};

// --- B.5. Tagged Treiber Stack (защита от ABA) ---
// Указатель и тег в одном атомарном слове: CAS меняет тег при каждой
// операции — «тот же адрес» больше не означает «то же состояние».
// Младшие биты свободны при 16-байтовом выравнивании new.
struct TaggedTreiberStack {
    struct Node { int val; Node* next; };
    atomic<uintptr_t> top{0};

    static uintptr_t pack(Node* p, int t) { return (uintptr_t)p | (unsigned)t; }
    static Node* ptr(uintptr_t w) { return (Node*)(w & ~(uintptr_t)7); }
    static int tag(uintptr_t w) { return (int)(w & 7); }

    void push(int x) {
        Node* n = new Node{x, nullptr};
        uintptr_t t = top.load(memory_order_relaxed);
        do { n->next = ptr(t); } while (!top.compare_exchange_weak(t, pack(n, tag(t) + 1), memory_order_release, memory_order_relaxed));
    }
    bool try_pop(int& out) {
        uintptr_t t = top.load(memory_order_relaxed);
        while (t) {
            if (top.compare_exchange_weak(t, pack(ptr(t)->next, tag(t) + 1), memory_order_acquire, memory_order_relaxed)) {
                out = ptr(t)->val;
                return true;
            }
        }
        return false;
    }
};

// =============================================================
// C. WAIT-FREE СТРУКТУРЫ
// =============================================================

// --- C.1. Wait-free Queue (SPSC, кольцо) ---
// Продюсер владеет tail (store), потребитель — head: гонка только
// на «чужих» концах (load) — каждая операция — конечное число
// атомарных шагов (wait-free). Данные — release/acquire.
struct SPSCWaitFreeQueue {
    int cap;
    vector<atomic<int>> slots;
    atomic<int> head{0}, tail{0};

    SPSCWaitFreeQueue(int cap_) : cap(cap_), slots(cap_) {}
    bool push(int x) {  // продюсер
        int t = tail.load(memory_order_relaxed);
        if (t - head.load(memory_order_acquire) == cap) return false;
        slots[t % cap].store(x, memory_order_release);
        tail.store(t + 1, memory_order_release);
        return true;
    }
    bool pop(int& out) {  // потребитель
        int h = head.load(memory_order_relaxed);
        if (h == tail.load(memory_order_acquire)) return false;
        out = slots[h % cap].load(memory_order_acquire);
        head.store(h + 1, memory_order_release);
        return true;
    }
};

// --- C.2. Wait-free Stack (SPSC, ограниченный, массив + seq) ---
// Потребитель ЧИТАЕТ вершину ДО декремента top: продюсер пишет только
// свободные позиции (≥ top), занятые не трогает — чтение не гоняется
// с записью, позиция не может быть переиспользована раньше чтения.
// Готовность слота — монотонный seq[p] = 2k−1 (k-я запись позиции):
// потребитель ждёт значения своего k-го чтения и НЕ сбрасывает seq
// (нет потери готовности). k-я запись позиции читается её k-м pop
// (записи и чтения позиции чередуются — LIFO).
struct SPSCWaitFreeStack {
    int cap;
    vector<atomic<int>> slots;
    vector<atomic<int>> seq;  // k-я запись позиции p: seq[p] = 2k−1
    atomic<int> top{0};
    vector<int> wcnt, reads;  // локальные счётчики: записей (продюсер), чтений (потребитель)

    SPSCWaitFreeStack(int cap_) : cap(cap_), slots(cap_), seq(cap_), wcnt(cap_, 0), reads(cap_, 0) {
        for (auto& s : seq) s.store(0, memory_order_relaxed);
    }
    bool push(int x) {  // продюсер
        if (top.load(memory_order_relaxed) == cap) return false;
        int i = top.fetch_add(1, memory_order_release);
        if (i >= cap) { top.fetch_sub(1, memory_order_release); return false; }
        ++wcnt[i];
        slots[i].store(x, memory_order_release);
        seq[i].store(2 * wcnt[i] - 1, memory_order_release);
        return true;
    }
    bool pop(int& out) {  // потребитель
        while (true) {
            int t = top.load(memory_order_acquire);
            if (t == 0) return false;
            int p = t - 1;
            int want = 2 * reads[p] + 1;
            while (seq[p].load(memory_order_acquire) != want) this_thread::yield();
            int v = slots[p].load(memory_order_acquire);
            // подтверждение позиции: top не изменился (не вставил продюсер);
            // иначе — повтор (освободить можно только прочитанную позицию)
            if (top.compare_exchange_weak(t, t - 1, memory_order_release, memory_order_relaxed)) {
                out = v;
                ++reads[p];
                return true;
            }
        }
    }
};

// =============================================================
// D. CONCURRENT CONTAINERS
// =============================================================

// --- D.1. Concurrent Vector (сегменты; чтение без блокировок) ---
// Сегменты фиксированного размера B; заполненный сегмент неизменяем.
// Таблица сегментов — атомарные указатели (число сегментов ≤ max_seg);
// размер — атомарный счётчик (release/acquire): чтение data[i] для
// i < size безопасно без лоча (запись до release-публикации).
struct ConcurrentVector {
    int B;
    struct Seg { vector<int> data; };
    vector<atomic<Seg*>> segs;
    atomic<int> size{0};
    mutex grow_m;
    int cur_seg = -1;

    ConcurrentVector(int B_ = 16, int max_seg_ = 64) : B(B_), segs(max_seg_) {
        for (auto& p : segs) p.store(nullptr);
    }
    ~ConcurrentVector() { for (auto& p : segs) delete p.load(); }
    void push_back(int x) {
        lock_guard<mutex> l(grow_m);
        int k = size.load(memory_order_relaxed);
        int seg = k / B;
        if (seg > cur_seg) {
            Seg* s = new Seg();
            segs[seg].store(s, memory_order_release);
            cur_seg = seg;
        }
        segs[seg].load(memory_order_relaxed)->data.push_back(x);
        size.store(k + 1, memory_order_release);
    }
    int at(int i) const {
        return segs[i / B].load(memory_order_acquire)->data[i % B];
    }
    int count() const { return size.load(memory_order_acquire); }
};

// --- D.2. Concurrent HashMap (бакетные локи) ---
// Лок на бакет: insert/find/erase трогают только свою цепочку —
// разные хеши работают параллельно.
struct ConcurrentHashMap {
    struct Node { int key, val; Node* next; };
    int nb;
    mutable vector<mutex> locks;
    vector<Node*> buckets;

    ConcurrentHashMap(int nb_ = 16) : nb(nb_), locks(nb_), buckets(nb_, nullptr) {}
    int bucket(int key) const { return (int)((size_t)hash<int>{}(key) % nb); }
    void insert(int key, int val) {
        int b = bucket(key);
        lock_guard<mutex> l(locks[b]);
        for (Node* p = buckets[b]; p; p = p->next)
            if (p->key == key) { p->val = val; return; }
        buckets[b] = new Node{key, val, buckets[b]};
    }
    bool find(int key, int& val) const {
        int b = bucket(key);
        lock_guard<mutex> l(locks[b]);
        for (const Node* p = buckets[b]; p; p = p->next)
            if (p->key == key) { val = p->val; return true; }
        return false;
    }
    bool erase(int key) {
        int b = bucket(key);
        lock_guard<mutex> l(locks[b]);
        Node** pp = &buckets[b];
        while (*pp) {
            if ((*pp)->key == key) { Node* d = *pp; *pp = d->next; delete d; return true; }
            pp = &(*pp)->next;
        }
        return false;
    }
};

// --- D.3. concurrent_queue / concurrent_stack ---
// Мост: типовые обёртки уже реализованы в A.1–A.3 (MutexQueue,
// MutexStack, ConditionQueue) — отдельный код не нужен (см. md D.3).

// =============================================================
// E. СПЕЦИАЛЬНЫЕ ПРИМИТИВЫ
// =============================================================

// --- E.1. Seqlock (write-lock + версия) ---
// Читатель — два чтения версии вокруг данных (повтор при нечётной
// версии или изменении); писатель — нечётная версия → данные → чётная.
// Читатели не блокируются (только спин при активном писателе).
struct Seqlock {
    atomic<unsigned> version{0};
    static const int MAX_RETRY = 10000;
    int N;
    vector<long long> data;

    Seqlock(int n_ = 4) : N(n_), data(n_) {}
    void write(int idx, long long x) {  // один писатель
        unsigned v = version.load(memory_order_relaxed);
        version.store(v + 1, memory_order_relaxed);
        data[idx] = x;
        version.store(v + 2, memory_order_release);
    }
    void write_pair(long long a, long long b) {  // пара полей в одном разделе
        unsigned v = version.load(memory_order_relaxed);
        version.store(v + 1, memory_order_relaxed);
        data[0] = a;
        data[1] = b;
        version.store(v + 2, memory_order_release);
    }
    bool read(int idx, long long& out) const {  // читатель: retry-цикл
        for (int attempt = 0; attempt < MAX_RETRY; ++attempt) {
            unsigned v1 = version.load(memory_order_acquire);
            if (v1 & 1) continue;
            long long x = data[idx];
            if (v1 == version.load(memory_order_acquire)) { out = x; return true; }
        }
        return false;
    }
    // согласованный снимок всех данных (один retry-цикл на весь массив)
    bool snapshot(long long* out) const {
        for (int attempt = 0; attempt < MAX_RETRY; ++attempt) {
            unsigned v1 = version.load(memory_order_acquire);
            if (v1 & 1) continue;
            for (int i = 0; i < N; ++i) out[i] = data[i];
            if (v1 == version.load(memory_order_acquire)) return true;
        }
        return false;
    }
};

// --- E.2. RCU (Read-Copy-Update, epoch-освобождение) ---
// Читатель — атомарная загрузка указателя (счётчик активных);
// писатель — копия + правка + атомарная замена; старая копия
// освобождается после «благодатного периода» (нет активных
// читателей). Упрощение: один счётчик на всех читателей.
struct RCUStore {
    struct Data { long long a, b; };
    atomic<Data*> ptr;
    atomic<int> readers{0};
    mutex retire_m;
    vector<Data*> retired;
    int pending = 0;
    static const int RETIRE_BATCH = 8;

    RCUStore(long long a = 0, long long b = 0) { ptr.store(new Data{a, b}, memory_order_relaxed); }
    ~RCUStore() {
        for (Data* d : retired) delete d;
        delete ptr.load();
    }
    Data* read_begin() {
        readers.fetch_add(1, memory_order_acquire);
        return ptr.load(memory_order_acquire);
    }
    void read_end() { readers.fetch_sub(1, memory_order_release); }
    void synchronize() {  // ждать завершения всех критических секций
        while (readers.load(memory_order_acquire) > 0) this_thread::yield();
    }
    void update(const function<void(Data&)>& f) {
        Data* old = ptr.load(memory_order_acquire);
        Data* n = new Data(*old);
        f(*n);
        ptr.store(n, memory_order_release);
        lock_guard<mutex> l(retire_m);
        retired.push_back(old);
        if (++pending >= RETIRE_BATCH) {  // накопили — освобождаем батчем
            synchronize();
            for (Data* d : retired) delete d;
            retired.clear();
            pending = 0;
        }
    }
};

// --- E.3. Lock-free Skip List (упрощённый, без удаления) ---
// Skip List (hashing.md IV) с атомарными указателями уровней:
// вставка — CAS снизу вверх; неудача CAS верхнего уровня не ломает
// список (уровень 0 обязателен). Удаление (Harris-маркировка) — анализ.
struct LockFreeSkipList {
    static const int L = 8;
    struct Node {
        int key;
        array<atomic<Node*>, L> next;
        Node(int k) : key(k) { for (auto& p : next) p.store(nullptr, memory_order_relaxed); }
    };
    Node* head;
    mt19937 rng;

    LockFreeSkipList() : rng(4242) { head = new Node(INT_MIN); }
    int random_level() {
        int lv = 1;
        while (lv < L && (rng() & 1)) ++lv;
        return lv;
    }
    bool search(int key, array<Node*, L>* preds = nullptr,
                array<Node*, L>* succs = nullptr) const {
        Node* cur = head;
        for (int l = L - 1; l >= 0; --l) {
            Node* nxt = cur->next[l].load(memory_order_relaxed);
            while (nxt && nxt->key < key) { cur = nxt; nxt = cur->next[l].load(memory_order_relaxed); }
            if (preds) (*preds)[l] = cur;
            if (succs) (*succs)[l] = nxt;
        }
        Node* t = cur->next[0].load(memory_order_relaxed);
        return t && t->key == key;
    }
    bool contains(int key) const { return search(key); }
    void insert(int key) {
        if (search(key)) return;
        array<Node*, L> preds, succs;
        search(key, &preds, &succs);
        int lv = random_level();
        Node* n = new Node(key);
        for (int l = 0; l < lv; ++l) n->next[l].store(succs[l], memory_order_relaxed);
        Node* s0 = succs[0];
        while (!preds[0]->next[0].compare_exchange_weak(s0, n, memory_order_release, memory_order_relaxed)) {
            search(key, &preds, &succs);
            if (succs[0] && succs[0]->key == key) { delete n; return; }  // кто-то вставил
            s0 = succs[0];
            for (int l = 0; l < lv; ++l) n->next[l].store(succs[l], memory_order_relaxed);
        }
        for (int l = 1; l < lv; ++l) {  // верхние уровни — оптимизация
            Node* s = succs[l];
            if (!preds[l]->next[l].compare_exchange_weak(s, n, memory_order_release, memory_order_relaxed)) break;
        }
    }
    vector<int> inorder() const {
        vector<int> res;
        for (Node* cur = head->next[0].load(memory_order_acquire); cur; cur = cur->next[0].load(memory_order_acquire))
            res.push_back(cur->key);
        return res;
    }
    int size() const {
        int n = 0;
        for (Node* cur = head->next[0].load(memory_order_acquire); cur; cur = cur->next[0].load(memory_order_acquire)) ++n;
        return n;
    }
};

};  // struct ConcurrentStructures

#ifndef STRUCT_H_MAIN
#define STRUCT_H_MAIN

int main() {
    using H = ConcurrentStructures;
    cout << "=== VIII. КОНКУРЕНТНЫЕ (ПАРАЛЛЕЛЬНЫЕ) СТРУКТУРЫ ===" << endl;

    // ---------- A.1 Mutex Queue / Stack ----------
    {
        H::MutexQueue q;
        for (int x : {1, 2, 3}) q.push(x);
        int a, b, c;
        cout << "MutexQueue pop = " << (q.try_pop(a) ? a : -1)
             << " " << (q.try_pop(b) ? b : -1)
             << " " << (q.try_pop(c) ? c : -1)
             << ", empty pop = " << q.try_pop(a)
             << " (ожидаем 1 2 3 0)" << endl;
        H::MutexStack s;
        for (int x : {1, 2, 3}) s.push(x);
        cout << "MutexStack pop = " << (s.try_pop(a) ? a : -1)
             << " " << (s.try_pop(b) ? b : -1)
             << " " << (s.try_pop(c) ? c : -1)
             << " (ожидаем 3 2 1)" << endl;
    }

    // ---------- A.2 RWLock ----------
    {
        H::RWLock rw;
        rw.read_lock();
        rw.read_unlock();
        rw.write_lock();
        rw.write_unlock();
        rw.read_lock();
        rw.read_unlock();
        cout << "RWLock single-thread ok (expect 1) = 1" << endl;
    }

    // ---------- A.3 ConditionQueue ----------
    {
        H::ConditionQueue q;
        for (int x : {1, 2, 3}) q.push(x);
        int a, b, c;
        cout << "ConditionQueue pop = " << (q.pop(a) ? a : -1)
             << " " << (q.pop(b) ? b : -1)
             << " " << (q.pop(c) ? c : -1)
             << " (ожидаем 1 2 3)" << endl;
    }

    // ---------- A.4 SpinLock ----------
    {
        H::SpinLock sp;
        long long cnt = 0;
        sp.lock();
        for (int i = 0; i < 1000; ++i) ++cnt;
        sp.unlock();
        cout << "SpinLock guarded count = " << cnt
             << " (ожидаем 1000)" << endl;
    }

    // ---------- B.1 Treiber Stack ----------
    {
        H::TreiberStack st;
        for (int x : {1, 2, 3}) st.push(x);
        int a, b, c;
        cout << "TreiberStack pop = " << (st.try_pop(a) ? a : -1)
             << " " << (st.try_pop(b) ? b : -1)
             << " " << (st.try_pop(c) ? c : -1)
             << ", empty = " << st.try_pop(a)
             << " (ожидаем 3 2 1 0)" << endl;
    }

    // ---------- B.2 Michael-Scott Queue ----------
    {
        H::MichaelScottQueue q;
        for (int x : {1, 2, 3}) q.push(x);
        int a, b, c;
        cout << "MichaelScottQueue pop = " << (q.try_pop(a) ? a : -1)
             << " " << (q.try_pop(b) ? b : -1)
             << " " << (q.try_pop(c) ? c : -1)
             << ", empty = " << q.try_pop(a)
             << " (ожидаем 1 2 3 0)" << endl;
    }

    // ---------- B.3 Lock-free Deque ----------
    {
        H::LockFreeDeque dq(8);
        for (int x : {1, 2, 3, 4}) dq.push_back(x);
        int a, b;
        cout << "LockFreeDeque pop_back = " << (dq.pop_back(a) ? a : -1)
             << ", pop_front = " << (dq.pop_front(b) ? b : -1)
             << ", pop_front = " << (dq.pop_front(a) ? a : -1)
             << ", pop_back = " << (dq.pop_back(b) ? b : -1)
             << " (ожидаем 4 1 2 3)" << endl;
    }

    // ---------- B.4 Атомика ----------
    {
        H::AtomicCounter c;
        for (int i = 0; i < 100; ++i) c.add_fetch(1);
        for (int i = 0; i < 100; ++i) c.add_cas(1);
        cout << "AtomicCounter = " << c.get()
             << " (ожидаем 200)" << endl;
    }

    // ---------- B.5 Tagged Treiber Stack ----------
    {
        H::TaggedTreiberStack st;
        for (int x : {1, 2, 3}) st.push(x);
        int a, b, c;
        cout << "TaggedTreiberStack pop = " << (st.try_pop(a) ? a : -1)
             << " " << (st.try_pop(b) ? b : -1)
             << " " << (st.try_pop(c) ? c : -1)
             << " (ожидаем 3 2 1)" << endl;
    }

    // ---------- C.1 Wait-free SPSC Queue ----------
    {
        H::SPSCWaitFreeQueue q(4);
        bool full = true;
        for (int x : {1, 2, 3, 4}) full &= q.push(x);
        cout << "SPSCQueue push 4/4 = " << full
             << ", 5th = " << q.push(5)
             << " (ожидаем 1 0)" << endl;
        int a, b, c, d;
        cout << "SPSCQueue pop = " << (q.pop(a) ? a : -1)
             << " " << (q.pop(b) ? b : -1)
             << " " << (q.pop(c) ? c : -1)
             << " " << (q.pop(d) ? d : -1)
             << ", 5th pop = " << q.pop(a)
             << " (ожидаем 1 2 3 4 0)" << endl;
    }

    // ---------- C.2 Wait-free SPSC Stack ----------
    {
        H::SPSCWaitFreeStack st(4);
        bool full = true;
        for (int x : {1, 2, 3, 4}) full &= st.push(x);
        cout << "SPSCStack push 4/4 = " << full
             << ", 5th = " << st.push(5)
             << " (ожидаем 1 0)" << endl;
        int a, b, c, d;
        cout << "SPSCStack pop = " << (st.pop(a) ? a : -1)
             << " " << (st.pop(b) ? b : -1)
             << " " << (st.pop(c) ? c : -1)
             << " " << (st.pop(d) ? d : -1)
             << " (ожидаем 4 3 2 1)" << endl;
    }

    // ---------- D.1 Concurrent Vector ----------
    {
        H::ConcurrentVector v(4, 16);
        for (int i = 1; i <= 20; ++i) v.push_back(i);
        bool ok = v.count() == 20;
        for (int i = 0; i < 20; ++i) ok &= v.at(i) == i + 1;
        cout << "ConcurrentVector count = " << v.count()
             << ", series ok = " << ok
             << " (ожидаем 20 1)" << endl;
    }

    // ---------- D.2 Concurrent HashMap ----------
    {
        H::ConcurrentHashMap hm(8);
        hm.insert(5, 50);
        hm.insert(3, 30);
        hm.insert(8, 80);
        int v;
        cout << "HashMap find(5) = " << hm.find(5, v) << " (val " << v
             << "), find(3) = " << hm.find(3, v) << " (val " << v
             << "), find(7) = " << hm.find(7, v)
             << " (ожидаем 1 (50) 1 (30) 0)" << endl;
        hm.erase(5);
        cout << "HashMap after erase(5): find(5) = " << hm.find(5, v)
             << ", find(8) = " << hm.find(8, v) << " (val " << v
             << " (ожидаем 0 1 (80))" << endl;
    }

    // ---------- E.1 Seqlock ----------
    {
        H::Seqlock sl;
        for (int i = 0; i < 4; ++i) sl.write(i, i * 10);
        long long v;
        cout << "Seqlock read(0) = " << (sl.read(0, v) ? v : -1)
             << ", read(3) = " << (sl.read(3, v) ? v : -1)
             << " (ожидаем 0 30)" << endl;
    }

    // ---------- E.2 RCU ----------
    {
        H::RCUStore rcu(1, 2);
        rcu.update([](H::RCUStore::Data& d) { d.a = 10; d.b = 20; });
        H::RCUStore::Data* p = rcu.read_begin();
        long long a = p->a, b = p->b;
        rcu.read_end();
        cout << "RCU read = " << a << " " << b
             << " (ожидаем 10 20)" << endl;
    }

    // ---------- E.3 Lock-free Skip List ----------
    {
        H::LockFreeSkipList sl;
        for (int x : {5, 3, 8, 1, 9, 4, 7, 2, 10, 6}) sl.insert(x);
        vector<int> in = sl.inorder();
        cout << "LF SkipList inorder =";
        for (int x : in) cout << " " << x;
        cout << " (ожидаем 1 2 3 4 5 6 7 8 9 10)" << endl;
        cout << "LF SkipList contains(7) = " << sl.contains(7)
             << ", contains(11) = " << sl.contains(11)
             << ", size = " << sl.size()
             << " (ожидаем 1 0 10)" << endl;
    }

    cout << "\n=== ОБЩЕЕ: многопоточные дымовые тесты ===" << endl;

    // TreiberStack: 4 потока × 1000 вставок, главный вынимает все
    {
        H::TreiberStack st;
        const int T = 4, N = 1000;
        vector<thread> ts;
        for (int t = 0; t < T; ++t)
            ts.emplace_back([&st, t] {
                for (int i = 0; i < N; ++i) st.push(t * N + i);
            });
        for (auto& th : ts) th.join();
        set<int> got;
        int v;
        while (st.try_pop(v)) got.insert(v);
        bool ok = (int)got.size() == T * N;
        for (int i = 0; i < T * N && ok; ++i) ok &= got.count(i) > 0;
        cout << "TreiberStack 4x1000 all found (expect 1) = " << ok << endl;
    }

    // Michael-Scott: продюсеры + потребитель
    {
        H::MichaelScottQueue q;
        const int T = 4, N = 1000;
        vector<thread> ts;
        for (int t = 0; t < T; ++t)
            ts.emplace_back([&q, t] {
                for (int i = 0; i < N; ++i) q.push(t * N + i);
            });
        for (auto& th : ts) th.join();
        set<int> got;
        int v;
        while (q.try_pop(v)) got.insert(v);
        bool ok = (int)got.size() == T * N;
        for (int i = 0; i < T * N && ok; ++i) ok &= got.count(i) > 0;
        cout << "MichaelScottQueue 4x1000 all found (expect 1) = " << ok << endl;
    }

    // SPSC wait-free queue: продюсер ↔ потребитель
    {
        H::SPSCWaitFreeQueue q(64);
        vector<int> got;
        atomic<bool> done{false};
        thread prod([&] { for (int i = 0; i < 2000; ++i) while (!q.push(i)) {} done.store(true, memory_order_release); });
        thread cons([&] {
            int v;
            while (true) {
                if (q.pop(v)) got.push_back(v);
                else if (done.load(memory_order_acquire)) break;
            }
        });
        prod.join();
        cons.join();
        bool ok = (int)got.size() == 2000;
        for (int i = 0; i < 2000 && ok; ++i) ok &= got[i] == i;
        cout << "SPSCWaitFreeQueue 2000 in order (expect 1) = " << ok << endl;
    }

    // SPSC wait-free stack: продюсер ↔ потребитель
    {
        H::SPSCWaitFreeStack st(64);
        vector<int> got;
        atomic<bool> done{false};
        thread prod([&] { for (int i = 0; i < 2000; ++i) while (!st.push(i)) {} done.store(true, memory_order_release); });
        thread cons([&] {
            int v;
            while (true) {
                if (st.pop(v)) got.push_back(v);
                else if (done.load(memory_order_acquire)) break;
            }
        });
        prod.join();
        cons.join();
        set<int> s(got.begin(), got.end());
        bool ok = (int)got.size() == 2000;
        for (int i = 0; i < 2000 && ok; ++i) ok &= s.count(i) > 0;
        cout << "SPSCWaitFreeStack 2000 all found (expect 1) = " << ok << endl;
    }

    // ConcurrentVector: 4 потока push_back
    {
        H::ConcurrentVector v(32, 256);
        const int T = 4, N = 500;
        vector<thread> ts;
        for (int t = 0; t < T; ++t)
            ts.emplace_back([&v, t] {
                for (int i = 0; i < N; ++i) v.push_back(t * N + i);
            });
        for (auto& th : ts) th.join();
        set<int> got;
        for (int i = 0; i < v.count(); ++i) got.insert(v.at(i));
        bool ok = v.count() == T * N;
        for (int i = 0; i < T * N && ok; ++i) ok &= got.count(i) > 0;
        cout << "ConcurrentVector 4x500 all found (expect 1) = " << ok << endl;
    }

    // ConcurrentHashMap: 4 потока вставляют разные ключи
    {
        H::ConcurrentHashMap hm(8);
        const int T = 4, N = 500;
        vector<thread> ts;
        for (int t = 0; t < T; ++t)
            ts.emplace_back([&hm, t] {
                for (int i = 0; i < N; ++i) hm.insert(t * N + i, t * N + i);
            });
        for (auto& th : ts) th.join();
        int v;
        bool ok = true;
        for (int i = 0; i < T * N && ok; ++i) ok &= hm.find(i, v) && v == i;
        cout << "ConcurrentHashMap 4x500 all found (expect 1) = " << ok << endl;
    }

    // Seqlock: писатель держит инвариант data[0]==data[1], читатели проверяют
    {
        H::Seqlock sl;
        sl.write(0, 0);
        sl.write(1, 0);
        atomic<bool> stop{false};
        thread writer([&] {
            for (long long i = 1; !stop.load(memory_order_relaxed); ++i) {
                sl.write_pair(i, i);  // инвариант data[0] == data[1] — в одном разделе
            }
        });
        thread reader([&] {
            long long out[4];
            for (int i = 0; i < 20000; ++i) {
                bool ok1 = sl.snapshot(out);
                if (ok1 && out[0] != out[1]) { cout << "Seqlock INCONSISTENT READ\n"; return; }
            }
        });
        reader.join();
        stop.store(true, memory_order_relaxed);
        writer.join();
        cout << "Seqlock consistent reads (expect 1) = 1" << endl;
    }

    // Lock-free Skip List: 4 потока вставляют разные ключи
    {
        H::LockFreeSkipList sl;
        const int T = 4, N = 500;
        vector<thread> ts;
        for (int t = 0; t < T; ++t)
            ts.emplace_back([&sl, t] {
                for (int i = 0; i < N; ++i) sl.insert(t * N + i);
            });
        for (auto& th : ts) th.join();
        vector<int> in = sl.inorder();
        bool ok = (int)in.size() == T * N;
        for (int i = 0; i < (int)in.size() && ok; ++i) ok &= in[i] == i;
        for (int i = 0; i < T * N && ok; ++i) ok &= sl.contains(i);
        cout << "LockFreeSkipList 4x500 sorted+found (expect 1) = " << ok << endl;
    }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_H_MAIN

#endif // STRUCT_H_CPP
