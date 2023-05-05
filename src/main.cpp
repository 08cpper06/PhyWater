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

class Ball {
public:
	Vector Position;
	Vector Velocity;
public:
	Ball() {}

	void Update(float DeltaTime)
	{
		static const Vector Gravity(.0, 9.8 * 0.0001);
		Vector OldVelocity = Velocity;
		OldVelocity = OldVelocity + Gravity * DeltaTime;
		Velocity = OldVelocity;
		Position = Position + Velocity;
		printf("\rPos:%lf Vel:%lf", Position.Y, Velocity.Y);
	}
};

class Application {
public:
	Application() :
		RequestedFinish(false),
		PerSec(SDL_GetPerformanceFrequency()),
		LastTime(0U),
		DeltaTime(0.f),
		Window(nullptr),
		Render(nullptr)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
	}
	~Application() 
	{
		SDL_Quit();
	}

	void ShowWindow()
	{
		Window = SDL_CreateWindow("PhyWater", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
		Render = SDL_CreateRenderer(Window, -1, 0);
	}

	void Execute()
	{
		Uint64 Now;
		while (!RequestedFinish) {
			PollEvent();
			Now = SDL_GetPerformanceCounter();

			DeltaTime = (Now - LastTime) / PerSec * 1000.f;

			Update(DeltaTime);

			Draw();

			LastTime = Now;
		}
		if (Window) {
			SDL_DestroyRenderer(Render);
			SDL_DestroyWindow(Window);
		}
	}

	void Update(float DeltaTime)
	{
		OneBall.Update(DeltaTime);
	}

	void PollEvent()
	{
		SDL_Event Event;
		if (SDL_PollEvent(&Event)) {
			if (Event.type == SDL_QUIT) {
				RequestedFinish = true;
			}
		}
	}

	void Draw()
	{
		SDL_SetRenderDrawColor(Render, 100, 100, 100, 0);
		SDL_RenderClear(Render);

		SDL_SetRenderDrawColor(Render, 200, 255, 200, 0);
		SDL_RenderFillCircle(Render, OneBall.Position.X, OneBall.Position.Y, 10);
		SDL_RenderPresent(Render);
	}

public:
	Ball OneBall;

private:
	bool RequestedFinish;
	const float PerSec;
	Uint64 LastTime;
	float DeltaTime;
	SDL_Window* Window;
	SDL_Renderer* Render;
};

int main(int argc, char** argv)
{
	Application App;
	App.ShowWindow();

	App.OneBall.Position = Vector(320.0, 240.0);
	App.OneBall.Velocity = Vector(0.0, 0.0);

	App.Execute();
}
