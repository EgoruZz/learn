#ifndef ANALYSIS_J_CPP
#define ANALYSIS_J_CPP

#include "../i/i.cpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include <cassert>
using namespace std;

struct SpecialFunctions : OperationalCalculus {

// =============================================================
// A. СПЕЦИАЛЬНЫЕ ФУНКЦИИ
// =============================================================

// --- A.1. Гамма-функция (интеграл) ---
double gamma_function(double z, int n = 10000) {
    if (z < 0.5) return M_PI / (sin(M_PI*z) * gamma_function(1-z, n));
    auto integrand = [&](double t) { return pow(t, z-1) * exp(-t); };
    return integral_simpson(integrand, 0, 20, n);
}

// --- A.2. Бета-функция ---
double beta_function(double x, double y, int n = 10000) {
    return gamma_function(x)*gamma_function(y)/gamma_function(x+y, n);
}

// --- A.3. Сигмоида ---
double sigmoid(double x) { return 1.0/(1.0+exp(-x)); }
double sigmoid_derivative(double x) { double s=sigmoid(x); return s*(1-s); }

// --- A.4. Softmax ---
vector<double> softmax(const vector<double>& z) {
    double max_z = *max_element(z.begin(), z.end());
    vector<double> e(z.size()); double s=0;
    for (size_t i=0;i<z.size();++i){e[i]=exp(z[i]-max_z);s+=e[i];}
    for (auto& v:e) v/=s; return e;
}

// --- A.5. Интегральные тригонометрические ---
double Si(double x, int n = 10000) {
    auto f=[&](double t){return (abs(t)<1e-15)?1.0:sin(t)/t;};
    return integral_simpson(f,0,x,n);
}
double Ei(double x, int n = 10000) {
    auto f=[&](double t){return exp(t)/t;};
    return integral_simpson(f,-100,x,n);
}

// --- A.6. Эллиптические функции Якоби ( численно) ---
// sn(u,k), cn(u,k), dn(u,k) через несобственный интеграл
// u = F(φ,k) = ∫₀^φ dθ/√(1−k²sin²θ) → φ=am(u) → sn=sin φ, cn=cos φ
double elliptic_F(double phi, double k, int n = 10000) {
    auto f=[&](double t){return 1.0/sqrt(1.0-k*k*sin(t)*sin(t));};
    return integral_simpson(f,0,phi,n);
}

// Обратная: φ = am(u,k) через бисекцию
double am_function(double u, double k, double eps=1e-10) {
    double l=0, r=M_PI/2;
    while(r-l>eps){double m=(l+r)/2;(elliptic_F(m,k)<u)?l=m:r=m;}
    return (l+r)/2;
}

double jacobi_sn(double u, double k) { return sin(am_function(u,k)); }
double jacobi_cn(double u, double k) { return cos(am_function(u,k)); }
double jacobi_dn(double u, double k) { return sqrt(1.0-k*k*jacobi_sn(u,k)*jacobi_sn(u,k)); }

// =============================================================
// B. ВЫЧИСЛЕНИЕ КОНСТАНТ
// =============================================================

// --- B.1. π: Монте-Карло ---
double pi_monte_carlo(int n = 1000000) {
    int inside=0;
    for(int i=0;i<n;++i){
        double x=(double)rand()/RAND_MAX*2-1, y=(double)rand()/RAND_MAX*2-1;
        if(x*x+y*y<=1) inside++;
    }
    return 4.0*inside/n;
}

// --- B.2. π: Bailey–Borwein–Plouffe ---
double pi_bbp(int terms = 20) {
    double pi=0;
    for(int k=0;k<terms;++k){
        double p=pow(16,k);
        pi+=(1.0/p)*(4.0/(8*k+1)-2.0/(8*k+4)-1.0/(8*k+5)-1.0/(8*k+6));
    }
    return pi;
}

// --- B.3. π: Chudnovsky (~14 знаков/итерация) ---
double pi_chudnovsky(int terms = 5) {
    double sum=0, f6=1, f3=1, fk=1;
    for(int k=0;k<terms;++k){
        if(k>0){f6*=(6*k-5)*(6*k-4)*(6*k-3)*(6*k-2)*(6*k-1)*(6*k);
                 f3*=(3*k-2)*(3*k-1)*(3*k); fk*=k;}
        double num=(13591409+545140134.0*k)*(k%2==0?1:-1);
        sum+=num/(f6*pow(640320.0,3*k)*f3*fk*fk*fk);
    }
    return 1.0/(12.0*sum*sqrt(640320.0));
}

// --- B.4. π: Архимед (96 сторон) ---
double pi_archimedes() {
    // Вписанный: aₙ=2n sin(π/n); описанный: bₙ=2n tan(π/n)
    // Начинаем с 6 сторон
    double a = 3.0; // вписанный hexagon side = r = 1
    double b = 2.0*sqrt(3.0); // описанный
    for (int n = 6; n <= 96; n *= 2) {
        a = 2.0*a*b/(a+b); // среднее гармоническое
        b = sqrt(a*b); // среднее геометрическое
    }
    return (a+b)/2;
}

// --- B.5. e: ряд ---
double compute_e(int n = 20) { return maclaurin_exp(1.0, n); }

// --- B.6. √n: Ньютон ---
double sqrt_newton(double n, double eps = 1e-15) {
    double x=n;
    for(int i=0;i<100;++i){double xn=(x+n/x)/2;if(abs(xn-x)<eps)return xn;x=xn;}
    return x;
}

// =============================================================
// C. ТЕОРИЯ ПРИБЛИЖЕНИЙ
// =============================================================

// --- C.1. МНК (метод наименьших квадратов) ---
vector<double> least_squares(const vector<double>& x, const vector<double>& y,
                              int degree) {
    int n=x.size(), m=degree+1;
    vector<vector<double>> X(n,vector<double>(m));
    for(int i=0;i<n;++i) for(int j=0;j<m;++j) X[i][j]=pow(x[i],j);
    vector<vector<double>> XtX(m,vector<double>(m,0));
    vector<double> Xty(m,0);
    for(int i=0;i<m;++i){
        for(int j=0;j<m;++j) for(int k=0;k<n;++k) XtX[i][j]+=X[k][i]*X[k][j];
        for(int k=0;k<n;++k) Xty[i]+=X[k][i]*y[k];
    }
    vector<double> a(m);
    for(int i=0;i<m;++i){
        int mx=i; for(int j=i+1;j<m;++j) if(abs(XtX[j][i])>abs(XtX[mx][i])) mx=j;
        swap(XtX[i],XtX[mx]); swap(Xty[i],Xty[mx]);
        for(int j=i+1;j<m;++j){double f=XtX[j][i]/XtX[i][i];
            for(int k=i;k<m;++k) XtX[j][k]-=f*XtX[i][k]; Xty[j]-=f*Xty[i];}
    }
    for(int i=m-1;i>=0;--i){a[i]=Xty[i];for(int j=i+1;j<m;++j)a[i]-=XtX[i][j]*a[j];a[i]/=XtX[i][i];}
    return a;
}

// =============================================================
// D. ИСТОРИЧЕСКИЕ АЛГОРИТМЫ
// =============================================================

// --- D.1. Zeller's Congruence ---
int zeller_congruence(int d, int m, int y) {
    if(m<3){m+=12;y--;}
    int K=y%100, J=y/100;
    int h=(d+(13*(m+1))/5+K+K/4+J/4-2*J)%7;
    return ((h+6)%7)+1;
}

// --- D.2. Горнера ---
double horner(const vector<double>& a, double x) {
    double r=0; for(int i=a.size()-1;i>=0;--i) r=r*x+a[i]; return r;
}

// --- D.3. Быстрое возведение в степень ---
double fast_pow(double base, long long exp) {
    double r=1; while(exp>0){if(exp&1)r*=base;base*=base;exp>>=1;} return r;
}

// --- D.4. Быстрое возведение матрицы в степень ---
vector<vector<double>> mat_mul(const vector<vector<double>>& A,
                                const vector<vector<double>>& B) {
    int n=A.size(); vector<vector<double>> C(n,vector<double>(n,0));
    for(int i=0;i<n;++i) for(int j=0;j<n;++j) for(int k=0;k<n;++k) C[i][j]+=A[i][k]*B[k][j];
    return C;
}
vector<vector<double>> mat_pow(vector<vector<double>> M, long long p) {
    int n=M.size(); vector<vector<double>> R(n,vector<double>(n,0));
    for(int i=0;i<n;++i) R[i][i]=1;
    while(p>0){if(p&1)R=mat_mul(R,M);M=mat_mul(M,M);p>>=1;} return R;
}

};

#ifdef ANALYSIS_J_MAIN
int main() {
    SpecialFunctions sf;
    cout << "=== J. Специальные функции ===" << endl;

    // Γ(5)=4!=24
    cout << "Γ(5) = " << sf.gamma_function(5.0) << " (ожидается 24.0)" << endl;

    // π через BBP
    cout << "π (BBP) = " << sf.pi_bbp(20) << endl;

    // МНК: y=2x+1 по точкам
    vector<double> x = {0, 1, 2, 3, 4}, y = {1, 3, 5, 7, 9};
    auto c = sf.least_squares(x, y, 1);
    cout << "МНК линейный: y = " << c[1] << "x + " << c[0] << endl;

    return 0;
}
#endif

#endif // ANALYSIS_J_CPP
