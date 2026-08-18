#ifndef ANALYSIS_B_CPP
#define ANALYSIS_B_CPP

#include "../a/a.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>
#include <limits>
#include <tuple>
#include <cassert>
using namespace std;

// =============================================================
// B. ДИФФЕРЕНЦИАЛЬНОЕ ИСЧИСЛЕНИЕ ФУНКЦИЙ ОДНОЙ ПЕРЕМЕННОЙ
// =============================================================
// DifferentialCalculus : LimitsAndContinuity
// Переиспользование: LimitsAndContinuity (a.cpp) — пределы

struct DifferentialCalculus : LimitsAndContinuity {

// =============================================================
// A. ПРОИЗВОДНЫЕ (ЧАСТЬ 1)
// =============================================================

// --- A.1. Центральная разность ---
double derivative_central(function<double(double)> f, double x, double h = 1e-8) {
    return (f(x + h) - f(x - h)) / (2.0 * h);
}

// --- A.2. Левая разность ---
double derivative_forward(function<double(double)> f, double x, double h = 1e-8) {
    return (f(x + h) - f(x)) / h;
}

// --- A.3. Цепное правило (численно) ---
double chain_rule(function<double(double)> f, function<double(double)> g,
                  function<double(double)> df, function<double(double)> dg,
                  double x) { return df(g(x)) * dg(x); }

// --- A.4. Dual Number AD ---
struct Dual {
    double val, deriv;
    Dual(double v = 0, double d = 0) : val(v), deriv(d) {}
    Dual operator+(const Dual& o) const { return {val+o.val, deriv+o.deriv}; }
    Dual operator-(const Dual& o) const { return {val-o.val, deriv-o.deriv}; }
    Dual operator*(const Dual& o) const { return {val*o.val, deriv*o.val+val*o.deriv}; }
    Dual operator/(const Dual& o) const {
        double q = val/o.val; return {q, (deriv*o.val-val*o.deriv)/(o.val*o.val)};
    }
    Dual operator-() const { return {-val, -deriv}; }
    static Dual sin(const Dual& x) { return {::sin(x.val), ::cos(x.val)*x.deriv}; }
    static Dual cos(const Dual& x) { return {::cos(x.val), -::sin(x.val)*x.deriv}; }
    static Dual exp(const Dual& x) { double e=::exp(x.val); return {e, e*x.deriv}; }
    static Dual log(const Dual& x) { return {::log(x.val), x.deriv/x.val}; }
    static Dual pow(const Dual& x, double n) {
        double p=::pow(x.val,n); return {p, n*::pow(x.val,n-1)*x.deriv};
    }
};

Dual dual_eval(function<Dual(Dual)> f, double x) { return f(Dual(x, 1.0)); }

// =============================================================
// B. ПРОИЗВОДНЫЕ (ЧАСТЬ 2)
// =============================================================

// --- B.1. Логарифмическая производная ---
double log_derivative(function<double(double)> f, function<double(double)> df,
                      double x) {
    double fx = f(x); return (abs(fx)<1e-15)? NAN : df(x)/fx;
}

// --- B.2. Неявная производная ---
double implicit_derivative(function<double(double,double)> F,
                            double x, double y, double h = 1e-8) {
    double Fx = (F(x+h,y)-F(x-h,y))/(2*h);
    double Fy = (F(x,y+h)-F(x,y-h))/(2*h);
    return (abs(Fy)<1e-15)? NAN : -Fx/Fy;
}

// --- B.3. Параметрическая ---
double parametric_derivative(function<double(double)> xf,
                              function<double(double)> yf,
                              double t, double h = 1e-8) {
    double dx = (xf(t+h)-xf(t-h))/(2*h);
    double dy = (yf(t+h)-yf(t-h))/(2*h);
    return (abs(dx)<1e-15)? NAN : dy/dx;
}

// =============================================================
// C. ПРОИЗВОДНЫЕ ВЫСШИХ ПОРЯДКОВ
// =============================================================

// --- C.1. n-я производная (центральные разности) ---
double derivative_nth(function<double(double)> f, double x,
                      int n, double h = 1e-5) {
    if (n == 0) return f(x);
    double sum = 0;
    for (int k = 0; k <= n; ++k) {
        double C = 1;
        for (int i = 0; i < k; ++i) { C *= (n-i); C /= (i+1); }
        sum += ((n-k)%2==0 ? 1 : -1) * C * f(x + k*h);
    }
    return sum / pow(h, n);
}

// --- C.2. Лейбница: n-я производная произведения ---
double leibnitz_nth(function<double(double)> f, function<double(double)> g,
                     double x, int n, double h = 1e-5) {
    double sum = 0;
    for (int k = 0; k <= n; ++k) {
        double C = 1;
        for (int i = 0; i < k; ++i) { C *= (n-i); C /= (i+1); }
        sum += C * derivative_nth(f,x,k,h) * derivative_nth(g,x,n-k,h);
    }
    return sum;
}

// =============================================================
// D. РЯД ТЕЙЛОРА
// =============================================================

// --- D.1. Коэффициенты Тейлора: aₖ = f⁽ᵏ⁾(a)/k! ---
vector<double> taylor_coefficients(function<double(double)> f,
                                    double a, int n, double h = 1e-5) {
    vector<double> c(n+1); double fact = 1;
    for (int k = 0; k <= n; ++k) {
        if (k > 0) fact *= k;
        c[k] = derivative_nth(f, a, k, h) / fact;
    }
    return c;
}

// --- D.2. Оценка полинома Тейлора ---
double taylor_eval(const vector<double>& c, double a, double x) {
    double r = 0, p = 1;
    for (size_t k = 0; k < c.size(); ++k) { r += c[k]*p; p *= (x-a); }
    return r;
}

// --- D.3. Стандартные разложения ---
double maclaurin_exp(double x, int n = 20) {
    double s=0, t=1; for(int k=0;k<=n;++k){s+=t; t*=x/(k+1);} return s;
}
double maclaurin_sin(double x, int n = 20) {
    double s=0, t=x; for(int k=0;k<=n;++k){s+=t; t*=-x*x/((2*k+2)*(2*k+3));} return s;
}
double maclaurin_cos(double x, int n = 20) {
    double s=0, t=1; for(int k=0;k<=n;++k){s+=t; t*=-x*x/((2*k+1)*(2*k+2));} return s;
}
double maclaurin_ln(double x, int n = 20) {
    assert(x > -1 && x <= 1); double s=0, t=x;
    for(int k=1;k<=n;++k){s+=(k%2==1?1:-1)*t/k; t*=x;} return s;
}
double maclaurin_binomial(double x, double alpha, int n = 20) {
    assert(x > -1 && x < 1); double s=0, C=1;
    for(int k=0;k<=n;++k){if(k>0)C*=(alpha-k+1)/k; s+=C*pow(x,k);} return s;
}

// =============================================================
// E. ОСНОВНЫЕ ТЕОРЕМЫ
// =============================================================

// --- E.1. Ролль (numerical) ---
double roolle(function<double(double)> df, double a, double b,
              double eps = 1e-10) {
    double l=a, r=b;
    while(r-l>eps){double m=(l+r)/2;(df(m)>0)?l=m:r=m;} return (l+r)/2;
}

// --- E.2. Лагранж (numerical) ---
double lagrange_mean_value(function<double(double)> f,
                            function<double(double)> df,
                            double a, double b, double eps = 1e-10) {
    double target=(f(b)-f(a))/(b-a), l=a, r=b;
    for(int i=0;i<100;++i){double m=(l+r)/2;(df(m)<target)?l=m:r=m;} return (l+r)/2;
}

// --- E.3. Коши (numerical) ---
double cauchy_mean_value(function<double(double)> f, function<double(double)> g,
                          function<double(double)> df, function<double(double)> dg,
                          double a, double b, double eps = 1e-10) {
    double target=(f(b)-f(a))/(g(b)-g(a)), l=a, r=b;
    auto h=[&](double x){return df(x)-target*dg(x);};
    for(int i=0;i<100;++i){double m=(l+r)/2;(h(m)<0)?l=m:r=m;} return (l+r)/2;
}

// =============================================================
// F. ИССЛЕДОВАНИЕ ФУНКЦИЙ
// =============================================================

// --- F.1. Бисекция ---
double bisection(function<double(double)> f, double a, double b,
                  double eps = 1e-10) {
    assert(f(a)*f(b)<0);
    while(b-a>eps){double m=(a+b)/2;(f(m)*f(a)<=0)?b=m:a=m;} return (a+b)/2;
}

// --- F.2. Ньютон ---
double newton_raphson(function<double(double)> f, function<double(double)> df,
                       double x0, int max_iter=100, double eps=1e-15) {
    double x=x0;
    for(int i=0;i<max_iter;++i){
        double fx=f(x), dfx=df(x); if(abs(dfx)<1e-15)break;
        double xn=x-fx/dfx; if(abs(xn-x)<eps)return xn; x=xn;
    } return x;
}

// --- F.3. Секущие ---
double secant_method(function<double(double)> f, double x0, double x1,
                      int max_iter=100, double eps=1e-15) {
    for(int i=0;i<max_iter;++i){
        double f0=f(x0), f1=f(x1); if(abs(f1-f0)<1e-15)break;
        double xn=x1-f1*(x1-x0)/(f1-f0); if(abs(xn-x1)<eps)return xn;
        x0=x1; x1=xn;
    } return x1;
}

// --- F.4. Интерполяция Лагранжа ---
double lagrange_interpolation(const vector<double>& xp,
                               const vector<double>& yp, double x) {
    int n=xp.size(); double r=0;
    for(int i=0;i<n;++i){
        double b=1;
        for(int j=0;j<n;++j) if(i!=j) b*=(x-xp[j])/(xp[i]-xp[j]);
        r+=yp[i]*b;
    } return r;
}

// --- F.5. Разделённые разности Ньютона ---
vector<double> newton_divided_differences(const vector<double>& xp,
                                           const vector<double>& yp) {
    int n=xp.size(); vector<double> dd(yp);
    for(int j=1;j<n;++j) for(int i=n-1;i>=j;--i)
        dd[i]=(dd[i]-dd[i-1])/(xp[i]-xp[i-j]);
    return dd;
}

double newton_eval(const vector<double>& dd, const vector<double>& xp, double x) {
    int n=dd.size(); double r=dd[n-1];
    for(int i=n-2;i>=0;--i) r=r*(x-xp[i])+dd[i];
    return r;
}

// --- F.6. Ternary search ---
double ternary_search_min(function<double(double)> f, double a, double b,
                           double eps = 1e-10) {
    while(b-a>eps){
        double m1=a+(b-a)/3, m2=b-(b-a)/3;
        (f(m1)<f(m2))?b=m2:a=m1;
    } return (a+b)/2;
}

// --- F.7. Золотое сечение ---
double golden_section_min(function<double(double)> f, double a, double b,
                           double eps = 1e-10) {
    const double phi=(1+sqrt(5.0))/2;
    double c=b-(b-a)/phi, d=a+(b-a)/phi;
    while(b-a>eps){
        if(f(c)<f(d)){b=d;d=c;c=b-(b-a)/phi;}
        else{a=c;c=d;d=a+(b-a)/phi;}
    } return (a+b)/2;
}

// --- F.8. Классификация критической точки ---
// 0=нет экстр, 1=мин, 2=макс
int classify_critical_point(function<double(double)> df,
                             function<double(double)> ddf,
                             double a, double h = 1e-6) {
    if(abs(df(a))>1e-6) return -1;
    double dd=ddf(a);
    if(abs(dd)>1e-6) return (dd>0)?1:2;
    double dl=df(a-h), dr=df(a+h);
    if(dl<0&&dr>0) return 1; if(dl>0&&dr<0) return 2; return 0;
}

}; // struct DifferentialCalculus

#ifdef ANALYSIS_B_MAIN
int main() {
    DifferentialCalculus dc;
    cout << "=== B. Дифференциальное исчисление ===" << endl;

    // Центральная разность: d/dx sin(x)|_{x=π/4} = cos(π/4) ≈ 0.7071
    auto fsin = [](double x) { return sin(x); };
    cout << "d/dx sin(π/4) = " << dc.derivative_central(fsin, M_PI/4) << endl;

    // Цепное правило: d/dx sin(x²) = cos(x²)·2x
    auto g = [](double x) { return x*x; };
    auto dgsin = [](double x) { return cos(x); };
    auto dg2 = [](double x) { return 2*x; };
    cout << "d/dx sin(x²)|_{x=1} = " << dc.chain_rule(fsin, g, dgsin, dg2, 1.0) << endl;

    // Dual number: d/dx (sin(x)·exp(x))|_{x=0}
    auto fDual = [](DifferentialCalculus::Dual x) { return DifferentialCalculus::Dual::sin(x) * DifferentialCalculus::Dual::exp(x); };
    cout << "d/dx sin·exp|_{x=0} = " << dc.dual_eval(fDual, 0.0).deriv << " (ожидается 1.0)" << endl;

    return 0;
}
#endif

#endif // ANALYSIS_B_CPP
