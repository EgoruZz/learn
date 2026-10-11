#ifndef ALGEBRA_L_CPP
#define ALGEBRA_L_CPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
using namespace std;

// =============================================================
// XII. ДЛИННАЯ АРИФМЕТИКА
// =============================================================

struct BigInt {

static const int BASE = 1000000000;
static const int BASE_DIGITS = 9;

struct BigNum {
    vector<int> digits;
    bool negative;

    BigNum() : negative(false) {}
    BigNum(long long v) { *this = v; }
    BigNum(const string& s) { read(s); }

    void normalize() {
        while ((int)digits.size() > 1 && digits.back() == 0) digits.pop_back();
        if (digits.size() == 1 && digits[0] == 0) negative = false;
    }

    BigNum& operator=(long long v) {
        negative = v < 0; if (negative) v = -v; digits.clear();
        do { digits.push_back(v % BASE); v /= BASE; } while (v > 0);
        return *this;
    }

    int cmpAbs(const BigNum& o) const {
        if (digits.size() != o.digits.size()) return digits.size() < o.digits.size() ? -1 : 1;
        for (int i = (int)digits.size()-1; i >= 0; i--)
            if (digits[i] != o.digits[i]) return digits[i] < o.digits[i] ? -1 : 1;
        return 0;
    }

    bool operator<(const BigNum& o) const {
        if (negative != o.negative) return negative;
        int c = cmpAbs(o); return negative ? c > 0 : c < 0;
    }
    bool operator==(const BigNum& o) const { return negative == o.negative && cmpAbs(o) == 0; }
    bool operator<=(const BigNum& o) const { return !(o < *this); }
    bool operator>(const BigNum& o) const { return o < *this; }
    bool operator>=(const BigNum& o) const { return !(*this < o); }
    bool operator!=(const BigNum& o) const { return !(*this == o); }

    void read(const string& s) {
        negative = false; digits.clear(); int start = 0;
        if (!s.empty() && s[0] == '-') { negative = true; start = 1; }
        for (int i = (int)s.size()-1; i >= start; i -= BASE_DIGITS) {
            int chunk = 0;
            for (int j = max(start, i-BASE_DIGITS+1); j <= i; j++)
                chunk = chunk*10 + (s[j]-'0');
            digits.push_back(chunk);
        }
        normalize();
    }

    string str() const {
        if (digits.empty()) return "0";
        string s; if (negative) s += '-';
        s += to_string(digits.back());
        for (int i = (int)digits.size()-2; i >= 0; i--) {
            string p = to_string(digits[i]);
            s += string(BASE_DIGITS - p.size(), '0') + p;
        }
        return s;
    }
};

};

// Определяем BigNum-подтип через алиас
using BigNum = BigInt::BigNum;

// Вспомогательные функции
static BigNum addAbs(const BigNum& a, const BigNum& b) {
    BigNum res; int carry = 0;
    int n = max((int)a.digits.size(), (int)b.digits.size());
    for (int i = 0; i < n || carry; i++) {
        int sum = carry;
        if (i < (int)a.digits.size()) sum += a.digits[i];
        if (i < (int)b.digits.size()) sum += b.digits[i];
        res.digits.push_back(sum % BigInt::BASE);
        carry = sum / BigInt::BASE;
    }
    return res;
}

static BigNum subAbs(const BigNum& a, const BigNum& b) {
    BigNum res; int carry = 0;
    for (int i = 0; i < (int)a.digits.size(); i++) {
        int diff = a.digits[i] - carry - (i < (int)b.digits.size() ? b.digits[i] : 0);
        if (diff < 0) { diff += BigInt::BASE; carry = 1; } else carry = 0;
        res.digits.push_back(diff);
    }
    res.normalize(); return res;
}

// Операторы (свободные функции)
BigNum operator+(const BigNum& a, const BigNum& b) {
    if (a.negative == b.negative) {
        BigNum res = addAbs(a, b); res.negative = a.negative; res.normalize(); return res;
    }
    if (a.cmpAbs(b) >= 0) { BigNum res = subAbs(a, b); res.negative = a.negative; res.normalize(); return res; }
    BigNum res = subAbs(b, a); res.negative = b.negative; res.normalize(); return res;
}

BigNum operator-(const BigNum& a) {
    BigNum res = a;
    if (res.digits.size() > 1 || res.digits[0] != 0) res.negative = !res.negative;
    return res;
}

BigNum operator-(const BigNum& a, const BigNum& b) { return a + (-b); }

static BigNum mulNaive(const BigNum& a, const BigNum& b) {
    BigNum res; res.digits.assign(a.digits.size() + b.digits.size(), 0);
    for (int i = 0; i < (int)a.digits.size(); i++) {
        int carry = 0;
        for (int j = 0; j < (int)b.digits.size() || carry; j++) {
            long long cur = res.digits[i+j] + (long long)a.digits[i] * (j < (int)b.digits.size() ? b.digits[j] : 0) + carry;
            res.digits[i+j] = (int)(cur % BigInt::BASE); carry = (int)(cur / BigInt::BASE);
        }
    }
    res.negative = a.negative != b.negative; res.normalize(); return res;
}

static BigNum mulKaratsuba(const BigNum& a, const BigNum& b) {
    int n = max((int)a.digits.size(), (int)b.digits.size());
    if (n <= 64) return mulNaive(a, b);
    int k = n / 2;
    BigNum a0, a1, b0, b1;
    for (int i = 0; i < (int)a.digits.size(); i++) (i < k ? a0 : a1).digits.push_back(a.digits[i]);
    for (int i = 0; i < (int)b.digits.size(); i++) (i < k ? b0 : b1).digits.push_back(b.digits[i]);
    a0.normalize(); a1.normalize(); b0.normalize(); b1.normalize();
    BigNum p1 = mulKaratsuba(a1, b1), p2 = mulKaratsuba(a0, b0);
    BigNum mid = mulKaratsuba(a1 + a0, b1 + b0) - p1 - p2;
    BigNum r1 = p1; r1.digits.insert(r1.digits.begin(), 2*k, 0);
    BigNum r2 = mid; r2.digits.insert(r2.digits.begin(), k, 0);
    BigNum res = r1 + r2 + p2; res.negative = a.negative != b.negative; res.normalize(); return res;
}

BigNum operator*(const BigNum& a, const BigNum& b) { return mulKaratsuba(a, b); }

// Тесты
void testBigInt() {
    BigNum a("12345678901234567890"), b("98765432109876543210");
    assert(a < b); assert(a != b);
    assert((a + b).str() == "111111111011111111100");
    assert((b - a).str() == "86419753208641975320");
    assert((BigNum("999999999") * BigNum("999999999")).str() == "999999998000000001");
    BigNum x("123456789012345678901234567890"), y("987654321098765432109876543210");
    assert((x * y).str() == "121932631137021795226185032733866911");
    cout << "BigInt: all tests passed" << endl;
}

#endif // ALGEBRA_L_CPP
