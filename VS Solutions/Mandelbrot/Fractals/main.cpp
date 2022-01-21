#define _CRT_SECURE_NO_WARNINGS
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "utils.cpp"
#include "Image.cpp"
#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <omp.h>
#include <vector>

using namespace std;
using namespace olc;

const int altura = 1080;
const int allargada = altura * 1.7778;
int iteracions = 100;

double limits_x[2] = { -2.5, 2.5 };
double limits_y[2] = { -1.5, 1.5 };

double suma_x;
double suma_y;

int xCursorAbans;
int yCursorAbans;

bool actualitzar = false;

float* fractal = new float[allargada * altura];
bool dibuixarIteracions = false;

float llindar = 4;

NewColor colorFractal = NewColor(1, 1, 1);


void MostrarLimits() {
	cout << endl << "Limits actuals:" << endl;
	cout << "\t" << "Minim X: " << limits_x[0] << endl;
	cout << "\t" << "Maxim X: " << limits_x[1] << endl;
	cout << "\t" << "Minim Y: " << limits_y[0] << endl;
	cout << "\t" << "Maxim Y: " << limits_y[1] << endl;
}


vd2d CursorACoordenadesFractals(vi2d CoordenadesCursor, float limit_x_0 = limits_x[0], float limit_x_1 = limits_x[1], float limit_y_0 = limits_y[0], float limit_y_1 = limits_y[1]) {
	double normalizedX = (double)CoordenadesCursor.x / (double)allargada;
	double normalizedY = (double)CoordenadesCursor.y / (double)altura;

	double espacioX = absolut((double)limit_x_0 - (double)limit_x_1);
	double espacioY = absolut((double)limit_y_0 - (double)limit_y_1);

	return vd2d(limit_x_0 + normalizedX * espacioX, limit_y_1 - normalizedY * espacioY);
}

vi2d FractalACoordenadesCursor(vd2d CoordenadesFractals, float limit_x_0, float limit_x_1, float limit_y_0, float limit_y_1) {
	// El mateix que a CursorACoordenadesFractals però invers.
	return vi2d((CoordenadesFractals.x - limit_x_0) * (double)allargada / absolut((double)limit_y_0 - (double)limit_y_1), (-CoordenadesFractals.y + limit_y_1) * (double)altura / (absolut((double)limit_y_0 - (double)limit_y_1)));
}


double MultiplicacioX(double x, double y, double cr) {
	x = x * x - y * y + cr;											// f(z) = z^2 + c
	//x = x * x - y * y + -(x) + cr;									// f(z) = z^2 - z + c
	//x = x * x * x + (-3) * x * y * y + cr;							// f(z) = z^3 + c
	//x = (x * x * x - 3 * x * y * y) + (x * x - y * y) + cr;			// f(z) = z^3 + z^2 + c
	//x = x * x * x * x + y * y * y * y - 6 * x * x * y * y + cr;		// f(z) = z^4 + c
	return x;
}

double MultiplicacioY(double x, double y, double ci) {
	y = 2 * y * x + ci;												// f(z) = z^2 + c
	//y = 2 * y * x - y + ci;											// f(z) = z^2 - z + c
	//y = -y * y * y + 3 * x * x * y + ci;								// f(z) = z^3 + c
	//y = (-y * y * y + 3 * x * x * y) + (2 * y * x) + ci;				// f(z) = z^3 + z^2 + c
	//y = 4 * x * x * x * y - 4 * x * y * y * y + ci;					// f(z) = z^4 + c
	return y;
}

// Aquí es fa la màgia.
double iterate(int n, double cr, double ci, float threshold) {
	// Com que hem de començar amb z_0 = 0:
	double x = 0;
	double y = 0;

	for (int k = 0; k < n; k++)
	{
		double x_t = x;

		x = MultiplicacioX(x, y, cr);
		y = MultiplicacioY(x_t, y, ci);

		// Mirem si s'escapa
		if (x * x + y * y >= threshold) {
			return (double)k/(double)n;
		}
	}

	// Si ha arribat a aquest punt no s'ha escapat.
	return 1;
}

// Aquí comencem la representació gràfica.
// Algunes coses son pura sintaxis.
class RepresentacioFractal : public PixelGameEngine
{
	// Pura sintaxis.
public:
	RepresentacioFractal()
	{
		sAppName = "Conjunt de Mandelbrot";
	}

public:
	byte runningFrames;

	// Aquest és la posició on ens trobem de tots els límits.
	// Quan ens apropem, creem uns nous límits, per tant, sumem 1 a aquesta variable.
	int nivell = 0;

	// Creem una estructura per almegatzemar els límits en una sola variable.
	struct limit
	{
		double x_0, x_1, y_0, y_1;
		limit(double x0, double x1, double y0, double y1) {
			x_0 = x0;
			x_1 = x1;
			y_0 = y0;
			y_1 = y1;
		}
	};
	// Aquesta serà la nostra llista de límits; vector<> funciona com una llista.
	vector<limit> limits;

	// Aquí borrarem els límits al crear un nou límit si estem enrera.
	void EsborrarLimitsAdelantats() {
		size_t midaLimits = limits.size() - 1;

		for (size_t i = nivell; i < midaLimits; i++)
		{
			// Borrem l'últim.
			limits.pop_back();
		}
	}

	// Actualitzarem els límits globals.
	void ActualitzarLimits() {

		if (nivell > limits.size() - 1) {
			nivell = limits.size() - 1;
		}
		else if (nivell < 0)
		{
			nivell = 0;
		}

		limits_x[0] = limits[nivell].x_0;
		limits_x[1] = limits[nivell].x_1;
		limits_y[0] = limits[nivell].y_0;
		limits_y[1] = limits[nivell].y_1;
		MostrarLimits();
		ActualitzarFractal();
	}

	bool OnUserCreate() override
	{
		/*	Aquí farem que els límits tinguin una relació d'aspecte de 16:9.
		*	absolut(limits_x[0] - limits_x[1]) / absolut(limits_y[0] - limits_y[1]) = 16 / 9
		* 	(16/9) / absolut(limits_x[0] - limits_x[1]) = 1 / absolut(limits_y[0] - limits_y[1])
		*	(9/16) * absolut(limits_x[0] - limits_x[1]) = absolut(limits_y[0] - limits_y[1])
		*
		*	Com sabem que els límits_y han de ser inversos (raons estètiques) => limits_y[1] = -limits_y[0]
		*	(9/16) * absolut(limits_x[0] - limits_x[1]) = absolute(2 * limits_y[0])
		*	(9/16) * absolut(limits_x[0] - limits_x[1]) / 2 = limits_y[0]
		*/
		limits_y[1] = (float)9 / (float)16 * absolut(limits_x[0] - limits_x[1]) / (float)2;
		limits_y[0] = -limits_y[1];

		limit començament = limit(limits_x[0], limits_x[1], limits_y[0], limits_y[1]);
		// El posem dins dels límits.
		limits.insert(limits.end(), començament);
		ActualitzarLimits();

		runningFrames = 0;
		MostrarLimits();
		ActualitzarFractal();

		return true;
	}

	vi2d posicioCursorInicial;
	vi2d posicioCursorFinal;

	// Variables útils a l'hora de fer click per saber que fa un punt.
	double actualX;
	double actualY;
	double inicialX;
	double inicialY;
	int iter = 0;

	bool OnUserUpdate(float fElapsedTime) override
	{
		runningFrames += 1;
		
		actualitzar = RevisarBotonsPresionats();

		if (dibuixarIteracions && !actualitzar) {
			if (iter <= iteracions) {
				// Aquí fem la iteració del punt en sí i la dibuixem.
				// El que faré serà crear una variable on ens dirà on està el punt pero en coordenades del fractal.
				// Tenint aquesta informació podem començar a iterar, així doncs per cada iteració dibuixarem un cercle
				//	 on es troba i una linea de l'anterior a aquest.
				// L'anterior el posarem a la mateixa variable inicial ja uq ja no serà necessària un cop feta servir inicialment.

				// Calculem les altres iteracions.
				// La derivada eren probes que vaig fer.
				double derivadaX, derivadaY;
				double x_t = actualX;
				actualX = MultiplicacioX(actualX, actualY, inicialX);
				actualY = MultiplicacioY(x_t, actualY, inicialY);

				derivadaX = actualX;
				derivadaY = actualY;

				//cout << "Derivada X: " << derivadaX << endl << "Derivada Y: " << derivadaY << endl;
				//cout << "Módul: " << sqrt(derivadaX * derivadaX + derivadaY * derivadaY) << endl;
				if (actualX * actualX + actualY * actualY > 4) {
					iter = iteracions;
				}
				// Dibuixem el cercle.
				DrawCircle(FractalACoordenadesCursor(vd2d(actualX, actualY) - CursorACoordenadesFractals(posicioCursorInicial), -4.0f, 4.0f, -4.0f, 4.0f), 3);
				//cout << "Current X: " << actualX << endl << "Current Y: " << actualY << endl;
				//cout << "Derivada X: " << derivadaX << endl << "Derivada Y: " << derivadaY << endl;
				// Fem que el programa es pausi 10 milisegons per aque així no es posarà tot de cop.
				Sleep(10);
				iter++;
				return true;
			}
			// Ja no dibuixem més iteracions.
			dibuixarIteracions = false;
		}
		

		// Aquí dibuixarem el quadradet que surt quan ens volem apropar.
		if (actualitzarFractalCadaFotograma && runningFrames % 2 == 0) {
			// Tornem a dibuixar tota la pantalla per borrar el quadrat anterior.
			DibuixarFractal(false);
			
			// Aquí tindrem els 4 punts del rectangle.
			// Un vi2d es un vecotor int 2 dimensions, és a dir -> és el mateix que un punt com ara (2, -5).
			vi2d rectangle[2] = {
				posicioCursorInicial,
				vi2d()
			};
			
			// Simples comprobacions per saber com tractar les dades que ens han arribat (explico una perque son tot el mateix canviant poques coses).
			switch (posicioCursorInicial.x > GetMousePos().x)
			{
			case true:
				if (posicioCursorInicial.y > GetMousePos().y) {
					// La posició x del rectangle final serà la posició final del cursor x. En canvi, l'altura serà l'aïllament de posició_y en:	allargada / altura = posició_x / posició_y 
					// Es fa per preservar la relaciño d'apecte 16:9.
					rectangle[1] = vi2d(GetMousePos().x - posicioCursorInicial.x, (float)altura / (float)allargada * (GetMousePos().x - posicioCursorInicial.x));
				}
				else {
					rectangle[1] = vi2d(GetMousePos().x - posicioCursorInicial.x, (float)altura / (float)allargada * (-GetMousePos().x + posicioCursorInicial.x));
				}
				break;
			case false:
				if (posicioCursorInicial.y < GetMousePos().y) {
					rectangle[1] = vi2d(GetMousePos().x - posicioCursorInicial.x, (float)altura / (float)allargada * (GetMousePos().x - posicioCursorInicial.x));
				}
				else {
					rectangle[1] = vi2d(GetMousePos().x - posicioCursorInicial.x, (float)altura / (float)allargada * (-GetMousePos().x + posicioCursorInicial.x));
				}
				break;
			}
			
			// Dibuixem el rectangle en sí.
			DrawRect(rectangle[0], rectangle[1], Pixel(200, 200, 200));

			return true;
		}

		if (!actualitzar) return true;

		auto begin = chrono::high_resolution_clock::now();

		ActualitzarFractal();


		actualitzar = false;

		auto end = chrono::high_resolution_clock::now();
		auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);

		float elapsedTime = elapsed.count() * 1e-9;
		cout << "L'actualització ha durat: " << elapsedTime << " segons. A " << iteracions << " iteracions." << endl;
		// Percentatge:
		if (!(lastElapsedTime == 0)) {
			float plusPercentage = 100 * elapsedTime / lastElapsedTime - 100;

			char whattosay = (plusPercentage > 0) ? '+' : '-';
			cout << whattosay;
			cout << absolut(plusPercentage) << "%." << endl;
		}
		lastElapsedTime = elapsedTime;

		MostrarLimits();

		return true;
	}

private:
	float lastElapsedTime = 0;
	vi2d posicioInicialCursor, posicioFinalCursor;
	bool actualitzarFractalCadaFotograma = false;

	void StartIterate() {
		DibuixarFractal(false);
		
		iter = 0;
		actualX = 0;
		actualY = 0;
		inicialX = CursorACoordenadesFractals(posicioInicialCursor).x;
		inicialY = CursorACoordenadesFractals(posicioInicialCursor).y;

		// Dibuixem un cercle on hem clicat.
		DrawCircle(posicioInicialCursor, 3, olc::RED);

		dibuixarIteracions = true;
	}


	bool RevisarBotonsPresionats() {
		bool anyInput = false;

		if (GetKey(olc::H).bPressed) {
			// Per si es necessita ajuda amb totes les posibilitats que tens. Està en anglés per fer-me l'interessant.
			cout << endl;
			cout << "Iterations edit commands." << endl;
			cout << "\tNumPad+ or S: Add 10 iteracions;" << endl;
			cout << "\tNumPad- or R: Substract 10 iteracions;" << endl;
			cout << "\tNumPad* or M: Multiply by 2 the iteracions;" << endl;
			cout << "\tNumPad/ or D: Divide by 2 the iteracions." << endl;
			cout << endl;
			cout << "Iterate points." << endl;
			cout << "\tI: Introduce number of iteracions;" << endl;
			cout << "\tP: Introduce mouse coordenades to iterate;" << endl;
			cout << "\tMouse Left Button Click: Iterate that point in specific." << endl;
			cout << endl;
			cout << "Movement." << endl;
			cout << "\tThe program is made to match the aspect-ratio of 16:9 everytime." << endl;
			cout << "\tHeld Mouse Left Button and drag it wherever you want." << endl;
			cout << "\tReleased to calculate the actualitzard fractal." << endl;
			cout << "\tLeft-arrow: back to the previous bounds." << endl;
			cout << "\tRight-arrow: forward to the bounds." << endl;
			cout << endl;
			cout << "Have fun!" << endl;
			cout << endl;
		}

		// Moure's pels límits.
		if (GetKey(olc::LEFT).bReleased && nivell != 0) {
			cout << "Going backwards..." << endl;
			nivell--;
			ActualitzarLimits();
		}
		if(GetKey(olc::RIGHT).bReleased && nivell != limits.size()-1)
		{
			cout << "Going forward..." << endl;
			nivell++;
			ActualitzarLimits();
		}

		// Fixar les iteracions al valor que volguem.
		if (GetKey(olc::I).bReleased) {

			int i;
			cout << "Introdueix el nomre d'iteracions:" << endl;
			cin >> i;
			iteracions = i;
			anyInput = true;
		}

		// Iterar un sol punt amb coordenades del cursor per saber que fa.
		if (GetKey(olc::P).bReleased) {
			// Aquí és on té sentit aquest tros de codi.
			string x, y;
			cout << "Introdueix les coordenades:" << endl;
			cout << "X: ";
			cin >> x;
			cout << "Y: ";
			cin >> y;
			posicioInicialCursor = vi2d(stoi(x), stoi(y));

			StartIterate();
		}


		// Aquí podrem guardar imatges de la fractal!
		if (GetKey(olc::ENTER).bPressed) {
			int imageHeight = altura;
			int imageWidth = allargada;
			Image fractalImage(allargada, altura);

			// Si volem guardar més d'una imatge el nom ha de ser únic, per tant ens assegurarem de que així sigui.
			// Podriem també passar tota la imatge per l'algoritme sha256() però ens quedaría un nom irreconeixible, encara que així evitariem imatges duplicades.
			auto end = std::chrono::system_clock::now();
			std::time_t end_time = std::chrono::system_clock::to_time_t(end);
			string date = std::ctime(&end_time);

			for (int c = 0; c < date.size(); c++)
			{
				if (date[c] == ' ')
					date[c] = '_';
				if (date[c] == ':' || date[c] == '\n')
					date[c] = '-';
			}
			string path = "Captured_On_" + date + ".bmp";
			
			int y_f = imageHeight;
			// Aquí fem la imatge en sí.s
			for (int y = 0; y < imageHeight; y++)
			{
				for (int x = 0; x < imageWidth; x++)
				{
					int index = x * imageHeight + y_f;
					//fractalImage.SetColor(NewColor(1 - fractal[index] * (float)0 / (float)255, 1 - fractal[index] * (float)92/(float)255, 1 - fractal[index] * (float)139 / (float)255), x, y);
					fractalImage.SetColor(NewColor(colorFractal.r * (log10(fractal[index] + 0.1) + 1), colorFractal.g * (log10(fractal[index] + 0.1) + 1), colorFractal.b * (log10(fractal[index] + 0.1) + 1)), x, y);
				}
				y_f -= 1;
			}
			
			// Guardem la imatge.
			fractalImage.Export(path.c_str());
		}

		// Canviar les iteracions.
		if (GetKey(NP_ADD).bReleased || GetKey(S).bReleased) {
			iteracions += 10;
			anyInput = true;
		}
		if (GetKey(NP_SUB).bReleased || GetKey(R).bReleased) {
			iteracions -= 10;
			anyInput = true;
		}
		if (GetKey(NP_MUL).bReleased || GetKey(M).bReleased) {
			iteracions *= 2;
			anyInput = true;
		}
		if (GetKey(NP_DIV).bReleased || GetKey(D).bReleased) {
			iteracions /= 2;
			anyInput = true;
		}

		// Entrada del ratolí.
		// Botó dret.
		if (GetMouse(0).bPressed) {
			dibuixarIteracions = false;
			posicioInicialCursor = GetMousePos();
			cout << "Entrada de Cursor.\n\tComencem a: " << posicioInicialCursor << endl;
			posicioCursorInicial = GetMousePos();
			actualitzarFractalCadaFotograma = true;
		}
		if (!GetMouse(0).bHeld) {
			actualitzarFractalCadaFotograma = false;
		}
		if (GetMouse(0).bReleased) {

			posicioFinalCursor = GetMousePos();

			// Creem un rectangle per saber on està cada punt i tenir-ho a una sola variable. 
			vi2d rectangle[2] = {
				posicioInicialCursor,
				vi2d(posicioFinalCursor.x - posicioInicialCursor.x, (float)altura / (float)allargada * (posicioFinalCursor.x - posicioInicialCursor.x))
			};

			// Corregim la posició final, depenent de la posició final del cursor per si estar més a dalt, més a baix, més a l'esquerra o més a la dreta.
			vi2d endedCorrectedMousePosition = vi2d();
			switch (posicioInicialCursor.x > posicioFinalCursor.x)
			{
			case true:
				if (posicioInicialCursor.y > posicioFinalCursor.y) {
					endedCorrectedMousePosition = vi2d(posicioFinalCursor.x, (posicioFinalCursor.x - posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				else {
					endedCorrectedMousePosition = vi2d(posicioFinalCursor.x, (-posicioFinalCursor.x + posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				break;
			case false:
				if (posicioInicialCursor.y < posicioFinalCursor.y) {
					endedCorrectedMousePosition = vi2d(posicioFinalCursor.x, (posicioFinalCursor.x - posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				else {
					endedCorrectedMousePosition = vi2d(posicioFinalCursor.x, (-posicioFinalCursor.x + posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				break;
			}
			// Diem on ha acabat la posició corregida.
			cout << "\tEnded at : " << endedCorrectedMousePosition << endl;

			// Dibuixem un últim rectangle.
			DrawRect(rectangle[0], rectangle[1], Pixel(255, 105, 105));

			// Mirem si el quadrat té una distancia suficient, si no llavors serà que hem pulsat per mirar que ha fet el lloc inicial.
			if ((posicioInicialCursor - posicioFinalCursor).mag2() < 900) {
				// Comencem a iterar el lloc inicial.
				StartIterate();
				return anyInput;
			}

			// Aquí ja tenim les posicions finals passades a coordenades del fractal.
			vd2d fractalCoordenadesRectangle[2] = {
				CursorACoordenadesFractals(rectangle[0]),
				CursorACoordenadesFractals(endedCorrectedMousePosition)
			};

			// Aquí, depenent de la posició final del cursor per si estar més a dalt, més a baix, més a l'esquerra o més a la dreta, actualitzem els límits globals.
			switch (posicioInicialCursor.x > posicioFinalCursor.x)
			{
			case true:
				limits_x[0] = fractalCoordenadesRectangle[1].x;
				limits_x[1] = fractalCoordenadesRectangle[0].x;
				if (posicioInicialCursor.y < posicioFinalCursor.y) {
					limits_y[0] = fractalCoordenadesRectangle[1].y;
					limits_y[1] = fractalCoordenadesRectangle[0].y;
				}
				else {
					limits_y[0] = fractalCoordenadesRectangle[0].y;
					limits_y[1] = fractalCoordenadesRectangle[1].y;
				}
				break;
			case false:
				limits_x[0] = fractalCoordenadesRectangle[0].x;
				limits_x[1] = fractalCoordenadesRectangle[1].x;
				if (posicioInicialCursor.y < posicioFinalCursor.y) {
					limits_y[0] = fractalCoordenadesRectangle[1].y;
					limits_y[1] = fractalCoordenadesRectangle[0].y;
				}
				else {
					limits_y[0] = fractalCoordenadesRectangle[0].y;
					limits_y[1] = fractalCoordenadesRectangle[1].y;
				}
				break;
			}

			// Actualitzem els límits.
			EsborrarLimitsAdelantats();
			// Afagirem a la llista de límits els nous.
			limits.insert(limits.end(), limit(limits_x[0], limits_x[1], limits_y[0], limits_y[1]));
			// Sumarem un nivell.
			nivell++;

			MostrarLimits();

			anyInput = true;
		}

		return anyInput;
	}

	void ActualitzarFractal() {
		auto begin = chrono::high_resolution_clock::now();

		// La suma surt de saber quina allargada o altura fractal hi ha i la dividim entre l'allargada o l'altura de l'aplicació.
		suma_x = (absolut(limits_x[0] - limits_x[1])) / (double)allargada;
		suma_y = (absolut(limits_y[0] - limits_y[1])) / (double)altura;

		// Posició inicial als límits inicials.
		double posicio[2] = { limits_x[0], limits_y[1] };

		// Recorrarem tota l'allargada de la pantalla punt per punt.
		for (int x = 0; x < allargada; x++)
		{
			// Recorrarem tota l'altura de la pantalla punt per punt en cada punt x.
			for (int y = 0; y < altura; y++)
			{
				// Iterem per aconseguir el valor, si està o no està a la fractal.
				double value = iterate(iteracions, posicio[0], posicio[1], llindar);
				// Guardarem el valor.
				fractal[altura * x + y] = value;
				// Actualitzarem la posició y restant-li la suma_y.
				posicio[1] = limits_y[1] - suma_y * y;
			}
			// Actualitzem la posició x sumant-li el que pertoca.
			posicio[0] = limits_x[0] + suma_x * x;
		}

		auto end = chrono::high_resolution_clock::now();
		auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
		float elapsedTime = elapsed.count() * 1e-9;
		cout << "Update elapsed: " << elapsedTime << " seconds." << endl;

		// Dibuixem la fractal.
		DibuixarFractal(true);
	}

	// Aquí es dibuixarà la fractal.
	void DibuixarFractal(bool time) {
		// Per saber el temps que ha durat el pintar.
		chrono::steady_clock::time_point begin;
		if (time)
			begin = chrono::high_resolution_clock::now();

		// Aquesta línia és important, ens permet paral·lelitzar el pintar la fractal.
		//#pragma omp parallel for
		for (int x = 1; x < allargada; x++) {		// Recorrerem cada punt de la pantalla per pintar-hi el valor corresponent
			for (int y = altura; y > 0; y--) {	// de la fractal.
				int index = altura * x + y;
				// Pintarem cada punt.
				//Draw(x, y, Pixel(255-fractal[index] * 10, 255-fractal[index] * 92, 255-fractal[index] * 139));
				Draw(x - 1, y - 1, Pixel(absolut((colorFractal.r * (log10(fractal[index] + 0.1)) * 255) - 255), absolut((colorFractal.g * (log10(fractal[index] + 0.1)) * 255) - 255), absolut((colorFractal.b * (log10(fractal[index] + 0.1)) * 255) - 255)));
			}
		}

		if (time) {
			// Per saber el temps trascorregut.
			auto end = chrono::high_resolution_clock::now();
			auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
			float elapsedTime = elapsed.count() * 1e-9;
			cout << "The drawing elapsed: " << elapsedTime << " seconds." << endl;
		}
	}
};

int main()
{
	// Crearem una Representació.
	RepresentacioFractal principal;
	// La "Construirem" i si ha anat bé la construció, és a dir, sempre, començarem el programa.
	if (principal.Construct(allargada, altura, 1, 1, true))
		principal.Start();

	return 0;
}