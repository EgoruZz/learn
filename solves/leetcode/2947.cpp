#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    int beautifulSubstrings(string s, int k) {
        int ans = 0, n = (int) s.length();

        vector<int> vowels(n + 1, 0), consonants(n + 1, 0);
        for (int i = 0; i < n; i++)
            if (string("aeiou").find(s[i]) != string::npos) vowels[i + 1] = vowels[i] + 1, consonants[i + 1] = consonants[i];
            else vowels[i + 1] = vowels[i], consonants[i + 1] = consonants[i] + 1;
        
        for (int i = 1; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                int v = vowels[j] - vowels[i - 1], c = consonants[j] - consonants[i - 1];
                if ((v == c) && ((v * c) % k == 0)) ans++;
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
