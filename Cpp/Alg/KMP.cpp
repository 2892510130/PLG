#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<int> get_next(const string &pattern);
int kmp(const string &haystack, const string &needle, const vector<int> &next);
void print_vec(const vector<int> &v);

int main()
{
	string p = string("aaabbab");
	cout << "original:" << p << '\n';
	string big = string("ocpsaaabbabkkpaaabbab");
	vector<int> next = get_next(p);
	print_vec(next);
	int index = kmp(big, p, next);
	cout << big.substr(index, p.size()) << '\n';
	cout << index << '\n';
}

int kmp(const string &haystack, const string &needle, const vector<int> &next)
{
	int n = haystack.size(), m = needle.size();
	int left = 0, right = 0;
	while (right < n)
	{
		while (left > 0 && haystack[right] != needle[left])
		{
			left = next[left - 1];
		}
		
		if (haystack[right] == needle[left])
		{
			left++;
		}
		
		if (left == m)
		{
			return right - m + 1; // to continue find the next one, maybe record here?
		}
		
		right++;
	}
	
	return -1;
}
			
vector<int> get_next(const string &pattern)
{
	size_t n = pattern.size();
	vector<int> next(n, 0);
	int left = 0, right = 1;
	while (right < n)
	{
		if (pattern[left] == pattern[right])
		{
			next[right] = left + 1;
			left++;
		}
		else
		{
			while (pattern[right] != pattern[left] && left != 0)
			{
				left = next[left - 1];
			}
			
			if (left == 0 && pattern[left] != pattern[right])
			{
				// next[right] = 0;
			}
			else
			{
				next[right] = left + 1;
				left++;
			}
		}
		
		right++;
	}
	return next;
}

void print_vec(const vector<int> &v)
{
	for (const auto &num : v)
	{
		cout << num << " ";
	}
	cout << '\n';
}