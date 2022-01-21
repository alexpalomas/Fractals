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
#define PI 3.14159265358979323846

using namespace std;
using namespace olc;

const int altura = 1080;
const int allargada = altura * 1.7778;
int iteracions = 40;

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

// −0.8 + 0.156i
float cReal = -0.8, cImaginaria = 0.156;
bool animar = true;

float velocitat = 1;

class Complex{
public:

	Complex();
	~Complex();

	bool polar;
	float M;
	float theta;

	float x;
	float y;

	void A_Polar() {
		if (polar)
			return;

		polar = true;
		M = sqrtf(pow(x, 2) + pow(y, 2));

		theta = atan2f(y, x);
	}

	void A_Cartesiana() {
		if (!polar)
			return;

		polar = false;
		float tan = tanf(theta);
		if (absolut(tan) < 0.0000001) {
			y = 0;
		}
		else
		{
			// No demostraré aquesta formula.
			y = sqrtf(pow(M, 2) / (1 + 1 / pow(tan, 2)));
		}

		x = sqrtf(pow(M, 2) - pow(y, 2));

		if (theta > PI) {
			theta -= 2 * PI;
		}
		else if (theta < -PI) {
			theta += 2 * PI;
		}

	
		// Veiem a quin quadrant està i ho arreglem.
		if (theta >= 0 && theta <= PI / 2) {
			x = x;		// Primer quadrant.
			y = y;
		}
		else if (theta > PI / 2 && theta <= PI) {
			x = -x;		// Segon quadrant.
			y = y;
		}
		else if (theta >= -PI && theta <= -PI / 2) {
			x = -x;		// Tercer quadrant.
			y = -y;
		}
		else if(theta > -PI / 2 && theta < 0) {
			x = x;		// Quart quadrant.
			y = -y;
		}
	}

};

// Pura sintaxis.
Complex::Complex(){}
Complex::~Complex(){}

// Complex que rotarem.
Complex c;


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

	double espacioX = absolut(limit_x_0 - limit_x_1);
	double espacioY = absolut(limit_y_0 - limit_y_1);

	return vd2d(limit_x_0 + normalizedX * espacioX, limit_y_1 - normalizedY * espacioY);
}

vi2d FractalACoordenadesCursor(vd2d CoordenadesFractals, float limit_x_0, float limit_x_1, float limit_y_0, float limit_y_1) {
	// El mateix que a CursorACoordenadesFractals però invers.
	return vi2d((CoordenadesFractals.x - limit_x_0) * (double)allargada / (absolut(limit_x_0 - limit_x_1)), (-CoordenadesFractals.y + limit_y_1) * (double)altura / (absolut(limit_y_0 - limit_y_1)));
}


double MultiplicacioX(double x, double y, double cr) {
	//x = x * x - y * y + cr;											// f(z) = z^2 + c
	//x = x * x - y * y + -(x) + cr;									// f(z) = z^2 - z + c
	//x = x * x * x + (-3) * x * y * y + cr;							// f(z) = z^3 + c
	//x = (x * x * x - 3 * x * y * y) + (x * x - y * y) + cr;			// f(z) = z^3 + z^2 + c
	//x = x * x * x * x + y * y * y * y - 6 * x * x * y * y + cr;		// f(z) = z^4 + c
	x = (x * x * x + (-3) * x * y * y) - y * 0.1008317508132964 + 1.004954206930806 * x;
	return x;
}

double MultiplicacioY(double x, double y, double ci) {
	//y = 2 * y * x + ci;												// f(z) = z^2 + c
	//y = 2 * y * x - y + ci;											// f(z) = z^2 - z + c
	//y = -y * y * y + 3 * x * x * y + ci;								// f(z) = z^3 + c
	//y = (-y * y * y + 3 * x * x * y) + (2 * y * x) + ci;				// f(z) = z^3 + z^2 + c
	//y = 4 * x * x * x * y - 4 * x * y * y * y + ci;					// f(z) = z^4 + c
	y = (-y * y * y + 3 * x * x * y) + 1.004954206930806 * y + 0.1008317508132964 * x;
	return y;
}
//z ^ 3 + z * (0.1008317508132964 * i + 1.004954206930806)



// Aquí es fa la màgia.
double Iterar(int n, double cr, double ci, float llindar) {
	// Com que hem de començar amb z_0 = punt:
	double x = cr;
	double y = ci;

	for (int k = 0; k < n; k++)
	{
		double x_t = x;

		x = MultiplicacioX(x, y, c.x);
		y = MultiplicacioY(x_t, y, c.y);

		// Mirem si s'escapa
		if (x * x + y * y >= llindar) {
			return (double)k / (double)n;
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
		sAppName = "Conjunt de Julia";
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

		return true;
	}

	vi2d posicioCursorInicial;
	vi2d posicioCursorFinal;


	// Variables útils a l'hora de fer click per saber que fa un punt. //
	double actualX;
	double actualY;
	double inicialX;
	double inicialY;
	int iter = 0;


	double totalTime = 0;
	bool OnUserUpdate(float fElapsedTime) override
	{
		//totalTime += fElapsedTime;
		runningFrames += 1;
		
		actualitzar = RevisarBotonsPresionats();

		if (animar) {
			bool stop = false;
			// Fem que giri.
			c.theta += fElapsedTime * velocitat;
			//c.theta += PI / (double)180 * velocitat;

			if (totalTime >= 2 * PI) {
				stop = true;
			}

			totalTime += PI / (double)180;

			// La normalitzem -> estalvi de problemes.
			if (c.theta >= 2 * PI) {
				c.theta -= 2 * PI;
			}

			// La passem a cartesiana peque ens interesa la x i la y.
			c.A_Cartesiana();
			// La tornem a passar a polar per després poder sumar-hi a la theta.
			c.A_Polar();

			// Aquí podriem saber en tot moment la C.
			//cout << "C: (" << c.x << ", " << c.y << ")" << endl;
			//if(!stop) cout << "C: " << c.M << "e^(" << c.theta << " * i)" << endl;

			
			/*if (totalTime >= 4 * PI)
				stop = true;				// ESTADÍSTIQUES.
			*/
			ActualitzarFractal(stop);
			// Tornem que tot ha anat bé perque no volem seguir.
			return true;
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

		// Actualitzar dependrà de les entrades de teclat i ratolí, com hem vist a d'alt, línia 285.
		if (actualitzar && !animar) {
			ActualitzarFractal(false);
			MostrarLimits();
		}

		return true;
	}

private:
	float lastElapsedTime = 0;
	vi2d posicioInicialCursor, posicioFinalCursor;
	bool actualitzarFractalCadaFotograma = false;

	void ComençarAIterar() {
		DibuixarFractal(false);
		
		iter = 0;
		actualX = 0;
		actualY = 0;
		inicialX = CursorACoordenadesFractals(posicioInicialCursor).x;
		inicialY = CursorACoordenadesFractals(posicioInicialCursor).y;

		dibuixarIteracions = true;
	}


	bool RevisarBotonsPresionats() {
		bool anyInput = false;

		if (GetKey(olc::P).bReleased) {
			// Pausem o reanudem l'animació.
			animar = !animar;

			// En una altra fractal això té sentit.
			/*string x, y;
			cout << "Introdueix les coordenades:" << endl;
			cout << "X: ";
			cin >> x;
			cout << "Y: ";
			cin >> y;
			startingMousePosition = vi2d(stoi(x), stoi(y));
			StartIterate();*/
		}

		if (GetKey(olc::H).bPressed) {
			// Per si es necessita ajuda amb totes les posibilitats que tens. Està en anglés per fer-me l'interessant.
			cout << endl;
			cout << "Animation." << endl;
			cout << "\tP: Pause or Reanude the animation. With the fractal paused you can use movement." << endl;
			cout << "\tV: Faster." << endl;
			cout << "\tB: Slower." << endl;
			cout << "Iterations edit commands." << endl;
			cout << "\tNumPad+ or S: Add 10 iterations;" << endl;
			cout << "\tNumPad- or R: Substract 10 iterations;" << endl;
			cout << "\tNumPad* or M: Multiply by 2 the iterations;" << endl;
			cout << "\tNumPad/ or D: Divide by 2 the iterations." << endl;
			cout << endl;
			cout << "Iterate points." << endl;
			cout << "\tI: Introduce number of iterations;" << endl;
			cout << endl;
			cout << "Movement." << endl;
			cout << "\tThe program is made to match the aspect-ratio of 16:9 everytime." << endl;
			cout << "\tHeld Mouse Left Button and drag it wherever you want." << endl;
			cout << "\tReleased to calculate the updated fractal." << endl;
			cout << "\tLeft-arrow: back to the previous bounds." << endl;
			cout << "\tRight-arrow: forward to the bounds." << endl;
			cout << "Other." << endl;
			cout << "\tMake an image! -> NumPad-Enter or Enter.";
			cout << endl;
			cout << "Have fun!" << endl;
			cout << endl;
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

		if (GetKey(olc::V).bPressed) {
			velocitat += 0.1f;
			cout << "VELOCITY: " << velocitat << endl;
		}
		if (GetKey(olc::B).bPressed) {
			if (velocitat - 0.1 <= 0)
				velocitat -= 0.01;
			else
				velocitat -= 0.1f;

			cout << "VELOCITY: " << velocitat << endl;
		}

		// Aquí podrem guardar imatges de la fractal!
		if (GetKey(olc::ENTER).bPressed) {
			cout << "M: " << c.M << endl;
			cout << "Theta: " << c.theta << endl;
			cout << "X: " << c.x << endl;
			cout << "Y: " << c.y << endl;

			int imageHeight = altura;
			int imageWidth = allargada;
			Image fractalImage(allargada, altura);

			// Si volem guardar més d'una imatge el nom ha de ser únic, per tant ens assegurarem de que així sigui.
			// Podriem també passar tota la imatge per l'algoritme sha256() però ens quedaría un nom irreconeixible, encara que així evitariem imatges duplicades.
			auto temps = std::chrono::system_clock::now();
			std::time_t temps_a_temps = std::chrono::system_clock::to_time_t(temps);
			string data = std::ctime(&temps_a_temps);
			for (int c = 0; c < data.size(); c++)
			{
				if (data[c] == ' ')
					data[c] = '_';
				if (data[c] == ':' || data[c] == '\n')
					data[c] = '-';
			}
			string path = "Captured_On_" + data + ".bmp";

			int y_f = imageHeight;
			// Aquí fem la imatge en sí.s
			for (int y = 0; y < imageHeight; y++)
			{
				for (int x = 0; x < imageWidth; x++)
				{
					int index = x * imageHeight + y_f;
					fractalImage.SetColor(NewColor(fractal[index], fractal[index], fractal[index]), x, y);
				}
				y_f -= 1;
			}

			// Guardem la imatge.
			fractalImage.Export(path.c_str());
		}

		// Si estem animant ja no es necessari seguir.
		if (animar)
			return anyInput;

		// Aquí podem fixar la C al valor que volguem!
		if (GetKey(olc::C).bReleased) {
			float x, y;
			cout << "X: ";
			cin >> x;
			cout << "Y: ";
			cin >> y;

			c.x = x;
			c.y = y;
			c.polar = false;
			c.A_Polar();
			anyInput = true;
		}

		// Moure's pels límits.
		if (GetKey(olc::LEFT).bReleased && nivell != 0) {
			cout << "Anant enrrera..." << endl;
			nivell--;
			ActualitzarLimits();
		}
		if(GetKey(olc::RIGHT).bReleased && nivell != limits.size()-1)
		{
			cout << "Anant cap endevant..." << endl;
			nivell++;
			ActualitzarLimits();
		}

		// Fixar les iteracions a qualsevol nombre. No el poso quan estigui animant-se perque al introduir dades pausa el bucle de l'aplicació.
		if (GetKey(olc::I).bReleased) {

			int i;
			cout << "Introdueix el nombre d'iteracions:" << endl;
			cin >> i;
			iteracions = i;
			anyInput = true;
		}


		

		// Entrada del ratolí.
		// Botó dret.
		if (GetMouse(0).bPressed) {
			// Aquí comencem a dibuixar un quadrat.
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
			vi2d posicioCursorCorregidaFinal = vi2d();
			switch (posicioInicialCursor.x > posicioFinalCursor.x)
			{
			case true:
				if (posicioInicialCursor.y > posicioFinalCursor.y) {
					posicioCursorCorregidaFinal = vi2d(posicioFinalCursor.x, (posicioFinalCursor.x - posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				else {
					posicioCursorCorregidaFinal = vi2d(posicioFinalCursor.x, (-posicioFinalCursor.x + posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				break;
			case false:
				if (posicioInicialCursor.y < posicioFinalCursor.y) {
					posicioCursorCorregidaFinal = vi2d(posicioFinalCursor.x, (posicioFinalCursor.x - posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				else {
					posicioCursorCorregidaFinal = vi2d(posicioFinalCursor.x, (-posicioFinalCursor.x + posicioInicialCursor.x) * altura / allargada + posicioInicialCursor.y);
				}
				break;
			}
			// Diem on ha acabat la posició corregida.
			cout << "\tAcabem a: " << posicioCursorCorregidaFinal << endl;

			// Dibuixem un últim rectangle.
			DrawRect(rectangle[0], rectangle[1], Pixel(255, 105, 105));

			// Mirem si el quadrat té una distancia suficient, si no llavors serà que hem pulsat per mirar que ha fet el lloc inicial.
			if ((posicioInicialCursor - posicioFinalCursor).mag2() < 900) {
				// Comencem a iterar el lloc inicial.
				ComençarAIterar();
				return anyInput;
			}

			// Aquí ja tenim les posicions finals passades a coordenades del fractal.
			vd2d coordenadesFractalsRectangle[2] = {
				CursorACoordenadesFractals(rectangle[0]),
				CursorACoordenadesFractals(posicioCursorCorregidaFinal)
			};

			// Aquí, depenent de la posició final del cursor per si estar més a dalt, més a baix, més a l'esquerra o més a la dreta, actualitzem els límits globals.
			switch (posicioInicialCursor.x > posicioFinalCursor.x)
			{
			case true:
				limits_x[0] = coordenadesFractalsRectangle[1].x;
				limits_x[1] = coordenadesFractalsRectangle[0].x;
				if (posicioInicialCursor.y < posicioFinalCursor.y) {
					limits_y[0] = coordenadesFractalsRectangle[1].y;
					limits_y[1] = coordenadesFractalsRectangle[0].y;
				}
				else {
					limits_y[0] = coordenadesFractalsRectangle[0].y;
					limits_y[1] = coordenadesFractalsRectangle[1].y;
				}
				break;
			case false:
				limits_x[0] = coordenadesFractalsRectangle[0].x;
				limits_x[1] = coordenadesFractalsRectangle[1].x;
				if (posicioInicialCursor.y < posicioFinalCursor.y) {
					limits_y[0] = coordenadesFractalsRectangle[1].y;
					limits_y[1] = coordenadesFractalsRectangle[0].y;
				}
				else {
					limits_y[0] = coordenadesFractalsRectangle[0].y;
					limits_y[1] = coordenadesFractalsRectangle[1].y;
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

	void ActualitzarFractal(bool stop) {
		// Començarem un rellotge per saber quan tarda la actualització de la fractal. Per estadístiques.
		auto begin = chrono::high_resolution_clock::now();

		// La suma surt de saber quina allargada o altura fractal hi ha i la dividim entre l'allargada o l'altura de l'aplicació.
		suma_x = (absolut(limits_x[0] - limits_x[1])) / allargada;
		suma_y = (absolut(limits_y[0] - limits_y[1])) / altura;

		// Posició inicial als límits inicials.
		double posicio[2] = { limits_x[0], limits_y[1] };

		// Recorrarem tota l'allargada de la pantalla punt per punt.
		for (int x = 0; x < allargada; x++)
		{
			// Recorrarem tota l'altura de la pantalla punt per punt en cada punt x.
			for (int y = 0; y < altura; y++)
			{
				// Iterem per aconseguir el valor, si està o no està a la fractal.
				double valor = Iterar(iteracions, posicio[0], posicio[1], llindar);
				// Guardarem el valor.
				fractal[altura * x + y] = valor;
				// Actualitzarem la posició y restant-li la suma_y.
				posicio[1] -= suma_y;
			}
			// Posarem la posició y a la inicial i actualitzarem la posició x sumant-li el que pertoca.
			posicio[1] = limits_y[1];
			posicio[0] += suma_x;
		}

		// Si volem saber quan ha durat l'actualització de la fractal.
		if (!stop) {
			auto end = chrono::high_resolution_clock::now();
			auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
			float tempsTranscorregut = elapsed.count() * 1e-9;
			//cout << "L'actualització ha durat: " << tempsTranscorregut << " segons." << endl;

			ofstream myfile;
			myfile.open("stadistics - CPU.txt", ios::app);			// ESTADÍSTIQUES.
			myfile << c.theta << " & " << tempsTranscorregut << "\n";
			myfile.close();
		}
		// Dibuixem la fractal.
		DibuixarFractal(false);
	}

	// Aquí es dibuixarà la fractal.
	void DibuixarFractal(bool time) {
		// Per saber el temps que ha durat el pintar.
		chrono::steady_clock::time_point begin;
		if (time)
			begin = chrono::high_resolution_clock::now();

		// Aquesta línia és important, ens permet paral·lelitzar el pintar la fractal.
		//#pragma omp parallel for
		for (int x = 0; x < allargada; x++) {		// Recorrerem cada punt de la pantalla per pintar-hi el valor corresponent
			for (int y = altura-1; y >= 0; y--) {	// de la fractal.
				int index = altura * x + y;
				if (fractal[index] == 1) {
					// Si el punt pertany a la fractal, sortirà en negre pur.
					Draw(x, y, Pixel(0, 0, 0));
					continue;
				}
				// Si el punt no pertany a la fractal es pintarà segons com de ràpid ha sortit de la fractal.
				Draw(x, y, Pixel(fractal[index] * 255, fractal[index] * 255, fractal[index] * 255));
			}
		}

		if (time) {
			// Per saber el temps trascorregut.
			auto end = chrono::high_resolution_clock::now();
			auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
			float tempsTranscorregut = elapsed.count() * 1e-9;
			cout << "El dibuix ha durat: " << tempsTranscorregut << " segons." << endl;
		}
	}
};

int main()
{
	c.M = 0.815068;
	c.theta = 0;
	c.polar = true;
	c.A_Cartesiana();

	// Crearem una Representació.
	RepresentacioFractal principal;
	// La "Construirem" i si ha anat bé la construció, és a dir, sempre, començarem el programa.
	if (principal.Construct(allargada, altura, 1, 1, true))
		principal.Start();

	return 0;
}