#include <iostream>
#include "utils.cpp"
#include <list>

using namespace std;

int pow(int b, int e) {
	int result = 1;

	for (int i = 0; i < e; i++)
		result = result * b;
	
	return result;
}

class Num {
public:
	list <int> number;
	list <int>::iterator it;
	bool positive = true;
};

class Operate {
private:
	Num AddValues(Num ab, Num result, int startID) {
		ab.it = ab.number.begin();
		advance(ab.it, startID);
		for (;ab.it != ab.number.end(); ++ab.it)
			result.number.insert(result.number.end(), *ab.it);

		return result;
	}

public:
	Num add(Num a, Num b) {
		int residuo = 0;
		Num result;

		if (a.positive == b.positive) {
			int i = 0;

			bool bigger = BiggerThan(a, b);
			if (bigger) {
				for (a.it = a.number.begin(), b.it = b.number.begin(); b.it != b.number.end(); ++a.it, ++b.it, i++)
				{
					int sum = *a.it + *b.it + residuo;
					if (sum >= 256) {
						std::cout << "Int large" << std::endl;

						result.it = result.number.begin();
						advance(result.it, i);
						result.number.insert(result.it, (sum - 256));
						residuo = 1;
					}
					else
					{
						result.it = result.number.begin();
						advance(result.it, i);
						result.number.insert(result.it, sum);
						if (residuo != 0)
							residuo = 0;
					}
				}


			}
			else
			{
				for (a.it = a.number.begin(), b.it = b.number.begin(); a.it != a.number.end(); ++a.it, ++b.it, i++)
				{
					int sum = *a.it + *b.it + residuo;
					if (sum >= 256) {
						std::cout << "Int large" << std::endl;

						result.it = result.number.begin();
						advance(result.it, i);
						result.number.insert(result.it, (sum - 256));
						residuo = 1;
					}
					else
					{
						result.it = result.number.begin();
						advance(result.it, i);
						result.number.insert(result.it, sum);
						if (residuo != 0)
							residuo = 0;
					}

					cout << *a.it << endl;
				}
			}

			a.it = a.number.begin();
			advance(a.it, i - 1);

			if (a.number.size() != b.number.size()) {
				Num r = AddValues(a, result, i);
				result.number.assign(r.number.begin(), r.number.end());
			}
			

			result.positive = a.positive;
		}
		else
		{
			int i = 0;
			bool biggerthan = BiggerThan(a, b);
			for (a.it = a.number.begin(), b.it = b.number.begin(); a.it != a.number.end(); ++a.it, ++b.it, i++)
			{
				int sum;

				if(biggerthan)
					sum = *a.it - *b.it + residuo;
				else
					sum = - *a.it + *b.it + residuo;

				if (sum < 0) {
					std::cout << sum << " int minus zero" << std::endl;
					
					result.it = result.number.begin();
					advance(result.it, i);
					result.number.insert(result.it, (sum + 256));
					residuo = -1;
				}
				else
				{
					result.it = result.number.begin();
					advance(result.it, i);
					result.number.insert(result.it, absolute(sum));
				}

				
			}
			if (biggerthan && a.positive || !biggerthan && b.positive) {
				result.positive = true;
				return result;
			}
			result.positive = false;
		}

		return result;
	}

public:
	bool BiggerThan(Num a, Num b) {
		if ((int)a.number.size() != (int)b.number.size()) {
			return a.number.size() > b.number.size();
		}
		else
		{
			return a.number.back() > b.number.back();
		}
	}
};

int main()
{
	list<int> l1 = { 1, 2 };
	list<int> l2 = { 3 };
	Num num1;
	Num num2;

	num1.number = l1;
	num2.number = l2;
	
	Operate operate;
	num1.positive = true;
	num2.positive = true;

	Num result = operate.add(num1, num2);

	cout << "Results:\n" << endl;

	cout << "First number: \t";
	for (auto it = num1.number.begin(); it != num1.number.end(); ++it)
		cout << *it << " ";

	cout << endl << "Second number: \t";
	for (auto it = num2.number.begin(); it != num2.number.end(); ++it)
		cout << *it << " ";

	cout << endl << "Result number: \t";
	for (auto it = result.number.begin(); it != result.number.end(); ++it)
		cout << *it << " ";

	cout << endl << "Sign (0, 1):\t" << result.positive << endl;
	
	return 0;
}