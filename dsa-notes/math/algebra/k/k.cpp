#ifndef ALGEBRA_K_CPP
#define ALGEBRA_K_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <cstring>
using namespace std;

// =============================================================
// XI. БИТОВЫЕ ОПЕРАЦИИ
// =============================================================
// Структура md: A. Базовые побитовые операции
//               B. Представления чисел
//               C. Анализ и манипуляция битами
//               D. Битовые трюки и оптимизации
//               E. Упаковка данных и битовые множества
//               F. Встроенные функции и SIMD
//
// BitwiseOps — раздел ветки algebra. Вводит базовые побитовые
// операции, сдвиги, представления чисел, подсчёт и манипуляцию
// битами, битовые трюки, упаковку данных. Используется в
// struct (битовые множества), dynamic (битовое ДП), graph
// (битовые маски), number-theory (модулярная арифметика).
//
// Типы: int64_t для 64-битных операций, uint64_t для unsigned.

struct BitwiseOps {

// =============================================================
// A. БАЗОВЫЕ ПОБИТОВЫЕ ОПЕРАЦИИ
// =============================================================

// --- A.1. Логические операции ---
// Все операции: O(1) на машинном уровне.

// --- A.2. Манипуляции отдельными битами ---

// Установка k-го бита (считая с 0)
static int64_t setBit(int64_t a, int k) {
    return a | (INT64_C(1) << k);
}

// Сброс k-го бита
static int64_t clearBit(int64_t a, int k) {
    return a & ~(INT64_C(1) << k);
}

// Инверсия k-го бита
static int64_t flipBit(int64_t a, int k) {
    return a ^ (INT64_C(1) << k);
}

// Проверка k-го бита
static int testBit(int64_t a, int k) {
    return (a >> k) & 1;
}

// Усечение младших k бит
static int64_t maskLowBits(int64_t a, int k) {
    return a & ((INT64_C(1) << k) - 1);
}

// Младший установленный бит (lowbit)
static int64_t lowbit(int64_t a) {
    return a & (-a);
}

// Сброс младшего установленного бита
static int64_t resetLowestBit(int64_t a) {
    return a & (a - 1);
}

// Проверка на степень двойки (strict: a > 0)
static bool isPowerOfTwo(int64_t a) {
    return a > 0 && (a & (a - 1)) == 0;
}

// =============================================================
// B. ПРЕДСТАВЛЕНИЯ ЧИСЕЛ
// =============================================================

// --- B.4. Дополнительный код ---

// Быстрое отрицание через биты: −a = (a ^ mask) + (1 & mask)
static int64_t fastNegate(int64_t a) {
    int64_t mask = a >> 63; // арифметический сдвиг: 0 или −1
    return (a ^ mask) + (1 & mask);
}

// Абсолютное значение без ветвления
static int64_t fastAbs(int64_t a) {
    int64_t mask = a >> 63;
    return (a + mask) ^ mask;
}

// Среднее двух чисел без переполнения
static int64_t safeAverage(int64_t a, int64_t b) {
    return (a & b) + ((a ^ b) >> 1);
}

// --- B.5. Системы счисления ---

// Десятичная → строка в основании base (base ∈ [2, 36])
static string toBase(int64_t n, int base) {
    if (n == 0) return "0";
    bool neg = n < 0;
    if (neg) n = -n;
    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string res;
    while (n > 0) {
        res += digits[n % base];
        n /= base;
    }
    if (neg) res += '-';
    reverse(res.begin(), res.end());
    return res;
}

// Строка в основании base → десятичная
static int64_t fromBase(const string& s, int base) {
    int64_t res = 0;
    for (char c : s) {
        int d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'A' && c <= 'Z') d = c - 'A' + 10;
        else if (c >= 'a' && c <= 'z') d = c - 'a' + 10;
        else continue;
        res = res * base + d;
    }
    return res;
}

// =============================================================
// C. АНАЛИЗ И МАНИПУЛЯЦИЯ БИТАМИ
// =============================================================

// --- C.6. Подсчёт битов ---

// Число единиц: наивный O(w)
static int popcountNaive(int64_t a) {
    int cnt = 0;
    while (a) {
        cnt += (int)(a & 1);
        a >>= 1;
    }
    return cnt;
}

// Число единиц: Brian Kernighan O(k), k = число единиц
static int popcountKernighan(int64_t a) {
    int cnt = 0;
    while (a) {
        a &= a - 1;
        cnt++;
    }
    return cnt;
}

// Число единиц: builtin O(1) аппаратно
static int popcount(int64_t a) {
    return __builtin_popcountll((uint64_t)a);
}

// Число ведущих нулей (undefined при a = 0)
static int clz(int64_t a) {
    return __builtin_clzll((uint64_t)a);
}

// Число хвостовых нулей (undefined при a = 0)
static int ctz(int64_t a) {
    return __builtin_ctzll((uint64_t)a);
}

// --- C.7. Поиск позиций битов ---

// Позиция старшего установленного бита (0-based, -1 при a = 0)
static int highestBit(int64_t a) {
    if (a == 0) return -1;
    return 63 - __builtin_clzll((uint64_t)a);
}

// Позиция младшего установленного бита (0-based, -1 при a = 0)
static int lowestBit(int64_t a) {
    if (a == 0) return -1;
    return __builtin_ctzll((uint64_t)a);
}

// --- C.8. Реверс битов ---

// Реверс 64-битного числа: пошаговый swap блоков
static uint64_t reverseBits(uint64_t a) {
    a = ((a & 0x5555555555555555ULL) << 1)  | ((a & 0xAAAAAAAAAAAAAAAAULL) >> 1);
    a = ((a & 0x3333333333333333ULL) << 2)  | ((a & 0xCCCCCCCCCCCCCCCCULL) >> 2);
    a = ((a & 0x0F0F0F0F0F0F0F0FULL) << 4)  | ((a & 0xF0F0F0F0F0F0F0F0ULL) >> 4);
    a = ((a & 0x00FF00FF00FF00FFULL) << 8)  | ((a & 0xFF00FF00FF00FF00ULL) >> 8);
    a = ((a & 0x0000FFFF0000FFFFULL) << 16) | ((a & 0xFFFF0000FFFF0000ULL) >> 16);
    a = (a << 32) | (a >> 32);
    return a;
}

// =============================================================
// D. БИТОВЫЕ ТРЮКИ И ОПТИМИЗАЦИИ
// =============================================================

// --- D.9. Проверки свойств ---

// Чётность
static bool isEven(int64_t a) { return (a & 1) == 0; }

// --- D.10. Арифметические трюки ---

// Минимум без ветвления
static int64_t fastMin(int64_t a, int64_t b) {
    int64_t diff = a - b;
    int64_t mask = diff >> 63; // −1 если a < b, 0 иначе
    return b + (diff & mask);
}

// Максимум без ветвления
static int64_t fastMax(int64_t a, int64_t b) {
    int64_t diff = a - b;
    int64_t mask = diff >> 63;
    return a - (diff & mask);
}

// Битовое сложение двух чисел (рекурсивно, O(log w))
static int64_t addBitwise(int64_t a, int64_t b) {
    while (b != 0) {
        int64_t carry = a & b;
        a = a ^ b;
        b = carry << 1;
    }
    return a;
}

// Наибольшая степень двойки ≤ n (при n > 0)
static int64_t largestPowerOfTwoLeq(int64_t n) {
    if (n <= 0) return 0;
    return INT64_C(1) << (63 - __builtin_clzll((uint64_t)n));
}

// Следующее число с тем же числом единиц (алгоритм Госпера)
static uint64_t gosperNext(uint64_t a) {
    uint64_t c = a & (-a);
    uint64_t r = a + c;
    return (((r ^ a) >> 2) / c) | r;
}

// =============================================================
// E. УПАКОВКА ДАННЫХ И БИТОВЫЕ МНОЖЕСТВА
// =============================================================

// --- E.12. Битовые множества ---

// Подмножество {0, ..., w-1} как uint64_t
// Мощность множества
static int setSize(uint64_t s) {
    return __builtin_popcountll(s);
}

// Проверка принадлежности
static bool inSet(uint64_t s, int k) {
    return (s >> k) & 1;
}

// Добавление элемента
static uint64_t setAdd(uint64_t s, int k) {
    return s | (UINT64_C(1) << k);
}

// Удаление элемента
static uint64_t setRemove(uint64_t s, int k) {
    return s & ~(UINT64_C(1) << k);
}

// Перебор всех подмножеств множества mask
// Для каждого подмножества s вызывается callback(s)
static void iterateSubsets(uint64_t mask, void (*callback)(uint64_t)) {
    for (uint64_t s = mask; ; s = (s - 1) & mask) {
        callback(s);
        if (s == 0) break;
    }
}

// Перебор всех непустых подмножеств mask
static void iterateNonEmptySubsets(uint64_t mask, void (*callback)(uint64_t)) {
    for (uint64_t s = mask; s != 0; s = (s - 1) & mask) {
        callback(s);
    }
}

// --- E.13. Bit packing ---

// Упаковка k значений по b бит в одно 64-битное слово
// Запись i-го значения (0-based)
static uint64_t packSet(uint64_t word, int i, int b, uint64_t val) {
    int shift = i * b;
    uint64_t mask = (UINT64_C(1) << b) - 1;
    word = (word & ~(mask << shift)) | ((val & mask) << shift);
    return word;
}

// Чтение i-го значения
static uint64_t packGet(uint64_t word, int i, int b) {
    int shift = i * b;
    uint64_t mask = (UINT64_C(1) << b) - 1;
    return (word >> shift) & mask;
}

// =============================================================
// F. ВСТРОЕННЫЕ ФУНКЦИИ
// =============================================================

// Паритет числа единиц (XOR всех битов)
static int parity(int64_t a) {
    return __builtin_parityll((uint64_t)a);
}

// Позиция младшего установленного + 1 (0 при a = 0)
static int ffs(int64_t a) {
    return __builtin_ffsll(a);
}

// =============================================================
// ПРИМЕРЫ И ТЕСТЫ
// =============================================================
static void runTests() {
    // A. Базовые операции
    assert(setBit(0b1010, 1) == 0b1011);
    assert(clearBit(0b1011, 0) == 0b1010);
    assert(flipBit(0b1010, 0) == 0b1011);
    assert(testBit(0b1010, 3) == 1);
    assert(testBit(0b1010, 0) == 0);
    assert(lowbit(12) == 4);        // 12 = 1100, lowbit = 100 = 4
    assert(resetLowestBit(12) == 8); // 12 = 1100 → 1000 = 8
    assert(isPowerOfTwo(16) == true);
    assert(isPowerOfTwo(12) == false);

    // B. Представления
    assert(toBase(255, 16) == "FF");
    assert(toBase(10, 2) == "1010");
    assert(fromBase("FF", 16) == 255);
    assert(fastAbs(-42) == 42);
    assert(safeAverage(INT32_MAX, INT32_MAX) == INT32_MAX);

    // C. Подсчёт битов
    assert(popcount(0b10101010) == 4);
    assert(clz(1) == 63);  // 64-бит: 63 ведущих нуля
    assert(ctz(8) == 3);   // 8 = 1000, 3 хвостовых нуля
    assert(highestBit(12) == 3); // 12 = 1100, старший бит 3
    assert(lowestBit(12) == 2);  // 12 = 1100, младший бит 2

    // D. Трюки
    assert(fastMin(5, 3) == 3);
    assert(fastMax(5, 3) == 5);
    assert(addBitwise(17, 5) == 22);
    assert(largestPowerOfTwoLeq(13) == 8);
    assert(gosperNext(0b01011000ULL) == 0b01100001ULL);

    // E. Битовые множества
    uint64_t s = 0;
    s = setAdd(s, 2); s = setAdd(s, 5); s = setAdd(s, 7);
    assert(setSize(s) == 3);
    assert(inSet(s, 5) == true);
    assert(inSet(s, 3) == false);
    s = setRemove(s, 5);
    assert(setSize(s) == 2);

    cout << "BitwiseOps: all tests passed" << endl;
}

};

#endif // ALGEBRA_K_CPP
