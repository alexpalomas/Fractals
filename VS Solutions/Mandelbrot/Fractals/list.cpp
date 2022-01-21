#include <iostream>
#include <list>
using namespace std;
int main(void) {
	list<int> l;
	list<int> l1 = { 10, 20, 30, 40 };
	list<int> l2 = { 40, 30, 30, 20, 10 };
	list<int> l3 = (move(l1));


	cout << "Size of list l1: " << l2.size() << endl;
	cout << "List l2 contents: " << endl;
	for (auto it = l2.begin(); it != l2.end(); ++it)
		cout << *it << endl;
	cout << "List l3 contents: " << endl;
	for (auto it = l3.begin(); it != l3.end(); ++it)
		cout << *it << endl;
	return 0;
}