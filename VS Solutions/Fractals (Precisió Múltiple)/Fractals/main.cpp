#define _CRT_SECURE_NO_WARNINGS
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "utils.cpp"
#include "Image.cpp"
#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <omp.h>
#include "DecimalesInfinitos.cpp"

using namespace std;
using namespace olc;


const int height = 108;
const int width = height * 1.7778;
int iterations = 100;

Num x_bounds[2] = { -2, 1 };
Num y_bounds[2] = { -1, 1 };

Num x_sum;
Num y_sum;

int mouseXBefore;
int mouseYBefore;

bool update = false;

float* fractal = new float[width * height];
bool drawIterations = false;

int threshold = 4;


void PrintBounds() {
	cout << endl << "Current BOUNDS:" << endl;
	cout << "\t" << "X_MIN: " << x_bounds[0].String() << endl;
	cout << "\t" << "X_MAX: " << x_bounds[1].String() << endl;
	cout << "\t" << "Y_MIN: " << y_bounds[0].String() << endl;
	cout << "\t" << "Y_MAX: " << y_bounds[1].String() << endl;
}


/*vd2d MouseToFractalCoordenades(vi2d MouseCoordenades, float x_bound_min = x_bounds[0], float x_bound_max = x_bounds[1], float y_bound_min = y_bounds[0], float y_bound_max = y_bounds[1]) {
	double normalizedX = (double)MouseCoordenades.x / (double)width;
	double normalizedY = (double)MouseCoordenades.y / (double)height;

	double espacioX = absolute(x_bound_min - x_bound_max);
	double espacioY = absolute(y_bound_min - y_bound_max);

	return vd2d(x_bound_min + normalizedX * espacioX, y_bound_max - normalizedY * espacioY);
}*/

vi2d FractalToMouseCoordenades(vd2d FractalCoordenades, float x_bound_min, float x_bound_max, float y_bound_min, float y_bound_max) {
	return vi2d((FractalCoordenades.x - x_bound_min) * (double)width / (absolute(x_bound_min - x_bound_max)), (-FractalCoordenades.y + y_bound_max) * (double)height / (absolute(y_bound_min - y_bound_max)));
}


Num XMultiplication(Num x, Num y, Num cr) {
	Operador o;
	x = o.Sumar(o.Restar(o.Multiplicar(x, x), o.Multiplicar(y , y)), cr);
	return x;
}

Num YMultiplication(Num x, Num y, Num ci) {
	Operador o;
	y = o.Sumar(o.Multiplicar(o.Multiplicar(Num(2), y), x), ci);
	return y;
}

double iterate(int n, Num cr, Num ci, int threshold) {
	Num x = Num();
	Num y = Num();
	Num x2 = Num();
	Num y2 = Num();

	Operador o;
	for (int k = 0; k < n; k++)
	{
		if (o.MasGrandeQue(o.Sumar(o.Multiplicar(x, x), o.Multiplicar(y, y)), Num(threshold))) {
			return (double)k / (double)n;
		}

		Num x_t = x;

		x2 = o.Multiplicar(x, x);
		y2 = o.Multiplicar(y, y);

		x = o.Sumar(o.Restar(x2, y2), cr);
		y = o.Sumar(o.Multiplicar(o.Multiplicar(Num(2), y), x_t), ci);
	}

	return 0;
}

class Example : public PixelGameEngine
{
public:
	Example()
	{
		sAppName = "Fractals?";
	}

public:
	byte runningFrames;

	bool OnUserCreate() override
	{
		runningFrames = 0;
		PrintBounds();
		UpdateFractal();

		return true;
	}

	vi2d startingPos;
	vi2d endingPos;


	// DRAW ITERATIONS VARIABLES //
	double currentX;
	double currentY;
	double initialX;
	double initialY;
	int iter = 0;
	// DRAW ITERATIONS VARIABLES //

	bool OnUserUpdate(float fElapsedTime) override
	{				
		runningFrames += 1;
		
		// INPUT //
		update = HandleInput();
		// INPUT //

		if (drawIterations && !update) {
			if (iter <= iterations) {
				// Aquí fem la iteració del punt en sí i la dibuixem.
				// 
				// El que faré serà crear una variable on ens dirà on està el punt pero en coordenades del fractal.
				// Tenint aquesta informació podem començar a iterar, així doncs per cada iteració dibuixarem un cercle
				//	on es troba i una linea de l'anterior a aquest.
				// L'anterior el posarem a la mateixa variable inicial ja uq ja no serà necessària un cop feta servir inicialment.

				// Calculem les altres iteracions.
				double x_t = currentX;
				//currentX = XMultiplication(currentX, currentY, initialX);
				//currentY = YMultiplication(x_t, currentY, initialY);

				//DrawCircle(FractalToMouseCoordenades(vd2d(currentX, currentY) - MouseToFractalCoordenades(startingPos), -4.0f, 4.0f, -4.0f, 4.0f), 3);
				cout << "Current X: " << currentX << endl << "Current Y: " << currentY << endl;
				Sleep(10);
				iter++;
				return true;
			}
			drawIterations = false;
		}
		

		// DIBUAR EL CUADRADITO ESE //
		if (updateDrawingEveryFrame && runningFrames % 2 == 0) {
			DrawScreen(false);

			vi2d rectangle[2] = {
				startingPos,
				vi2d()
			};

			switch (startingPos.x > GetMousePos().x)
			{
			case true:
				if (startingPos.y > GetMousePos().y) {
					rectangle[1] = vi2d(GetMousePos().x - startingPos.x, (float)height / (float)width * (GetMousePos().x - startingPos.x));
				}
				else {
					rectangle[1] = vi2d(GetMousePos().x - startingPos.x, (float)height / (float)width * (-GetMousePos().x + startingPos.x));
				}
				break;
			case false:
				if (startingPos.y < GetMousePos().y) {
					rectangle[1] = vi2d(GetMousePos().x - startingPos.x, (float)height / (float)width * (GetMousePos().x - startingPos.x));
				}
				else {
					rectangle[1] = vi2d(GetMousePos().x - startingPos.x, (float)height / (float)width * (-GetMousePos().x + startingPos.x));
				}
				break;
			}
			

			DrawRect(rectangle[0], rectangle[1], Pixel(200, 200, 200));

			return true;
		}
		// DIBUAR EL CUADRADITO ESE //


		if (!update) return true;

		auto begin = chrono::high_resolution_clock::now();


		// ACTUALIZAR EL FRACTAL //
		UpdateFractal();
		// ACTUALIZAR EL FRACTAL //


		update = false;

		auto end = chrono::high_resolution_clock::now();
		auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);

		float elapsedTime = elapsed.count() * 1e-9;
		cout << "The update elapsed: " << elapsedTime << " seconds. Currently are " << iterations << " iterations." << endl;
		if (!(lastElapsedTime == 0)) {
			float plusPercentage = 100 * elapsedTime / lastElapsedTime - 100;

			char whattosay = (plusPercentage > 0) ? '+' : '-';
			cout << whattosay;
			cout << absolute(plusPercentage) << "%." << endl;
		}
		lastElapsedTime = elapsedTime;


		PrintBounds();

		return true;
	}

private:
	float lastElapsedTime = 0;
	vi2d startingMousePosition, endingMousePosition;
	bool updateDrawingEveryFrame = false;

	void StartIterate() {
		DrawScreen(false);
		
		iter = 0;
		currentX = 0;
		currentY = 0;
		//initialX = MouseToFractalCoordenades(startingMousePosition).x;
		//initialY = MouseToFractalCoordenades(startingMousePosition).y;

		// Dibuixem un cercle on hem clicat.
		DrawCircle(startingMousePosition, 3, olc::RED);

		drawIterations = true;
	}


	bool HandleInput() {
		bool anyInput = false;

		if (GetKey(olc::H).bPressed) {
			// HELP!
			cout << endl;
			cout << "Iterations edit commands." << endl;
			cout << "\tNumPad+ or S: Add 10 iterations;" << endl;
			cout << "\tNumPad- or R: Substract 10 iterations;" << endl;
			cout << "\tNumPad* or M: Multiply by 2 the iterations;" << endl;
			cout << "\tNumPad/ or D: Divide by 2 the iterations." << endl;
			cout << endl;
			cout << "Iterate points." << endl;
			cout << "\tI: Introduce mouse coordenades to iterate;" << endl;
			cout << "\tMouse Left Button Click: Iterate that point in specific." << endl;
			cout << endl;
			cout << "Movement." << endl;
			cout << "\tThe program is made to match the aspect-ratio of 16:9 everytime." << endl;
			cout << "\tHeld Mouse Left Button and drag it wherever you want." << endl;
			cout << "\tReleased to calculate the updated fractal." << endl;
			cout << endl;
			cout << "Have fun!" << endl;
			cout << endl;
		}


		if (GetKey(olc::I).bReleased) {
			
			string x, y;
			cout << "Introdueix les coordenades:" << endl;
			cout << "X: ";
			cin >> x;
			cout << "Y: ";
			cin >> y;
			startingMousePosition = vi2d(stoi(x), stoi(y));

			StartIterate();
		}


		// SAVE IMAGE //
		if (GetKey(olc::ENTER).bPressed) {
			int imageHeight = height;
			int imageWidth = width;
			Image fractalImage(width, height);

			// IMAGE NAME HAS TO BE UNIQUE IF WE WANT TO STORE MORE THAN ONE //
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
			// IMAGE NAME HAS TO BE UNIQUE IF WE WANT TO STORE MORE THAN ONE //


			/*	S'HAURÀ DE RECALCULAR EL FRACTAL SI ES VOL GUARDAR EN UNA 
				RESOLUCIÓ MÉS GRAN QUE LA QUE HI HA AL PROJECTE.

				CANVIAR TAMBÉ LA PART D'ABAIX.
			*/
			
			// SET ALL IMAGE COLORS //
			int y_f = imageHeight;  // Per revertir la imatge que es genera al revés si no.
			for (int y = 0; y < imageHeight; y++)
			{
				for (int x = 0; x < imageWidth; x++)
				{
					int index = x * imageHeight + y_f;
					fractalImage.SetColor(NewColor(fractal[index] * (float)255 / (float)255, fractal[index] * (float)105/(float)255, fractal[index] * (float)105 / (float)255), x, y);
				}
				y_f -= 1;
			}
			// SET ALL IMAGE COLORS //

			// SAVE THE IMAGE //
			fractalImage.Export(path.c_str());
			// SAVE THE IMAGE //
		}
		// SAVE IMAGE //

		// ITERATIONS //
		if (GetKey(NP_ADD).bReleased || GetKey(S).bReleased) {
			iterations += 10;
			anyInput = true;
		}
		if (GetKey(NP_SUB).bReleased || GetKey(R).bReleased) {
			iterations -= 10;
			anyInput = true;
		}
		if (GetKey(NP_MUL).bReleased || GetKey(M).bReleased) {
			iterations *= 2;
			anyInput = true;
		}
		if (GetKey(NP_DIV).bReleased || GetKey(D).bReleased) {
			iterations /= 2;
			anyInput = true;
		}

		// MOUSE INPUT //
		if (GetMouse(0).bPressed) {
			drawIterations = false;
			startingMousePosition = GetMousePos();
			cout << "Mouse Input.\n\tStarted at: " << startingMousePosition << endl;
			startingPos = GetMousePos();
			updateDrawingEveryFrame = true;
		}
		if (!GetMouse(0).bHeld) {
			updateDrawingEveryFrame = false;
		}
		if (GetMouse(0).bReleased) {

			endingMousePosition = GetMousePos();

			vi2d rectangle[2] = {
				startingMousePosition,
				vi2d(endingMousePosition.x - startingMousePosition.x, (float)height / (float)width * (endingMousePosition.x - startingMousePosition.x))
			};

			// CORRIGIENDO LA POSICIÓN FINAL //
			vi2d endedCorrectedMousePosition = vi2d();
			switch (startingMousePosition.x > endingMousePosition.x)
			{
			case true:
				if (startingMousePosition.y > endingMousePosition.y) {
					endedCorrectedMousePosition = vi2d(endingMousePosition.x, (endingMousePosition.x - startingMousePosition.x) * height / width + startingMousePosition.y);
				}
				else {
					endedCorrectedMousePosition = vi2d(endingMousePosition.x, (-endingMousePosition.x + startingMousePosition.x) * height / width + startingMousePosition.y);
				}
				break;
			case false:
				if (startingMousePosition.y < endingMousePosition.y) {
					endedCorrectedMousePosition = vi2d(endingMousePosition.x, (endingMousePosition.x - startingMousePosition.x) * height / width + startingMousePosition.y);
				}
				else {
					endedCorrectedMousePosition = vi2d(endingMousePosition.x, (-endingMousePosition.x + startingMousePosition.x) * height / width + startingMousePosition.y);
				}
				break;
			}
			// CORRIGIENDO LA POSICIÓN FINAL //

			cout << "\tEnded at : " << endedCorrectedMousePosition << endl;

			DrawRect(rectangle[0], rectangle[1], Pixel(255, 105, 105));

			//vd2d fractalCoordenadesRectangle[2] = {
			//	MouseToFractalCoordenades(rectangle[0]),
			//	MouseToFractalCoordenades(endedCorrectedMousePosition)
			//};


			if ((startingMousePosition - endingMousePosition).mag2() < 900) {

				StartIterate();
				return anyInput;
			}

			cout << "Moving to:" << endl;
			/*
			// No preguntes cómo. //
			switch (startingMousePosition.x > endingMousePosition.x)
			{
			case true:
				x_bounds[0] = fractalCoordenadesRectangle[1].x;
				x_bounds[1] = fractalCoordenadesRectangle[0].x;
				if (startingMousePosition.y < endingMousePosition.y) {
					y_bounds[0] = fractalCoordenadesRectangle[1].y;
					y_bounds[1] = fractalCoordenadesRectangle[0].y;
				}
				else {
					y_bounds[0] = fractalCoordenadesRectangle[0].y;
					y_bounds[1] = fractalCoordenadesRectangle[1].y;
				}
				break;
			case false:
				x_bounds[0] = fractalCoordenadesRectangle[0].x;
				x_bounds[1] = fractalCoordenadesRectangle[1].x;
				if (startingMousePosition.y < endingMousePosition.y) {
					y_bounds[0] = fractalCoordenadesRectangle[1].y;
					y_bounds[1] = fractalCoordenadesRectangle[0].y;
				}
				else {
					y_bounds[0] = fractalCoordenadesRectangle[0].y;
					y_bounds[1] = fractalCoordenadesRectangle[1].y;
				}
				break;
			}
			// No preguntes cómo. //

			cout << "\tX_MIN: " << x_bounds[0] << endl;
			cout << "\tX_MAX: " << x_bounds[1] << endl;
			cout << "\tY_MIN: " << y_bounds[0] << endl;
			cout << "\tY_MAX: " << y_bounds[1] << endl;
			*/
			anyInput = true;
		}

		return anyInput;
	}

	void UpdateFractal() {
		Operador o;
		x_sum = o.Dividir(o.Abs(o.Restar(x_bounds[0], x_bounds[1])), Num(width));
		y_sum = o.Dividir(o.Abs(o.Restar(y_bounds[0], y_bounds[1])), Num(height));

		cout << "X_SUM: " << x_sum.String() << "\nY_SUM: " << y_sum.String() << endl;

		Num position[2] = { Num(0), Num(0) };

		position[0] = x_bounds[0];
		position[1] = y_bounds[1];

		auto begin = chrono::high_resolution_clock::now();

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				double value = iterate(iterations, position[0], position[1], threshold);

				fractal[height * x + y] = value;

				position[1] = o.Restar(position[1], y_sum);
			}
			position[1] = y_bounds[1];
			position[0] = o.Sumar(position[0], x_sum);
			
			cout << x << endl;
		}

		auto end = chrono::high_resolution_clock::now();
		auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
		float elapsedTime = elapsed.count() * 1e-9;
		cout << "The calculation elapsed: " << (elapsedTime / 60) << " minutes." << endl;

		DrawScreen(true);
	}

	void DrawScreen(bool time) {
		chrono::steady_clock::time_point begin;
		if (time)
			begin = chrono::high_resolution_clock::now();

		//#pragma omp parallel for
		for (int x = 0; x < width; x++) {
			for (int y = height-1; y >= 0; y--) {
				int index = height * x + y;
				Draw(x, y, Pixel(fractal[index] * 255, fractal[index] * 163, fractal[index] * 119));
			}
		}

		if (time) {
			auto end = chrono::high_resolution_clock::now();
			auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
			float elapsedTime = elapsed.count() * 1e-9;
			cout << "The drawing elapsed: " << elapsedTime << " seconds." << endl;
		}
	}
};

int main()
{
	Example demo;
	if (demo.Construct(width, height, 1, 1, true))
		demo.Start();

	return 0;
}