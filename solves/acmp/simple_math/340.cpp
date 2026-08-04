#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

bool foo(vector<int>& box1, vector<int>& box2) {
    for (int i = 0; i < 3; i++) if (!(box1[i] <= box2[i])) return false;
    return true;
}

signed main() {
    int a1, b1, c1;
    int a2, b2, c2;
    cin >> a1 >> b1 >> c1;
    cin >> a2 >> b2 >> c2;

    vector<int> box1 = {a1, b1, c1};
    vector<int> box2 = {a2, b2, c2};
    sort(box1.begin(), box1.end());
    sort(box2.begin(), box2.end());

    if (box1 == box2) {
        cout << "Boxes are equal";
    } else {
        if (foo(box1, box2)) {
            cout << "The first box is smaller than the second one";
        } else {
            if (foo(box2, box1)) {
                cout << "The first box is larger than the second one";
            } else {
                cout << "Boxes are incomparable";
            }
        }
    }
}
