#include "SDL_render.h"
#include "SDL_timer.h"
#include <SDL.h>
#include <cmath>


int SDL_RenderDrawCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

int SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {

		status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
									 x + offsety, y + offsetx);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
									 x + offsetx, y + offsety);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
									 x + offsetx, y - offsety);
		status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
									 x + offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

class Vector {
public:
	double X;
	double Y;
public:
	Vector() {}
	Vector(double X, double Y) :
		X(X),
		Y(Y)
	{}
	~Vector() {}

	Vector operator+(const Vector& Rhs) const
	{
		return Vector(X + Rhs.X, Y + Rhs.Y);
	}
	Vector operator*(const double& Rhs) const
	{
		return Vector(X * Rhs, Y * Rhs);
	}
};

void ApplyGravity(Vector& Velocity, Vector& Position, double DeltaTime)
{
	static const Vector Gravity(.0, 9.8 * 0.0001);
	Vector OldVelocity = Velocity;
	OldVelocity = OldVelocity + Gravity * DeltaTime;
	Velocity = OldVelocity;
	Position = Position + Velocity;
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* Window = SDL_CreateWindow("PhyWater", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	SDL_Renderer* Render = SDL_CreateRenderer(Window, -1, 0);

	Vector Position(320.0,240.0);
	Vector Velocity(.0, .0);

	SDL_Event Event;
	static const Uint64 PerSec = SDL_GetPerformanceFrequency();
	Uint64 Last = SDL_GetPerformanceCounter();
	Uint64 Now = 0;
	for (;;) {
		while(SDL_PollEvent(&Event)) {
			if (Event.type == SDL_QUIT) {
				SDL_Quit();
				return 0;
			}
		}
		Now = SDL_GetPerformanceCounter();
		ApplyGravity(Velocity, Position, (Now - Last) / float(PerSec) * 1000.f);
		Last = Now;
		printf("\r%lf %lf", Position.X, Position.Y);

		SDL_SetRenderDrawColor(Render, 100, 100, 100, 0);
		SDL_RenderClear(Render);

		SDL_SetRenderDrawColor(Render, 200, 255, 200, 0);
		SDL_RenderFillCircle(Render, Position.X, Position.Y, 10);
		SDL_RenderPresent(Render);
	}
	SDL_Quit();
}
