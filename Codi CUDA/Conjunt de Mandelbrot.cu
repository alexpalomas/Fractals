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

// FILE MANATGMENT:
#include <fstream>

using namespace std;
using namespace olc;


const int height = 1080;
const int width = height * 1.7778;
int iterations = 100;

double x_bounds[2] = { -2.5, 2.5 };
double y_bounds[2] = { -1.5, 1.5 };

double x_sum;
double y_sum;

int mouseXBefore;
int mouseYBefore;

bool update = false;

float* fractal = new float[width * height];
bool drawIterations = false;

float threshold = 4;
		
int dimensions = width * height;


void PrintBounds() {
	cout << endl << "Current BOUNDS:" << endl;
	cout << "\t" << "X_MIN: " << x_bounds[0] << endl;
	cout << "\t" << "X_MAX: " << x_bounds[1] << endl;
	cout << "\t" << "Y_MIN: " << y_bounds[0] << endl;
	cout << "\t" << "Y_MAX: " << y_bounds[1] << endl;
}


vd2d MouseToFractalCoordenades(vi2d MouseCoordenades, float x_bound_min = x_bounds[0], float x_bound_max = x_bounds[1], float y_bound_min = y_bounds[0], float y_bound_max = y_bounds[1]) {
	double normalizedX = (double)MouseCoordenades.x / (double)width;
	double normalizedY = (double)MouseCoordenades.y / (double)height;

	double espacioX = absolute(x_bound_min - x_bound_max);
	double espacioY = absolute(y_bound_min - y_bound_max);

	return vd2d(x_bound_min + normalizedX * espacioX, y_bound_max - normalizedY * espacioY);
}

vi2d FractalToMouseCoordenades(vd2d FractalCoordenades, float x_bound_min, float x_bound_max, float y_bound_min, float y_bound_max) {
	return vi2d((FractalCoordenades.x - x_bound_min) * (double)width / (absolute(x_bound_min - x_bound_max)), (-FractalCoordenades.y + y_bound_max) * (double)height / (absolute(y_bound_min - y_bound_max)));
}

// ITERACIONS AQUI //
__global__
void FractalCalculator(int dimensions, float *fractal, int iter, double x_bounds_s, double x_bounds_e, double y_bounds_s, double y_bounds_e) 
{
	int index = threadIdx.x;
	int stride = blockDim.x;
	
	
	for (int i=index; i < dimensions; i+=stride) {
		fractal[i] = 0;
		
		int division = (int)((float)i / (float)height);
		int residual = i % height;
		
		int coordenades[2] = { division, residual };
		double normalizedX = (double)coordenades[0] / (double)width;
		double normalizedY = (double)coordenades[1] / (double)height;

		double espacioX = x_bounds_s - x_bounds_e;
		double espacioY = y_bounds_s - y_bounds_e;
		
		// ABSOLUTE //
		if(espacioX < 0) {
			espacioX = -espacioX;
		}
		if(espacioY < 0) {
			espacioY = -espacioY;
		}

		double x = 0;
		double y = 0;

		double cr = x_bounds_s + normalizedX * espacioX;
		double ci = y_bounds_e - normalizedY * espacioY;

		for (int k = 0; k < iter; k++)
		{
			if (x * x + y * y > 4)
			{
				fractal[i] = (float)k / (float)iter;
				break;
			}

			double x_t = x;

			x = x * x - y * y + cr;
			y = 2 * y * x_t + ci;

		}
			//x = x * x * x * x + y * y * y * y - 6 * x * x * y * y + cr;
			//y = 4 * x_t * x_t * x_t * y - 4 * x_t * y * y * y + ci;
	}
}
// ITERACIONS AQUI //

class Example : public PixelGameEngine
{
public:
	Example()
	{
		sAppName = "Conjunt de Julia";
	}

public:
	byte runningFrames;

	int level = 0;

	struct bound
	{
		double x_0, x_1, y_0, y_1;
		bound(double x0, double x1, double y0, double y1) {
			x_0 = x0;
			x_1 = x1;
			y_0 = y0;
			y_1 = y1;
		}
	};

	vector<bound> bounds;

	void EraseBounds() {
		size_t boundsSize = bounds.size() - 1;

		for (size_t i = level; i < boundsSize; i++)
		{
			bounds.pop_back();
		}
	}

	void UpdateBounds() {

		if (level > bounds.size() - 1) {
			level = bounds.size() - 1;
		}
		else if (level < 0)
		{
			level = 0;
		}

		x_bounds[0] = bounds[level].x_0;
		x_bounds[1] = bounds[level].x_1;
		y_bounds[0] = bounds[level].y_0;
		y_bounds[1] = bounds[level].y_1;
	}

	bool OnUserCreate() override
	{
		// Match boundings to 16:9 aspect-ratio
		//absolute(x_bounds[0] - x_bounds[1]) / absolute(y_bounds[0] - y_bounds[1]) = 16 / 9
		//(16/9)/ absolute(x_bounds[0] - x_bounds[1]) = 1 / absolute(y_bounds[0] - y_bounds[1])
		//(9/16)* absolute(x_bounds[0] - x_bounds[1]) = absolute(y_bounds[0] - y_bounds[1])
		// Com sabem que y_bounds[0] i y_bounds[1] han de ser inversos, y_bounds[1] = -y_bounds[0]
		//(9/16)* absolute(x_bounds[0] - x_bounds[1]) = absolute(2 * y_bounds[0])
		//(9/16)* absolute(x_bounds[0] - x_bounds[1]) / 2 = y_bounds[0]
		y_bounds[1] = (float)9 / (float)16 * absolute(x_bounds[0] - x_bounds[1]) / (float)2;
		y_bounds[0] = -y_bounds[1];

		bound starting = bound(x_bounds[0], x_bounds[1], y_bounds[0], y_bounds[1]);
		bounds.insert(bounds.end(), starting);
		UpdateBounds();

		runningFrames = 0;
		PrintBounds();
		
		
		cudaMallocManaged(&fractal, dimensions*sizeof(float));
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

	double totalTime = 0;
	bool OnUserUpdate(float fElapsedTime) override
	{
		runningFrames += 1;
		
		// INPUT //
		update = HandleInput();
		// INPUT //

		totalTime += fElapsedTime;

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

		if (update) {
			UpdateFractal();
		}
		return true;


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
		initialX = MouseToFractalCoordenades(startingMousePosition).x;
		initialY = MouseToFractalCoordenades(startingMousePosition).y;

		// Dibuixem un cercle on hem clicat.
		DrawCircle(startingMousePosition, 3, olc::RED);

		drawIterations = true;
	}


	bool HandleInput() {
		bool anyInput = false;

		if (GetKey(olc::H).bPressed) {
			// HELP!
			cout << endl;
			cout << "Animation." << endl;
			cout << "\tP: Pause or Reanude the animation. With the fractal paused you can use movement." << endl;
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

		if (GetKey(olc::LEFT).bReleased && level != 0) {
			cout << "Going backwards..." << endl;
			level--;
			UpdateBounds();
			UpdateFractal();
		}
		if(GetKey(olc::RIGHT).bReleased && level != bounds.size()-1)
		{
			cout << "Going forward..." << endl;
			level++;
			UpdateBounds();
			UpdateFractal();
		}
		if (GetKey(olc::I).bReleased) {

			int i;
			cout << "Introdueix el nomre d'iteracions:" << endl;
			cin >> i;
			iterations = i;
			anyInput = true;
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
					fractalImage.SetColor(NewColor(fractal[index], fractal[index], fractal[index]), x, y);
				}
				y_f -= 1;
			}
			// SET ALL IMAGE COLORS //

			// SAVE THE IMAGE //
			fractalImage.Export(path.c_str());
			// SAVE THE IMAGE //
		}
		// SAVE IMAGE //


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

			vd2d fractalCoordenadesRectangle[2] = {
				MouseToFractalCoordenades(rectangle[0]),
				MouseToFractalCoordenades(endedCorrectedMousePosition)
			};


			if ((startingMousePosition - endingMousePosition).mag2() < 900) {

				StartIterate();
				return anyInput;
			}

			cout << "Moving to:" << endl;

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


			// UPDATE BOUNDS:
			EraseBounds();
			bounds.insert(bounds.end(), bound(x_bounds[0], x_bounds[1], y_bounds[0], y_bounds[1]));
			level++;
			// UPDATE BOUNDS:

			cout << "\tX_MIN: " << x_bounds[0] << endl;
			cout << "\tX_MAX: " << x_bounds[1] << endl;
			cout << "\tY_MIN: " << y_bounds[0] << endl;
			cout << "\tY_MAX: " << y_bounds[1] << endl;

			anyInput = true;
		}

		return anyInput;
	}

	void UpdateFractal() {		
		auto begin = chrono::high_resolution_clock::now();
		
		x_sum = (absolute(x_bounds[0] - x_bounds[1])) / width;
		y_sum = (absolute(y_bounds[0] - y_bounds[1])) / height;
		
		FractalCalculator<<<1, 1024>>>(dimensions, fractal, iterations, x_bounds[0], x_bounds[1], y_bounds[0], y_bounds[1]);
		
		cudaDeviceSynchronize();
		
		auto end = chrono::high_resolution_clock::now();
		auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
		float elapsedTime = elapsed.count() * 1e-9;
		cout << "Update elapsed: " << elapsedTime << " seconds." << endl;
		
		DrawScreen(false);
	}

	void DrawScreen(bool time) {
		chrono::steady_clock::time_point begin;
		if (time)
			begin = chrono::high_resolution_clock::now();

		//#pragma omp parallel for
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				int index = height * x + y;
				float value = fractal[index];
				
				if (value == 1) {
					Draw(x, y, Pixel(0, 0, 0));
					continue;
				}
				Draw(x, y, Pixel(value * 255, value * 255, value * 255));
				
				//Draw(x, y, pix);
			}
		}

		if (time) {
			auto end = chrono::high_resolution_clock::now();
			auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
			float elapsedTime = elapsed.count() * 1e-9;
			cout << "The paralelized drawing elapsed: " << elapsedTime << " seconds." << endl;
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