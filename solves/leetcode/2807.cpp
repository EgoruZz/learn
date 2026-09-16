#include <iostream>
#include <vector>
using namespace std;

struct SNode {
    int val;
    SNode* next;
    SNode(int v, SNode* nx = nullptr) : val(v), next(nx) {}
};

SNode* slist_build(const vector<int>& seq) {
    SNode* head = nullptr, **tail = &head;
    for (int x : seq) { *tail = new SNode(x); tail = &(*tail)->next; }
    return head;
}

class Solution {
public:
    SNode* insertGreatestCommonDivisors(SNode* head) {
        for (SNode* p = head; p && p -> next;) {
            SNode* newNode = new SNode(gcd(p->val, p ->next->val), p -> next);
            p->next = newNode;
            p = newNode->next;
        }
        return head;
    }

    int gcd(int a, int b) {
        while (b) swap(a %= b, b);
        return a;
    }

    void slist_print(SNode* head) {
        for (SNode* p = head; p; p = p->next) cout << p->val << (p->next ? " -> " : "");
        cout << '\n';
    }
};

signed main() {
    vector<int> seq = {18, 10, 6, 3};

    SNode* head = slist_build(seq); Solution().slist_print(head);
    auto res = Solution().insertGreatestCommonDivisors(head);

    Solution().slist_print(res);
}