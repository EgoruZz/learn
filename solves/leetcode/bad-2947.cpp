#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    int beautifulSubstrings(string s, int k) {
        int l = 0, r = 0, n = (int) s.length();
        int vowels = 0, consonants = 0, ans = 0;
        
        while (r < n) {
            while (r < n) {
                (string("ieoua").find(s[r]) != string::npos) ? vowels++ : consonants++;
                if (vowels == consonants) ans++;
                r++;
            }
            while (!((vowels * consonants) % k == 0)) {
                (string("ieoua").find(s[l]) != string::npos) ? vowels-- : consonants--;
                if (vowels == consonants) ans++;
                l++;
            }
        }

        return ans;
    }
};

signed main() {
    string s = "baeyh"; // "hfuhwccewfeeeca", k = 5;
    int k = 2;
    auto res = Solution().beautifulSubstrings(s, k);
    cout << res << '\n';
}
