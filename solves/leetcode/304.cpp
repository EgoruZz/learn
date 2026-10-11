#include <vector>
#include <iostream>
using namespace std;

class NumMatrix {
public:
    vector<vector<int>> p;

    NumMatrix(vector<vector<int>>& matrix) {
        int n = (int) matrix.size();
        int m = n ? (int) matrix[0].size() : 0;
        p.assign(n + 1, vector<int>(m + 1, 0));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                p[i + 1][j + 1] = p[i][j + 1] + p[i + 1][j] - p[i][j] + matrix[i][j];
            }
        }
    }
    
    int sumRegion(int row1, int col1, int row2, int col2) {
        return p[row2][col2] - p[row1][col2] - p[row2][col1] + p[row1][col1];
    }
};

void func() {
    vector<vector<int>> matrix = {{3, 0, 1, 4, 2}, {5, 6, 3, 2, 1}, {1, 2, 0, 1, 5}, {4, 1, 0, 1, 7}, {1, 0, 3, 0, 5}};
    vector<vector<int>> regions = {{2, 1, 4, 3}, {1, 1, 2, 2}, {1, 2, 2, 4}};
    
    NumMatrix* obj = new NumMatrix(matrix);
    for (auto region : regions) {
        int res = obj -> sumRegion(region[0], region[1], region[2] + 1, region[3] + 1);
        cout << res << ' ';
    }
}

signed main() {
    func();
}

/**
* Your NumMatrix object will be instantiated and called as such:
* NumMatrix* obj = new NumMatrix(matrix);
* int param_1 = obj->sumRegion(row1,col1,row2,col2);
*/