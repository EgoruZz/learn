#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    vector<vector<int>> generate(int numRows) {
        vector<vector<int>> pascal(numRows);
        for (int i = 0; i < numRows; i++) {
            pascal[i].assign(i + 1, 1);
            for (int j = 1; j < i; j++)
                pascal[i][j] = pascal[i - 1][j - 1] + pascal[i - 1][j];
        }
        return pascal;
    }
};

void print(const vector<vector<int>>& res) {
    for (auto vec : res) {
        for (auto elem : vec) cout << elem << ' ';
        cout << '\n';
    }
}

signed main() {
    int numRows;
    cin >> numRows;

    auto res = Solution().generate(numRows);
    print(res);
}