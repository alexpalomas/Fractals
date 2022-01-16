#include <iostream>
#include <list>
#include <string>
#include <cmath>

using namespace std;

// Xifres de decimals dels nombres.
int truncament = 15;

double absolut(double a) {
	if(a >= 0)
		return a;
	if(a < 0)
		return -a;
}

class Num
{
private:
	void InicialitzarComADouble(double a) {
		enter = floor(absolut(a));
		signe = a >= 0;

		string s = to_string(absolut(a));

		for (size_t c = 2; c < s.size(); c++)
			decimals.insert(decimals.end(), s[c] - '0');
	}


public:
	Num(string number, bool zeros);
	Num(int i);
	Num(float f);
	Num(double d);
	Num();

	int enter;
	list<int> decimals;
	bool signe = true;

	// En aquesta funcio es borren els zeros finals.
	void BorrarZeros() {
		// Comença pel final i si es un zero esborra l'ultima xifra.
		for (auto it = decimals.rbegin(); it != decimals.rend();)
		{
			if (*it == 0) {
				// Esborra la xifra.
				decimals.pop_back();
			}
			else {
				// Deixa d'executar la funcio.
				return;
			}
		}
	}

	// Inicialitzar el nombre amb el tipus string.
	void IniciarNombreAmbCadenaDeCaracters(string s, bool zeros = false) {
		BuidarNumero();
		int inici = 0;
		
		// Si el nombre introduit es negatiu.
		if (s[0] == '-') {
			signe = false;
			inici++;
		}
		
		// En aquesta part es mirara si el nombre introduit te decimals.
		bool teDecimal = false;
		for(int i = 0; i < s.size(); i++) {
			teDecimal = s[i] == '.' || s[i] == ',';
			if(teDecimal)
				break;
		}

		// Si aquest te decimal...
		if(teDecimal) {
			string e_s;

			// Es va emmagatzemant l'enter mentres no s'arribi als decimals.
			while (s[inici] != '.' && s[inici] != ',') {
				e_s += s[inici];
				inici++;
			}
		
			// Es converteix la string en enter.
			enter = stoi(e_s);
			inici++;
			
			// S'insereixen tots els decimals per ordre.
			for (size_t i = inici; i < s.size(); i++)
			{
				decimals.insert(decimals.end(), s[i] - '0');
			}
		} else {
			enter = stoi(s);
			enter = (int)absolut(enter);
		}
		
		if(!zeros)
			BorrarZeros();
	}

	void EscriureElNombre() {
		cout << CadenaDeCaracters() << endl;
	}

	string CadenaDeCaracters() {
		string aEmplenar;

		aEmplenar += signe ? "" : "-";
		aEmplenar += to_string(enter);
		
		if (decimals.size() != 0) {
			aEmplenar += ".";

			for (auto it = decimals.begin(); it != decimals.end(); ++it)
			{
				aEmplenar += to_string(*it);
			}
		}

		return aEmplenar;
	}

	void InvertirSigne() {
		signe = !signe;
	}

	void ImprimirDecimalsUnPerUn() {
		int i = 0;
		cout << "Decimals:" << endl;
		for (auto it = decimals.begin(); it != decimals.end(); ++it, i++)
		{
			cout << i << ": " << *it << endl;
		}
	}

	void BuidarNumero() {
		enter = 0;
		decimals.clear();
		signe = true;
	}
	
	void Truncar(int t = truncament) {
		for(int i = decimals.size(); i > t; i--){
			decimals.pop_back();
		}
	}
};

Num::Num(string number, bool zeros = false) {
	BuidarNumero();

	IniciarNombreAmbCadenaDeCaracters(number, zeros);
	
	Truncar();
}

Num::Num(int i) {
	BuidarNumero();

	enter = (int)absolut(i);
	signe = i >= 0;
	decimals.clear();
	BorrarZeros();
	Truncar();
}

Num::Num() { BuidarNumero(); }

class Operador {

private:
	bool IgualQueSinsigne(Num a, Num b) {
		a.BorrarZeros();
		b.BorrarZeros();	
		
		bool returnable = true;
		returnable = a.enter == b.enter;
		if(returnable){
			returnable = a.decimals == b.decimals;
		}
		return returnable;
	}
	
	bool MasGrandeQueSinsigne(Num a, Num b) {
		if (a.enter != b.enter) {
			return a.enter > b.enter;
		}

		if (a.decimals.size() <= b.decimals.size()) {
			list<int>::iterator b_it = b.decimals.begin();
			for (auto a_it = a.decimals.begin(); a_it != a.decimals.end(); ++a_it, ++b_it)
				if (*a_it != *b_it)
					return *a_it > *b_it;

			return false;
		}

		if (a.decimals.size() > b.decimals.size()) {
			list<int>::iterator a_it = a.decimals.begin();
			for (auto b_it = b.decimals.begin(); b_it != b.decimals.end(); ++a_it, ++b_it)
				if (*a_it != *b_it)
					return *a_it > *b_it;

			return true;
		}

		// Si els dos son iguals.
		return true;
	}

public:	
	
	
	Num Absolut(Num a) {
		a.signe = true;
		return a;
	}
	
	Num InvertirSigne(Num a) {
		a.signe = !a.signe;
		return a;
	}

	bool IgualQue(Num a, Num b) {
		return IgualQueSinsigne(a, b) == (a.signe == b.signe);
	
	}
	
	bool MasGrandeQue(Num a, Num b) {
		if (a.signe == b.signe) {
			if (a.enter != b.enter) {
				if (a.signe)
					return a.enter > b.enter;
				return a.enter < b.enter;
			}

			if (a.decimals.size() <= b.decimals.size()) {
				list<int>::iterator b_it = b.decimals.begin();
				for (auto a_it = a.decimals.begin(); a_it != a.decimals.end(); ++a_it, ++b_it)
					if (*a_it != *b_it) {
						if (a.signe)
							return *a_it > *b_it;
						return *a_it < *b_it;
					}

				return !a.signe;
			}

			if (a.decimals.size() > b.decimals.size()) {
				list<int>::iterator a_it = a.decimals.begin();
				for (auto b_it = b.decimals.begin(); b_it != b.decimals.end(); ++a_it, ++b_it)
					if (*a_it != *b_it) {
						if (a.signe)
							return *a_it > *b_it;
						return *a_it < *b_it;
					}
				return a.signe;
			}

			// Si els dos son iguals
			return true;
		}

		return a.signe;
	}
	
	Num Sumar(Num a, Num b, bool truncar = true) {
		/*
			INFORMACIO QUE M'ARRIVA:

			Els enters i els decimals 1 a 1.

			Llavors, per a fer la suma haurem de:
				1.- Veure quin te mes decimals.
				2.- Sumar els decimals, amb el residu, de la dreta a l'esquerra.
				3.- Sumar els enters, amb el residu.
			
			Nota {
				Es interesant com a la suma simplement també es resta, aixi que aqui tambe es fa la resta.
			}

			Possibilitats de variants de signes:

					|	+	|	-	|
					|		|		|
			|	+	|	+	|	d	|
			|		|		|		|
			|	-	|	d	|	-	|

			d = depen, de què? Doncs de quin sigui el mes gran.
			Si el negatiu es mes gran, de forma absoluta, al positiu, doncs el resultat sera negatiu, en el 
			cas contrari, positiu.


			Exemple:
			  1   1 1 1   1         1 1   1 1
			+ 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			+ 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1
			------------------------------------------------
			+ 5 , 2 3 4 1 5 3 9 8 4 7 2 1 2 3 2 4 6 6 5 1 2


			Exemple:
			  1   1 1 1   1         1 1   1 1
			- 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			- 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1
			------------------------------------------------
			- 5 , 2 3 4 1 5 3 9 8 4 7 2 1 2 3 2 4 6 6 5 1 2
			

			Exemple:
			      1 1 1     1           1   1 1 1 1 1 1 1 
			- 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1 0 0 0 0 0
			+ 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			------------------------------------------------
			- 2 , 4 9 6	3 2	4 9	2 0	0 3	0 9	1 5 7 3	3 4 8 8


			Exemple:
			      1 1 1     1           1   1 1 1 1 1 1 1 
			+ 3 , 8 6 5 2 3 9 4 5 2 3 7 6 0 7 4 1 0 0 0 0 0			
			- 1 , 3 6 8 9 1 4 5 3 2 3 4 5 1 5 8 3 6 6 5 1 2
			------------------------------------------------
			+ 2 , 4 9 6	3 2	4 9	2 0	0 3	0 9	1 5 7 3	3 4 8 8


			Conclusions:
				- En signes oposats, el signe resultatant sera el del mes gran. 
				  En aquest cas s'ha de tenir en compte quin es mes gran que l'altre.
				
				- Amb el mateix signe, es quedara el signe sera el mateix. 
				  No s'haura de tenir en compte quin es mes gran.
		*/

		// Declarem el resultat.
		Num resultat = Num();

		// Declarem el residu fora de tot.
		int residu = 0;
		
		// Com ho hem de fer desde dreta a esquerra, comencem pel final fent servir els iteradors invertits.
		list<int>::reverse_iterator a_it = a.decimals.rbegin(), b_it = b.decimals.rbegin();
		
		// Mirem el signe.
		if (a.signe == b.signe) {
			// Els signes son iguals.
			
			// Comencem la suma:

			// Es mira quin te mes decimals.
			if (a.decimals.size() >= b.decimals.size()) {
				for (int i = a.decimals.size(); a_it != a.decimals.rend(); ++a_it) {

					// Si el b encara no te decimals, doncs no sumem.
					if (i == b.decimals.size()) {
						int aInserir = *a_it + *b_it + residu;

						if (aInserir >= 10) {
							// Fem que es porti una.
							aInserir -= 10;
							residu = 1;
						}
						else
						{
							// Fem el residu 0.
							residu = 0;
						}
						// Insertem el resultat just al principi de tots els decimals, ja que anem del reves.
						resultat.decimals.insert(resultat.decimals.begin(), aInserir);
						++b_it;
					}
					else {
						resultat.decimals.insert(resultat.decimals.begin(), *a_it);
						i--;
					}
				}
			}
			else
			{
				// El mateix pero invertit.
				for (int i = b.decimals.size(); b_it != b.decimals.rend(); ++b_it) {
					if (i == a.decimals.size()) {
						int aInserir = *a_it + *b_it + residu;

						if (aInserir >= 10) {
							aInserir -= 10;
							residu = 1;
						}
						else
						{
							residu = 0;
						}
						resultat.decimals.insert(resultat.decimals.begin(), aInserir);
						++a_it;
					}
					else {
						resultat.decimals.insert(resultat.decimals.begin(), *b_it);
						i--;
					}
				}
			}
			
			
			// Finalment sumem els enters
			// No ens hem d'ovlidar del residu.
			resultat.enter = a.enter + b.enter + residu;

			// El signe segueix sent el mateix.
			resultat.signe = a.signe;
		}
		else
		{
			// Son signes oposats.
			// Afegim 0s per tant de poder restar, que ens donin negatius i consequentment sumi 1 al residu.
			int decimals = a.decimals.size() - b.decimals.size();
				
			if(decimals > 0) {
				for(int i = 0; i < decimals; i++){
					b.decimals.insert(b.decimals.end(), 0);
				}
				
			} else {
				for(int i = decimals; i < 0; i++){
					a.decimals.insert(a.decimals.end(), 0);
				}
				
			}
			
			// Mirem quin es mes gran independentment del signe
			if (MasGrandeQue(Absolut(a), Absolut(b))) {
				// En aquest cas a es mes gran que b.
				for(;a_it != a.decimals.rend(); ++a_it, ++b_it) {
					int aInserir = *a_it - (*b_it + residu);
					
					residu = 0;
					
					if(aInserir < 0) {
						aInserir = 10 + aInserir;
						residu = 1;
					}
					
					resultat.decimals.insert(resultat.decimals.begin(), aInserir);
				}				
				
				resultat.enter = (int)absolut((double)(a.enter - (b.enter + residu)));
				resultat.signe = a.signe;
			}
			else {
				// En aquest cas b es mes gran que a.
				for(;a_it != a.decimals.rend(); ++a_it, ++b_it) {
					
					int aInserir = *b_it - (*a_it + residu);
					
					residu = 0;
					
					if(aInserir < 0) {
						aInserir = 10 + aInserir;
						residu = 1;
					}
					
					resultat.decimals.insert(resultat.decimals.begin(), aInserir);
				}	
				
				resultat.enter = (int)absolut((double)(b.enter - (a.enter + residu)));
				resultat.signe = b.signe;
			}

		}
		
		if(truncar)
			resultat.Truncar();
		
		return resultat;
	}

	
	Num Restar(Num a, Num b) {
		return Sumar(a, InvertirSigne(b));
	}

	
	Num Producte(Num a, Num b) {
		Num resultat = Num();
		Num novaA = Num(), novaB = Num();
		
		resultat.signe = a.signe == b.signe;
		
		novaA.decimals = a.decimals;
		novaB.decimals = b.decimals;
		
		int posicioComa = 0;
		// Pasem els dos numeros a llistes sense importar l'enter.
		if(a.enter != 0) {
			for(int s = to_string(a.enter).size() - 1; s >= 0; s--) {
				novaA.decimals.insert(novaA.decimals.begin(), to_string(a.enter)[s] - '0');
			}
		}
		if(b.enter != 0) {
			for(int s = to_string(b.enter).size() - 1; s >= 0; s--) {
				novaB.decimals.insert(novaB.decimals.begin(), to_string(b.enter)[s] - '0');
			}
		}
		
		posicioComa = a.decimals.size() + b.decimals.size();
			
		list<int>::reverse_iterator a_it = novaA.decimals.rbegin();
		list<int>::reverse_iterator b_it = novaB.decimals.rbegin();
		
		Num nombreASumar = Num();
		int residu = 0;
		
		for(int i = 0; a_it != novaA.decimals.rend(); ++a_it, i++) {
			for(;b_it != novaB.decimals.rend(); ++b_it) {
				
				int aInserir = *b_it * *a_it + residu;
				residu = 0;
				
				while(aInserir >= 10) {
					aInserir -= 10;
					residu++;
				}
				
				
				nombreASumar.decimals.insert(nombreASumar.decimals.begin(), aInserir);
				
			}
			nombreASumar.decimals.insert(nombreASumar.decimals.begin(), residu);
			residu = 0;
			
			resultat.decimals.insert(resultat.decimals.begin(), 0);
			
			resultat = Sumar(resultat, nombreASumar, false);			
			nombreASumar.BuidarNumero();
			
			b_it = novaB.decimals.rbegin();
		}
		
		int resultatEnter = 0;
		int enterPosition = 1;
		
		// Posem la posicio de la coma perque comenci desde el principi.
		posicioComa = resultat.decimals.size() - posicioComa;
		for(int i = posicioComa - 1; i >= 0; i--) {
			enterPosition = pow(10, i);
			
			resultatEnter += *resultat.decimals.begin() * enterPosition;
			
			resultat.decimals.pop_front();
			
		}
		
		
		resultat.enter = resultatEnter;
		
		resultat.BorrarZeros();
		resultat.Truncar();
		
		return resultat;
	}

	
	Num Dividir(Num divisor, Num divident) {
		return Producte(divisor, Inverso(divident));
	}

	Num Inverso(Num n) {
		Num resultat = Num();

		resultat.signe = n.signe;
		// resultat = 1 / n;
		/*
			Farem servir un metode de prova i error.
			Sabent que n es el enter que ens donen i c el nombre que volem. 
			Tenim que 1 / n = c;
			
			Aillem l'1 i ens queda: 
			n * c = 1. 
			
			Llavors:
			Si n > 1:
				c ha de começar per 0, i anar fent.
			Si n < 1:
				c ha de começar per l'1 i cap amunt.
			Si n = 1:
				c = 1 tambe.
				
			Com fem el metde de prova i error?
			Exemple:
				1 / 4 = c;
				
				c * 4 = 1;
				
			Com sabem que 4 es mes gran que 1, comencem pel 0:
			Llavors tenim que:
				
				0,1 * 4 = 0,4; Seguim;
				0,2 * 4 = 0,8; Seguim;
				0,3 * 4 = 1,2; Mes gran que 1, canviem de decimal;
				0,21 * 4 = 0,84; Seguim;
				0,22 * 4 = 0,88; Seguim;
				0,23 * 4 = 0,92; Seguim;
				0,24 * 4 = 0,96; Seguim;
				0,25 * 4 = 1; Trobat!
				
			
		*/
		
		resultat.enter = 1;
		bool sumantEnter = true;
		
		for(int i = 0; i < truncament; i++) {
			
			while(!MasGrandeQue(Absolut(Producte(resultat, n)), Num(1))) {
				if(sumantEnter) {
					resultat.enter += 1;
				}
				else {
					resultat.decimals.back() += 1;
				}
			}
			if(sumantEnter) {
				resultat.enter -= 1;
				sumantEnter = false;	
			}
			else {
				resultat.decimals.back() -= 1;
			}
			
			if(IgualQue(Absolut(Producte(resultat, n)), Num(1))) {
				resultat.Truncar();
				resultat.EscriureElNombre();
				return resultat;
			}
			resultat.decimals.insert(resultat.decimals.end(), 1);
		}
			
		return resultat;
	}
};


int main(void) {
	
	Operador o;
	
	string a_s, b_s;
	
	cin >> a_s;
	cin >> b_s;
	
	Num a = Num(a_s);
	Num b = Num(b_s);
	
	
	cout << endl << a_s << " + " << b_s << " = ";
	o.Sumar(a, b).EscriureElNombre();
	
	cout << endl << a_s << " - " << b_s << " = ";
	o.Restar(a, b).EscriureElNombre();
	
	cout << endl << a_s << " * " << b_s << " = ";
	o.Producte(a, b).EscriureElNombre();
	
	cout << endl << a_s << " / " << b_s << " = ";
	o.Dividir(a, b).EscriureElNombre();
	
	return 0;
}