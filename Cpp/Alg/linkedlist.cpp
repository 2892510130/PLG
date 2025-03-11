#include <iostream>

using namespace std;

struct ListNode 
{
    int val;
    ListNode* next;
    ListNode(int var): val(var), next(nullptr) {};
};

ListNode* merge_sort(ListNode* left, ListNode* right);
ListNode* merge(ListNode* l1, ListNode* l2);

int main(int argc, char* argv[])
{
    ListNode* n0 = new ListNode(4);
    ListNode* n1 = new ListNode(2);
    ListNode* n2 = new ListNode(3);
    ListNode* n3 = new ListNode(1);
    n0->next = n1;
    n1->next = n2;
    n2->next = n3;
    cout << n0->next->val << endl;
    merge_sort(n0, nullptr);
}

ListNode* merge_sort(ListNode* left, ListNode* right)
{
    if (left == nullptr) return left;
    if (left->next == right)
    {
        left->next = nullptr;
        return left;
    }

    ListNode* slow = left, * fast = left;
    while (fast != right)
    {
        slow = slow->next;
        fast = fast->next;
        if (fast != right) fast = fast->next;
    }
    ListNode* mid = slow;
    return merge(merge_sort(left, mid), merge_sort(mid, right));
}

ListNode* merge(ListNode* l1, ListNode* l2)
{
    ListNode* pre_head = new ListNode(-1);
    ListNode* prev = pre_head;

    while (l1 != nullptr && l2 != nullptr)
    {
        if (l1->val < l2->val)
        {
            prev->next = l1;
            l1 = l1->next;
        }
        else
        {
            prev->next = l2;
            l2 = l2->next;
        }

        prev = prev->next;
    }

    prev->next = l1 == nullptr ? l2 : l1;
    return pre_head->next;
}
