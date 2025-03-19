#include <iostream>

using namespace std;

struct ListNode {
	int val;
	ListNode *next;
	ListNode() : val(0), next(nullptr) {}
	ListNode(int x) : val(x), next(nullptr) {}
	ListNode(int x, ListNode *next) : val(x), next(next) {}
};

ListNode* merge_sort(ListNode* left, ListNode* right);
ListNode* merge(ListNode* l1, ListNode* l2);
void print_linked_list(ListNode* node);
ListNode* reverse_linked_list(ListNode* node);
ListNode* reverseBetween(ListNode* head, int left, int right) ;

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
    print_linked_list(n0);
    auto sorted_node = merge_sort(n0, nullptr);
    print_linked_list(sorted_node);
    auto reversed_node = reverse_linked_list(sorted_node);
    print_linked_list(reversed_node);
}

ListNode* reverse_linked_list(ListNode* node)
{
    // ListNode* dummy = new ListNode(-1);
    ListNode* cur = node;
    ListNode* prev = nullptr;
    while (cur)
    {
        ListNode* next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    return prev;
}

ListNode* reverseBetween(ListNode* head, int left, int right) 
{
	ListNode* dummy = new ListNode(-1);
	dummy->next = head;
	ListNode* prev = dummy;
	for (int i = 0; i < left - 1; ++i)
	{
		prev = prev->next;
	}

	ListNode* cur = prev->next;
	ListNode* next;
	for (int i = 0; i < right - left; ++i)
	{
		next = cur->next;
		cur->next = next->next;
		next->next = prev->next;
		prev->next = next;
	}
	return dummy->next;
}


void print_linked_list(ListNode* node)
{
    while (node != nullptr)
    {
        cout << node->val << " ";
        node = node->next;
    }
    cout << '\n';
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
