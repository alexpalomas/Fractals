#include <iostream>
#include <list>
#include <string>
#include <cmath>

using namespace std;

int truncament = 20;

double absolut(double a) {
	if(a >= 0)
		return a;
	if(a < 0)
		return -a;
}

class Num
{
private:
	void SetAsDouble(double a) {
		entero = floor(absolut(a));
		signo = a >= 0;

		string s = to_string(absolut(a));

		for (size_t c = 2; c < s.size(); c++)
			decimales.insert(decimales.end(), s[c] - '0');
	}


public:
	Num(string number, bool ceros);
	Num(int i);
	Num(float f);
	Num(double d);
	Num();

	int entero;
	list<int> decimales;
	bool signo = true;

	
	void BorrarCeros() {
		for (auto it = decimales.rbegin(); it != decimales.rend();)
		{
			if (*it == 0) {
				decimales.pop_back();
			}
			else {
				return;
			}
		}
	}

	void SetNumberString(string s, bool ceros = false) {
		ResetNumber();

		int start = 0;

		if (s[0] == '-') {
			signo = false;
			start++;
		}

		string e_s;

		while (s[start] != '.' && s[start] != ',')
		{
			e_s += s[start];
			start++;
		}
		entero = stoi(e_s);
		start++;
		
		for (size_t i = start; i < s.size(); i++)
		{
			decimales.insert(decimales.end(), s[i] - '0');
		}
		
		if(!ceros)
			BorrarCeros();
	}

	void WriteNumber() {
		cout << String() << endl;
	}

	string String() {
		string aRellenar;

		aRellenar += signo ? "" : "-";
		aRellenar += to_string(entero);
		
		if (decimales.size() != 0) {
			aRellenar += ".";

			for (auto it = decimales.begin(); it != decimales.end(); ++it)
			{
				aRellenar += to_string(*it);
			}
		}

		return aRellenar;
	}

	// Float i Double
	/*double Double() {
		return stod(String());
	}

	float Float() {
		return stof(String());
	}*/ 

	void InvertirSigno() {
		signo = !signo;
	}

	void ImprimirDecimalesUnoPorUno() {
		int i = 0;
		cout << "Decimales:" << endl;
		for (auto it = decimales.begin(); it != decimales.end(); ++it, i++)
		{
			cout << i << ": " << *it << endl;
		}
	}

	void ResetNumber() {
		entero = 0;
		decimales.clear();
		signo = true;
	}
	
	void Truncar(int t = truncament) {
		for(int i = decimales.size(); i > t; i--){
			decimales.pop_back();
		}
	}
};

Num::Num(string number, bool ceros = false) {
	ResetNumber();

	SetNumberString(number, ceros);
	
	Truncar();
}

Num::Num(int i) {
	ResetNumber();

	entero = (int)absolut(i);
	signo = i >= 0;
	decimales.clear();
	BorrarCeros();
	Truncar();
}

// Float i Double
/*Num::Num(float f) {
	ResetNumber();

	SetAsDouble((double)f);
	BorrarCeros();
	Truncar();
}

Num::Num(double d) {
	ResetNumber();

	SetAsDouble(d);
	BorrarCeros();
	Truncar();
}*/

Num::Num() { ResetNumber(); }

class Operador {

private:
	bool MasGrandeQueSinSigno(Num a, Num b) {
		if (a.entero != b.entero) {
			return a.entero > b.entero;
		}

		if (a.decimales.size() <= b.decimales.size()) {
			list<int>::iterator b_it = b.decimales.begin();
			for (auto a_it = a.decimales.begin(); a_it != a.decimales.end(); ++a_it, ++b_it)
				if (*a_it != *b_it)
					return *a_it > *b_it;

			return false;
		}

		if (a.decimales.size() > b.decimales.size()) {
			list<int>::iterator a_it = a.decimales.begin();
			for (auto b_it = b.decimales.begin(); b_it != b.decimales.end(); ++a_it, ++b_it)
				if (*a_it != *b_it)
					return *a_it > *b_it;

			return true;
		}

		// IF BOTH ARE EQUAL
		return true;
	}

public:	
	
	
	Num abs(Num a) {
		a.signo = true;
		return a;
	}

	
	Num InvertirSigno(Num a) {
		a.signo = !a.signo;
		return a;
	}

	
	bool MasGrandeQue(Num a, Num b, bool view = false) {
		if (a.signo == b.signo) {
			if (a.entero != b.entero) {
				if (a.signo)
					return a.entero > b.entero;
				return a.entero < b.entero;
			}

			if (a.decimales.size() <= b.decimales.size()) {
				list<int>::iterator b_it = b.decimales.begin();
				for (auto a_it = a.decimales.begin(); a_it != a.decimales.end(); ++a_it, ++b_it)
					if (*a_it != *b_it) {
						if (a.signo)
							return *a_it > *b_it;
						return *a_it < *b_it;
					}

				return !a.signo;
			}

			if (a.decimales.size() > b.decimales.size()) {
				list<int>::iterator a_it = a.decimales.begin();
				for (auto b_it = b.decimales.begin(); b_it != b.decimales.end(); ++a_it, ++b_it)
					if (*a_it != *b_it) {
						if (a.signo)
							return *a_it > *b_it;
						return *a_it < *b_it;
					}
				return a.signo;
			}

			// IF BOTH ARE EQUAL
			return true;
		}

		return a.signo;
	}

	
	Num Sumar(Num a, Num b, bool truncar = true, bool view = false) {
		/*
			INFORMACIÓ QUE M'ARRIVA:

			Els enters i els decimals 1 a 1.

			Llavors, per a fer la suma haurem de:
				1.- Veure quin té més decimals.
				2.- Sumar els decimals, amb el residu, de la dreta a l'esquerra.
				3.- Sumar els enters, amb el residu.
			
			Nota {
				Es interesant com a la suma simplement també es resta, així que aquí també es fa la resta.
			}

			Possibilitats de variants de signes:

					|	+	|	-	|
					|		|		|
			|	+	|	+	|	d	|
			|		|		|		|
			|	-	|	d	|	-	|

			d = depen, de què? Doncs de quin sigui el més gran!
			Si el negatiu es més gran, de forma absoluta, al positiu, doncs el resultat serà negatiu, en el 
			cas contrari, positiu.


			Exemple:
			  1	  1	1 1	  1			1 1	  1	1
			+ 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			+ 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1
			------------------------------------------------
			+ 5 , 2 3 4 1 5 3 9 8 4 7 2 1 2 3 2 4 6 6 5 1 2


			Exemple:
			  1	  1	1 1	  1			1 1	  1	1
			- 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			- 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1
			------------------------------------------------
			- 5 , 2 3 4 1 5 3 9 8 4 7 2 1 2 3 2 4 6 6 5 1 2
			

			Exemple:
				  1	1 1		1			1	1 1	1 1 1 1 1 
			- 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1 0 0 0 0 0			Si el numero de decimals del petit és més gran que el del gran, es segueix: 10 - ((l'altre) + residu).
			+ 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			------------------------------------------------
			- 2 , 4 9 6	3 2	4 9	2 0	0 3	0 9	1 5 7 3	3 4 8 8


			Exemple:
				  1	1 1		1			1	1 1	1 1 1 1 1
			+ 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1 0 0 0 0 0			
			- 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			------------------------------------------------
			+ 2 , 4 9 6	3 2	4 9	2 0	0 3	0 9	1 5 7 3	3 4 8 8


			Conclusions:
				- En signes oposats, el signe resultant serà el del més gran. 
				  En aquest cas s'ha de tenir en compte quin és més gran que l'altre.
				
				- Amb el mateix signe, es quedarà el signe serà el mateix. 
				  No s'haurà de tenir en compte quin es més gran.
		*/

		// Declarem el resultat.
		Num resultat = Num();

		// Declarem el residu fora de tot.
		int residu = 0;
		
		// Com ho hem de fer desde dreta a esquerra, comencem pel final fent servir els iteradors invertits.
		list<int>::reverse_iterator a_it = a.decimales.rbegin(), b_it = b.decimales.rbegin();
		
		// Mirem el signe
		if (a.signo == b.signo) {
			// Els signes son iguals.
			
			// Comencem la suma:

			// PROGRAMACIÓ: Per saber quin té més decimals.
			if (a.decimales.size() >= b.decimales.size()) {
				for (int i = a.decimales.size(); a_it != a.decimales.rend(); ++a_it) {

					// Si el b encara no té decimals, doncs no sumem.
					if (i == b.decimales.size()) {
						int aInsertar = *a_it + *b_it + residu;

						if (aInsertar >= 10) {
							// Fem que es porti una.
							aInsertar -= 10;
							residu = 1;
						}
						else
						{
							// Fem el residu 0.
							residu = 0;
						}
						// Insertem el resultat just al principi de tots els decimals, com que anem al revés ens anirà perfecte!
						resultat.decimales.insert(resultat.decimales.begin(), aInsertar);
						++b_it;
					}
					else {
						resultat.decimales.insert(resultat.decimales.begin(), *a_it);
						i--;
					}
				}
			}
			else
			{
				// El mateix pero invertit.
				
				for (int i = b.decimales.size(); b_it != b.decimales.rend(); ++b_it) {
					if (i == a.decimales.size()) {
						int aInsertar = *a_it + *b_it + residu;

						if (aInsertar >= 10) {
							aInsertar -= 10;
							residu = 1;
						}
						else
						{
							residu = 0;
						}
						resultat.decimales.insert(resultat.decimales.begin(), aInsertar);
						++a_it;
					}
					else {
						resultat.decimales.insert(resultat.decimales.begin(), *b_it);
						i--;
					}
				}
			}
			
			
			// Finalment sumem els enters
			// No ens hem d'ovlidar del residu!
			resultat.entero = a.entero + b.entero + residu;

			// El signe segueix sent el mateix.
			resultat.signo = a.signo;
		}
		else
		{
			// Son signes oposats.
			// Afegim 0s per tant de poder restar, que ens donin negatius i conseqüentment sumi 1 al residu.
			int decimales = a.decimales.size() - b.decimales.size();
				
			if(decimales > 0) {
				for(int i = 0; i < decimales; i++){
					b.decimales.insert(b.decimales.end(), 0);
				}
				
			} else {
				for(int i = decimales; i < 0; i++){
					a.decimales.insert(a.decimales.end(), 0);
				}
				
			}
			
			// Mirem quin es més gran independentment del signe
			if (MasGrandeQueSinSigno(a, b)) {
				// a es més gran que b
				for(;a_it != a.decimales.rend(); ++a_it, ++b_it) {
					int aInsertar = *a_it - (*b_it + residu);
					
					residu = 0;
					
					if(aInsertar < 0) {
						aInsertar = 10 + aInsertar;
						residu = 1;
					}
					
					resultat.decimales.insert(resultat.decimales.end(), aInsertar);
				}				
				
				resultat.entero = (int)absolut((double)(a.entero - (b.entero + residu)));
				resultat.signo = a.signo;
			}
			else {
				// b es més gran que a
				for(;a_it != a.decimales.rend(); ++a_it, ++b_it) {
					
					int aInsertar = *b_it - (*a_it + residu);
					
					residu = 0;
					
					if(aInsertar < 0) {
						aInsertar = 10 + aInsertar;
						residu = 1;
					}
					
					resultat.decimales.insert(resultat.decimales.end(), aInsertar);
				}	
				
				resultat.entero = (int)absolut((double)(b.entero - (a.entero + residu)));
				resultat.signo = b.signo;
			}
			resultat.decimales.reverse();

		}

		if(truncar)
			resultat.Truncar();
		// Tornem el resultat.
		return resultat;
	}

	
	Num Restar(Num a, Num b, bool view = false) {
		return Sumar(a, InvertirSigno(b), view);
	}

	
	Num Multiplicar(Num a, Num b, bool view = false) {
		// IT'S YOUR TIME.
		Num result = Num();
		
		Num newA = Num(), newB = Num();
		
		newA.decimales = a.decimales;
		newB.decimales = b.decimales;
		
		int comaPosition = 0;
		// Pasem els dos numeros a llistes sense importar l'enter.
		if(a.entero != 0) {
			for(int s = to_string(a.entero).size() - 1; s >= 0; s--) {
				newA.decimales.insert(newA.decimales.begin(), to_string(a.entero)[s] - '0');
			}
		}
		if(b.entero != 0) {
			for(int s = to_string(b.entero).size() - 1; s >= 0; s--) {
				newB.decimales.insert(newB.decimales.begin(), to_string(b.entero)[s] - '0');
			}
		}
		
		comaPosition = a.decimales.size() + b.decimales.size();
			
		
		
		list<int>::reverse_iterator a_it = newA.decimales.rbegin();
		list<int>::reverse_iterator b_it = newB.decimales.rbegin();
		newB.WriteNumber();
		
		Num plusNum = Num();
		int residu = 0;
		
		for(int i = 0; a_it != newA.decimales.rend(); ++a_it, i++) {
			for(;b_it != newB.decimales.rend(); ++b_it) {
				
				int aInsertar = *b_it * *a_it + residu;
				residu = 0;
				
				while(aInsertar >= 10) {
					aInsertar -= 10;
					residu++;
				}
				
				
				plusNum.decimales.insert(plusNum.decimales.end(), aInsertar);
				
			}
			plusNum.decimales.insert(plusNum.decimales.end(), residu);
			residu = 0;
			plusNum.decimales.reverse();
			
			result.decimales.insert(result.decimales.begin(), 0);
			
			plusNum.WriteNumber();
			result.WriteNumber();
			
			result = Sumar(result, plusNum, false);
			
			
			if(result.entero != 0) {
				for(int s = to_string(result.entero).size() - 1; s >= 0; s--) {
					result.decimales.insert(result.decimales.begin(), to_string(result.entero)[s] - '0');
				}
				result.entero = 0;
			}
			result.WriteNumber();
			
			plusNum.ResetNumber();	
			
			
			cout << endl << *a_it << endl << endl;
			b_it = newB.decimales.rbegin();
		}
		
		int enteroResult = 0;
		int enteroPosition = 1;
		
		// Posem la posició de la coma perque comenci desde el principi.
		comaPosition = result.decimales.size() - comaPosition;
		for(int i = comaPosition - 1; i >= 0; i--) {
			enteroPosition = pow(10, i);
			
			enteroResult += *result.decimales.begin() * enteroPosition;
			
			result.decimales.pop_front();
			cout << enteroResult<< endl;
		}
		
		result.entero = enteroResult;
		
		result.signo = a.signo == b.signo;
		
		result.BorrarCeros();
		result.Truncar();
		
		return result;
	}

	
	Num Dividir(Num divisor, Num divident) {
		// First do 1 / divident
		// Then multiply the result by the divisor

		Num result = Num(0);

		// 1 / divident
		result = Multiplicar(divisor, Inverso(divident));

		// Put the sign, if both have the same, the result sign will be positive, if not negative.
		result.signo = divident.signo == divisor.signo;

		return result;
	}

	Num Inverso(Num n) {

		// COMO HAGO ESTOOO :(
		Num result = Num(0);

		result.signo = n.signo;
		//result = 1 / n;

		return result;
	}
};


int main(void) {
	
	Operador o;

	//Num a = Num("-14.854");
	//Num b = Num("12.945");
	
	Num a = Num("1,368914532345158366512");
	Num b = Num("-3.8652394523760741");
	
	//Num a = Num(100);
	//Num b = Num("0,01");
	
	//cout << "Resultado: " << endl;
	o.Multiplicar(a, b).WriteNumber();

	return 0;
}