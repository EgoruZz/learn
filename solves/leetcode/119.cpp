#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    using ll = long long;
    vector<int> getRow(int rowIndex) {
        vector<int> row(rowIndex + 1, 0);
        row[0] = 1;
        for (int i = 1; i <= rowIndex; i++)
            row[i] = row[i - 1] * (rowIndex - i + 1) / i;
        return row;
    }
};

signed main() {
    int numRows = 5;

    auto row = Solution().getRow(numRows);
    for (auto elem : row) cout << elem << ' ';
}